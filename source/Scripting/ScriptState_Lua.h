/*----==== SCRIPTSTATE.H ====----
	Author: Jeffrey Kiah
	Orig.Date: 09/15/2011
	Rev.Date:  09/15/2011
-------------------------------*/

#pragma once

#include <string>
#include <hash_map>
#include <LuaPlus/LuaPlus.h>
#include "ScriptFile_Lua.h"

using std::string;
using std::hash_map;
using LuaPlus::LuaStateOwner;
using LuaPlus::LuaObject;
using LuaPlus::LuaFunction;

typedef hash_map<string, LuaObject> LuaObjectMap;

/*=============================================================================
class ScriptState_Lua
=============================================================================*/
class ScriptState_Lua {
	friend class ScriptManager_Lua;

	private:
		// Variables
		LuaStateOwner	mState;
		LuaObjectMap	mTables;

		// Functions
		/*---------------------------------------------------------------------
			Executes a Lua script file, takes a (const char *) for use with
			RegisterObjectDirect - called from script
		---------------------------------------------------------------------*/
		bool doFile(const char *filename) {
			return executeFile(filename);
		}

		/*---------------------------------------------------------------------
			Debug print function, called from script
		---------------------------------------------------------------------*/
		void	debugPrint(LuaObject debugObject);

	public:
		/*---------------------------------------------------------------------
			Executes a Lua script file in the provided state
		---------------------------------------------------------------------*/
		int execute(const ScriptFile_Lua &res)
		{
			const string script(res.dataPtr().get(), res.sizeB());
			executeString(script.c_str());
		}

		/*---------------------------------------------------------------------
			Executes a Lua command. scriptStr must be a null-terminated string.
		---------------------------------------------------------------------*/
		int	executeString(const char *scriptStr)
		{
			return mState->DoString(scriptStr);
		}

		/*---------------------------------------------------------------------
			Executes a Lua script file in the provided state
		---------------------------------------------------------------------*/
		bool executeFile(const string &filename);

		/*---------------------------------------------------------------------
			Creates a new table, returns false if name already exists
		---------------------------------------------------------------------*/
		bool createTable(const string &name);

		/*---------------------------------------------------------------------
			Makes an existing table a metatable for code-script interface
			functions.
		---------------------------------------------------------------------*/
		bool makeMetaTable(const string &name);

		/*---------------------------------------------------------------------
			Adds a record to an existing table
		---------------------------------------------------------------------*/
		bool addTableValue(const string &tableName, const string &key, const string &value, bool concatCSV = false);

		/*---------------------------------------------------------------------
			Registers a function to be called from script
		---------------------------------------------------------------------*/
		template <typename Callee, typename Func>
		bool registerFunction(const char *tableName, const char *funcName,
							  const Callee *callee, Func func)
		{
			auto i = mTables.find(tableName);
			if (i == mTables.end()) { return false; }
			i->second.RegisterObjectDirect(funcName, callee, func);
			return true;
		}

		/*---------------------------------------------------------------------
			accessor to the lua state
		---------------------------------------------------------------------*/
		LuaStateOwner &getState() { return mState; }

		// Constructor/destructor
		explicit ScriptState_Lua();
		~ScriptState_Lua() {}
};