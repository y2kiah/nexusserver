#pragma once

#include <string>
#include <vector>
#include "NameValuePair.h"
#include "cgi.h"
#include "../Utility/Typedefs.h"

using std::string;
using std::vector;

typedef vector<NameValuePair>	NVPList;

// A request received from a client
class HTTPRequest {
	public:
		// Variables
		string method;
		string uri;
		int httpVersionMajor;
		int httpVersionMinor;
		int contentLength; // from "Content-Length" header
		string content;
		bool expectMessageBody; // for POSTs, with "Content-Length" header
		bool expect100Continue; // from "Expect: 100-continue" header

		string scriptName;
		string queryString;
		NVPList headers;
		NVPList cookies;
		NVPList urlParams;
		NVPList formFields;
		string cgiVars[CGIVar_Count];

		// Functions
		string getNVPValue(const string &name, const NVPList &list) const;
		NVPList::const_iterator getNVPValue(const string &name, string &outValue,
											const NVPList &list, NVPList::const_iterator &start) const;
		void parseHeaders();
		void reset();

		explicit HTTPRequest()
		{
			reset();
		}
};