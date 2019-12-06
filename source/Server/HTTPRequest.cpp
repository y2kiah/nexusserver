#include "HTTPRequest.h"
#include <algorithm>
#include <functional>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

using std::find_if;
using std::bind;
using std::for_each;
using namespace boost;

// Defintions

const char *CGIVarName[] = {
	"CONTENT_LENGTH",
	"DOCUMENT_ROOT",
	"HTTP_REFERER",
	"HTTP_USER_AGENT",
	"PATH_INFO",
	"PATH_TRANSLATED",
	"QUERY_STRING",
	"REMOTE_ADDR",
	"REMOTE_HOST",
	"REQUEST_METHOD",
	"REQUEST_URI",
	"SCRIPT_FILENAME",
	"SCRIPT_NAME",
	"SERVER_NAME",
	"SERVER_PORT",
	"SERVER_PROTOCOL"
};

////////// class HTTPRequest //////////

string HTTPRequest::getNVPValue(const string &name, const NVPList &list) const
{
	NVPList::const_iterator i =
		find_if(list.begin(), list.end(), [&name](const NameValuePair &nvp) {
			return (nvp.name == name);
		});
	if (i != list.end()) { return i->value; }
	return string();
}

NVPList::const_iterator HTTPRequest::getNVPValue(
	const string &name, string &outValue,
	const NVPList &list, NVPList::const_iterator &start) const
{
	auto i = find_if(start, list.end(),
					 [&name](const NameValuePair &nvp) { return (nvp.name == name); });

	if (i != list.end()) {
		outValue.assign(i->value);
		return i;
	}
	outValue.clear();
	return list.end();
}

void HTTPRequest::parseHeaders()
{
	// get content length
	string value = getNVPValue("Content-Length", headers);
	if (value.length() > 0) {
		try {
			contentLength = boost::lexical_cast<int>(value.c_str());
		} catch (boost::bad_lexical_cast &) {
			debugPrintf("Bad lexical cast for Content-Length value %s\n", value.c_str());
		}
		expectMessageBody = true;
	}

	// get 100-continue
	value = getNVPValue("Expect", headers);
	expect100Continue = (_strnicmp(value.c_str(), "100-continue", 12) == 0);

	// get cookies
	value = getNVPValue("Cookie", headers);
	char_separator<char> sep("; ");
	tokenizer<char_separator<char>> tokens(value, sep);
	vector<string> nvp(2);
	for_each(tokens.begin(), tokens.end(), [this, &nvp](const string &t) {
		nvp.clear();
		split(nvp, t, is_any_of("= "), token_compress_on);
		if (nvp.size() == 2) {
			cookies.push_back(NameValuePair(nvp[0], nvp[1]));
		}
	});
}

void HTTPRequest::reset()
{
	method.clear(); uri.clear(); content.clear(); scriptName.clear(); queryString.clear();
	httpVersionMajor = httpVersionMinor = contentLength = 0;
	expectMessageBody = expect100Continue = false;
	headers.clear(); cookies.clear(); urlParams.clear(); formFields.clear();
	for (int i = 0; i < CGIVar_Count; ++i) { cgiVars[i].clear(); }
}