#pragma once

#include <string>
#include <hash_map>
#include "../Scripting/ScriptState_Lua.h"
#include "LuaSession.h"

using std::string;
using std::hash_map;

class HTTPRequest;
struct HTTPReply;

typedef hash_map<string, LuaSession>	LuaSessionMap;

class LuaRequestHandler {
	private:
		ScriptState_Lua			mRequestState;
		string					mScript;
		const HTTPRequest &		mRequest;
		HTTPReply &				mReply;
		string					mSessionId; // session id when request linked to a session, otherwise blank
		bool					mAbortFlag; // can be set from lua to bail out early, stops processing

		// session state is a lua state that persists between requests and stores a global table of tables
		// the global table is indexed by session id, each record is a table storing session variables
		static LuaSessionMap	sessions;

		// Functions
		void setupRequestState();
		bool startSession(const string &name = "sessid", int timeoutSeconds = 1200); // default timeout 20 mins
		bool newSession(string &outId, ptime expires);
		void killSession(const string &name = "sessid");

		// Function accessible from Lua
		void luaResponseWrite(LuaObject str);
		bool luaStartSession(const char *name, int timeoutSeconds);
		void luaSaveSession(const char *newSerSessTbl);
		void luaKillSession(const char *name);
		bool luaSetHeader(const char *name, const char *value);
		bool luaSetCookie(const char *name, const char *value, const char *domain = 0,
						  const char *path = 0, int timeoutSeconds = -1, bool httpOnly = false);
		void luaLocation(const char *uri);
		void luaAbort() { mAbortFlag = true; }

	public:
		bool parse();

		explicit LuaRequestHandler(const string &script, const HTTPRequest &req, HTTPReply &reply) :
			mScript(script), mRequest(req), mReply(reply), mAbortFlag(false)
		{
			setupRequestState();
		}
};