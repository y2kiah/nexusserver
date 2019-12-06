#pragma once

#include <memory>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;

struct LuaSession {
	// Variables
	ptime expires;
	string serializedSessionTable;

	// Functions
	bool expired() const
	{
		if (expires.is_not_a_date_time()) { return false; }
		return (expires < second_clock::universal_time());
	}

	explicit LuaSession() : expires(not_a_date_time) {}
};
