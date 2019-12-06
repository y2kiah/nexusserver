#pragma once

#include <string>

using std::string;

struct NameValuePair {
	string name;
	string value;
	
	explicit NameValuePair(const string &_name, const string &_value) :
		name(_name), value(_value)
	{}
	explicit NameValuePair() {}
};