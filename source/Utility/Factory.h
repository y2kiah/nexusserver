/* ----==== FACTORY.H ====---- */

#pragma once

#include <crtdbg.h>
#include <map>
#include "Singleton.h"


///// STRUCTURES /////

/*=============================================================================
	class  Factory

=============================================================================*/
template <class T>
class Factory : public Singleton<Factory<T>> {
	protected:
		///// VARIABLES /////
		
		std::map<unsigned int, T*>		resourceList;		// list of created, indexed by resID		
		std::map<unsigned int, int>		resLockCount;		// locks placed on resource

		///// FUNCTIONS /////		

		

	public:
				
		inline T &				getResource(unsigned int resID) const;

		// Resource management
		unsigned int	createResource(T* resPtrOut);

		// Constructor / Destructor
		explicit Factory();
		~Factory();
};


///// INLINE FUNCTIONS /////

template <class T>
inline T &Factory<T>::getResource(unsigned int resID) const
{
	_ASSERTE(resourceList.find(resID) != resourceList.end());
	
	return *resourceList[resID];
}