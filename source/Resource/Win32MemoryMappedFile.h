/*----==== WIN32MEMORYMAPPEDFILE.H ====----
	Author:	Jeffrey Kiah
	Date:	7/18/2007
-----------------------------------------*/

#pragma once

#include <string>
#include "ResCache.h"

using std::wstring;

///// STRUCTURES /////

/*=============================================================================
class Win32MemoryMappedFile
=============================================================================*/
class Win32MemoryMappedFile : public IResourceSource {
	private:
		// Variables
		void *		mFileHandle;	// set with CreateFile, void* = HANDLE
		void *		mFileMapping;	// set with CreateFileMapping
		void *		mFileBase;		// set with MapViewOfFileEx
		wstring		mFileName;		// relative path to the file on disk
		uchar		mIsOpen, mIsMapped, mIsViewed;		// indicates the file is open, mapped, and viewed

		void			close();

	public:
		// Interface implementations
		virtual char *	get() const;
		virtual uint	getSize() const;
		virtual int		getResourceSize(const Resource &r) { return 0; }
		virtual int		getResource(const Resource &r, char *buffer) { return 0; }

		// Member Functions
		bool			isOpen() const	{ return (mIsOpen == 1); }

		bool			open();

		// Constructor / Destructor
		explicit Win32MemoryMappedFile(const wstring &fileName);
		virtual ~Win32MemoryMappedFile();
};

