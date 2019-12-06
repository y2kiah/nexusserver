/*----==== SCRIPTMANAGER_LUA.H ====----
	Author:		Jeffrey Kiah
	Orig.Date:	05/05/2009
	Rev.Date:	05/13/2009
-------------------------------------*/

#pragma once

#include <string>
#include <hash_map>
#include <memory>
#include <LuaPlus/LuaLink.h>
#include <LuaPlus/LuaPlus.h>
#include <LuaPlus/LuaObject.h>
#include <LuaPlus/LuaFunction.h>
#include "../Event/EventListener.h"
#include "ScriptState_Lua.h"

using std::string;
using std::hash_map;
using std::shared_ptr;
using LuaPlus::LuaStateOwner;
using LuaPlus::LuaObject;
using LuaPlus::LuaFunction;

/*=============================================================================
class ScriptManager_Lua
=============================================================================*/
class ScriptManager_Lua : public EventListener {
	friend class ScriptState_Lua;

	public:
		// Structs
		typedef shared_ptr<ScriptState_Lua>				ScriptStatePtr;
		typedef hash_map<string, ScriptStatePtr>		ScriptStateMap;
		typedef pair<ScriptStateMap::iterator, bool>	ScriptStateMapResult;	// result of inserting elements into the hashmap

		// Functions
		bool requestNewState(string &outId);

		// Static Functions
		/*---------------------------------------------------------------------
			Debug function for determining an object's type
		---------------------------------------------------------------------*/
		#ifdef _DEBUG
		static void	getLuaObjectType(LuaObject &obj);
		#else
		#define		getLuaObjectType(...)
		#endif

		// Constructor / destructor
		explicit ScriptManager_Lua(const string &initFilename);
		~ScriptManager_Lua();

	private:
		// Variables
		ScriptStateMap	mStateMap;
		string			mInitFilename;

		// Functions
		/*---------------------------------------------------------------------
			Debug print function (callable from script)
		---------------------------------------------------------------------*/
		void	debugPrint(LuaObject debugObject);

		/*---------------------------------------------------------------------
			Callable from script, calls inherited registerEventHandler with
			a LuaEventHandler to create a script event handler
		---------------------------------------------------------------------*/
		bool	registerScriptHandler(const char *eventType, LuaObject luaFunc, uint priority);

		/*---------------------------------------------------------------------
			Callable from script, calls inherited unregisterEventHandler
		---------------------------------------------------------------------*/
		bool	unregisterScriptHandler(const char *eventType, LuaObject luaFunc);

		/*---------------------------------------------------------------------
			Callable from script, registers a script-defined event type
		---------------------------------------------------------------------*/
		bool	registerEventType(const char *eventType);

		/*---------------------------------------------------------------------
			Callable from script, causes event to be raised
		---------------------------------------------------------------------*/
		void	raiseEventFromScript(const char *eventType, LuaObject eventDataTbl) {
					fireEventFromScript(eventType, eventDataTbl, true);
				}
		/*---------------------------------------------------------------------
			Callable from script, causes event to be triggered
		---------------------------------------------------------------------*/
		void	triggerEventFromScript(const char *eventType, LuaObject eventDataTbl) {
					fireEventFromScript(eventType, eventDataTbl, false);
				}
		/*-------------------------------------------------------------
			Calls RegisteredEvent::raiseEventFromScript or "trigger".
			For raising pass true, for trigger pass false.
		-------------------------------------------------------------*/
		void	fireEventFromScript(const char *eventType, LuaObject &eventDataTbl, bool raise);

		/*---------------------------------------------------------------------
			Handler for the LuaFunctionEvent event for code to call functions
			defined by script. Event data is updated with any return value.
		---------------------------------------------------------------------*/
		bool	handleLuaFunctionEvent(const EventPtr &ePtr);
};

/*=============================================================================
class LuaEventHandler
	This implementation of IEventHandler is for handling events with functions
	defined in Lua script.
	**NOTE**
	Because EventListeners impose a limit of one handler per event type, and we
	need a way to enable many functions in script to handle any event type,
	this handler provides another level of indirection where many Lua functions
	are stored, and a single call to the functor's operator() results in all of
	the script functions called in priority (or FIFO) order. A "true" return
	value will still be honored, consuming the event.
=============================================================================*/
class LuaEventHandler : public IEventHandler {
	public:
		///// DEFINITIONS /////
		typedef pair<LuaObject, uint>		LuaFuncListValue;
		typedef list<LuaFuncListValue>		LuaFuncList;

		///// FUNCTIONS /////
		// Operators
		/*---------------------------------------------------------------------
			Calls each lua function that has been registered for the event type
			and returns true if event consumed by one of the handlers in the
			list. Lua event data is built if it hasn't already been before the
			first handler is called.
		---------------------------------------------------------------------*/
		virtual bool operator()(const EventPtr &ePtr);

		// Accessors
		/*---------------------------------------------------------------------
			Returns true if the handler list is empty
		---------------------------------------------------------------------*/
		bool	isFuncListEmpty() const { return mLuaFuncList.empty(); }
		/*---------------------------------------------------------------------
			Returns the number of elements in the list
		---------------------------------------------------------------------*/
		int		getFuncListSize() const { return mLuaFuncList.size(); }

		// Mutators
		/*---------------------------------------------------------------------
			Adds a lua function to the list. Returns true on success. Fails if
			the function already exists in this list.
		---------------------------------------------------------------------*/
		bool	addLuaFunction(const LuaObject &luaFunc, uint priority);

		/*---------------------------------------------------------------------
			Removes a lua function from the list. Returns true on success.
		---------------------------------------------------------------------*/
		bool	removeLuaFunction(const LuaObject &luaFunc);

		// Constructors / destructor
		explicit LuaEventHandler(LuaObject &luaFunc, uint priority) :
			mLuaFuncList(1, LuaFuncListValue(luaFunc, priority)) // create the list with one initial value
		{
			_ASSERTE(luaFunc.IsFunction() && "Lua handler is not a function");
		}
		virtual ~LuaEventHandler() { mLuaFuncList.clear(); }

	private:
		///// VARIABLES /////
		LuaFuncList		mLuaFuncList;
};