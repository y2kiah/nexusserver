/*----==== RESCACHE.CPP ====----
	Author:		Jeff Kiah
	Orig.Date:	07/16/2007
	Rev.Date:	09/24/2011
------------------------------*/

#include "ResCache.h"
#include "ResourceProcess.h"
#include "../Event/EventManager.h"

////////// class ResCache //////////

/*---------------------------------------------------------------------
	Calls freeOneResource until new request can fit, returns false when
	request is too large for cache
---------------------------------------------------------------------*/
bool ResCache::makeRoom(uint sizeB)
{
	debugPrintf("size requested %u is larger than max cache size %u\n", sizeB, mMaxSizeB);

	if (hasRoom(sizeB)) { return true; }

	while (freeOneResource()) {
		if (hasRoom(sizeB)) { return true; }
	}
	return false; // freed all resources and still no room
}

/*---------------------------------------------------------------------
	Deletes one resource, the least recently used. Returns false if
	cache is already empty.
---------------------------------------------------------------------*/
bool ResCache::freeOneResource()
{
	if (mLRU.empty()) return false;
	ResPtr gonner(mLRU.back());
	mLRU.pop_back();
	mResMap.erase(gonner->name());
	return true;
}

/*---------------------------------------------------------------------
	Pushes a resource to the front of the LRU list.
---------------------------------------------------------------------*/
void ResCache::makeMostRecent(const ResMap::iterator &mi)
{
	ResPtr resPtr(*mi->second);	// temporarily store the erased handle
	mLRU.erase(mi->second);		// erase the element from its position in the list
	mLRU.push_front(resPtr);	// make it the most recently used
	mi->second = mLRU.begin();	// fix the iterator stored in the map
}

/*---------------------------------------------------------------------
	If the resource indexed by [key] is present in the cache, return
	true and point resPtr to the resource. Returns false if resource
	not present.
---------------------------------------------------------------------*/
bool ResCache::getResource(ResPtr &resPtr, const string &key)
{
	ResMap::iterator mi = mResMap.find(key);
	if (mi == mResMap.end()) { return false; }

	// resource loaded in the cache
	makeMostRecent(mi);		// and make it the most recent
	resPtr = (*mi->second); // return the ResPtr
	return true;
}

/*---------------------------------------------------------------------
	adds a resource to the cache
---------------------------------------------------------------------*/
bool ResCache::addToCache(uint sizeB, const ResHandle &h)
{
	_ASSERTE(h.isLoaded() && "Trying to add an empty ResPtr to the cache");
	_ASSERTE(!h.name().empty() && "Can't add a Resource to the cache with an empty name");

	// try to find the name in the cache, if it already exists, return false
	if (mResMap.find(h.name()) != mResMap.end()) {
		debugPrintf("ResCache: \"%s\" already exists, add to cache failed!\n", h.name().c_str());
		return false;
	}
	// make sure there is room in the cache
	if (makeRoom(sizeB)) {
		mLRU.push_front(h.mResPtr);	// add the resource to the front of the list
		mResMap[h.name()] = mLRU.begin();	// and insert the iterator into the map
		mUsedB += sizeB;			// and allocate the size in the cache
		return true;
	}
	// when mAllowOversizedResources is true, we indicate that the resource has been
	// added so loading succeeds, but it hasn't actually been added
	return mAllowOversizedResources;
}

/*---------------------------------------------------------------------
	adds a resource to cache, name and sizeB taken from the Resource
---------------------------------------------------------------------*/
bool ResCache::addToCache(const ResPtr &resPtr)
{
	const string &resName = resPtr->name();
	uint sizeB = resPtr->sizeB();

	_ASSERTE(resPtr.get() != 0 && "Can't to add an empty ResPtr to the cache");
	_ASSERTE(!resName.empty() && "Can't add a Resource to the cache with an empty name");

	// try to find the name in the cache, if it already exists, return false
	if (mResMap.find(resName) != mResMap.end()) {
		debugPrintf("ResCache: \"%s\" already exists, add to cache failed!\n", resName.c_str());
		return false;
	}
	// make sure there is room in the cache
	if (makeRoom(sizeB)) {
		mLRU.push_front(resPtr);	// add the resource to the front of the list
		mResMap[resName] = mLRU.begin();	// and insert the iterator into the map
		mUsedB += sizeB;			// and allocate the size in the cache
		return true;
	}
	// when mAllowOversizedResources is true, we indicate that the resource has been
	// added so loading succeeds, but it hasn't actually been added
	return mAllowOversizedResources;
}

/*---------------------------------------------------------------------
	removes a specific resource from the cache
---------------------------------------------------------------------*/
bool ResCache::removeResource(const string &key)
{
	ResMap::iterator mi = mResMap.find(key);
	if (mi != mResMap.end()) {
		mLRU.erase(mi->second);	// erase from the LRU list
		mResMap.erase(mi);		// erase from the hash map
		debugPrintf("ResCache: \"%s\" removed from cache: %i remaining\n", key.c_str(), mLRU.size());
		return true;
	}
	return false;
}

// Constructor / destructor
ResCache::ResCache(uint sizeMB, bool allowOversizedResources) :
	mMaxSizeB(sizeMB*1024*1024), mUsedB(0), mAllowOversizedResources(allowOversizedResources)
{}

ResCache::~ResCache()
{
	clearCache();
}

////////// class ResCacheManager //////////

/*---------------------------------------------------------------------
	creates the cache of a certain type passing in the budget, only one
	cache of each type allowed
---------------------------------------------------------------------*/
void ResCacheManager::createCache(ResCacheType cacheType, uint maxSizeMB, bool allowOversizedResources)
{
	if (mCacheList[cacheType].get() != 0) {
		debugPrintf("ResCacheManager: cache %i already created\n", (int)cacheType);
		return;
	}
	ResCachePtr cPtr(new ResCache(maxSizeMB, allowOversizedResources));
	mCacheList[cacheType] = cPtr;
}

/*---------------------------------------------------------------------
	Pushes an AsyncLoadDoneEvent into the staging list to be picked up
	by tryLoad(). Also removes the entry from the request queue.
---------------------------------------------------------------------*/
void ResCacheManager::addToStagingList(const string &resName, const string &source, const EventPtr &ePtr)
{
	std::stringstream ss;
	ss << source << '/' << resName;
	resMgr.mStagingList[ss.str()] = ePtr;
	resMgr.mRequestList.erase(ss.str());	// remove entry from the request queue
	debugPrintf("ResCacheManager: \"%s\" added to staging\n", ss.str().c_str());
}

/*---------------------------------------------------------------------
	This will just attempt to pull a resource from a specific cache. If
	the resource does not exist, false is returned and h.mResPtr will
	be empty. h.mSource is ignored with this method, as it bypasses
	the mapping of ResourceSource name to cache type.
---------------------------------------------------------------------*/
bool ResCacheManager::getFromCache(ResHandle &h, ResCacheType cacheType)
{
	_ASSERTE(cacheType < ResCache_MAX && "Bad cacheType");
	// try to find the resource in cache
	ResCachePtr &cache = mCacheList[cacheType];
	if (!cache->getResource(h.mResPtr, h.name())) {
		// not in cache, so return false
		debugPrintf("ResCacheManager: getFromCache(\"%s\", %u) failed, not in cache!\n", h.name().c_str(), cacheType);
		return false;
	}
	return true; // found in the cache
}

/*---------------------------------------------------------------------
	load a new IResourceSource into the system, it should already be
	initialized for use (open() has already been called)
---------------------------------------------------------------------*/
bool ResCacheManager::registerSource(const string &srcName, const ResSourcePtr &srcPtr)
{
	ResSourceMap::const_iterator mi = mSourceMap.find(srcName);
	if (mi == mSourceMap.end()) { // source not already registered
		mSourceMap[srcName] = srcPtr;
		debugPrintf("ResCacheManager: source \"%s\" registered\n", srcName.c_str());
		return true;
	} else {
		debugPrintf("ResCacheManager: source \"%s\" already registered!\n", srcName.c_str());
		return false;
	}
}

// Constructor / destructor
ResCacheManager::ResCacheManager(uint availableSysMemMB, uint availableVidMemMB) :
	Singleton<ResCacheManager>(*this)
{
	// reserve space for the caches
	mCacheList.reserve(ResCache_MAX);
	for (int c = 0; c < ResCache_MAX; ++c) {
		mCacheList.push_back(ResCachePtr((ResCache*)0));
	}

	// ** NOTE ** these three lines have been moved to Application.cpp to allow
	// for app.config to vary the allocations
	// the following percentages should be data driven not hard coded
	// also should incorporate vid mem into total sizes
/*	createCache(ResCache_Web,		(uint)(availableSysMemMB * 0.30f));
	createCache(ResCache_Project,	(uint)(availableSysMemMB * 0.30f));
	createCache(ResCache_Script,	(uint)(availableSysMemMB * 0.30f));*/
	// leaving a 10% buffer
	createCache(ResCache_OnDemand,		0); // zero size means anything can load, but will never be cached
	createCache(ResCache_KeepLoaded,	availableSysMemMB); // large size means always keep resources cached

	// create the thread process that will process async loading requests
	CProcessPtr procPtr(new AsyncLoadProcess("AsyncLoadProcess"));
	mThreadProcPtr = procPtr;
	procMgr.attach(mThreadProcPtr);
}

ResCacheManager::~ResCacheManager()
{
	// send shutdown event to wake up the thread and allow it to exit incase it's idle
	eventMgr.trigger(AsyncLoadProcess::sAsyncLoadShutdownEvent);
}

////////// class AsyncLoadDoneListener //////////

bool ResCacheManager::AsyncLoadDoneListener::handleEvent(const EventPtr &ePtr)
{
	// the loading is done, copy the event to a staging area where it will be picked up and
	// put into cache the next time tryLoad is run requesting the resource
	AsyncLoadDoneEvent &e = *(static_cast<AsyncLoadDoneEvent*>(ePtr.get()));
	resMgr.addToStagingList(e.mResName, e.mSourceName, ePtr);
	return false; // allow event to propagate
}

ResCacheManager::AsyncLoadDoneListener::AsyncLoadDoneListener() :
	EventListener("AsyncLoadDoneListener")
{
	IEventHandlerPtr p(new EventHandler<AsyncLoadDoneListener>(this, &AsyncLoadDoneListener::handleEvent));
	registerEventHandler(AsyncLoadDoneEvent::sEventType, p, 1);
}
