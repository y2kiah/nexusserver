/*----==== EVENTLISTENER.H ====----
	Author:		Jeffrey Kiah
	Orig.Date:	07/28/2007
	Rev.Date	05/04/2009
---------------------------------*/

#pragma once;

#include <hash_map>
#include <string>
#include <memory>
#include "EventHandler.h"

using stdext::hash_map;
using std::string;
using std::pair;
using std::shared_ptr;

/*=============================================================================
class EventListener
	EventListener is registered with EventManager to receive notifications of
	its registered event types. A single listener can be used to handle many
	different event types, and each type can have only one handler function
	registered in the listener class. Handlers are stored as functors to
	member, static, or free functions.
	**Wildcard Handlers**
	A listener may register a wildcard handler to receive all event types to a
	single generic function. Specific handlers can be defined in this case to
	override the generic for those specific event types, while all others are
	picked up by the generic handler. To override, use the insertEventHandler
	and removeEventHandler functions in the constructor/destructor instead of
	registerEventHandler and unregisterEventHandler.
=============================================================================*/
class EventListener {
	public:
		///// DEFINITIONS /////
		typedef shared_ptr<IEventHandler>				IEventHandlerPtr;
		typedef pair<string, IEventHandlerPtr>			EventHandlerMapValue;
		typedef hash_map<string, IEventHandlerPtr>		EventHandlerMap;
		typedef pair<EventHandlerMap::iterator, bool>	EventHandlerMapResult;

		static const string	sWildcardType;	// stores the wildcard event type string
		
	protected:
		///// VARIABLES /////
		string				mName;			// name of the listener, mostly for debugging
		EventHandlerMap		mHandlerMap;	// map of functors, one for each event type that is listened for
											// the listener registers event types and functors with itself which
		///// FUNCTIONS /////				// also registers the listener with the event manager for that event type

		/*---------------------------------------------------------------------
			Inserts a handler functor for an event type, adding it to the
			EventHandlerMap. Returns true if insert succeeds.
		---------------------------------------------------------------------*/
		bool	insertEventHandler(const string &eventType, const IEventHandlerPtr &handler);

		/*---------------------------------------------------------------------
			Removes a handler functor for an event type from EventHandlerMap.
			Returns true if removal succeeds.
		---------------------------------------------------------------------*/
		bool	removeEventHandler(const string &eventType);

		/*---------------------------------------------------------------------
			Calls insertEventHandler, and registers the listener with
			EventManager for the event type. Passes priority along to listener
			registration function
		---------------------------------------------------------------------*/
		bool	registerEventHandler(const string &eventType, const IEventHandlerPtr &handler, uint priority);
		
		/*---------------------------------------------------------------------
			Register a handler with listener priority 0 (FIFO)
		---------------------------------------------------------------------*/
		bool	registerEventHandler(const string &eventType, const IEventHandlerPtr &handler) {
					return registerEventHandler(eventType, handler, 0);
				}

		/*---------------------------------------------------------------------
			Calls removeEventHandler, also unregisters the listener for that
			event type with the EventManager. Returns true if removal succeeds.
		---------------------------------------------------------------------*/
		bool	unregisterEventHandler(const string &eventType);

		/*---------------------------------------------------------------------
			Cleanup for when listener is destroyed or being reset. Unregisters
			all remaining handlers in the map. If the derived listener hasn't
			explicitly unregistered them, this will catch it.
		---------------------------------------------------------------------*/
		void	clearHandlers();

	public:
		/*---------------------------------------------------------------------
			Used to identify the listener in debug logs
		---------------------------------------------------------------------*/
		const string & name() const { return mName; }

		/*---------------------------------------------------------------------
			shared_ptr passes the event, so even if frame ends and list is
			purged, events with data that is needed longer are not actually
			deleted until the shared_ptr goes out of scope. Returns true if the
			event is consumed and propagation should be stopped.
		---------------------------------------------------------------------*/
		bool	handle(const EventPtr &ePtr);	// return type could be enum

		// Constructor / destructor
		explicit EventListener(const string &listenerName) : mName(listenerName) {}
		virtual ~EventListener() { clearHandlers(); }
};