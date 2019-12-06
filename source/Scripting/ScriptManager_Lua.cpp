/*----==== SCRIPTMANAGER_LUA.CPP ====----
	Author:		Jeffrey Kiah
	Orig.Date:	05/05/2009
	Rev.Date:	05/13/2009
---------------------------------------*/

#include "ScriptManager_Lua.h"
#include "ScriptEvents_Lua.h"
#include "../Utility/Typedefs.h"
#include "../Event/EventManager.h"
#include "../Event/RegisteredEvents.h"
#include "../Utility/FastMath.h"

using namespace LuaPlus;
using boost::any_cast;

////////// class LuaFunctionEvent //////////

///// STATIC VARIABLES /////
const string LuaFunctionEvent::sEventType("SYS_SCRIPT_CALL_FUNCTION");

////////// class ScriptManager_Lua //////////

///// FUNCTIONS /////

/*---------------------------------------------------------------------
	Debug print function (callable from script)
---------------------------------------------------------------------*/
void ScriptManager_Lua::debugPrint(LuaObject debugObject)
{
	const char *debugStr = debugObject.ToString();
	debugPrintf((debugStr == NULL) ? "Lua: Invalid debug string!\n" : debugStr);
}

#ifdef _DEBUG
/*---------------------------------------------------------------------
	Debug function for determining an object's type
---------------------------------------------------------------------*/
void ScriptManager_Lua::getLuaObjectType(LuaObject &obj)
{
	_ASSERTE(!obj.IsNil() && "Nil");
	_ASSERTE(!obj.IsBoolean() && "Boolean");
	_ASSERTE(!obj.IsCFunction() && "C-Function");
	_ASSERTE(!obj.IsFunction() && "Function");
	_ASSERTE(!obj.IsInteger() && "Integer");
	_ASSERTE(!obj.IsLightUserData() && "Light User Data");
	_ASSERTE(!obj.IsNone() && "None");
	_ASSERTE(!obj.IsNumber() && "Number");
	_ASSERTE(!obj.IsString() && "String");
	_ASSERTE(!obj.IsTable() && "Table");
	_ASSERTE(!obj.IsUserData() && "User Data");
//	_ASSERTE(!obj.IsWString() && "Wide String");
	_ASSERTE(0 && "Unknown");
}
#endif

/*---------------------------------------------------------------------
	Callable from script, calls inherited registerEventHandler with
	a LuaEventHandler to create a script event handler.
---------------------------------------------------------------------*/
bool ScriptManager_Lua::registerScriptHandler(const char *eventType, LuaObject luaFunc, uint priority)
{
	_ASSERTE(luaFunc.IsFunction() && "Lua handler is not a function"); // debug error checking

	EventHandlerMap::const_iterator i = mHandlerMap.find(eventType);
	if (i == mHandlerMap.end()) {
		// if handler does not already exist, create new handler and register
		IEventHandlerPtr p(new LuaEventHandler(luaFunc, priority));
		debugPrintf("Lua: script function registered for \"%s\" event with priority %u\n", eventType, priority);
		// The priority passed in determines the calling order relative to other script handlers.
		// Notice that the listener is ALWAYS registered in FIFO order, so any code-side handlers
		// that specify a priority will always come first. This will protect high priority events
		// from being hijacked and consumed by script before code handlers see them. Does not
		// protect FIFO (priority=0) code handlers in the same way.
		return registerEventHandler(eventType, p, 0);
	}
	// else with the existing handler, add new function to list in priority order
	LuaEventHandler &e = *(static_cast<LuaEventHandler*>(i->second.get()));
	bool retVal = e.addLuaFunction(luaFunc, priority);
	if (retVal) {
		debugPrintf("Lua: script function registered for \"%s\" event with priority %u\n", eventType, priority);
	} else {
		debugPrintf("Lua: could not register function for \"%s\" event, already exists\n", eventType);
	}
	return retVal;
}

/*---------------------------------------------------------------------
	Callable from script, calls inherited unregisterEventHandler.
---------------------------------------------------------------------*/
bool ScriptManager_Lua::unregisterScriptHandler(const char *eventType, LuaObject luaFunc)
{
	_ASSERTE(luaFunc.IsFunction() && "Lua object is not a function");

	// find the handler for this event type
	EventHandlerMap::const_iterator i = mHandlerMap.find(eventType);
	if (i != mHandlerMap.end()) {
		LuaEventHandler &e = *(static_cast<LuaEventHandler*>(i->second.get()));
		bool retVal = e.removeLuaFunction(luaFunc);	// tries to remove the function from the handler's list
		if (retVal) {								// if it succeeds, check if the list is now empty
			debugPrintf("Lua: script function removed for \"%s\" event\n", eventType);
			if (e.isFuncListEmpty()) {				// and if so, unregister the listener
				unregisterEventHandler(eventType);
			}
			return true;
		} else {
			debugPrintf("Lua: could not find function to unregister for \"%s\" event\n", eventType);
			return false;
		}
	}
	debugPrintf("Lua: could not find \"%s\" event to unregister\n", eventType);
	return false;
}

/*---------------------------------------------------------------------
	Handler for the LuaFunctionEvent event for code to call functions
	defined by script. Event data is updated with any return value.
---------------------------------------------------------------------*/
bool ScriptManager_Lua::handleLuaFunctionEvent(const EventPtr &ePtr)
{
	LuaFunctionEvent &e = *(static_cast<LuaFunctionEvent*>(ePtr.get()));
	// find the lua state by id
	auto i = mStateMap.find(e.stateId());
	if (i != mStateMap.end()) {
		// if the object is a Lua function, call it
		LuaObject luaObj = i->second->mState->GetGlobal(e.funcName().c_str());
		if (luaObj.IsFunction()) {
			LuaFunction<LuaObject> luaFunc(luaObj);
			e.setReturnObj(luaFunc(e.paramTable())); // calls the function passing paramTable and sets return value in event
		} else {
			debugPrintf("%s: event \"%s\": object \"%s\" not a function\n", mName.c_str(), e.type().c_str(), e.funcName().c_str());
			return false; // could not handle event, so let it propagate
		}
		return true; // there should be no reason for another system to see or handle this event type, so mark it consumed
	}
	debugPrintf("%s: event \"%s\": state \"%s\" not found\n", mName.c_str(), e.type().c_str(), e.stateId().c_str());
	return false;
}

/*---------------------------------------------------------------------
	Callable from script, registers a script-defined event type
---------------------------------------------------------------------*/
bool ScriptManager_Lua::registerEventType(const char *eventType)
{
	return eventMgr.registerEventType(eventType, RegEventPtr(new ScriptDefinedEvent()));
}

/*---------------------------------------------------------------------
	Calls RegisteredEvent::raiseEventFromScript or "trigger". For
	raising pass true, for trigger pass false.
---------------------------------------------------------------------*/
void ScriptManager_Lua::fireEventFromScript(const char *eventType, LuaObject &eventDataTbl, bool raise)
{
	// look up item in event registry
	RegEventPtr rePtr = eventMgr.getRegEventPtr(eventType);
	if (!rePtr) {
		debugPrintf("Lua: cannot create event \"%s\", not found in registry!\n", eventType);
	} else {
		// found the event type in registry, but before doing anything, use a little RTTI to find out
		// if this event is allowed to be triggered by script
		if (rePtr->scriptAllowed()) {
			AnyVars eventData;
			// convert lua table data to list of pair<string, boost::any>
			if (eventDataTbl.IsTable()) {
				if (rePtr->getEventSource() == EventSource_ScriptOnly) {
					// for script-defined events, pass LuaObject into first position of list, don't translate
					eventData.push_back(AnyVarsValue(string(), any(eventDataTbl)));
				} else {
					// for code-defined events iterate through the lua table
					for (LuaTableIterator it(eventDataTbl); it; it.Next()) {
						const char *key = it.GetKey().GetString();
						LuaObject luaValue = it.GetValue();
						if (luaValue.IsNumber()) { // returns true for all lua numbers
							// test if the number is an integer
							float fVal = luaValue.GetFloat();
							int iVal = FastMath::f2iTrunc(fVal);
							if ((float)iVal == fVal) {
								eventData.push_back(AnyVarsValue(string(key), iVal));
							} else {
								eventData.push_back(AnyVarsValue(string(key), fVal));
							}
						} else if (luaValue.IsString()) {
							eventData.push_back(AnyVarsValue(string(key), string(luaValue.GetString())));
						} else if (luaValue.IsBoolean()) {
							eventData.push_back(AnyVarsValue(string(key), luaValue.GetBoolean()));
						}
					}
				}
			} else { // if what's passed in from lua isn't a table, enter a friendly reminder in debug log
				if (!eventDataTbl.IsNone() && !eventDataTbl.IsNil()) { // but passing nothing IS legal
					debugPrintf("Lua: cannot create event \"%s\", data is not a table!\n", eventType);
				}
			}
			if (raise) {
				rePtr->raiseEventFromSource(eventType, eventData);
			} else {
				rePtr->triggerEventFromSource(eventType, eventData);
			}
		} else {
			debugPrintf("Lua: cannot create event \"%s\", not allowed in script!\n", eventType);
		}
	}
}

// Constructor / destructor
ScriptManager_Lua::ScriptManager_Lua(const string &initFilename) :
	EventListener("ScriptManager_Lua"), // call the base class constructor
	mInitFilename(initFilename)
{
	// register the LuaFunctionEvent with EventManager
	eventMgr.registerEventType(LuaFunctionEvent::sEventType,
								RegEventPtr(new CodeOnlyEvent(EventDataType_NotEmpty)));
	// and register the handler for it
	IEventHandlerPtr p(new EventHandler<ScriptManager_Lua>(this, &ScriptManager_Lua::handleLuaFunctionEvent));
	registerEventHandler(LuaFunctionEvent::sEventType, p, 1); // register as priority 1 to ensure this handles the event first
}

ScriptManager_Lua::~ScriptManager_Lua()
{
	// Need to call this early here in the derived class destructor (even though the base EventListener
	// destructor calls it) to prevent heap corruption. The order of execution of destructors is
	// 1)derived 2)base but by the time the base destructor is called, Lua state is gone and the
	// LuaObjects in any remaining handlers will corrupt the heap upon deletion.
	clearHandlers();
}

////////// class LuaEventHandler //////////

/*---------------------------------------------------------------------
	Adds a lua function to the list. Returns true on success. Fails if
	the function already exists in this list.
---------------------------------------------------------------------*/
bool LuaEventHandler::addLuaFunction(const LuaObject &luaFunc, uint priority)
{
	LuaFuncList::const_iterator li, lPriorityInsert = mLuaFuncList.begin(),
								end = mLuaFuncList.end();
	for (li = mLuaFuncList.begin(); li != end; ++li) {
		// fail if this is a duplicate function registration
		if (li->first == luaFunc) return false;
		// or find where it would be inserted if based on priority
		if (((priority >= (*li).second) && ((*li).second != 0)) || (priority == 0)) ++lPriorityInsert;
	}
	// if function not found in list
	if (priority == 0) { // just add to back of list for FIFO order
		mLuaFuncList.push_back(LuaFuncListValue(luaFunc, priority));
	} else { // add it in priority order
		mLuaFuncList.insert(lPriorityInsert, LuaFuncListValue(luaFunc, priority));
	}
	return true;
}

/*---------------------------------------------------------------------
	Removes a lua function from the list. Returns true on success.
---------------------------------------------------------------------*/
bool LuaEventHandler::removeLuaFunction(const LuaObject &luaFunc)
{
	LuaFuncList::iterator li, end = mLuaFuncList.end();
	for (li = mLuaFuncList.begin(); li != end; ++li) {
		if (li->first == luaFunc) {
			mLuaFuncList.erase(li);
			return true;
		}
	}
	return false;
}

/*---------------------------------------------------------------------
	Calls each lua function that has been registered for the event type
	and returns true if event consumed by one of the handlers in the
	list. Lua event data is built if it hasn't already been before the
	first handler is called.
	**NOTE**
	Could have the AnyVars list hold the resulting LuaObject instead of
	parsing the boost::any values each time. Then, just check for the
	LuaObject at the end of the list
---------------------------------------------------------------------*/
bool LuaEventHandler::operator()(const EventPtr &ePtr)
{
	RegEventPtr rePtr = eventMgr.getRegEventPtr(ePtr->type());
	if (!rePtr->scriptAllowed()) { // this handler was added before the event type was registered, but it's not valid
		debugPrintf("LuaEventHandler: handler for \"%s\" code-only event not allowed\n", ePtr->type().c_str());
		return false; // allow the event to propagate
	}
	// cycle through all LuaFunctions in the list, if true is returned from one of the handlers
	// the loop will exit early
	bool retVal = false;
	LuaFuncList::iterator fi = mLuaFuncList.begin(), end = mLuaFuncList.end();
	while (fi != end && !retVal) {
		LuaFunction<bool> luaFunc(fi->first); // prepare the function for calling
		
		// empty events do not need event data processing
		if (!rePtr->isEmpty()) {
			ScriptableEvent &e = *(static_cast<ScriptableEvent*>(ePtr.get()));

			// if script event data hasn't been built do it now - this is possible
			// when the event was created in code, but has script handlers
			if (!e.isScriptDataBuilt()) e.buildScriptData();

			// script-defined events only hold one object, the LuaObject to pass back
			if (rePtr->getEventSource() == EventSource_ScriptOnly) {
				_ASSERTE(e.getScriptData().size() <= 1 && "The list size should be 0 or 1 - just a LuaObject");
				if (e.getScriptData().size() == 0) {
					// passing nothing through a script event is legal
					retVal = luaFunc();
				} else {
					_ASSERTE(e.getScriptData().front().second.type() == typeid(LuaObject) && "Should be a LuaObject");
					// call the function passing table LuaObject with event data as the only parameter
					retVal = luaFunc(any_cast<LuaObject>(e.getScriptData().front().second));
				}
			} else {
				// ** NOTE **
				// Here is where I could check the last list element and see if it's a LuaObject.
				// If not, do the parsing below and push LuaObject onto the list (front or back).
				// I could also erase all the other list elements and leave only the LuaObject.

				// non script-defined events needs each item in the AnyVars list translated
				// create the lua table object to be passed
				LuaObject eventDataTbl;
				eventDataTbl.AssignNewTable(fi->first.GetState()); // use lua state already stored in the function object
				// iterate through the event data list, convert to lua table
				AnyVars::const_iterator i, end = e.getScriptData().end();
				for (i = e.getScriptData().begin(); i != end; ++i) {
					try {
						if (i->second.type() == typeid(int)) {
							eventDataTbl.SetInteger(i->first.c_str(), any_cast<int>(i->second));
						} else if (i->second.type() == typeid(float)) {
							eventDataTbl.SetNumber(i->first.c_str(), any_cast<float>(i->second));
						} else if (i->second.type() == typeid(string)) {
							eventDataTbl.SetString(i->first.c_str(), any_cast<string>(i->second).c_str());
						} else if (i->second.type() == typeid(bool)) {
							eventDataTbl.SetBoolean(i->first.c_str(), any_cast<bool>(i->second));
						}
					} catch (boost::bad_any_cast &ex) {
						debugPrintf("Lua: bad_any_cast \"%s\"\n", ex.what());
					}
				}
				// call the function passing table LuaObject with event data as the only parameter
				retVal = luaFunc(eventDataTbl);
			}
		} else {
			// handles EmptyEvent types
			retVal = luaFunc();
		}
		++fi;
	}
	return retVal;
}
