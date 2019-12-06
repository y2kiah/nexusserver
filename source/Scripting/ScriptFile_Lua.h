/*----==== SCRIPTFILE_LUA.H ====----
	Author:		Jeff Kiah
	Orig.Date:	08/25/2009
	Rev.Date:	09/19/2011
----------------------------------*/

#pragma once

#include "../Resource/ResHandle.h"

///// STRUCTURES /////

/*=============================================================================
class ScriptFile_Lua
	This class derived from Resource enables Lua script files (.lua or .luc) to
	be loaded from any IResourceSource using the resource caching system.
=============================================================================*/
class ScriptFile_Lua : public Resource {
	private:
		///// VARIABLES /////
		BufferPtr	mDataPtr; // resource file data

	public:
		static const ResCacheType	sCacheType = ResCache_Script;

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
		explicit ScriptFile_Lua(const string &name, uint sizeB, const ResCachePtr &resCachePtr) :
			Resource(name, sizeB, resCachePtr)
		{}
		/*---------------------------------------------------------------------
			default constructor used to create resources without caching, or
			for cache injection method
		---------------------------------------------------------------------*/
		explicit ScriptFile_Lua() {}

		virtual ~ScriptFile_Lua() {}
};