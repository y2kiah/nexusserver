/*----==== EVENT.H ====----
	Author: Jeffrey Kiah
	Orig.Date: 07/28/2007
	Rev.Date:  11/17/2010
-------------------------*/

#pragma once

#include <string>
#include <list>
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>
#include <iostream>
#include "../Utility/Typedefs.h"

using std::string;	using std::shared_ptr;
using std::ostream;	using std::istream;
using std::list;	using std::pair;
using boost::any;	using boost::any_cast;

///// DEFINITIONS /////

enum EventState : uchar {
	EventState_New = 0,
	EventState_Raised,
	EventState_Triggered,
	EventState_Handled
};

enum EventSource : uchar {
	EventSource_CodeOnly = 0,	// allow the event to be raised/triggered from code only
	EventSource_ScriptOnly,		// allow the event to be raised/triggered from script only
	EventSource_CodeAndScript,	// allow the event to be raised/triggered from code or script
	EventSource_Remote			// allow the event to be raised/triggered from remote client
};

/*=============================================================================
	Empty events can be triggered by string shortcut (instead of defining an
	explicit class for each type. Also is treated as a special case in the
	RegisteredEvent triggerEventFromScript method. NotEmpty events must be
	defined explicitly and passed via shared_ptr to raise and trigger.
=============================================================================*/
enum EventDataType : uchar {
	EventDataType_Empty = 0,	// determines that EventManager's raise and trigger by string can be used
	EventDataType_NotEmpty		// determines that EventManager's raise and trigger by string cannot be used
};

class Event;
class RegisteredEvent;
typedef shared_ptr<Event>	EventPtr;
typedef shared_ptr<RegisteredEvent>	RegEventPtr;
typedef pair<string, any>	AnyVarsValue;	// key/value pair where string is key and value utilizes boost::any
typedef list<AnyVarsValue>	AnyVars;		// list of key/value pairs. This does not provide constant time
											// random access to elements, so lists should generally be short
///// STRUCTURES /////

/*=============================================================================
class Event
	An abstract base class for all Event types to inherit from. There are pure
	virtual methods to be overloaded. type() should usually return a const
	reference to a static string variable with the event type name. mTime and
	mState are private because derived classes should not try to manage those
	attributes since EventManager does that job.
=============================================================================*/
class Event : private boost::noncopyable {
	friend class EventManager;	// allow EventManager to reach private and protected members

	private:
		// Don't allow derived classes to modify time and state, we want the EventManager to have control
		__int64		mTime;		// time (in counts) that the event was created
		EventState	mState;		// stores new, triggered, raised, and handled - use to query invokation method

	protected:
		#if defined(_DEBUG) || defined(DEBUG_CONSOLE) // track event instantiations to ensure no memory leaks
		static int	sNumEventsCreated;
		static int	sNumEventsDestroyed;
		#endif

	public:
		virtual const string &	type() const = 0;
		__int64					time() const	{ return mTime; }
		EventState				state() const	{ return mState; }

		// Constructor
		explicit Event() :
			mTime(0),				// Don't record at instantiation, EventManager records when queued or triggered.
			mState(EventState_New)	// EventManager records when queued or triggered.
		{
			#if defined(_DEBUG) || defined(DEBUG_CONSOLE)
			++sNumEventsCreated;
			#endif
		}

		// Destructor
		virtual ~Event() {
			#if defined(_DEBUG) || defined(DEBUG_CONSOLE)
			++sNumEventsDestroyed;
			#endif
		}
};

/*=============================================================================
class ScriptableEvent
	This is the abstract base class for any event with a registered event type
	of ScriptCallableCodeEvent. If you don't intend for your event to be fired
	from script, just inherit from Event instead. This adds two new attributes,
	some public interfaces to deal with the new attributes, a new virtual
	function to be overloaded, and requires a new constructor signature to be
	implemented in the derived class (const AnyVars &).
	**NOTE**
	Event handlers for ScriptableEvents should treat any custom event data as
	immutable, since serialization to/from script data only occurs once in an
	event's lifespan. If a handler were to alter the data, any remaining
	handlers on the "other side" of the script/code bridge would not see those
	changes reflected. All handlers should be given the opportunity to see the
	event's original data. Also note that this limitation does not apply to
	script-only or code-only events, where data translation is not neccesary.
=============================================================================*/
class ScriptableEvent : public Event {
	protected:
		AnyVars	mScriptData;		// adds a private var to store event data for script handlers
		bool	mScriptDataBuilt;	// tracks whether the event data has been built (only built when
									// fired from script, or if it actually has a script handler)
	public:
		/*---------------------------------------------------------------------
			Check this prior to calling buildScriptData when handling a script
			event so you only incur the overhead of building once for any event
		---------------------------------------------------------------------*/
		bool			isScriptDataBuilt() const { return mScriptDataBuilt; }
		/*---------------------------------------------------------------------
			Get access to the script data
		---------------------------------------------------------------------*/
		const AnyVars &	getScriptData() const { return mScriptData; }

		/*---------------------------------------------------------------------
			Scriptable events should overload this to build the mEventData list
			from its native property values. It should also set mEventDataBuilt
			to true before returning.
		---------------------------------------------------------------------*/
		virtual void	buildScriptData() = 0;

		/*---------------------------------------------------------------------
			This constructor would be called on the code side, does not set
			event data (initializes empty list).
		---------------------------------------------------------------------*/
		explicit ScriptableEvent() :
			Event(),
			mScriptData(),
			mScriptDataBuilt(false)
		{}
		/*---------------------------------------------------------------------
			This constructor would be called by the script manager for an event
			fired from script. Derived classes should implement a constructor
			with the same signature and convert AnyVar values to native object
			properties. The constructor would call this to init the the base
			class.
		---------------------------------------------------------------------*/
		explicit ScriptableEvent(const AnyVars &eventData) :
			Event(),
			mScriptData(eventData),	// nice, the std::list, std::pair and boost::any are all copy constructible
			mScriptDataBuilt(true)
		{}
		virtual ~ScriptableEvent() {}
};

/*=============================================================================
class ScriptEvent
	ScriptEvent is registered in script, so we know it will only have script
	handlers. For that reason, any data passed in the event doesn't need to be
	translated to and from native C++ types, basically pass whatever data
	object as the only occupant of the AnyVars list, and it will just be passed
	right back untouched. This acts as a simple pass-through class.
=============================================================================*/
class ScriptEvent : public ScriptableEvent {
	friend class ScriptDefinedEvent;
	private:
		string	mEventType;

		/*---------------------------------------------------------------------
			To prevent programmers from inheriting this by mistake (instead of
			ScriptableEvent) made constructor private and friended
			class ScriptDefinedEvent where this is called.
		---------------------------------------------------------------------*/
		explicit ScriptEvent(const string &eventType, const AnyVars &eventData) :
			ScriptableEvent(eventData),
			mEventType(eventType)
		{}
	public:
		virtual const string & type() const { return mEventType; }

		/*---------------------------------------------------------------------
			Since this is a pass-through object, the event data will always be
			built in the constructor (notice there is no default constructor),
			so there is never a need to call this method.
		---------------------------------------------------------------------*/
		virtual void	buildScriptData() { // a debug build will catch this method being called
							_ASSERTE(false && "Should never call this method for script-only events!");
						}

		virtual ~ScriptEvent() {}
};
			
/*=============================================================================
class EmptyEvent
	At the cost of carrying the event type string as a member var (rather than
	a static class var) it saves us from having to derive a new class for every
	empty event type defined in code, avoids writing a lot of duplicate code.
	**WARNING**
	You need to be careful when using this class, it would be possible to
	hijack an explicitly defined event type's string and trigger the event,
	resulting in a type mismatch when the handler casts from Event* to the
	derived class. There will be no validation of event type in the listener,
	so it's generally UNSAFE to use this class. That being said, forcing event
	type registration before triggering would solve this issue, so long as the
	programmer remembers to always register new event types.
=============================================================================*/
class EmptyEvent : public Event {
	friend class EventManager;
	private:
		string	mEventType;

		// Constructors
		explicit EmptyEvent(const string &eventType) :	// We don't want empty events being created anywhere, so to avoid the
			Event(),									// unsafe practice of constructing these manually, it is made private.
			mEventType(eventType)						// Friending EventManager lets it create these from raise and trigger
		{}												// by string methods.
	public:
		virtual const string & type() const { return mEventType; }
		
		/*---------------------------------------------------------------------
			This contructor is only made public so the program will compile
			without having to declare every ScriptCallableCodeEvent<T> class
			as a friend. Assert on debug build ensures it is not called. but in
			a release build it's left unchecked. DO NOT USE!!
			**Note**
			Empty events can be created from script, but are handled as a
			special case to avoid overhead, so no need for this constructor to
			ever be called (just needs to be here to compile). If I had derived
			from ScriptableEvent, it would carry an unneeded AnyVars attribute.
		---------------------------------------------------------------------*/
		explicit EmptyEvent(const AnyVars &) {
			_ASSERTE(false && "Shouldn't be calling EmptyEvent(const AnyVars &) constructor!");
		}
		// Destructor
		virtual ~EmptyEvent() {}
};

/*=============================================================================
class RegisteredEvent
	This abstract base class is inherited by all RegisteredEvent types. Every
	event type must be registered with the manager before it can be triggered
	or raised. They are registered using one of the derived types.
=============================================================================*/
class RegisteredEvent {
	private:
		const EventSource		mEventSource;
		const EventDataType		mEventDataType;

	public:
		/*---------------------------------------------------------------------
			Called from managers for events triggered/raised remotely
		---------------------------------------------------------------------*/
		virtual	bool triggerEventFromSource(const string &eventType, const AnyVars &eventData) const = 0;
		virtual	bool raiseEventFromSource(const string &eventType, const AnyVars &eventData) const = 0;
		/*---------------------------------------------------------------------
			This is basically RTTI for this class hierarchy
		---------------------------------------------------------------------*/
		const EventSource	getEventSource() const	{ return mEventSource; }
		/*---------------------------------------------------------------------
			Returns data type value
		---------------------------------------------------------------------*/
		const EventDataType	getEventDataType() const { return mEventDataType; }
		/*---------------------------------------------------------------------
			returns true if script is allowed to trigger this event type
		---------------------------------------------------------------------*/
		bool scriptAllowed() const
		{
			return (mEventSource == EventSource_ScriptOnly ||
					mEventSource == EventSource_CodeAndScript);
		}
		/*---------------------------------------------------------------------
			returns true if event type can be triggered remotely
		---------------------------------------------------------------------*/
		bool remoteAllowed() const {
			return (mEventSource == EventSource_Remote);
		}
		/*---------------------------------------------------------------------
			returns true if event type is EmptyEvent
		---------------------------------------------------------------------*/
		bool isEmpty() const			{ return (mEventDataType == EventDataType_Empty); }

		// Constructor / destructor
		explicit RegisteredEvent(const EventSource src, const EventDataType dt) :
			mEventSource(src),
			mEventDataType(dt)
		{}
		virtual ~RegisteredEvent() {}
};