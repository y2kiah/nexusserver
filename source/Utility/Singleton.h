/*----==== SINGLETON.H ====----
	Author:		Jeffrey Kiah
	Orig.Date:	01/2004
	Rev.Date:	05/04/2009
-----------------------------*/

#pragma once

#include <crtdbg.h>

// Disable the warning regarding 'this' pointers being used in 
// base member initializer list. Singletons rely on this action
#pragma warning(disable : 4355)

/*=============================================================================
class Singleton
	The Singleton is a type of which only one instance can exist. It is ideal
	for system resource managers, texture managers, or any class meant for
	global access. Along with guaranteeing only one instance, it provides
	global access to all other objects through static member functions. A class
	need only inherit from the Singleton class and construct the Singleton in
	its constructor with (*this) passed as the argument.
=============================================================================*/
template <class T>
class Singleton {
	private:
		static T * inst;

		// Disable copying
		Singleton(const Singleton &);
		Singleton &operator=(const Singleton &);

	protected:
		// Constructors / Destructor
		// Protected because Singleton not intended for Polymorphism
		explicit Singleton(T & object) {			
			_ASSERTE(!inst);	// Break if an object of this class is already instantiated
			inst = &object;
		}

		~Singleton() {
			_ASSERTE(inst);		// Break if object does not exist
			inst = 0;
		}

	public:
		// Accessors
		static T & instance() {
			_ASSERTE(inst);		// Break if object does not exist
			return (*inst);
		}

		static bool exists() {
			return (inst != 0);
		}
};

///// STATIC MEMBERS /////

template <class T> T * Singleton<T>::inst = 0;
