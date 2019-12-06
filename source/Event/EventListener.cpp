/*----==== EVENTLISTENER.CPP ====----
	Author:		Jeffrey Kiah
	Orig.Date:	07/28/2007
	Rev.Date:	05/04/2009
-----------------------------------*/

#include "EventListener.h"
#include "EventManager.h"

////////// class EventListener //////////

const string EventListener::sWildcardType("*"); // defines the wildcard event type string for listeners

/*-----------------------------------------------------------------------------
	Inserts a handler functor for an event type, adding it to the
	EventHandlerMap. Returns true if insert succeeds.
-----------------------------------------------------------------------------*/
bool EventListener::insertEventHandler(const string &eventType, const IEventHandlerPtr &handler)
{
	EventHandlerMap::const_iterator ei = mHandlerMap.find(eventType);
	if (ei == mHandlerMap.end()) { // event type handler does not exist yet
		EventHandlerMapResult r = mHandlerMap.insert(EventHandlerMapValue(eventType, handler));
		debugPrintf("%s: handler created for event type \"%s\"\n", mName.c_str(), eventType.c_str());
		_ASSERTE(r.second == true);
	} else { // event type handler already exists
		debugPrintf("%s: handler for event type \"%s\" already exists, not added\n", mName.c_str(), eventType.c_str());
		return false;
	}
	return true;
}

/*-----------------------------------------------------------------------------
	Removes a handler functor for an event type from EventHandlerMap. Returns
	true if removal succeeds.
-----------------------------------------------------------------------------*/
bool EventListener::removeEventHandler(const string &eventType)
{
	// check for event type in the map
	EventHandlerMap::iterator ei = mHandlerMap.find(eventType);
	if (ei == mHandlerMap.end()) {
		debugPrintf("%s: handler not found for event type \"%s\", not removed\n", mName.c_str(), eventType.c_str());
		return false;
	}
	mHandlerMap.erase(ei);
	debugPrintf("%s: handler for event type \"%s\" removed\n", mName.c_str(), eventType.c_str());
	return true;
}

/*-----------------------------------------------------------------------------
	Calls insertEventHandler, and registers the listener with EventManager for
	the event type. Passes priority along to listener registration function
-----------------------------------------------------------------------------*/
bool EventListener::registerEventHandler(const string &eventType, const IEventHandlerPtr &handler, uint priority)
{
	// perform the insert, and if it fails, exit early
	if (!insertEventHandler(eventType, handler)) return false;
	// if the insert succeeds, register the handler with EventManager
	eventMgr.registerListener(eventType, this, priority); // register listener with manager
	return true;
}

/*-----------------------------------------------------------------------------
	Calls removeEventHandler, also unregisters the listener for that event type
	with the EventManager. Returns true if removal succeeds.
-----------------------------------------------------------------------------*/
bool EventListener::unregisterEventHandler(const string &eventType)
{
	// remove the handler, if the removal fails exit early
	if (!removeEventHandler(eventType)) return false;
	// if removal from map succeeds, unregister with EventManager
	eventMgr.removeListener(eventType, this); // remove listener from manager
	return true;
}

/*-----------------------------------------------------------------------------
	Cleanup for when listener is destroyed or being reset. Unregisters all
	remaining handlers in the map. If the derived listener hasn't explicitly
	unregistered them, this will catch it.
-----------------------------------------------------------------------------*/
void EventListener::clearHandlers()
{
	// this loop unregisters all remaining handlers
	EventHandlerMap::const_iterator ei = mHandlerMap.begin(),
									end = mHandlerMap.end();
	while (ei != end) {
		const string key(ei->first);
		++ei;
		unregisterEventHandler(key);
	}
	mHandlerMap.clear();
}

/*-----------------------------------------------------------------------------
	shared_ptr passes the event, so even if frame ends and list is purged,
	events with info that is needed longer are not actually deleted until the
	shared_ptr goes out of scope. Returns true if the event is consumed and
	propagation should be stopped.
-----------------------------------------------------------------------------*/
bool EventListener::handle(const EventPtr &ePtr)
{
	// handle specific event type registrations
	EventHandlerMap::const_iterator ei = mHandlerMap.find((*ePtr).type());
	if (ei == mHandlerMap.end()) {
		// if a specific event handler is not found for this type, check for any wildcard handlers
		// so in this case, specific handlers in a listener will override the wildcard for any event type
		ei = mHandlerMap.find(EventListener::sWildcardType);
		if (ei != mHandlerMap.end()) {
			(*ei->second)(ePtr); // ignore return value for wildcard event handlers
			return false;
		}
		// if this is being reached, probably because you called
		// removeEventHandler somwhere prior to listener destructor
		debugPrintf("%s: handler for event type \"%s\" not found\n", mName.c_str(), (*ePtr).type().c_str());
		return false; // allow continued propagation of the event
	}
	
	return (*ei->second)(ePtr);
}