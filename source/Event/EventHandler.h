/*----==== EVENTHANDLER.H ====----
	Author:		Jeffrey Kiah
	Orig.Date:	07/28/2007
	Rev.Date:	05/04/2009
--------------------------------*/

#pragma once

#include "Event.h"
#include "../Utility/ConcurrentQueue.h"

typedef ConcurrentQueue<EventPtr>	ThreadSafeEventQueue;

/*=============================================================================
class IEventHandler
	This interface defines a basic functor for all event handlers to inherit
=============================================================================*/
class IEventHandler {
	public:
		/*---------------------------------------------------------------------
			return true to consume event and stop propagation to more listeners
		---------------------------------------------------------------------*/
		virtual bool operator()(const EventPtr &ePtr) = 0;

		explicit IEventHandler() {}
		virtual ~IEventHandler() {}
};

/*=============================================================================
class EventHandler
	This templated implementation of IEventHandler is for pointing to class
	member functions. It can be used for most listener class implementations.
	Doesn't handle static class functions or free functions.
=============================================================================*/
template <typename T>
class EventHandler : public IEventHandler {
	public:
		///// DEFINITIONS /////
		typedef bool	(T::*handlerProc)(const EventPtr &);

		///// FUNCTIONS /////
		// Operators
		virtual bool	operator()(const EventPtr &ePtr) { return (*mListenerPtr.*mProc)(ePtr); }	// return type could be enum

		// Constructor / destructor
		explicit EventHandler(T *listenerPtr, handlerProc proc) : mListenerPtr(listenerPtr), mProc(proc) {}

	private:
		///// VARIABLES /////
		T				*mListenerPtr;
		handlerProc		mProc;
};

/*=============================================================================
class EventHandlerStatic
	This templated implementation of IEventHandler is for pointing to static
	class functions.
=============================================================================*/
class EventHandlerStatic : public IEventHandler {
	public:
		typedef bool	(*handlerProc)(const EventPtr &);

		virtual bool	operator()(const EventPtr &ePtr) { return (*mProc)(ePtr); }

		explicit EventHandlerStatic(handlerProc proc) : mProc(proc) {}
		
	private:
		handlerProc		mProc;
};

/*=============================================================================
class ThreadEventHandler
	This class can be used to handle events in worker threads. It is made
	thread safe by queueing events in a thread safe queue as they are sent to
	the handler, instead of calling a functor to handle them. The handler
	should be created in the main thread, because registration with the manager
	is not thread safe. The thread process can waitPop() items from the queue
	to handle them.
=============================================================================*/
class ThreadEventHandler : public IEventHandler {
	public:
		ThreadSafeEventQueue	mEventQueue;

		virtual bool	operator()(const EventPtr &ePtr) {
			mEventQueue.push(ePtr);
			return false; // allow the event to propagate
		}

		explicit ThreadEventHandler() {}
};