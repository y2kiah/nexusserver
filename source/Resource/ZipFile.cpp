/*----==== ZIPFILE.CPP ====----
	Author:		Jeff Kiah
	Orig.Date	05/25/2009
	Rev.Date	09/22/2011
	Purpose:	The declaration of a quick'n dirty ZIP file reader class. Original code from Javier Arevalo.
				zlib at http://www.cdrom.com/pub/infozip/zlib/
				Got code from Game Coding Complete 3rd Edition and modified it for this engine.
-----------------------------*/

#include "ZipFile.h"
#include <zlib.h>
#include <string>
#include <algorithm>
#include <boost/checked_delete.hpp>

using namespace std;
using boost::checked_array_deleter;

///// DEFINITIONS /////

typedef unsigned long	dword;
typedef unsigned short	word;
typedef unsigned char	byte;

///// STRUCTURES /////

#pragma pack(1)		// these structures have to be packed

struct ZipFile::TZipLocalHeader {
	enum {
		SIGNATURE = 0x04034b50,
	};
	dword	sig;
	word	version;
	word	flag;
	word	compression;	// Z_NO_COMPRESSION or Z_DEFLATED
	word	modTime;
	word	modDate;
	dword	crc32;
	dword	cSize;
	dword	ucSize;
	word	fnameLen;		// Filename string follows header.
	word	xtraLen;		// Extra field follows filename.
};

struct ZipFile::TZipDirHeader {
	enum {
		SIGNATURE = 0x06054b50
	};
	dword	sig;
	word	nDisk;
	word	nStartDisk;
	word	nDirEntries;
	word	totalDirEntries;
	dword	dirSize;
	dword	dirOffset;
	word	cmntLen;
};

class ZipFile::TZipDirFileHeader {
public:
	enum {
		SIGNATURE   = 0x02014b50
	};
	dword	sig;
	word	verMade;
	word	verNeeded;
	word	flag;
	word	compression;	// COMP_xxxx
	word	modTime;
	word	modDate;
	dword	crc32;
	dword	cSize;			// Compressed size
	dword	ucSize;			// Uncompressed size
	word	fnameLen;		// Filename string follows header.
	word	xtraLen;		// Extra field follows filename.
	word	cmntLen;		// Comment field follows extra field.
	word	diskStart;
	word	intAttr;
	dword	extAttr;
	dword	hdrOffset;

	char *getName() const		{ return (char *)(this + 1); }
	char *getExtra() const		{ return getName() + fnameLen; }
	char *getComment() const	{ return getExtra() + xtraLen; }
};

#pragma pack()

///// FUNCTIONS /////

/*---------------------------------------------------------------------
	Initialize the object and read the zip file directory
---------------------------------------------------------------------*/
bool ZipFile::open()
{
	errno_t err = _wfopen_s(&mFile[0], mZipFilename.c_str(), L"rb");
	if (err != 0) { return false; }
	mInitFlags[INIT_OPEN] = true; // set the open init flag, so fclose will be called in destructor

	// Assuming no extra comment at the end, read the whole end record.
	TZipDirHeader dh;

	fseek(mFile[0], -(int)sizeof(dh), SEEK_END);
	long dhOffset = ftell(mFile[0]);
	memset(&dh, 0, sizeof(dh));
	fread(&dh, sizeof(dh), 1, mFile[0]);

	// Check
	if (dh.sig != TZipDirHeader::SIGNATURE) { return false; }

	// Go to the beginning of the directory.
	fseek(mFile[0], dhOffset - dh.dirSize, SEEK_SET);

	// Allocate the data buffer, and read the whole thing.
	mDirData = new char[dh.dirSize + dh.nDirEntries*sizeof(*mDirHdr)];
	if (!mDirData) { return false; }
	
	memset(mDirData, 0, dh.dirSize + dh.nDirEntries*sizeof(*mDirHdr));
	fread(mDirData, dh.dirSize, 1, mFile[0]);

	// Now process each entry.
	char *pfh = mDirData;
	mDirHdr = (const TZipDirFileHeader **)(mDirData + dh.dirSize);

	bool success = true;

	for (int i = 0; i < dh.nDirEntries && success; ++i) {
		TZipDirFileHeader &fh = *(TZipDirFileHeader*)pfh;

		// Store the address of nth file for quicker access.
		mDirHdr[i] = &fh;

		// Check the directory entry integrity.
		if (fh.sig != TZipDirFileHeader::SIGNATURE) {
			success = false;
		} else {
			pfh += sizeof(fh);

			// Convert UNIX slashes to DOS backlashes.
			for (int j = 0; j < fh.fnameLen; ++j) {
				if (pfh[j] == '/') pfh[j] = '\\';
			}

			char fileName[_MAX_PATH];
			memcpy(fileName, pfh, fh.fnameLen);
			fileName[fh.fnameLen]=0;
			_strlwr_s(fileName, _MAX_PATH);
			string spath = fileName;
			mZipContentsMap[spath] = i;

			// Skip name, extra and comment fields.
			pfh += fh.fnameLen + fh.xtraLen + fh.cmntLen;
		}
	}

	if (!success) {
		if (mDirData) {
			delete [] mDirData;
			mDirData = 0;
		}
	} else {
		mEntries = dh.nDirEntries;
	}

	return success;
}

optional<int> ZipFile::find(const char *path) const
{
	char lwrPath[_MAX_PATH];
	strcpy_s(lwrPath, _MAX_PATH, path);
	_strlwr_s(lwrPath, _MAX_PATH);
	ZipContentsMap::const_iterator i = mZipContentsMap.find(lwrPath);
	if (i == mZipContentsMap.end()) {
		debugPrintf("ZipFile: find(\"%s\") file not found!\n", path);
		return optional<int>();
	}
	return (*i).second;
}

/*---------------------------------------------------------------------
	Finish the object
---------------------------------------------------------------------*/
void ZipFile::close()
{
	mZipContentsMap.empty();
	if (mDirData) {
		delete [] mDirData;
		mDirData = 0;
	}
	mEntries = 0;
	if (mInitFlags[INIT_OPEN]) {
		// close all of the open file pointers
		for (uint f = 0; f < mFile.size(); ++f) {
			fclose(mFile[f]);
		}
	}
}

/*---------------------------------------------------------------------
	Returns the size in bytes of the resource with resName, or -1 on
	error.
---------------------------------------------------------------------*/
int	ZipFile::getResourceSize(const string &resName) const
{
	optional<int> resNum = find(resName.c_str());
	if (resNum) {
		return getFileLen(*resNum);
	} else {
		return -1;
	}
}

/*---------------------------------------------------------------------
	Returns the size in bytes of the resource with resName, or 0 on
	error, so return value may be tested as a boolean.
	shared_ptr<char> &dataPtr sets the passed-in shared pointer to
	contain a new buffer of the returned size which contains the data.
---------------------------------------------------------------------*/
int ZipFile::getResource(const string &resName, BufferPtr &dataPtr, int threadIndex)
{
	optional<int> resNum = find(resName.c_str());
	if (resNum) {
		int size = getFileLen(*resNum);
		if (size > 0) { // treat 0 size as an error, since resources must have size
			BufferPtr bPtr(new char[size], checked_array_deleter<char>());
			dataPtr = bPtr;
			void *buffer = static_cast<void *>(dataPtr.get());
			if (readFile(*resNum, buffer, threadIndex)) {
				return size;	// success, return the size
			} else {				// failed
				dataPtr.reset();	// make sure the returned shared_ptr is empty
			}
		}
	}
	return 0;	// return 0 to indicate error
}

/*---------------------------------------------------------------------
	Return the name of a file. Takes as parameters The file index and
	the buffer where to store the filename.
---------------------------------------------------------------------*/
void ZipFile::getFilename(int i, char *pszDest) const
{
	if (pszDest != NULL) {
		if (i < 0 || i >= mEntries) {
			*pszDest = '\0';
		} else {
			memcpy(pszDest, mDirHdr[i]->getName(), mDirHdr[i]->fnameLen);
			pszDest[mDirHdr[i]->fnameLen] = '\0';
		}
	}
}

/*---------------------------------------------------------------------
	Return the length of a file so a buffer can be allocated
---------------------------------------------------------------------*/
int ZipFile::getFileLen(int i) const
{
	if (i < 0 || i >= mEntries) {
		debugPrintf("ZipFile: getFileLen() failed!\n");
		return -1;
	} else {
		return mDirHdr[i]->ucSize;
	}
}

/*---------------------------------------------------------------------
	Uncompress a complete file. Takes as parameters the file index and
	the pre-allocated buffer.
---------------------------------------------------------------------*/
bool ZipFile::readFile(int i, void *pBuf, int threadIndex)
{
	_ASSERTE(threadIndex >= 0 && threadIndex < (int)mFile.size() && "Thread index out of range");

	if (pBuf == NULL || i < 0 || i >= mEntries) return false;

	// Quick'n dirty read, the whole file at once.
	// Ungood if the ZIP has huge files inside

	// Go to the actual file and read the local header.
	fseek(mFile[threadIndex], mDirHdr[i]->hdrOffset, SEEK_SET);
	TZipLocalHeader h;

	memset(&h, 0, sizeof(h));
	fread(&h, sizeof(h), 1, mFile[threadIndex]);
	if (h.sig != TZipLocalHeader::SIGNATURE) return false;

	// Skip extra fields
	fseek(mFile[threadIndex], h.fnameLen + h.xtraLen, SEEK_CUR);

	if (h.compression == Z_NO_COMPRESSION) {
		// Simply read in raw stored data.
		fread(pBuf, h.cSize, 1, mFile[threadIndex]);
		return true;
	} else if (h.compression != Z_DEFLATED) {
		return false;
	}

	// Alloc compressed data buffer and read the whole stream
	char *pcData = new char[h.cSize];
	if (!pcData) return false;

	memset(pcData, 0, h.cSize);
	fread(pcData, h.cSize, 1, mFile[threadIndex]);

	bool ret = true;

	// Setup the inflate stream.
	z_stream stream;
	stream.next_in = (Bytef*)pcData;
	stream.avail_in = (uInt)h.cSize;
	stream.next_out = (Bytef*)pBuf;
	stream.avail_out = h.ucSize;
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	// Perform inflation. wbits < 0 indicates no zlib header inside the data.
	int err = inflateInit2(&stream, -MAX_WBITS);
	if (err == Z_OK) {
		err = inflate(&stream, Z_FINISH);
		inflateEnd(&stream);
		if (err == Z_STREAM_END) err = Z_OK;
		inflateEnd(&stream);
	}
	if (err != Z_OK) ret = false;

	delete [] pcData;
	return ret;
}

/*---------------------------------------------------------------------
	Uncompress a complete file with callbacks. Takes as parameters the
	file index and the pre-allocated buffer.
---------------------------------------------------------------------*/
bool ZipFile::readLargeFile(int i, void *pBuf, void (*callback)(int, bool &), int threadIndex)
{
	_ASSERTE(threadIndex >= 0 && threadIndex < (int)mFile.size() && "Thread index out of range");

	if (pBuf == NULL || i < 0 || i >= mEntries) return false;

	// Quick'n dirty read, the whole file at once.
	// Ungood if the ZIP has huge files inside

	// Go to the actual file and read the local header.
	fseek(mFile[threadIndex], mDirHdr[i]->hdrOffset, SEEK_SET);
	TZipLocalHeader h;

	memset(&h, 0, sizeof(h));
	fread(&h, sizeof(h), 1, mFile[threadIndex]);
	if (h.sig != TZipLocalHeader::SIGNATURE) return false;

	// Skip extra fields
	fseek(mFile[threadIndex], h.fnameLen + h.xtraLen, SEEK_CUR);

	if (h.compression == Z_NO_COMPRESSION) {
		// Simply read in raw stored data.
		fread(pBuf, h.cSize, 1, mFile[threadIndex]);
		return true;
	} else if (h.compression != Z_DEFLATED) {
		return false;
	}

	// Alloc compressed data buffer and read the whole stream
	char *pcData = new char[h.cSize];
	if (!pcData) return false;

	memset(pcData, 0, h.cSize);
	fread(pcData, h.cSize, 1, mFile[threadIndex]);

	bool ret = true;

	// Setup the inflate stream.
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)pcData;
	stream.avail_in = (uInt)h.cSize;
	stream.next_out = (Bytef*)pBuf;
	stream.avail_out = (128 * 1024); //  read 128k at a time h.ucSize;
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	// Perform inflation. wbits < 0 indicates no zlib header inside the data.
	err = inflateInit2(&stream, -MAX_WBITS);
	if (err == Z_OK) {
		uInt count = 0;
		bool cancel = false;
		while (stream.total_in < (uInt)h.cSize && !cancel) {
			err = inflate(&stream, Z_SYNC_FLUSH);
			if (err == Z_STREAM_END) {
				err = Z_OK;
				break;
			} else if (err != Z_OK) {
				assert(0 && "Something happened.");
				break;
			}

			stream.avail_out = (128 * 1024); 
			stream.next_out += stream.total_out;
			callback(count * 100 / h.cSize, cancel);
		}
		inflateEnd(&stream);
	}
	if (err != Z_OK) ret = false;

	delete [] pcData;
	return ret;
}

/*---------------------------------------------------------------------
	Creates a new file pointer and returns the index, or -1 on error.
---------------------------------------------------------------------*/
int ZipFile::getNewThreadIndex()
{
	mFile.push_back(0);
	int index = mFile.size() - 1;
	errno_t err = _wfopen_s(&mFile[index], mZipFilename.c_str(), L"rb");
	if (err != 0) { return -1; }
	return index;
}

string ZipFile::dumpIndex() const
{
	string output("Index:\n");
	for_each(mZipContentsMap.begin(), mZipContentsMap.end(), [&](const pair<string,int> &p){
		output.append(p.first.c_str());
		output.append("\n");
	});
	return output;
}

/*
Example useage:

void makePath(const char *pszPath)
{
	if (pszPath[0] == '\0') return;

	char buf[1000];
	const char *p = pszPath;

	//printf("MakePath(\"%s\")\n", pszPath);

	// Skip machine name in network paths like \\MyMachine\blah...
	if (p[0] == '\\' && p[1] == '\\') p = strchr(p+2, '\\');

	while (p != NULL && *p != '\0') {
		p = strchr(p, '\\');

		if (p) {
			memcpy(buf, pszPath, p - pszPath);
			buf[p - pszPath] = 0;
			++p;
		} else {
			strcpy(buf, pszPath);
		}

		if (buf[0] != '\0' && strcmp(buf, ".") && strcmp(buf, "..")) {
			//printf("  Making path: \"%s\"\n", buf);
			mkdir(buf);
		}
	}
}

void main(int argc, const char *argv[])
{
	if (argc > 1) {
		FILE *f = fopen(argv[1], "rb");
		if (f) {
			ZipFile zip;

			if (true != zip.init(f)) {
				printf("Bad Zip file: \"%s\"\n", argv[1]);
			} else {
				for (int i = 0; i < zip.GetNumFiles(); ++i) {
					int len = zip.GetFileLen(i);
					char fname[1000];

					zip.GetFilename(i, fname);

					printf("File \"%s\" (%d bytes): ", fname, len);

					char *pData = new char[len];
					if (!pData) {
						printf("OUT OF MEMORY\n");
					} else if (true == zip.ReadFile(i, pData)) {
						printf("OK\n");
						char dpath[1000];

						sprintf(dpath, "Data\\%s", fname);
						char *p = strrchr(dpath, '\\');
						if (p) {
							*p = '\0';
							MakePath(dpath);
							*p = '\\';
						}
						FILE *fo = fopen(dpath, "wb");
						if (fo) {
							fwrite(pData, len, 1, fo);
							fclose(fo);
						}
					} else {
						printf("ERROR\n");
					}
					delete[] pData;
				}
				zip.end();
			}
			fclose(f);
		}
	}
}
*/

