#include "HTTPRequestParser.h"
#include <sstream>
#include "HTTPRequest.h"
#include "TCPConnection.h"
#include "HTTPRequestHandler.h"
#include "../Utility/Typedefs.h"

using namespace std;

////////// class HTTPRequestParser //////////

HTTPRequestParser::HTTPRequestParser() :
	state(method_start)
{}

void HTTPRequestParser::reset()
{
	state = method_start;
	mRequest.reset();
}

tuple<tribool, unsigned int> HTTPRequestParser::parse(istream &is, unsigned int size)
{
	for (unsigned int i = 0; i < size; ++i) {
		char c;
		is.get(c);
		tribool result = consume(c);
		mMsg << c;
		
		if (result || !result) {
			if (result) {
				bool p1 = parseURI();
				bool p2 = parseNVP(mRequest.content, mRequest.formFields);
				if (!p1 || !p2) { result = false; }
			}
			return boost::make_tuple(result, i);
		}
	}
	const tribool ind = boost::indeterminate;
	return boost::make_tuple(ind, size-1);
}

bool HTTPRequestParser::parseURI()
{
	size_t pos = mRequest.uri.find('?');
	bool decoded = urlDecode(mRequest.uri.substr(0,pos), mRequest.scriptName);
	if (!decoded) {
		debugPrintf("URL decoding script name ""%s"" failed\n", mRequest.scriptName.c_str());
		return false;
	}
	if (pos != string::npos) {
		mRequest.queryString = mRequest.uri.substr(pos+1);
		return parseNVP(mRequest.queryString, mRequest.urlParams);
	}
	return true;
}

bool HTTPRequestParser::parseNVP(const string &nvpString, NVPList &list)
{
	istringstream iss(nvpString);
	string nvp;
	while (getline(iss,nvp,'&')) {
		size_t eqPos = nvp.find('=');
		if (eqPos != string::npos) {
			string name, value;
			bool decoded1 = urlDecode(nvp.substr(0,eqPos), name);
			bool decoded2 = urlDecode(nvp.substr(eqPos+1), value);
			if (decoded1 && decoded2) {
				list.push_back(NameValuePair(name,value));
			} else {
				debugPrintf("URL decoding name value pair ""%s"" failed\n", name.c_str());
				return false;
			}
		} else {
			debugPrintf("Name value pair ""%s"" equal sign not found\n", nvp.c_str());
			return false;
		}
	}
	return true;
}

tribool HTTPRequestParser::consume(char input)
{
	//debugPrintf("%c",input); // TEMP

	switch (state) {
		case method_start:
			if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
				return false;
			} else {
				state = method;
				mRequest.method.push_back(input);
				return boost::indeterminate;
			}
		case method:
			if (input == ' ') {
				state = uri;
				return boost::indeterminate;
			}
			else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
				return false;
			} else {
				mRequest.method.push_back(input);
				return boost::indeterminate;
			}
		case uri_start:
			if (is_ctl(input)) {
				return false;
			} else {
				state = uri;
				mRequest.uri.push_back(input);
				return boost::indeterminate;
			}
		case uri:
			if (input == ' ') {
				state = http_version_h;
				return boost::indeterminate;
			} else if (is_ctl(input)) {
				return false;
			} else {
				mRequest.uri.push_back(input);
				return boost::indeterminate;
			}
		case http_version_h:
			if (input == 'H') {
				state = http_version_t_1;
				return boost::indeterminate;
			} else {
				return false;
			}
		case http_version_t_1:
			if (input == 'T') {
				state = http_version_t_2;
				return boost::indeterminate;
			} else {
				return false;
			}
		case http_version_t_2:
			if (input == 'T') {
				state = http_version_p;
				return boost::indeterminate;
			} else {
				return false;
			}
		case http_version_p:
			if (input == 'P') {
				state = http_version_slash;
				return boost::indeterminate;
			} else {
				return false;
			}
		case http_version_slash:
			if (input == '/') {
				state = http_version_major_start;
				return boost::indeterminate;
			} else {
				return false;
			}
		case http_version_major_start:
			if (is_digit(input)) {
				mRequest.httpVersionMajor = mRequest.httpVersionMajor * 10 + input - '0';
				state = http_version_major;
				return boost::indeterminate;
			} else {
				return false;
			}
		case http_version_major:
			if (input == '.') {
				state = http_version_minor_start;
				return boost::indeterminate;
			} else if (is_digit(input)) {
				mRequest.httpVersionMajor = mRequest.httpVersionMajor * 10 + input - '0';
				return boost::indeterminate;
			} else {
				return false;
			}
		case http_version_minor_start:
			if (is_digit(input)) {
				mRequest.httpVersionMinor = mRequest.httpVersionMinor * 10 + input - '0';
				state = http_version_minor;
				return boost::indeterminate;
			} else {
				return false;
			}
		case http_version_minor:
			if (input == '\r') {
				state = expecting_newline_1;
				return boost::indeterminate;
			} else if (is_digit(input)) {
				mRequest.httpVersionMinor = mRequest.httpVersionMinor * 10 + input - '0';
				return boost::indeterminate;
			} else {
				return false;
			}
		case expecting_newline_1:
			if (input == '\n') {
				state = header_line_start;
				return boost::indeterminate;
			} else {
				return false;
			}
		case header_line_start:
			if (input == '\r') {
				// headers are done, look for Content-Length for message body
				mRequest.parseHeaders();
				state = expecting_newline_3;
				return boost::indeterminate;
			} else if (!mRequest.headers.empty() && (input == ' ' || input == '\t')) {
				state = header_lws;
				return boost::indeterminate;
			} else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
				return false;
			} else {
				mRequest.headers.push_back(NameValuePair());
				mRequest.headers.back().name.push_back(input);
				state = header_name;
				return boost::indeterminate;
			}
		case header_lws:
			if (input == '\r') {
				state = expecting_newline_2;
				return boost::indeterminate;
			} else if (input == ' ' || input == '\t') {
				return boost::indeterminate;
			} else if (is_ctl(input)) {
				return false;
			} else {
				state = header_value;
				mRequest.headers.back().value.push_back(input);
				return boost::indeterminate;
			}
		case header_name:
			if (input == ':') {
				state = space_before_header_value;
				return boost::indeterminate;
			} else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
				return false;
			} else {
				mRequest.headers.back().name.push_back(input);
				return boost::indeterminate;
			}
		case space_before_header_value:
			if (input == ' ') {
				state = header_value;
				return boost::indeterminate;
			} else {
				return false;
			}
		case header_value:
			if (input == '\r') {
				state = expecting_newline_2;
				return boost::indeterminate;
			} else if (is_ctl(input)) {
				return false;
			} else {
				mRequest.headers.back().value.push_back(input);
				return boost::indeterminate;
			}
		case expecting_newline_2:
			if (input == '\n') {
				state = header_line_start;
				return boost::indeterminate;
			} else {
				return false;
			}
		case expecting_newline_3:
			if (mRequest.expectMessageBody) {
				// send 100 Continue to the client
				// only send it when a "Expect: 100-continue" header is present
				if (mRequest.expect100Continue) {
					HTTPRequestHandler &hndlr = *reinterpret_cast<HTTPRequestHandler*>(mConnection->getHandler().get());
					hndlr.setStockReply(HTTPReply::_continue);
					mConnection->writeReply();
				}
				state = content;
				return boost::indeterminate;
			} else {
				if (mRequest.method == "POST") { // POST message without Content-Length header
					HTTPRequestHandler &hndlr = *reinterpret_cast<HTTPRequestHandler*>(mConnection->getHandler().get());
					hndlr.setStockReply(HTTPReply::length_required);
					return false;
				}
				return (input == '\n');
			}
		case content: {
			mRequest.content.push_back(input);
			// see if all content has been sent
			if (mRequest.content.length() == mRequest.contentLength) {
				return true;
			}
			return boost::indeterminate;
		}
		default:
			return false;
	}
}

bool HTTPRequestParser::is_char(int c)
{
	return c >= 0 && c <= 127;
}

bool HTTPRequestParser::is_ctl(int c)
{
	return (c >= 0 && c <= 31) || (c == 127);
}

bool HTTPRequestParser::is_tspecial(int c)
{
	switch (c) {
		case '(': case ')': case '<': case '>': case '@':
		case ',': case ';': case ':': case '\\': case '"':
		case '/': case '[': case ']': case '?': case '=':
		case '{': case '}': case ' ': case '\t':
			return true;
		default:
			return false;
	}
}

bool HTTPRequestParser::is_digit(int c)
{
	return c >= '0' && c <= '9';
}

bool HTTPRequestParser::urlDecode(const string &in, string &out)
{
	out.clear();
	out.reserve(in.size());
	for (size_t i = 0; i < in.size(); ++i) {
		if (in[i] == '%') {
			if (i + 3 <= in.size()) {
				int value = 0;
				std::istringstream is(in.substr(i + 1, 2));
				if (is >> std::hex >> value) {
					out += static_cast<char>(value);
					i += 2;
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else if (in[i] == '+') {
			out += ' ';
		} else {
			out += in[i];
		}
	}
	return true;
}