/*----==== CVAR.CPP ====----
	Author: Jeff Kiah
	Date:	4/20/2007
--------------------------*/

#include <sstream>
#include "CVar.h"


///// FUNCTIONS /////

string CVar::toString() const
{
	std::stringstream s;

	switch (mType) {
		case CVAR_TYPE_UCHAR:
			s << "(uchar)" << uc;
		case CVAR_TYPE_CHAR:
			s << "(char)" << c;
		case CVAR_TYPE_UINT:
			s << "(uint)" << ui;
		case CVAR_TYPE_INT:
			s << "(int)" << i;
		case CVAR_TYPE_FLOAT:
			s << "(float)" << f;
		case CVAR_TYPE_BOOL:
			s << "(bool)" << b;
		case CVAR_TYPE_STRING:
			s << "(string)" << *sPtr;
		default:
			s << "(void*)";
	}

	return s.str();
}