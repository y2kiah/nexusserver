/*----==== RESHANDLE.H ====----
	Author:		Jeff Kiah
	Orig.Date:	06/07/2009
	Rev.Date:	06/18/2009
	Description:
		Resources are loaded from IResourceSource derived classes via the
		ResHandle interface described in the usage patterns below. The caching
		system sits underneath this system and silently manages a least-
		recently-used cache of resources. The system uses a virtual file path
		to locate the resource, where a path such as "textures/sometexture.dds"
		would consider "textures" the source name (of the registered
		IResourceSource) and "sometexture.dds" as the resource name to be
		retrieved from the source. Any additional path after the source name is
		considered to be relative pathing from the root of the source. For
		example "scripts/ai/bot.lua" would look in the source "scripts" for the
		resource "ai/bot.lua".
	
		Usage patterns:

		-----------------------------------------------------------------------
		Synchronous loading from ResourceSource
		-----------------------------------------------------------------------	
			ResHandle h;
			if (!h.load<TextureRes>("textures/texName.tga")) {
				// handle error
			}
			TextureRes *tex = static_cast<TextureRes *>(h.getResPtr().get());
			// use tex...

		-----------------------------------------------------------------------
		Asynchronous loading from ResourceSource
		-----------------------------------------------------------------------
			ResHandle h;
			int retVal = h.tryLoad<TextureRes>("textures/texName.tga");
			if (retVal == ResLoadResult_Success) {
				TextureRes *tex = static_cast<TextureRes *>(h.getResPtr().get());
				// use tex...
			} else if (retVal == ResLoadResult_Error) {
				// stop waiting, handle error
			}
			// keep waiting... try again later

		-----------------------------------------------------------------------
		Resource injection (manual instantiation) - note this pattern is for
		creation only and does not automatically retrieve from a cache if the
		resource already exists. ResourceSource and ResHandle not used for this
		method. You must pass the specific cache to inject to. You can manually
		retrieve from a cache with the second pattern.
		-----------------------------------------------------------------------
			ResPtr texPtr(new TextureRes());
			TextureRes *tex = static_cast<TextureRes *>(texPtr.get());
			if (tex->loadFromFile("texName.tga")) { // must internally set mName and mSizeB
				if (!TextureRes::injectIntoCache(texPtr, ResCache_Texture)) {
					// resource already exists with this name
					// handle injection error
				}
				// you can now use tex... (though it may or may not be in cache)
			}

		... then somewhere else in your program, retrieve it from cache ...

			ResHandle h;
			if (!h.getFromCache("texName.tga", ResCache_Texture)) {
				// not in cache, handle error
			}
			TextureRes *tex = static_cast<TextureRes *>(h.getResPtr().get());
			// use tex...

-----------------------------*/

#pragma once

#include <string>
#include <memory>
#include <boost/noncopyable.hpp>
#include "../Utility/Typedefs.h"

using std::string;
using std::shared_ptr;
using std::tr1::weak_ptr;

///// DEFINITIONS /////

/*=============================================================================
	Don't need a unique type for each conceivable type of resource loaded, only
	need the set of unique caches the engine will use.
=============================================================================*/
enum ResCacheType : uchar {
	ResCache_Web = 0,
	ResCache_Project,
	ResCache_Script,
	ResCache_OnDemand,		// a special cache where nothing is actually kept, always loaded on request
	ResCache_KeepLoaded,	// a special cache guaranteed not to free the resource until cache is destroyed
	ResCache_MAX			// not a cache, reference for array size
};

/*=============================================================================
	Used as a return value for tryLoad(), for asynchronous loading patterns
=============================================================================*/
enum ResLoadResult : uchar {
	ResLoadResult_Waiting = 0,
	ResLoadResult_Success,
	ResLoadResult_Error
};

///// STRUCTURES /////

class Resource;
class ResCache;
typedef shared_ptr<Resource>	ResPtr;
typedef shared_ptr<ResCache>	ResCachePtr;
typedef weak_ptr<ResCache>		ResCacheWeakPtr;

/*=============================================================================
class ResHandle
=============================================================================*/
class ResHandle : private boost::noncopyable {
	protected:
		string			mName;		// this is the resource name, could be a filename or application-assigned
		string			mSource;	// this is the source name, could be a filename or application-assigned

	public:
		ResPtr			mResPtr;	// shared_ptr to the resource, or empty if not yet loaded

		/*---------------------------------------------------------------------
			load is used to retrieve a resource from disk or the cache (if
			available) in a sychronous manner. When this blocking call returns,
			the resource will be available, or the loading process will have
			failed. Returns true on success, false on error.
		---------------------------------------------------------------------*/
		template <typename TResource>
		inline bool load(const string &resPath);

		/*---------------------------------------------------------------------
			tryLoad is used to retrieve a resource from disk or the cache (if
			available) in an asynchronous manner. When this non-blocking call
			returns, if the resource was already in the cache, it will be
			available, and otherwise, a job to load it will be queued for a
			worker thread to do the loading. A process should be created to
			monitor the resource handle for the completion or failure of the
			loading.
		---------------------------------------------------------------------*/
		template <typename TResource>
		inline ResLoadResult tryLoad(const string &resPath);

		/*---------------------------------------------------------------------
			This will just attempt to pull a resource from a specific cache. If
			the resource does not exist, false is returned and mResPtr will
			be empty. mSource is always empty with this method, as it bypasses
			the mapping of ResourceSource name to cache type.
		---------------------------------------------------------------------*/
		bool	getFromCache(const string &resName, ResCacheType cacheType);

		// Accessors
		const string &	name() const		{ return mName; }
		const string &	source() const		{ return mSource; }
		const ResPtr &	getResPtr() const	{ return mResPtr; }
		bool			isLoaded() const	{ return (mResPtr.get() != 0); }

		explicit ResHandle() :
			mName(), mSource(), mResPtr()
		{}
		~ResHandle() {}
};

/*=============================================================================
class Resource
	Resources are owned by the ResPtr, their lifespan is determined by the 
	ResCache and any ResHandle's currently in scope
=============================================================================*/
class Resource : private boost::noncopyable {
	protected:
		///// VARIABLES /////
		string		mName;			// this is the resource name, could be a filename or application-assigned
		uint		mSizeB;			// size in bytes

		ResCacheWeakPtr		mResCacheWeakPtr;	// points to the managing cache so memoryHasBeenFreed can be called

	public:
		/*---------------------------------------------------------------------
			specifies the cache that will manager the resource, must be defined
			in all derived classes exactly as written below if the ResHandle
			methods of loading from cache will be used. Not required for types
			that will use only injection.
		---------------------------------------------------------------------*/
		// static const ResCacheType	sCacheType;

		///// DEFINITIONS /////
		typedef shared_ptr<char>		BufferPtr;

		///// FUNCTIONS /////
		const string &	name() const	{ return mName; }
		uint			sizeB() const	{ return mSizeB; }

		/*---------------------------------------------------------------------
			Manually adds the resource to a cache. This would be done if the
			resource is created manually through a derived class constructor,
			and not through the ResHandle creation idiom. This may be
			desireable if a resource needs finer control over construction
			variables that the ResHandle and onLoad() convention don't provide
			for. Returns true on success, false if the resource name already
			exists in the cache.
		---------------------------------------------------------------------*/
		static bool		injectIntoCache(const ResPtr &resPtr, ResCacheType cacheType);

		/*---------------------------------------------------------------------
			Manually removes the resource from the managing cache (bypassing
			the cache LRU process). Use this to ensure ResPtr goes out of scope
			so the resource will actually be destroyed and free up system
			memory. Could use for this for resources that you know will not be
			used again.
		---------------------------------------------------------------------*/
		void	removeFromCache();

		/*---------------------------------------------------------------------
			onLoad should perform any processing that needs to be done after
			initial loading from source. It should take the raw char buffer
			passed in and initialize class members. This is automatically
			called from the ResCacheManager on initial load from source file.
			bool async is true if the call resulted from the tryLoad() method,
			false if called from the synchronous load() method. This infor-
			mation can be used to continue the appropriate pattern of loading
			any child resources (e.g. textures for a material).
		---------------------------------------------------------------------*/
		virtual bool	onLoad(const BufferPtr &dataPtr, bool async) = 0;

		// Constructor / destructor
		/*---------------------------------------------------------------------
			a constructor with this signature must be implemented in each
			derived class - required for the res loading system
		---------------------------------------------------------------------*/
		explicit Resource(const string &name, uint sizeB, const ResCachePtr &resCachePtr) :
			mName(name), mSizeB(sizeB), mResCacheWeakPtr(resCachePtr)
		{}
		/*---------------------------------------------------------------------
			this default constructor is provided for use from derived class
			constructors of resources that will be manually injected into the
			cache, or won't be cached at all.
		---------------------------------------------------------------------*/
		explicit Resource() :
			mName(), mSizeB(0), mResCacheWeakPtr()
		{}

		virtual ~Resource();
};

///// TEMPLATE FUNCTIONS /////

#include "ResCache.h"

////////// class ResHandle //////////

/*---------------------------------------------------------------------
	load is used to retrieve a resource from disk or the cache (if
	available) in a sychronous manner. When this blocking call returns,
	the resource will be available, or the loading process will have
	failed. Returns true on success, false on error.
---------------------------------------------------------------------*/
template <typename TResource>
inline bool ResHandle::load(const string &resPath)
{
	int i = resPath.find_first_of("/\\"); // find the first slash or backslash
	if (i == string::npos) { // if no slash found, cannot find the source so return false
		debugPrintf("ResHandle: invalid path in load: \"%s\"\n", resPath.c_str());
		return false;
	}
	mSource = resPath.substr(0, i);
	mName = resPath.substr(i+1);
	return ResCacheManager::instance().load<TResource>(*this);
}

/*---------------------------------------------------------------------
	tryLoad is used to retrieve a resource from disk or the cache (if
	available) in an asynchronous manner. When this non-blocking call
	returns, if the resource was already in the cache, it will be
	available, and otherwise, a job to load it will be queued for a
	worker thread to do the loading. A process should be created to
	monitor the resource handle for the completion or failure of the
	loading.
---------------------------------------------------------------------*/
template <typename TResource>
inline ResLoadResult ResHandle::tryLoad(const string &resPath)
{
	int i = resPath.find_first_of("/\\"); // find the first slash or backslash
	if (i == string::npos) { // if no slash found, cannot find the source so return false
		debugPrintf("ResHandle: invalid path in load: \"%s\"\n", resPath.c_str());
		return ResLoadResult_Error;
	}
	mSource = resPath.substr(0, i);
	mName = resPath.substr(i+1);
	return ResCacheManager::instance().tryLoad<TResource>(*this);
}