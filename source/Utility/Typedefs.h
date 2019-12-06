/*----==== TYPEDEFS.H ====----
	Author:		Jeffrey Kiah
	Orig.Date:	07/04/2007
	Rev.Date:	03/12/2010
----------------------------*/

#pragma once

#include <crtdbg.h>
#include "BitField.h"

///// DEFINITIONS /////

typedef unsigned char		uchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;
typedef __int64				int64;
typedef unsigned __int64	uint64;

typedef	BitField<uchar>		u8Flags;	// consider using boost's bitflags
typedef	BitField<ushort>	u16Flags;
typedef	BitField<uint>		u32Flags;

///// MACROS /////

#define SAFE_RELEASE(p)			{ if (p) { (p)->Release(); (p)=0; } }

#ifdef DEBUG_CONSOLE
	#include <cstdio> // for printf
	#define debugPrintf(...)	printf(__VA_ARGS__)
	#define debugTPrintf(s,...)	_tprintf(TEXT(s),__VA_ARGS__)
	#define ifDebug(...)		__VA_ARGS__
#else
	#define debugPrintf(...)
	#define debugTPrintf(...)
	#define ifDebug(...)
#endif
