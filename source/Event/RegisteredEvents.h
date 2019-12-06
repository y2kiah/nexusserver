/*----==== REGISTEREDEVENT.H ====----
	Author: Jeffrey Kiah
	Orig.Date: 05/07/2009
	Rev.Date:  05/07/2009
-----------------------------------*/

#pragma once

#include "../Event/EventManager.h"
#include "../Utility/Typedefs.h"

///// STRUCTURES /////

/*=============================================================================
class CodeOnlyEvent
=============================================================================*/
class CodeOnlyEvent : public RegisteredEvent {
	public:
		/*---------------------------------------------------------------------
			Should never even be called, scripting manager can check return of
			scriptAllowed() to check whether or not to call this
		---------------------------------------------------------------------*/
		virtual bool triggerEventFromSource(const string &eventType, const AnyVars &eventData) const {
			_ASSERTE(false && "Tried to trigger code-only event from script"); // in a debug build will halt execution
			// add message to release build logging
			return false; // in release mode, just don't do anything
		}
		virtual bool raiseEventFromSource(const string &eventType, const AnyVars &eventData) const {
			_ASSERTE(false && "Tried to raise code-only event from script");
			// add message to release build logging
			return false;
		}
		/*---------------------------------------------------------------------
			Construct this object as a code-only event
		---------------------------------------------------------------------*/
		explicit CodeOnlyEvent(const EventDataType dt) :
			RegisteredEvent(EventSource_CodeOnly, dt)
		{}
		virtual ~CodeOnlyEvent() {}
};

/*=============================================================================
class ScriptCallableCodeEvent
	This concrete registered event type is templated by any type of event that
	needs to be called from script. triggerEventFromScript is called by the
	script manager's trigger function (which is callable from script) and
	returns true on success.
=============================================================================*/
template <typename TEventType>
class ScriptCallableCodeEvent : public RegisteredEvent {
	public:
		/*---------------------------------------------------------------------
			Creates the event using the event's AnyVars constructor to pass in
			event data. This implies that all script callable code-defined
			event types must implement an AnyVars constructor. Then, calls
			EventManager::trigger.
		---------------------------------------------------------------------*/
		virtual bool triggerEventFromSource(const string &eventType, const AnyVars &eventData) const {
			if (isEmpty()) { // handle empty events as a special case, avoid calling the AnyVars constructor
				events.trigger(eventType);
				return true;
			} else { // for all non-empty events, call the AnyVars constructor
				EventPtr ePtr(new TEventType(eventData)); // use the event type's AnyVars constructor
				events.trigger(ePtr);
				return true;
			}
		}
		virtual bool raiseEventFromSource(const string &eventType, const AnyVars &eventData) const {
			if (isEmpty()) {
				events.raise(eventType);
				return true;
			} else {
				EventPtr ePtr(new TEventType(eventData));
				events.raise(ePtr);
				return true;
			}
		}

		/*---------------------------------------------------------------------
			Construct this object as a script callable code event
		---------------------------------------------------------------------*/
		explicit ScriptCallableCodeEvent(const EventDataType dt) :
			RegisteredEvent(EventSource_CodeAndScript, dt)
		{}
		virtual ~ScriptCallableCodeEvent() {}
};

/*=============================================================================
class ScriptDefinedEvent
	This concrete registered event type is defined in script, and will not have
	any listeners in code. It is not neccesary to translate data to and from
	native types to script objects, so this is basically a pass-through.
=============================================================================*/
class ScriptDefinedEvent : public RegisteredEvent {
	public:
		/*---------------------------------------------------------------------
			Creates a ScriptEvent, passing any data through the srcData
			parameter. Then, calls EventManager::trigger or raise.
		---------------------------------------------------------------------*/
		virtual bool triggerEventFromSource(const string &eventType, const AnyVars &eventData) const {
			EventPtr ePtr(new ScriptEvent(eventType, eventData));
			EventManager::instance().trigger(ePtr);
			return true;
		}
		virtual bool raiseEventFromSource(const string &eventType, const AnyVars &eventData) const {
			EventPtr ePtr(new ScriptEvent(eventType, eventData));
			EventManager::instance().raise(ePtr);
			return true;
		}

		/*---------------------------------------------------------------------
			Construct this object as a script-only event. Script-defined events
			are never considered empty, but they can choose to pass no data.
		---------------------------------------------------------------------*/
		explicit ScriptDefinedEvent() :
			RegisteredEvent(EventSource_ScriptOnly, EventDataType_NotEmpty)
		{}
		virtual ~ScriptDefinedEvent() {}
};

/*=============================================================================
class RemoteCallableEvent
	This concrete registered event type is raised by a remote client and passed
	to local listeners through a byte stream such as TCP or UDP networking,
	file IO, or other communication methods.
=============================================================================*/
template <typename TEventType, class Archive>
class RemoteCallableEvent : public RegisteredEvent {
	public:
		/*---------------------------------------------------------------------
			Creates the Event, deserializing from the byte stream. Then, calls
			EventManager::trigger or raise. eventData should contain two items,
			first in targetClientId, second pointer to the Archive.
		---------------------------------------------------------------------*/
		virtual bool triggerEventFromSource(const string &eventType, const AnyVars &eventData) const {
			if (isEmpty()) { // handle empty events as a special case, avoid calling deserialize
				eventMgr.trigger(eventType);
				return true;
			} else { // for all non-empty events, call deserialize after construction
				AnyVars::const_iterator i = eventData.begin();
				try {
					int targetClientId = any_cast<int>(i->second);
					EventPtr ePtr(new TEventType(targetClientId));
					TEventType &e = *(static_cast<TEventType*>(ePtr.get()));
					++i;
					//ostream &os = *(any_cast<ostream*>(i->second));
					//Archive archive(os);
					Archive &archive = *(any_cast<Archive*>(i->second));
					archive << e;
					eventMgr.trigger(ePtr);

				} catch (const boost::bad_any_cast &ex) {
					// nothing happens with a bad datatype in release build, silently ignores
					debugPrintf("RemoteCallableEvent: bad_any_cast \"%s\"\n", ex.what());
				}
				return true;
			}
		}
		virtual bool raiseEventFromSource(const string &eventType, const AnyVars &eventData) const {
			if (isEmpty()) {
				eventMgr.raise(eventType);
				return true;
			} else {
				AnyVars::const_iterator i = eventData.begin();
				try {
					int targetClientId = any_cast<int>(i->second);
					EventPtr ePtr(new TEventType(targetClientId));
					TEventType &e = *(static_cast<TEventType*>(ePtr.get()));
					++i;
					//ostream &os = *(any_cast<ostream*>(i->second));
					//Archive archive(os);
					Archive &archive = *(any_cast<Archive*>(i->second));
					archive << e;
					eventMgr.raise(ePtr);

				} catch (const boost::bad_any_cast &ex) {
					// nothing happens with a bad datatype in release build, silently ignores
					debugPrintf("RemoteCallableEvent: bad_any_cast \"%s\"\n", ex.what());
				}
				return true;
			}
		}
		/*---------------------------------------------------------------------
			Construct this object as a remote event
		---------------------------------------------------------------------*/
		explicit RemoteCallableEvent(const EventDataType dt) :
			RegisteredEvent(EventSource_Remote, dt)
		{}
		virtual ~RemoteCallableEvent() {}
};