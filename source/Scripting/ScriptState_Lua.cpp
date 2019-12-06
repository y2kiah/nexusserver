/*----==== SCRIPTSTATE.CPP ====----
	Author: Jeffrey Kiah
	Orig.Date: 09/15/2011
	Rev.Date:  09/15/2011
---------------------------------*/

#include "ScriptState_Lua.h"
#include "ScriptManager_Lua.h"

using namespace LuaPlus;

////////// class ScriptState_Lua //////////

void ScriptState_Lua::debugPrint(LuaObject debugObject)
{
	const char *debugStr = debugObject.ToString();
	debugPrintf(!debugStr ? "Error: Invalid Lua debug string!\n" : debugStr);
}

bool ScriptState_Lua::executeFile(const string &filename)
{
	int retVal = mState->DoFile(filename.c_str());
	bool success = (retVal == 0);
	if (!success) {
		LuaStackObject obj = mState->StackTop(); // top stack object is a string containing error
		const string objStr(obj.GetString());
		debugPrintf("Lua: error executing script \"%s\": %s\n", filename.c_str(), objStr.c_str());
	}
	return success;
}

bool ScriptState_Lua::createTable(const string &name)
{
	auto result = mTables.insert(make_pair(name,LuaObject()));
	if (!result.second) { return false; }
	LuaObject &newObject = result.first->second;
	newObject = mState->GetGlobals().CreateTable(name.c_str());
	newObject.SetObject("__index", newObject);
	return true;
}

bool ScriptState_Lua::addTableValue(const string &tableName, const string &key,
									const string &value, bool concatCSV)
{
	auto i = mTables.find(tableName);
	if (i == mTables.end()) { return false; }
	if (concatCSV) {
		// see if key already exists in table
		LuaObject searchVal = i->second[key.c_str()];
		if (searchVal.IsString()) {
			string assignVal(searchVal.GetString());
			assignVal.append(",");
			assignVal.append(value);
			i->second.SetString(key.c_str(), assignVal.c_str());
			return true;
		}
	}
	i->second.SetString(key.c_str(), value.c_str());
	return true;
}

bool ScriptState_Lua::makeMetaTable(const string &name)
{
	auto i = mTables.find(name);
	if (i == mTables.end()) { return false; }
	LuaObject luaStateManObj = mState->BoxPointer(this);
	luaStateManObj.SetMetaTable(i->second);
	// Expose the metatable as a named entity.
	mState->GetGlobals().SetObject(name.c_str(), luaStateManObj);
	return true;
}

ScriptState_Lua::ScriptState_Lua() :
	mState(true) // true indicates to init the standard Lua library
{
	// create host table
	createTable("host");
	makeMetaTable("host");
	registerFunction("host", "dofile",
		(ScriptState_Lua*)this, &ScriptState_Lua::doFile);
	registerFunction("host", "debugPrint",
		(ScriptState_Lua*)this, &ScriptState_Lua::debugPrint);
}