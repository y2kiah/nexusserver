/*----==== EVENT.CPP ====----
	Author:		Jeffrey Kiah
	Orig.Date:	07/28/2007
	Rev.Date:	05/04/2009
---------------------------*/

#include "Event.h"

///// STATIC VARIABLES /////

#if defined(_DEBUG) || defined(DEBUG_CONSOLE)
int Event::sNumEventsCreated = 0;
int Event::sNumEventsDestroyed = 0;
#endif

