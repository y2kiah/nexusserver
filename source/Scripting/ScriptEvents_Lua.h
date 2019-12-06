/*----==== SCRIPTEVENTS_LUA.H ====----
	Author: Jeffrey Kiah
	Orig.Date: 05/06/2009
	Rev.Date:  09/15/2011
------------------------------------*/

#pragma once

#include <string>
#include "../Event/Event.h"

using std::string;

/*=============================================================================
class LuaFunctionEvent
=============================================================================*/
class LuaFunctionEvent : public Event {
	private:
		string		mStateId;
		string		mFuncName;
		LuaObject	mParamTable;
		LuaObject	mReturnObj;

	public:
		///// VARIABLES /////
		static const string		sEventType; // made public so listeners can register via this variable
											// and not the actual string value itself
		///// FUNCTIONS /////
		// Accessors
		const string &		stateId() const		{ return mStateId; }
		const string &		funcName() const	{ return mFuncName; }
		const LuaObject &	paramTable() const	{ return mParamTable; }
		const LuaObject &	returnObj() const	{ return mReturnObj; }

		virtual const string & type() const { return sEventType; }

		// Mutators
		void	setReturnObj(const LuaObject &returnObj) { mReturnObj = returnObj; }

		// Constructor / destructor
		explicit LuaFunctionEvent(const string &stateId, const string &funcName) :
			mStateId(stateId), mFuncName(funcName)
		{}
		virtual ~LuaFunctionEvent() {}
};