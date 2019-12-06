/*----==== FILESYSTEMSOURCE.H ====----
	Author:		Jeff Kiah
	Orig.Date	09/23/2011
------------------------------------*/

#pragma once

#include <string>
#include "ResCache.h"

class FileSystemSource : public IResourceSource {
	private:
		///// VARIABLES /////
		string mRootPath;		// relative path to the root (also want to support full path)

		int loadResourceFile(const string &resName, BufferPtr &dataPtr, int loadData) const;

	public:
		///// FUNCTIONS /////
		// Interface functions
		virtual bool open();
		virtual int getResourceSize(const string &resName) const;
		virtual int getResource(const string &resName, BufferPtr &dataPtr, int threadIndex = 0);

		/*---------------------------------------------------------------------
			Returns a new index, or -1 on error.
		---------------------------------------------------------------------*/
		virtual int getNewThreadIndex() { return 0; } // function is not neccessary in this implementation

		// Constructor / destructor
		explicit FileSystemSource(const string &rootPath) :
			mRootPath(rootPath)
		{}
};