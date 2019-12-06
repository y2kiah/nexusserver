#pragma once

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

using std::string;
using namespace boost::posix_time;

struct HTTPCookie {
	// Variables
	string name;
	string value;
	string domain;
	string path;
	ptime expires;
	//int maxTime; // not supported, use expires instead
	//bool secure; // not supported, need ssl first
	bool httpOnly;

	// Functions
	void assign(const HTTPCookie &c)
	{
		*this = c;
	}

	string toString() const;

	// Constructors
	explicit HTTPCookie(const HTTPCookie &c) :
		name(c.name), value(c.value), domain(c.domain), path(c.path),
		expires(c.expires), httpOnly(c.httpOnly)
	{}

	explicit HTTPCookie(const string &_name, const string &_value, const string &_domain = string(),
						const string &_path = string(), const ptime &_expires = not_a_date_time,
						bool _httpOnly = true) :
		name(_name), value(_value), domain(_domain), path(_path),
		expires(_expires), httpOnly(_httpOnly)
	{}
};