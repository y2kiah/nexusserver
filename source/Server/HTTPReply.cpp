#include <boost/lexical_cast.hpp>
#include "HTTPReply.h"

using std::find_if;

namespace StatusStrings {
	const string _continue =
		"HTTP/1.1 100 Continue\r\n";
	const string ok =
		"HTTP/1.1 200 OK\r\n";
	const string created =
		"HTTP/1.1 201 Created\r\n";
	const string accepted =
		"HTTP/1.1 202 Accepted\r\n";
	const string no_content =
		"HTTP/1.1 204 No Content\r\n";
	const string multiple_choices =
		"HTTP/1.1 300 Multiple Choices\r\n";
	const string moved_permanently =
		"HTTP/1.1 301 Moved Permanently\r\n";
	const string moved_temporarily =
		"HTTP/1.1 302 Moved Temporarily\r\n";
	const string see_other =
		"HTTP/1.1 303 See Other\r\n";
	const string not_modified =
		"HTTP/1.1 304 Not Modified\r\n";
	const string bad_request =
		"HTTP/1.1 400 Bad Request\r\n";
	const string unauthorized =
		"HTTP/1.1 401 Unauthorized\r\n";
	const string forbidden =
		"HTTP/1.1 403 Forbidden\r\n";
	const string not_found =
		"HTTP/1.1 404 Not Found\r\n";
	const string length_required =
		"HTTP/1.1 411 Length Required\r\n";
	const string internal_server_error =
		"HTTP/1.1 500 Internal Server Error\r\n";
	const string not_implemented =
		"HTTP/1.1 501 Not Implemented\r\n";
	const string bad_gateway =
		"HTTP/1.1 502 Bad Gateway\r\n";
	const string service_unavailable =
		"HTTP/1.1 503 Service Unavailable\r\n";

	const string &toString(HTTPReply::StatusType status)
	{
		switch (status) {
			case HTTPReply::_continue:
				return _continue;
			case HTTPReply::ok:
				return ok;
			case HTTPReply::created:
				return created;
			case HTTPReply::accepted:
				return accepted;
			case HTTPReply::no_content:
				return no_content;
			case HTTPReply::multiple_choices:
				return multiple_choices;
			case HTTPReply::moved_permanently:
				return moved_permanently;
			case HTTPReply::moved_temporarily:
				return moved_temporarily;
			case HTTPReply::see_other:
				return see_other;
			case HTTPReply::not_modified:
				return not_modified;
			case HTTPReply::bad_request:
				return bad_request;
			case HTTPReply::unauthorized:
				return unauthorized;
			case HTTPReply::forbidden:
				return forbidden;
			case HTTPReply::not_found:
				return not_found;
			case HTTPReply::length_required:
				return length_required;
			case HTTPReply::internal_server_error:
				return internal_server_error;
			case HTTPReply::not_implemented:
				return not_implemented;
			case HTTPReply::bad_gateway:
				return bad_gateway;
			case HTTPReply::service_unavailable:
				return service_unavailable;
			default:
				return internal_server_error;
		}
	}

	boost::asio::const_buffer toBuffer(HTTPReply::StatusType status)
	{
		return boost::asio::buffer(toString(status));
	}
}

namespace MiscStrings {
	const char name_value_separator[] = { ':', ' ' };
	const char crlf[] = { '\r', '\n' };
}

namespace StockReplies {
	const char _continue[] = "";
	const char ok[] = "";
	const char created[] =
		"<html>"
		"<head><title>Created</title></head>"
		"<body><h1>201 Created</h1></body>"
		"</html>";
	const char accepted[] =
		"<html>"
		"<head><title>Accepted</title></head>"
		"<body><h1>202 Accepted</h1></body>"
		"</html>";
	const char no_content[] =
		"<html>"
		"<head><title>No Content</title></head>"
		"<body><h1>204 Content</h1></body>"
		"</html>";
	const char multiple_choices[] =
		"<html>"
		"<head><title>Multiple Choices</title></head>"
		"<body><h1>300 Multiple Choices</h1></body>"
		"</html>";
	const char moved_permanently[] =
		"<html>"
		"<head><title>Moved Permanently</title></head>"
		"<body><h1>301 Moved Permanently</h1></body>"
		"</html>";
	const char moved_temporarily[] =
		"<html>"
		"<head><title>Moved Temporarily</title></head>"
		"<body><h1>302 Moved Temporarily</h1></body>"
		"</html>";
	const char not_modified[] =
		"<html>"
		"<head><title>Not Modified</title></head>"
		"<body><h1>304 Not Modified</h1></body>"
		"</html>";
	const char bad_request[] =
		"<html>"
		"<head><title>Bad Request</title></head>"
		"<body><h1>400 Bad Request</h1></body>"
		"</html>";
	const char unauthorized[] =
		"<html>"
		"<head><title>Unauthorized</title></head>"
		"<body><h1>401 Unauthorized</h1></body>"
		"</html>";
	const char forbidden[] =
		"<html>"
		"<head><title>Forbidden</title></head>"
		"<body><h1>403 Forbidden</h1></body>"
		"</html>";
	const char not_found[] =
		"<html>"
		"<head><title>Not Found</title></head>"
		"<body><h1>404 Not Found</h1></body>"
		"</html>";
	const char internal_server_error[] =
		"<html>"
		"<head><title>Internal Server Error</title></head>"
		"<body><h1>500 Internal Server Error</h1></body>"
		"</html>";
	const char not_implemented[] =
		"<html>"
		"<head><title>Not Implemented</title></head>"
		"<body><h1>501 Not Implemented</h1></body>"
		"</html>";
	const char bad_gateway[] =
		"<html>"
		"<head><title>Bad Gateway</title></head>"
		"<body><h1>502 Bad Gateway</h1></body>"
		"</html>";
	const char service_unavailable[] =
		"<html>"
		"<head><title>Service Unavailable</title></head>"
		"<body><h1>503 Service Unavailable</h1></body>"
		"</html>";

	string toString(HTTPReply::StatusType status)
	{
		switch (status) {
			case HTTPReply::_continue:
				return _continue;
			case HTTPReply::ok:
				return ok;
			case HTTPReply::created:
				return created;
			case HTTPReply::accepted:
				return accepted;
			case HTTPReply::no_content:
				return no_content;
			case HTTPReply::multiple_choices:
				return multiple_choices;
			case HTTPReply::moved_permanently:
				return moved_permanently;
			case HTTPReply::moved_temporarily:
				return moved_temporarily;
			case HTTPReply::not_modified:
				return not_modified;
			case HTTPReply::bad_request:
				return bad_request;
			case HTTPReply::unauthorized:
				return unauthorized;
			case HTTPReply::forbidden:
				return forbidden;
			case HTTPReply::not_found:
				return not_found;
			case HTTPReply::internal_server_error:
				return internal_server_error;
			case HTTPReply::not_implemented:
				return not_implemented;
			case HTTPReply::bad_gateway:
				return bad_gateway;
			case HTTPReply::service_unavailable:
				return service_unavailable;
			default:
				return internal_server_error;
		}
	}
}

////////// class HTTPReply //////////

string HTTPReply::toString() const
{
	string buffer;
	buffer.reserve(256);
	buffer += StatusStrings::toString(status);
	for (std::size_t i = 0; i < headers.size(); ++i) {
		const NameValuePair &h = headers[i];
		buffer += h.name;
		buffer += MiscStrings::name_value_separator;
		buffer += h.value;
		buffer += MiscStrings::crlf;
	}
	buffer += MiscStrings::crlf;
	buffer += content;
	return buffer;
}

void HTTPReply::setCookie(const HTTPCookie &cookie)
{
	auto i =
		find_if(cookies.begin(), cookies.end(), [&cookie](const HTTPCookie &c) {
			return (c.name == cookie.name);
		});
	if (i == cookies.end()) { // didn't find cookie in list, add new
		cookies.push_back(cookie);

	} else { // found cookie, set new values
		i->assign(cookie);
	}
}

bool HTTPReply::addHeader(const string &name, const string &value, bool overwrite)
{
	if (name.length() == 0) { return false; }

	if (overwrite) { // if overwrite is true, look for an existing header of same name to overwrite
		auto i = find_if(headers.begin(), headers.end(),
			[&name](const NameValuePair &nvp){
				return (nvp.name == name);
			});
		if (i != headers.end()) {
			i->value = value;
			return true;
		}
	}
	
	headers.push_back(NameValuePair(name, value));
	return true;
}

const BufferList &HTTPReply::toBuffers()
{
	mReplyBuffers.clear();
	mReplyBuffers.push_back(StatusStrings::toBuffer(status));
	for (std::size_t i = 0; i < headers.size(); ++i) {
		NameValuePair &h = headers[i];
		mReplyBuffers.push_back(boost::asio::buffer(h.name));
		mReplyBuffers.push_back(boost::asio::buffer(MiscStrings::name_value_separator));
		mReplyBuffers.push_back(boost::asio::buffer(h.value));
		mReplyBuffers.push_back(boost::asio::buffer(MiscStrings::crlf));
	}
	mReplyBuffers.push_back(boost::asio::buffer(MiscStrings::crlf));
	mReplyBuffers.push_back(boost::asio::buffer(content));
	return mReplyBuffers;
}

HTTPReply HTTPReply::stockReply(HTTPReply::StatusType status)
{
	HTTPReply rep;
	rep.status = status;
	rep.content = StockReplies::toString(status);
	if (status > 200) {
		rep.addHeader("Connection", "close");
		rep.addHeader("Content-Length", boost::lexical_cast<string>(rep.content.size()));
		rep.addHeader("Content-Type", "text/html");
	}
	return rep;
}