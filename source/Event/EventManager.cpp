/*----==== EVENTMANAGER.CPP ====----
	Author:		Jeffrey Kiah
	Orig.Date:	07/23/2007
	Rev.Date:	05/22/2009
----------------------------------*/

#include "EventManager.h"
#include "../Win32/HighPerfTimer.h"
#include "../Utility/ConcurrentQueue.h"

////////// class EventManager //////////

/*-----------------------------------------------------------------------------
	Notifies listeners of a single event raised or triggered
-----------------------------------------------------------------------------*/
void EventManager::notifyListeners(const EventPtr &ePtr) const
{
	// this section is for listeners of the wildcard type EventListener::sWildcardType
	// if the handler returns true to consume, will not stop propagation here
	EventTypeMap::const_iterator ei = mEventTypeMap.find(EventListener::sWildcardType);
	if (ei != mEventTypeMap.end()) {
		ListenerList::const_iterator li, end = ei->second.end();
		for (li = ei->second.begin(); li != end; ++li) {
			(*li).first->handle(ePtr);
		}
	}

	// this section looks for listeners actually registered for the specific event
	// and will honor the return value of true for consumed events
	ei = mEventTypeMap.find((*ePtr).type());
	if (ei != mEventTypeMap.end()) {
		ListenerList::const_iterator li, end = ei->second.end();
		for (li = ei->second.begin(); li != end; ++li) {
			// if a handler returns true, it consumes the event and stops propagation
			bool consumed = (*li).first->handle(ePtr);
			if (consumed) {
				#ifdef _DEBUG
				ListenerList::const_iterator li_check = li;
				++li_check;
				if (li_check != end) {
					debugPrintf("EventMgr: Listener \"%s\" consumed event of type \"%s\", listeners skipped\n",
						(*li).first->name().c_str(), (*ePtr).type().c_str());
				}
				#endif
				break;
			}
		}
	}
	(*ePtr).mState = EventState_Handled;
}

/*-----------------------------------------------------------------------------
	Add event to the queue, queue is processed each frame
-----------------------------------------------------------------------------*/
void EventManager::raise(const EventPtr &ePtr)
{	// Take the pointer passed in and fill with info like time, class that raised event, etc.
	if (!isEventTypeRegistered((*ePtr).type())) {
		debugPrintf("EventMgr: cannot raise \"%s\" event, not registered\n", (*ePtr).type().c_str());
		return;
	}
	(*ePtr).mState = EventState_Raised;
	(*ePtr).mTime = HighPerfTimer::queryCounts();
	mEventQueue[mActiveQueue].push_back(ePtr);
	debugPrintf("EventMgr: \"%s\" event raised\n", (*ePtr).type().c_str());
}
/*-----------------------------------------------------------------------------
	Add a no-data event to the queue, queue is processed each frame.
	This cannot be used to trigger events where the registered type
	specifies "EventData_NotEmpty".
-----------------------------------------------------------------------------*/
void EventManager::raise(const string &eventType)
{
	RegEventMap::const_iterator ri = mRegEventMap.find(eventType);
	if (ri != mRegEventMap.end()) {
		_ASSERTE(ri->second->isEmpty() && "Cannot raise non-empty event with this interface, use EventPtr interface");
		if (ri->second->isEmpty()) {
			EventPtr ePtr(new EmptyEvent(eventType));
			(*ePtr).mState = EventState_Raised;
			(*ePtr).mTime = HighPerfTimer::queryCounts();
			mEventQueue[mActiveQueue].push_back(ePtr);
			debugPrintf("EventMgr: \"%s\" event raised\n", (*ePtr).type().c_str());
		} else {
			// add message to release logging
		}
	} else {
		debugPrintf("EventMgr: cannot raise \"%s\" event, not registered\n", eventType.c_str());
	}
}

/*-----------------------------------------------------------------------------
	Multithread safe raise methods
-----------------------------------------------------------------------------*/
void EventManager::raiseThreadSafe(const EventPtr &ePtr)
{
	if (!isEventTypeRegistered((*ePtr).type())) {
		debugPrintf("EventMgr: cannot raise \"%s\" event, not registered\n", (*ePtr).type().c_str());
		return;
	}
	(*ePtr).mState = EventState_Raised;
	(*ePtr).mTime = HighPerfTimer::queryCounts();
	mThreadEventQueue->push(ePtr);
	debugPrintf("EventMgr: thread safe \"%s\" event raised\n", (*ePtr).type().c_str());
}

void EventManager::raiseThreadSafe(const string &eventType)
{
	RegEventMap::const_iterator ri = mRegEventMap.find(eventType);
	if (ri != mRegEventMap.end()) {
		_ASSERTE(ri->second->isEmpty() && "Cannot raise non-empty event with this interface, use EventPtr interface");
		if (ri->second->isEmpty()) {
			EventPtr ePtr(new EmptyEvent(eventType));
			(*ePtr).mState = EventState_Raised;
			(*ePtr).mTime = HighPerfTimer::queryCounts();
			mThreadEventQueue->push(ePtr);
			debugPrintf("EventMgr: thread safe \"%s\" event raised\n", (*ePtr).type().c_str());
		} else {
			// add message to release logging
		}
	} else {
		debugPrintf("EventMgr: cannot raise \"%s\" event, not registered\n", eventType.c_str());
	}
}

/*-----------------------------------------------------------------------------
	Invoke listeners immediately, does not queue the event
-----------------------------------------------------------------------------*/
void EventManager::trigger(const EventPtr &ePtr)
{
	if (!isEventTypeRegistered((*ePtr).type())) {
		debugPrintf("EventMgr: cannot trigger \"%s\" event, not registered\n", (*ePtr).type().c_str());
		return;
	}
	debugPrintf("EventMgr: \"%s\" event triggered\n", (*ePtr).type().c_str());
	(*ePtr).mState = EventState_Triggered;
	(*ePtr).mTime = HighPerfTimer::queryCounts();
	notifyListeners(ePtr);
}
/*-----------------------------------------------------------------------------
	Invoke listeners immediately, does not queue the no-data event.
	This cannot be used to trigger events where the registered type
	specifies "EventData_NotEmpty".
-----------------------------------------------------------------------------*/
void EventManager::trigger(const string &eventType)
{
	RegEventMap::const_iterator ri = mRegEventMap.find(eventType);
	if (ri != mRegEventMap.end()) {
		_ASSERTE(ri->second->isEmpty() && "Cannot trigger non-empty event with this interface, use EventPtr interface");
		if (ri->second->isEmpty()) {
			EventPtr ePtr(new EmptyEvent(eventType));
			debugPrintf("EventMgr: \"%s\" event triggered\n", eventType.c_str());
			(*ePtr).mState = EventState_Triggered;
			(*ePtr).mTime = HighPerfTimer::queryCounts();
			notifyListeners(ePtr);
		} else {
			// add message to release logging
		}
	} else {
		debugPrintf("EventMgr: cannot trigger \"%s\" event, not registered\n", eventType.c_str());
	}
}

/*-----------------------------------------------------------------------------
	Run through the queue and notify listeners, then purge the queue
	If maxMillis > 0, the loop will exit after time expires, and remaining
	events will be processed next frame
-----------------------------------------------------------------------------*/
void EventManager::notifyQueued(ulong maxMillis)
{
	// This section handles all events pushed into the thread-safe queue. These are processed first
	// to make sure we maximize concurrency, but it opens up the possibility of a thread spamming
	// the event system, where events are added faster than they can be processed, causing the
	// program stutter or hang. Can't do much about this case except design worker threads carefully
	// to not send events too often.
	EventPtr ePtr;
	while (mThreadEventQueue->tryPop(ePtr)) {
		notifyListeners(ePtr);
	}

	// Now work on the regular event queue
	// flip the active queue so any events raised while processing the current queue will not set up an endless loop
	uint processQueue = mActiveQueue;
	mActiveQueue = (mActiveQueue == 0) ? 1 : 0;
	clearEventQueue(mActiveQueue); // make sure new active queue starts empty, but should already be empty

	// run through the now inactive queue and notify listeners to handle each event
	// may not reach end of queue if time expires
	HighPerfTimer timer;
	timer.start();
	EventQueue::const_iterator	ei = mEventQueue[processQueue].begin(),
								end = mEventQueue[processQueue].end();
	int temp = 0;
	while (ei != end) {
		notifyListeners(*ei);
		++temp;
		++ei;
		mEventQueue[processQueue].pop_front();
		// if maxMillis is exceeded, time to break out of the loop
		// timer.stop() is an expensive call, consider calling this conditional once per 10 events or something
		if (timer.stop() > maxMillis && maxMillis != 0) break;
	}

	// if there are remaining events in the queue, push them to front of active queue so they'll be processed first next frame
	// clears the inactive queue if not already empty
	if (mEventQueue[processQueue].size() > 0) {
		debugPrintf("%i queued events rolled over\n", mEventQueue[processQueue].size());
		do {
			mEventQueue[mActiveQueue].push_front(mEventQueue[processQueue].back());
			mEventQueue[processQueue].pop_back();
		} while (mEventQueue[processQueue].size() > 0);
	}
}

/*-----------------------------------------------------------------------------
	Registers an event type so that it may be triggered or raised. The
	RegisteredEvent implementation will determine if the event can be triggered
	from script or just code. Returns true if registration is successful.
-----------------------------------------------------------------------------*/
bool EventManager::registerEventType(const string &eventType, const RegEventPtr &regPtr)
{
	RegEventMap::iterator ri = mRegEventMap.find(eventType);
	if (ri == mRegEventMap.end()) { // not yet registered, good
		RegEventMapResult r = mRegEventMap.insert(RegEventMapValue(eventType, regPtr));
		if (r.second) {
			debugPrintf("EventMgr: event type \"%s\" registered\n", eventType.c_str());
		} else {
			debugPrintf("EventMgr: event type \"%s\" failed to register!\n", eventType.c_str());
		}
		return r.second;
	}
	// event type already registered, kick back
	debugPrintf("EventMgr: failed to register event type \"%s\", already exists\n", eventType.c_str());
	return false;
}

/*-----------------------------------------------------------------------------
	Returns true if added, false if already exists, with priority (1 is highest
	priority, 0 is no priority or FIFO order). If event type does not exist it
	is added. An O(n) operation since it traverses the existing list for
	duplicates.
-----------------------------------------------------------------------------*/
bool EventManager::registerListener(const string &eventType, EventListener *lPtr, uint priority)
{
	_ASSERTE(lPtr);

	EventTypeMap::iterator ei = mEventTypeMap.find(eventType);
	if (ei == mEventTypeMap.end()) {	// event type does not exist yet, so add it and register the listener
		EventTypeMapResult r = mEventTypeMap.insert(EventTypeMapValue(eventType, ListenerList()));
		debugPrintf("EventMgr: event type \"%s\" created in listener map\n", eventType.c_str());
		(*r.first).second.push_back(ListenerListValue(lPtr,priority));
		debugPrintf("EventMgr: listener \"%s\" for event type \"%s\" registered with priority %d\n", lPtr->name().c_str(), eventType.c_str(), priority);
		_ASSERTE(r.second == true);

	} else { // event type exists, check that listener doesn't already exist
		ListenerList::iterator	li, lPriorityInsert = (*ei).second.begin(),
								end = (*ei).second.end();
		for (li = (*ei).second.begin(); li != end; ++li) {
			// 1) check the whole list to see if it's a repeat
			if ((*li).first == lPtr) {
				debugPrintf("EventMgr: listener \"%s\" for event type \"%s\" already exists, not registered\n", lPtr->name().c_str(), eventType.c_str());
				return false;
			}
			// 2) find where it would be inserted if based on priority
			if (((priority >= (*li).second) && ((*li).second != 0)) || (priority == 0)) ++lPriorityInsert;
		}
		if (priority == 0) { // just add to back of list for FIFO order
			(*ei).second.push_back(ListenerListValue(lPtr,0));
			debugPrintf("EventMgr: listener \"%s\" for event type \"%s\" registered with priority %d\n", lPtr->name().c_str(), eventType.c_str(), priority);
		} else { // insert in priority order
			(*ei).second.insert(lPriorityInsert, ListenerListValue(lPtr,priority));
			debugPrintf("EventMgr: listener \"%s\" for event type \"%s\" registered with priority %d\n", lPtr->name().c_str(), eventType.c_str(), priority);
		}
	}

	return true;
}
		
/*-----------------------------------------------------------------------------
	Removes a listener from an event type, if event type has no more listeners
	it is removed
-----------------------------------------------------------------------------*/
bool EventManager::removeListener(const string &eventType, EventListener *lPtr)
{
	_ASSERTE(lPtr);

	// check for event type in the map
	EventTypeMap::iterator ei = mEventTypeMap.find(eventType);
	if (ei == mEventTypeMap.end()) {
		debugPrintf("EventMgr: event type \"%s\" not found, listener \"%s\" not removed\n", eventType.c_str(), lPtr->name().c_str());
		return false;
	}

	size_t startSize = (*ei).second.size();	// store size of the list before attempting a removal

	// remove the matching listener in the event type's list
	ListenerList::iterator li, end = (*ei).second.end();
	for (li = (*ei).second.begin(); li != end; ++li) {
		if ((*li).first == lPtr) {	// match
			(*ei).second.erase(li);
			debugPrintf("EventMgr: listener \"%s\" for event type \"%s\" removed\n", lPtr->name().c_str(), eventType.c_str());
			break;
		}
	}
	if ((*ei).second.size() == startSize) {
		debugPrintf("EventMgr: listener \"%s\" for event type \"%s\" not found, not removed\n", lPtr->name().c_str(), eventType.c_str());
		return false;	// listener not found for removal in the list
	}

	// if the event type has no more listeners, it can be removed from the map
	if ((*ei).second.size() == 0) {
		mEventTypeMap.erase(ei);
		debugPrintf("EventMgr: event type \"%s\" removed from listener map, no more listeners\n", eventType.c_str());
	}
	
	return true;	
}

EventManager::EventManager() :
	Singleton<EventManager>(*this),
	mActiveQueue(0),
	mThreadEventQueue(new ThreadSafeEventQueue()),
	mEventSnooper(new EventSnooper())
{
}

EventManager::~EventManager()
{
	clearEventQueue(0);
	clearEventQueue(1);
	delete mEventSnooper;
	delete mThreadEventQueue;
	clearListeners();
	debugPrintf("EventMgr: created %d events, destroyed %d\n", Event::sNumEventsCreated, Event::sNumEventsDestroyed);
}

////////// class EventSnooper //////////

bool EventSnooper::handleAllEvents(const EventPtr &ePtr)
{
	debugPrintf("%s: event \"%s\" handled by generic handler\n", mName.c_str(), (*ePtr).type().c_str());
	return false;
}

/*bool EventSnooper::handleSpecificEvent(const EventPtr &ePtr)
{
	debugPrintf("%s: event \"%s\" handled by specific handler\n", mName.c_str(), (*ePtr).type().c_str());
	return false;
}*/

EventSnooper::EventSnooper() :
	EventListener("EventSnooper")
{
	// here we don't register the specific handler because doing so would cause a duplicate
	// event notification to be sent to this listener for the same event, one for the generic
	// handler and one for the specific. Instead, just insert the specific handler to the map
	// and it will be used in lieu of the generic handler.
/*	IEventHandlerPtr p1(new EventHandler<EventSnooper>(this, &EventSnooper::handleSpecificEvent));
	insertEventHandler("EVENT_SPECIFIC", p1);
*/
	// register the wildcard event with EventManager
	IEventHandlerPtr p(new EventHandler<EventSnooper>(this, &EventSnooper::handleAllEvents));
	registerEventHandler(EventListener::sWildcardType, p);
}

EventSnooper::~EventSnooper()
{
	// handlers are unregistered automatically, but this one is a specific-override, so
	// to avoid a debug message from EventManager when it tries to unregister, we'll just
	// remove it manually
//	removeEventHandler("EVENT_SPECIFIC");
}