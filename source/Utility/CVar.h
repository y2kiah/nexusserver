/*----==== CVAR.H ====----
	Author:		Jeff Kiah
	Orig.Date:	04/20/2007
	Rev.Date:	05/04/2009
------------------------*/

#pragma once

#include <crtdbg.h>
#include <string>

using std::string;

///// STRUCTURES /////

enum CVarType : unsigned char {
	CVAR_TYPE_UCHAR = 0,
	CVAR_TYPE_CHAR,
	CVAR_TYPE_UINT,
	CVAR_TYPE_INT,
	CVAR_TYPE_FLOAT,
	CVAR_TYPE_BOOL,
	CVAR_TYPE_STRING
};

/*=============================================================================
	class CVar
		CVar means Console Variable or Concurrent Variable. It can be used in
		arrays, lists, etc. where the element type is not common.
=============================================================================*/
class CVar {
	friend class Console;
	friend class ConcurrentManager;

	private:
		///// VARIABLES /////
		union {
			unsigned char	uc;
			char			c;
			unsigned int	ui;
			int				i;
			float			f;
			bool			b;
			string *		sPtr;
			void *			voidPtr;	// pass data without type
		};

		// immutable, once created the CVar cannot change type
		CVarType			mType;

		///// FUNCTIONS /////
		explicit CVar(const CVar &v) {}	// private copy constructor to prevent implicit copy

	public:
		// Accessors				
		unsigned char		getUChar() const	{ _ASSERTE(mType == CVAR_TYPE_UCHAR);	return uc; }
		char				getChar() const		{ _ASSERTE(mType == CVAR_TYPE_CHAR);	return c; }
		unsigned int		getUInt() const		{ _ASSERTE(mType == CVAR_TYPE_UINT);	return ui; }
		int					getInt() const		{ _ASSERTE(mType == CVAR_TYPE_INT);		return i; }
		float				getFloat() const	{ _ASSERTE(mType == CVAR_TYPE_FLOAT);	return f; }
		bool				getBool() const		{ _ASSERTE(mType == CVAR_TYPE_BOOL);	return b; }
		const string &		getString() const	{ _ASSERTE(mType == CVAR_TYPE_STRING);	return *sPtr; }
		const void *		getVoidPtr() const	{ return voidPtr; }
		CVarType			getType() const		{ return mType; }
		string				toString() const;

		// Mutators
		void	setUChar(unsigned char _uc)		{ _ASSERTE(mType == CVAR_TYPE_UCHAR);	uc = _uc; }
		void	setChar(char _c)				{ _ASSERTE(mType == CVAR_TYPE_CHAR);	c = _c; }
		void	setUInt(unsigned int _ui)		{ _ASSERTE(mType == CVAR_TYPE_UINT);	ui = _ui; }
		void	setInt(int _i)					{ _ASSERTE(mType == CVAR_TYPE_INT);		i = _i; }
		void	setFloat(float _f)				{ _ASSERTE(mType == CVAR_TYPE_FLOAT);	f = _f; }
		void	setBool(bool _b)				{ _ASSERTE(mType == CVAR_TYPE_BOOL);	b = _b; }
		void	setString(const string &s)		{ _ASSERTE(mType == CVAR_TYPE_STRING);	(*sPtr) = s; }

		// Constructor / Destructor
		explicit CVar(CVarType type) :
			mType(type), i(0)
		{
			if (mType == CVAR_TYPE_STRING) sPtr = new string;
		}
		~CVar() { if (mType == CVAR_TYPE_STRING) delete sPtr; }
};
