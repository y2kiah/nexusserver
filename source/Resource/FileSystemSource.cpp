/*----==== FILESYSTEMSOURCE.CPP ====----
	Author:		Jeff Kiah
	Orig.Date	09/23/2011
--------------------------------------*/

#include "FileSystemSource.h"
#include <fstream>

using namespace std;

////////// class FileSystemSource //////////
bool FileSystemSource::open()
{
	// maybe check if directory exists first
	return true;
}

int FileSystemSource::loadResourceFile(const string &resName, BufferPtr &dataPtr, int loadData) const
{
	// get relative path of file
	string relPath(mRootPath);
	char lastChar = relPath.back();
	if (lastChar != '\\' && lastChar != '/') {
		relPath.append("\\");
	}
	relPath.append(resName);

	// open file for loading
	ifstream ifs(relPath, ios::binary);
	if (ifs.is_open()) {
		// get data size
		long size = (long)ifs.rdbuf()->pubseekoff(0, ios::end, ios::in);
		ifs.rdbuf()->pubseekpos(0, ios::in);
		
		// a loadData value of 0 in this case means a request for the size
		// without actually loading the buffer
		if (loadData != 0) {
			// construct new buffer
			BufferPtr bPtr(new char[size], checked_array_deleter<char>());
			dataPtr = bPtr;
			// read the data
			ifs.rdbuf()->sgetn(dataPtr.get(), size);
		}		
		ifs.close();
		return size;
	}
	dataPtr.reset();
	return 0;
}

int FileSystemSource::getResourceSize(const string &resName) const
{
	int result = loadResourceFile(resName, BufferPtr(), 0);
	return (result == 0 ? -1 : result); // convert 0 error code to -1
}

/*---------------------------------------------------------------------
	Uses threadIndex value of -1 to return size without loading data,
	useful for the getResourceSize function.
---------------------------------------------------------------------*/
int FileSystemSource::getResource(const string &resName, BufferPtr &dataPtr, int threadIndex)
{
	return loadResourceFile(resName, dataPtr, 1);
}