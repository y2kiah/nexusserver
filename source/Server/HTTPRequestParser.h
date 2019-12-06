#pragma once

#include <sstream>
#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/noncopyable.hpp>
#include "TCPTypes.h"
#include "Message.h"
#include "HTTPRequest.h"

using boost::tribool;
using boost::tuple;

/// Parser for incoming requests.
class HTTPRequestParser : public MessageParser, private boost::noncopyable
{
	public:
		/// Reset to initial parser state.
		virtual void reset();

		virtual tribool collectMessage(std::istream &is, unsigned int size, TCPConnection *cn)
		{
			mConnection = cn;
			tribool result;
			boost::tie(result, boost::tuples::ignore) = parse(is, size);
			return result;
		}

		virtual const std::stringstream &getMessage() const
		{
			return mMsg;
		}

		HTTPRequest &getRequest() { return mRequest; }
		
		// Perform URL-decoding on a string. Returns false if the encoding was invalid.
		static bool urlDecode(const string &in, string &out);

		static ParserPtr create()
		{
			ParserPtr p(new HTTPRequestParser());
			return p;
		}

		virtual ~HTTPRequestParser() {}

	private:
		// Variables
		std::stringstream mMsg;
		HTTPRequest	mRequest;
		TCPConnection *mConnection; // this is set by calls to collectMessage, and used
									// within consume to send specific codes back to client
		// Functions

		/// Parse some data. The tribool return value is true when a complete request
		/// has been parsed, false if the data is invalid, indeterminate when more
		/// data is required. The InputIterator return value indicates how much of the
		/// input has been consumed.
		tuple<tribool, unsigned int> parse(std::istream &is, unsigned int size);

		bool parseURI();
		bool parseNVP(const string &nvpString, NVPList &list);

		/// Handle the next character of input.
		tribool consume(char input);

		/// Check if a byte is an HTTP character.
		static bool is_char(int c);

		/// Check if a byte is an HTTP control character.
		static bool is_ctl(int c);

		/// Check if a byte is defined as an HTTP tspecial character.
		static bool is_tspecial(int c);

		/// Check if a byte is a digit.
		static bool is_digit(int c);

		/// The current state of the parser.
		enum State {
			method_start,
			method,
			uri_start,
			uri,
			content,
			http_version_h,
			http_version_t_1,
			http_version_t_2,
			http_version_p,
			http_version_slash,
			http_version_major_start,
			http_version_major,
			http_version_minor_start,
			http_version_minor,
			expecting_newline_1,
			header_line_start,
			header_lws,
			header_name,
			space_before_header_value,
			header_value,
			expecting_newline_2,
			expecting_newline_3
		} state;

		// Private constructor
		explicit HTTPRequestParser();
};