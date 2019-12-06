/*----==== WEBRESOURCE.H ====----
	Author:		Jeff Kiah
	Orig.Date:	09/15/2011
	Rev.Date:	09/15/2011
-------------------------------*/

#pragma once

#include "../Resource/ResHandle.h"

///// STRUCTURES /////

/*=============================================================================
class WebResource
	This class derived from Resource is for binary loading of documents and
	images from any IResourceSource using the resource caching system.
=============================================================================*/
class WebResource : public Resource {
	private:
		///// VARIABLES /////
		BufferPtr	mDataPtr; // resource file data

	public:
		static const ResCacheType	sCacheType = ResCache_Web;

		///// FUNCTIONS /////
		const BufferPtr &dataPtr() const { return mDataPtr; }

		/*---------------------------------------------------------------------
			onLoad is called automatically by the resource caching system when
			a resource is first loaded from disk and added to the cache.
		---------------------------------------------------------------------*/
		virtual bool onLoad(const BufferPtr &dataPtr, bool async)
		{
			mDataPtr = dataPtr;
			return true;
		}

		/*---------------------------------------------------------------------
			constructor with this signature is required for the resource system
		---------------------------------------------------------------------*/
		explicit WebResource(const string &name, uint sizeB, const ResCachePtr &resCachePtr) :
			Resource(name, sizeB, resCachePtr)
		{}
		/*---------------------------------------------------------------------
			default constructor used to create resources without caching, or
			for cache injection method
		---------------------------------------------------------------------*/
		explicit WebResource() {}

		virtual ~WebResource() {}
};