/*----==== RESHANDLE.CPP ====----
	Author:		Jeff Kiah
	Orig.Date:	06/07/2009
	Rev.Date:	06/14/2009
-------------------------------*/

#include "ResHandle.h"

////////// class ResHandle //////////

/*---------------------------------------------------------------------
	This will just attempt to pull a resource from a specific cache. If
	the resource does not exist, false is returned and mResPtr will
	be empty. mSource is always empty with this method, as it bypasses
	the mapping of ResourceSource name to cache type.
---------------------------------------------------------------------*/
bool ResHandle::getFromCache(const string &resName, ResCacheType cacheType)
{
	mName = resName;
	return resMgr.getFromCache(*this, cacheType);
}

////////// class Resource //////////

/*---------------------------------------------------------------------
	Manually adds the resource to a cache. This would be done if the
	resource is created manually through a derived class constructor,
	and not through the ResHandle creation idiom. This may be
	desireable if a resource needs finer control over construction
	variables that the ResHandle and onLoad() convention don't provide
	for. Returns true on success, false if the resource name already
	exists in the cache.
---------------------------------------------------------------------*/
bool Resource::injectIntoCache(const ResPtr &resPtr, ResCacheType cacheType)
{
	if (resPtr.get() == 0) {
		debugPrintf("Resource: Error: cannot inject an empty ResPtr to cache!\n");
		return false;
	}
	if (resPtr->name().empty()) {
		debugPrintf("Resource: Error: cannot inject a Resource with no name!\n");
		return false;
	}
	// returns true if added, false if no room or name already exists
	return resMgr.getResCache(cacheType)->addToCache(resPtr);
}

/*---------------------------------------------------------------------
	Manually removes the resource from the managing cache (bypassing
	the cache LRU process). Use this to ensure ResPtr goes out of scope
	so the resource will actually be destroyed and free up system
	memory. Could use for this for resources that you know will not be
	used again.
---------------------------------------------------------------------*/
void Resource::removeFromCache()
{
	// ** NOTE ** uses weak_ptr pattern to ResCache incase cache does not exist at time of call
	if (ResCachePtr r = mResCacheWeakPtr.lock()) {
		r->removeResource(name());
	}
}

Resource::~Resource()
{
	// Notifies cache that this object was destroyed. Because the object is owned by a shared_ptr,
	// this only happens once the cache has freed it via freeOneResource, and the game object(s)
	// holding onto it have also released it or are destroyed.
	// ** NOTE ** uses weak_ptr pattern to ResCache incase cache does not exist at time of call
	if (ResCachePtr r = mResCacheWeakPtr.lock()) {
		r->memoryHasBeenFreed(sizeB());
	}
}