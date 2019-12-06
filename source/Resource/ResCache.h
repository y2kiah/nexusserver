/*----==== RESCACHE.H ====----
	Author:		Jeff Kiah
	Orig.Date:	07/16/2007
	Rev.Date:	09/24/2011
----------------------------*/

#pragma once

#include <string>
#include <hash_map>
#include <hash_set>
#include <list>
#include <vector>
#include <memory>
#include "ResHandle.h"
#include "../Event/EventListener.h"
#include "../Utility/Typedefs.h"
#include "../Utility/Singleton.h"

using std::string;
using stdext::hash_map;
using stdext::hash_set;
using std::list;
using std::vector;
using std::shared_ptr;

///// DEFINITIONS /////

#define resMgr	ResCacheManager::instance()

// forward declarations
class ResCache;
class IResourceSource;
class CProcess;
typedef shared_ptr<ResCache>		ResCachePtr;
typedef shared_ptr<IResourceSource>	ResSourcePtr;
typedef shared_ptr<char>			BufferPtr; // use checked_array_deleter<char> to ensure delete[] called
typedef shared_ptr<CProcess>		CProcessPtr;

/*=============================================================================
class IResourceSource
	This class could be an interface to a file, memory mapped file, zip file,
	or even a class with logic to algorithmically create data. Another example
	could be a class that handles render-to-texture requests and buffers them.
=============================================================================*/
class IResourceSource : private boost::noncopyable {
	public:
		/*---------------------------------------------------------------------
			This initialized the source. Should be called right after creation.
		---------------------------------------------------------------------*/
		virtual bool	open() = 0;
		virtual int		getResourceSize(const string &resName) const = 0;
		/*---------------------------------------------------------------------
			This loads the resource from the source, passing it back through
			the dataPtr shared_ptr. Returns size, or 0 on error. The third
			parameter can be used for any purpose the derived class requires,
			or can be ignored or made optional.
		---------------------------------------------------------------------*/
		virtual int		getResource(const string &resName, BufferPtr &dataPtr, int threadIndex = 0) = 0;
		/*---------------------------------------------------------------------
			Utilize this method to assign unique id's to threads so the calling
			thread can be identified in calls to getResource().
		---------------------------------------------------------------------*/
		virtual int		getNewThreadIndex() = 0;

		// Constructor / destructor
		explicit IResourceSource() {}
		virtual ~IResourceSource() {}
};

/*=============================================================================
class ResCache
=============================================================================*/
class ResCache {
	friend class Resource;	// allows access to call memoryHasBeenFreed() from ~Resource()
	public:
		///// DEFINITIONS /////
		typedef list<ResPtr>	ResList;
		typedef hash_map<string, ResList::iterator>	ResMap;

	private:
		///// VARIABLES /////
		ResList	mLRU;		// least recently used list, back is least recent, front is most
		ResMap	mResMap;	// hash map linking the key to the handle

		uint	mMaxSizeB;	// total memory size in bytes
		uint	mUsedB;		// total memory allocated in bytes
		
		bool	mAllowOversizedResources; // when false, resources larger than mMaxSizeB will not be loaded

	protected:
		///// FUNCTIONS /////
		/*---------------------------------------------------------------------
			Calls freeOneResource until new request can fit, returns false when
			request is too large for cache
		---------------------------------------------------------------------*/
		bool	makeRoom(uint sizeB);

		/*---------------------------------------------------------------------
			Deletes one resource, the least recently used. Returns false if
			cache is already empty.
		---------------------------------------------------------------------*/
		bool	freeOneResource();

		/*---------------------------------------------------------------------
			Called when a resource is destroyed, reducing cache total allocated
		---------------------------------------------------------------------*/
		void	memoryHasBeenFreed(uint sizeB) {
					mUsedB -= ((sizeB > mUsedB) ? mUsedB : sizeB);
					debugPrintf("ResCache: memory freed, %u bytes\n", sizeB);
				}

		/*---------------------------------------------------------------------
			Pushes a resource to the front of the LRU list.
		---------------------------------------------------------------------*/
		void	makeMostRecent(const ResMap::iterator &mi);

	public:
		/*---------------------------------------------------------------------
			If the resource indexed by {key} is present in the cache, return
			true and point resPtr to the resource. Returns false if resource
			not present.
		---------------------------------------------------------------------*/
		bool	getResource(ResPtr &resPtr, const string &key);

		/*---------------------------------------------------------------------
			adds a resource to the cache
		---------------------------------------------------------------------*/
		bool	addToCache(uint sizeB, const ResHandle &h);
		
		/*---------------------------------------------------------------------
			adds a resource to cache, name and sizeB taken from the Resource
		---------------------------------------------------------------------*/
		bool	addToCache(const ResPtr &resPtr);

		/*---------------------------------------------------------------------
			removes a specific resource from the cache
		---------------------------------------------------------------------*/
		bool	removeResource(const string &key);

		/*---------------------------------------------------------------------
			clears the entire resource list
		---------------------------------------------------------------------*/
		void	clearCache()				{ mLRU.clear(); mResMap.clear(); }

		// Accessors
		bool	hasRoom(uint sizeB) const	{ return (mMaxSizeB - mUsedB >= sizeB); }
		uint	maxSizeBytes() const		{ return mMaxSizeB; }
		uint	usedBytes() const			{ return mUsedB; }

		// Constructor / destructor
		explicit ResCache(uint sizeMB, bool allowOversizedResources = true);
		~ResCache();
};

/*=============================================================================
class ResCacheManager
=============================================================================*/
class ResCacheManager : public Singleton<ResCacheManager> {
	friend class AsyncLoadDoneListener;		// provide access to staging list
	public:
		///// DEFINITIONS /////
		typedef hash_map<string, ResSourcePtr>	ResSourceMap;
		typedef shared_ptr<ResCache>			ResCachePtr;
		typedef vector<ResCachePtr>				ResCacheList;
		typedef hash_map<string, EventPtr>		EventQueue;
		typedef hash_set<string>				RequestQueue;

	private:
		///// STRUCTURES /////
		/*=====================================================================
		class AsyncLoadDoneListener
		=====================================================================*/
		class AsyncLoadDoneListener : public EventListener {
			friend class ResCacheManager;
			private:
				bool handleEvent(const EventPtr &ePtr);
			public:
				explicit AsyncLoadDoneListener();
		};

		///// VARIABLES /////
		ResSourceMap	mSourceMap;		// the table of registered source files
		ResCacheList	mCacheList;		// the list of resource caches, one for each ResCacheType

		// For async threaded loading
		AsyncLoadDoneListener	mListener;		// listens for AsyncLoadDone event and pushed event into staging queue
		EventQueue				mStagingList;	// data that has been loaded by another thread but not yet cached
		RequestQueue			mRequestList;	// list of pending resources already requested via tryLoad, makes sure
												// a request isn't submitted multiple times for the same resource
		CProcessPtr				mThreadProcPtr;	// pointer to the thread process, so it can be detached in destructor

		///// FUNCTIONS /////
		bool	inStagingList(const string &resName, const string &source);

		/*---------------------------------------------------------------------
			pushes an AsyncLoadDoneEvent into the staging list to be picked up
			by tryLoad()
		---------------------------------------------------------------------*/
		void	addToStagingList(const string &resName, const string &source, const EventPtr &ePtr);

	public:
		/*---------------------------------------------------------------------
			creates the cache of a certain type passing in the budget, only one
			cache of each type allowed
		---------------------------------------------------------------------*/
		void	createCache(ResCacheType cacheType, uint maxSizeMB, bool allowOversizedResources = true);

		/*---------------------------------------------------------------------
			returns a shared_ptr to the ResCache of a given type
		---------------------------------------------------------------------*/
		const ResCachePtr &	getResCache(ResCacheType cacheType) const {
				_ASSERTE(cacheType < ResCache_MAX && "Bad cacheType");
				return mCacheList[cacheType];
			}

		/*---------------------------------------------------------------------
			Fetch a resource from cache or a ResSource (disk), ResPtr passed in
			will hold resource if true is returned.
			** NOTE **
			The template param TResource should be a type derived from class
			Resource and MUST implement a constructor with the signature:
			(const string &name, uint sizeB, const ResCachePtr &resCachePtr)
		---------------------------------------------------------------------*/
		template <typename TResource>
		bool	load(ResHandle &h);

		/*---------------------------------------------------------------------
			Same as load method, but if resource does not exist in cache, this
			will return immediately and request the resource to be loaded in
			a separate thread, so this is a non blocking call. This method
			supports asynchronous resource loading. The client should call this
			function periodically until it returns success, and then take
			action with the resource. If error is returned the client should
			not expect the resource to load and should stop asking for it.
		---------------------------------------------------------------------*/
		template <typename TResource>
		ResLoadResult	tryLoad(ResHandle &h);

		/*---------------------------------------------------------------------
			This will just attempt to pull a resource from a specific cache. If
			the resource does not exist, false is returned and h.mResPtr will
			be empty. h.mSource is ignored with this method, as it bypasses
			the mapping of ResourceSource name to cache type.
		---------------------------------------------------------------------*/
		bool	getFromCache(ResHandle &h, ResCacheType cacheType);

		/*---------------------------------------------------------------------
			load a new IResourceSource into the system, it should be
			initialized for use externally (open() still needs to be called)
		---------------------------------------------------------------------*/
		bool	registerSource(const string &srcName, const ResSourcePtr &srcPtr);

		// Constructor / destructor
		explicit ResCacheManager(uint availableSysMemMB, uint availableVidMemMB);
		~ResCacheManager();
};

///// TEMPLATE FUNCTIONS /////

#include "ResourceProcess.h"
#include "../Event/EventManager.h"

/*---------------------------------------------------------------------
	Fetch a resource from cache or a ResSource (disk), ResPtr passed in
	will hold resource if true is returned.
	** NOTE **
	The template param TResource should be a type derived from class
	Resource and MUST implement a constructor with the signature:
		(const string &name, uint sizeB, ResCache *pResCache)
---------------------------------------------------------------------*/
template <typename TResource>
bool ResCacheManager::load(ResHandle &h)
{
	_ASSERTE(TResource::sCacheType < ResCache_MAX && "Bad cacheType");

	// try to find the resource in cache
	ResCachePtr &cache = mCacheList[TResource::sCacheType];
	if (!cache->getResource(h.mResPtr, h.name())) {
		// not in cache, so load it from source and put into cache
		ResSourceMap::const_iterator mi = mSourceMap.find(h.source());
		if (mi != mSourceMap.end()) {
			// loads the resource data from source, returning size or 0 on error
			BufferPtr dataPtr((char *)0);
			int size = mi->second->getResource(h.name(), dataPtr);
			if (size) {
				// construct a new Resource object, store it in a ResPtr
				// and pass into the ResHandle
				ResPtr resPtr(new TResource(h.name(), size, cache));
				h.mResPtr = resPtr;
				// store the resource in a cache (specified by the resource)
				bool added = cache->addToCache(size, h);
				if (added) {
					// call the resource's onLoad method
					TResource *pRes = static_cast<TResource*>(resPtr.get());
					pRes->onLoad(dataPtr, false);
					return true;
				} // if not added, cache has no room
			}
		}
		return false;
	}
	return true; // found in the cache
}

/*---------------------------------------------------------------------
	Same as load method, but if resource does not exist in cache, this
	will return immediately and request the resource to be loaded in
	a separate thread, so this is a non blocking call. This method
	supports asynchronous resource loading. The client should call this
	function periodically until it returns success, and then take
	action with the resource. If error is returned the client should
	not expect the resource to load and should stop asking for it.
---------------------------------------------------------------------*/
template <typename TResource>
ResLoadResult ResCacheManager::tryLoad(ResHandle &h)
{
	_ASSERTE(TResource::sCacheType < ResCache_MAX && "Bad cacheType");

	// try to find the resource in cache
	ResCachePtr &cache = mCacheList[TResource::sCacheType];
	if (!cache->getResource(h.mResPtr, h.name())) {
		// not in cache, check staging list to see if raw data has been loaded
		std::stringstream ss;
		ss << h.source() << '/' << h.name();
		EventQueue::iterator si = mStagingList.find(ss.str());
		if (si != mStagingList.end()) {
			// raw data loaded, but still need to construct the resource object and store in the cache
			AsyncLoadDoneEvent &e = *(static_cast<AsyncLoadDoneEvent*>(si->second.get()));
			if (!e.mSuccess) { return ResLoadResult_Error; }	// error while loading

			// construct a new Resource object, store it in a ResPtr
			// and pass into the ResHandle
			ResPtr resPtr(new TResource(h.name(), e.mSize, cache));
			h.mResPtr = resPtr;
			// store the resource in a cache (specified by the resource)
			bool added = cache->addToCache(e.mSize, h);
			if (added) {
				// call the resource's onLoad method
				TResource *pRes = static_cast<TResource*>(resPtr.get());
				pRes->onLoad(e.mDataPtr, true);
			}
			// remove event from staging list
			mStagingList.erase(si);
			// return success if added to cache
			return (added ? ResLoadResult_Success : ResLoadResult_Error);

		} else {
			// data not in staging area, check loading queue to see if it has already been requested
			RequestQueue::const_iterator ri = mRequestList.find(ss.str());
			if (ri == mRequestList.end()) {
				// not yet requested, queue it up to load asynchronously in a thread process
				ResSourceMap::const_iterator mi = mSourceMap.find(h.source());
				if (mi != mSourceMap.end()) {
					// add to request list, index by source/name
					mRequestList.insert(ss.str());
					// queues an event for thread to pick up
					EventPtr ePtr(new AsyncLoadEvent(h.name(), h.source(), mi->second));
					events.raise(ePtr);
				} else {
					return ResLoadResult_Error; // not in the queue, error requesting
				}
			}
		}

		return ResLoadResult_Waiting; // not in the queue, but requested for loading in the background
	}
	return ResLoadResult_Success; // found in the cache
}
