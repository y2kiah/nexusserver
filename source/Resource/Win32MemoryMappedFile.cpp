/*----==== WIN32MEMORYMAPPEDFILE.H ====----
	Author:	Jeffrey Kiah
	Date:	7/18/2007
-----------------------------------------*/

#include "Windows.h"
#include "Win32MemoryMappedFile.h"


////////// class Win32MemoryMappedFile //////////

///// FUNCTIONS /////

// if open returns false, close should be called to clean up
bool Win32MemoryMappedFile::open()
{
	// ensure length of filename is within limits
	if (mFileName.length() > MAX_PATH) {
		debugPrintf("Win32MemoryMappedFile::open: mFileName length greather than MAX_PATH\n");
		return false;
	}

	// Open memory mapped file
	mFileHandle = CreateFile(	mFileName.c_str(),
								GENERIC_READ,		// give read-only access
								FILE_SHARE_READ,	// enable subsequent open access for read
								NULL,
								OPEN_EXISTING,		// function fails if file does not exist
								FILE_FLAG_OVERLAPPED | FILE_FLAG_RANDOM_ACCESS,	// also check performance with SEQUENTIAL
								NULL);
	if (mFileHandle == INVALID_HANDLE_VALUE) { 
		debugPrintf("Win32MemoryMappedFile::open: Could not open file (error %d)\n", GetLastError());
        return false; 
    }
	mIsOpen = 1;

	// create a mapping object from the file
	mFileMapping = CreateFileMapping(	mFileHandle,
										NULL,
										PAGE_READONLY,
										0, 0,
										NULL);
	if (mFileMapping == NULL) {
		debugPrintf("Win32MemoryMappedFile::open: CreateFileMapping failed (error %d)\n", GetLastError());
        return false;
	}
	mIsMapped = 1;

	// map the file to a memory pointer
	mFileBase = MapViewOfFile(	mFileMapping,
								FILE_MAP_READ,
								0, 0,
								0);
	if (mFileBase == NULL) {
		debugPrintf("Win32MemoryMappedFile::open: MapViewOfFile failed (error %d)\n", GetLastError());
		return false;
	}
	mIsViewed = 1;

	return true;
}

// if open returns false, close should be called to clean up
void Win32MemoryMappedFile::close()
{
	if (mIsViewed == 1) {
		if (UnmapViewOfFile(mFileBase) == 0) {
			debugPrintf("Win32MemoryMappedFile::close: UnmapViewOfFile failed (error %d)\n", GetLastError());
		}
		mIsViewed = false;
	}
	if (mIsMapped == 1) {
		if (CloseHandle(mFileMapping) == 0) {
			debugPrintf("Win32MemoryMappedFile::close: CloseHandle(mFileMapping) failed (error %d)\n", GetLastError());
		}
		mIsMapped = false;
	}
	if (mIsOpen == 1) {
		if (CloseHandle(mFileHandle) == 0) {
			debugPrintf("Win32MemoryMappedFile::close: CloseHandle(mFileHandle) failed (error %d)\n", GetLastError());
		}
		mIsOpen = false;
	}
}

// Interface implementations
char * Win32MemoryMappedFile::get() const
{
	// find resource in index

	// offset mFileBase by location
	int offset = 0;

	// write to dest buffer
	char *dest = static_cast<char*>(mFileBase) + offset;
	// memcpy(dest, mFileBase, size);
	// use ReadFileEx for asynch read in asynch class, but will probably need a flag

	return 0;
}

uint Win32MemoryMappedFile::getSize() const
{
	return 0;
}

// Constructor / Destructor
Win32MemoryMappedFile::Win32MemoryMappedFile(const wstring &fileName) :
	IResourceSource(),
	mFileName(fileName), mFileHandle(0), mFileMapping(0), mFileBase(0),
	mIsOpen(0), mIsMapped(0), mIsViewed(0)
{}

Win32MemoryMappedFile::~Win32MemoryMappedFile()
{
	close();
}


