#include "LuaRequestHandler.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "HTTPRequest.h"
#include "HTTPReply.h"
#include "HTTPCookie.h"

using namespace std;

////////// class LuaRequestHandler //////////

// Statics
LuaSessionMap LuaRequestHandler::sessions;

// Functions

bool LuaRequestHandler::parse()
{
	size_t i = 0;
	while (i < mScript.length()) {
		size_t tagStart = mScript.find("<%", i);
		size_t tagEnd = tagStart;

		// add all content to response up to the tag start (or end of file)
		string output(mScript.substr(i, tagStart-i));
		if (output.length() > 0) {
			//debugPrintf("\noutput length=%i\n", output.length());
			mReply.content.append(output);
		}

		if (tagStart != string::npos) { // found a tag
			// find the end tag
			tagEnd = mScript.find("%>", tagStart+2);
			if (tagEnd == string::npos) {
				mReply.stockReply(HTTPReply::internal_server_error);
				return false; // if it's not present, return 500 error
			}
			size_t tagLen = tagEnd - (tagStart+2);
			// run the script between tags in request Lua state
			string runScript(mScript.substr(tagStart+2, tagLen));
			//debugPrintf("\nrunning script: \"%s\"\n", runScript.c_str());
			mRequestState.executeString(runScript.c_str());
			// stop processing if the abort flag was set
			if (mAbortFlag) { return true; }

			tagEnd += 2;
		}
		i = tagEnd;
	}
	return true;
}

void LuaRequestHandler::setupRequestState()
{
	bool init =	mRequestState.executeFile("lua/init_scriptstate.lua");
	if (!init) { return; }
	init = mRequestState.executeFile("lua/serialize_tbl.lua");
	if (!init) { return; }

	// create luap table
	mRequestState.createTable("luap");
	mRequestState.makeMetaTable("luap");
	mRequestState.registerFunction("luap", "write",
		(LuaRequestHandler*)this, &LuaRequestHandler::luaResponseWrite);
	// add header
	// add cookie
	mRequestState.registerFunction("luap", "startSession",
		(LuaRequestHandler*)this, &LuaRequestHandler::luaStartSession);
	mRequestState.registerFunction("luap", "saveSession",
		(LuaRequestHandler*)this, &LuaRequestHandler::luaSaveSession);
	mRequestState.registerFunction("luap", "killSession",
		(LuaRequestHandler*)this, &LuaRequestHandler::luaKillSession);
	mRequestState.registerFunction("luap", "setHeader",
		(LuaRequestHandler*)this, &LuaRequestHandler::luaSetHeader);
	mRequestState.registerFunction("luap", "setCookie",
		(LuaRequestHandler*)this, &LuaRequestHandler::luaSetCookie);
	mRequestState.registerFunction("luap", "location",
		(LuaRequestHandler*)this, &LuaRequestHandler::luaLocation);
	mRequestState.registerFunction("luap", "abort",
		(LuaRequestHandler*)this, &LuaRequestHandler::luaAbort);

	// Create URL table
	mRequestState.createTable("URL");
	for_each(mRequest.urlParams.begin(), mRequest.urlParams.end(), [&](const NameValuePair &nvp){
		mRequestState.addTableValue("URL", nvp.name.c_str(), nvp.value.c_str(), true);
	});
	
	// Create FORM table
	mRequestState.createTable("FORM");
	for_each(mRequest.formFields.begin(), mRequest.formFields.end(), [&](const NameValuePair &nvp){
		mRequestState.addTableValue("FORM", nvp.name.c_str(), nvp.value.c_str(), true);
	});

	// Create ENV table
	mRequestState.createTable("ENV");
	for (int i = 0; i < CGIVar_Count; ++i) {
		mRequestState.addTableValue("ENV", CGIVarName[i], mRequest.cgiVars[i]);
	}
}

// returns the session id or empty string on failure
bool LuaRequestHandler::startSession(const string &name, int timeoutSeconds)
{
	// find new expiration time
	ptime time(second_clock::universal_time() + seconds(timeoutSeconds));

	// look for a valid session
	string sessionId = mRequest.getNVPValue(name, mRequest.cookies);

	if (!sessionId.empty()) {
		auto i = sessions.find(sessionId);
		if (i != sessions.end()) {
			// check here if session has expired
			if (!i->second.expired()) {
				// set new expiration time
				i->second.expires = time;
				// add the SESSION table to request state
				mRequestState.executeString(i->second.serializedSessionTable.c_str());
				
				mSessionId = sessionId;
				return true;
			}
		}
	}

	// session doesn't already exist, start a new one
	if (newSession(sessionId, time)) {
		// find domain - TEMP, use cgiVars once they are all implemented
		// set session cookie
		mReply.setCookie(HTTPCookie(name, sessionId, "", mRequest.cgiVars[PATH_INFO]));

		mSessionId = sessionId;
		return true;
	}
	return false;
}

bool LuaRequestHandler::newSession(string &outId, ptime expires)
{
	// generate new random id
	boost::uuids::random_generator gen;
	boost::uuids::uuid u(gen());
	string id(boost::uuids::to_string(u));
	// create new session
	auto r = sessions.insert(std::pair<string, LuaSession>(id, LuaSession()));
	if (!r.second) { // insert failed
		return false;
	}
	r.first->second.expires = expires;
	// creates the session table in request state when executed
	r.first->second.serializedSessionTable = "SESSION = {}";
	// add the session table to request state
	mRequestState.executeString(r.first->second.serializedSessionTable.c_str());

	// set return values
	outId = id;
	return true;
}

void LuaRequestHandler::killSession(const string &name)
{
	// look for the session id, first look for mSessionId and if not set, look in cookies
	string sessionId(mSessionId.empty() ?
						mRequest.getNVPValue(name, mRequest.cookies) :
						mSessionId);
	// find the session
	if (!sessionId.empty()) {
		auto i = sessions.find(sessionId);
		if (i != sessions.end()) {
			sessions.erase(i); // delete it
		}
	}
}

///// Functions exposed to Lua /////

void LuaRequestHandler::luaResponseWrite(LuaObject str)
{
	const char *writeStr = str.ToString();
	mReply.content.append(writeStr);
}

bool LuaRequestHandler::luaStartSession(const char *name, int timeoutSeconds)
{
	// call startSession to retrieve or create new session
	return startSession(name, timeoutSeconds);
	//debugPrintf("\nluaStartSession: name=%s, timeoutSeconds=%i\n", (name ? name : "null"), timeoutSeconds);
}

void LuaRequestHandler::luaSaveSession(const char *newSerSessTbl)
{
	if (!newSerSessTbl) { return; }
	// find the session
	if (!mSessionId.empty()) {
		auto i = sessions.find(mSessionId);
		if (i != sessions.end()) {
			if (!i->second.expired()) {
				i->second.serializedSessionTable = newSerSessTbl;
			}
		}
	}
}

void LuaRequestHandler::luaKillSession(const char *name)
{
	killSession(name);
}

bool LuaRequestHandler::luaSetHeader(const char *name, const char *value)
{
	if (!name || !value) { return false; }
	return mReply.addHeader(name, value, true);
}

bool LuaRequestHandler::luaSetCookie(const char *name, const char *value, const char *domain,
									 const char *path, int timeoutSeconds, bool httpOnly)
{
	if (!name || !value) { return false; }

	// find expiration time
	ptime expires(not_a_date_time);
	if (timeoutSeconds >= 0) {
		expires = second_clock::universal_time() + seconds(timeoutSeconds);
	}

	HTTPCookie c(name, value,
				 (domain ? domain : string()),
				 (path ? path : string()),
				 expires,
				 httpOnly);

	mReply.setCookie(c);
	return true;
}

void LuaRequestHandler::luaLocation(const char *uri)
{
	mReply = HTTPReply::stockReply(HTTPReply::see_other);
	mReply.addHeader("Location", uri);
	luaAbort();
}