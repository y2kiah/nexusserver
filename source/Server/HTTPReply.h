#pragma once

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "NameValuePair.h"
#include "HTTPCookie.h"
#include "Message.h"

// A reply to be sent to a client
struct HTTPReply {
	// Variables
	enum StatusType {
		not_set = 0,
		_continue = 100,
		ok = 200,
		created = 201,
		accepted = 202,
		no_content = 204,
		reset_content = 205,
		partial_content = 206,
		multiple_choices = 300,
		moved_permanently = 301,
		moved_temporarily = 302,
		see_other = 303,
		not_modified = 304,
		use_proxy = 305,
		temporary_redirect = 307,
		bad_request = 400,
		unauthorized = 401,
		forbidden = 403,
		not_found = 404,
		method_not_allowed = 405,
		not_acceptable = 406,
		proxy_authentication_required = 407,
		request_timeout = 408,
		conflict = 409,
		gone = 410,
		length_required = 411,
		precondition_failed = 412,
		request_entity_too_large = 413,
		request_uri_too_long = 414,
		unsupported_media_type = 415,
		request_range_not_satisfiable = 416,
		expectation_failed = 417,
		internal_server_error = 500,
		not_implemented = 501,
		bad_gateway = 502,
		service_unavailable = 503,
		gateway_timeout = 504,
		http_version_not_supported = 505
	} status;

	vector<NameValuePair> headers; // The headers to be included in the reply
	vector<HTTPCookie> cookies; // The cookies to be sent to the client
	string content; // The content to be sent in the reply
	BufferList mReplyBuffers;

	// Statics
	static HTTPReply stockReply(StatusType status); // get a stock reply

	// Functions
	string HTTPReply::toString() const;
	void setCookie(const HTTPCookie &cookie);
	bool addHeader(const string &name, const string &value, bool overwrite = true);

	// Convert the reply into a vector of buffers. The buffers do not own the
	// underlying memory blocks, therefore the reply object must remain valid and
	// not be changed until the write operation has completed.
	const BufferList &toBuffers();

	bool statusSet() const { return (status != not_set); }

	// Constructor
	explicit HTTPReply() : status(not_set) {}
};