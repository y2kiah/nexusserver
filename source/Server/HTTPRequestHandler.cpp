#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "HTTPRequestHandler.h"
#include "HTTPRequestParser.h"
#include "MimeTypes.h"
#include "HTTPReply.h"
#include "HTTPRequest.h"
#include "../Resource/ResHandle.h"
#include "WebResource.h"
#include "LuaRequestHandler.h"

using std::ifstream;
using std::size_t;
using std::for_each;

////////// class HTTPRequestHandler //////////

// Functions

void HTTPRequestHandler::handleMessage(MessageParser *parser)
{
	// We know the parser type, cast it
	HTTPRequestParser &rp = *(reinterpret_cast<HTTPRequestParser*>(parser));
	handleRequest(rp.getRequest());
}

void HTTPRequestHandler::handleRequest(HTTPRequest &req)
{
	string scriptName = req.scriptName;

	// Request path must be absolute and not contain "..".
	if (scriptName.empty() || scriptName[0] != '/' ||
		scriptName.find("..") != string::npos)
	{
		mReply = HTTPReply::stockReply(HTTPReply::bad_request);
		return;
	}

	// If directory has no trailing slash add it and return 301 with location
	size_t last_slash_pos = scriptName.find_last_of("/");
	size_t last_dot_pos = scriptName.find_last_of(".");
	if ((last_dot_pos == string::npos || last_dot_pos < last_slash_pos)
		&& scriptName[scriptName.size() - 1] != '/')
	{
		scriptName.append("/");
		mReply = HTTPReply::stockReply(HTTPReply::moved_permanently);
		mReply.addHeader("Location",scriptName);
		return;
	}

	// If path ends in slash (i.e. is a directory) then add "index.html".
	if (scriptName[scriptName.size() - 1] == '/') {
		scriptName.append("index.luap");
	}

	// Determine the file extension
	last_slash_pos = scriptName.find_last_of("/");
	last_dot_pos = scriptName.find_last_of(".");
	string extension;
	if (last_dot_pos != string::npos && last_dot_pos > last_slash_pos) {
		extension = scriptName.substr(last_dot_pos + 1);
	}

	// get request path
	string requestPath(mDocRoot + scriptName);
	std::replace(requestPath.begin(), requestPath.end(), '/', '\\');

	// Open the requested resource
	ResHandle h;
	if (!h.load<WebResource>(requestPath)) {
		mReply = HTTPReply::stockReply(HTTPReply::not_found);
		return;
	}
	WebResource &res = *(reinterpret_cast<WebResource*>(h.getResPtr().get()));

	// make extension lowercase, extensions are now case insensitive
	std::transform(extension.begin(), extension.end(), extension.begin(), tolower);

	// Write CGI environment variables
	getCGIVars(req, scriptName, requestPath);

	// Fill out the reply to be sent to the client
	// Check file extension for registered markup pre-processors
	// currently Lua is the only available language
	if (extension == "luap") {
		mReply.status = HTTPReply::ok;
		// process markup here and add to mReply.content
		LuaRequestHandler lh(string(res.dataPtr().get(), res.sizeB()),
							 req, mReply);
		lh.parse();
	} else {
		// anything else is not a dynamic page and sent as-is
		mReply.status = HTTPReply::ok;
		mReply.content.append(res.dataPtr().get(), res.sizeB());
	}

	// Add common response headers
	mReply.addHeader("Connection", "close");
	mReply.addHeader("Content-Length", boost::lexical_cast<string>(mReply.content.size()));
	mReply.addHeader("Content-Type", MimeTypes::extension_to_type(extension));

	// Add cookies to response headers
	for_each(mReply.cookies.begin(), mReply.cookies.end(),
		[this](const HTTPCookie &c) {
			mReply.addHeader("Set-Cookie", c.toString(), false);
		});
}

void HTTPRequestHandler::getCGIVars(HTTPRequest &req, const string &scriptName,
									const string &requestPath)
{
	req.cgiVars[CONTENT_LENGTH] = boost::lexical_cast<string>(req.contentLength);
	req.cgiVars[DOCUMENT_ROOT] = mDocRoot;
	req.cgiVars[HTTP_REFERER] = req.getNVPValue("Referer", req.headers);
	req.cgiVars[HTTP_USER_AGENT] = req.getNVPValue("User-Agent", req.headers);
	req.cgiVars[PATH_INFO] = (scriptName[0] == '/' ? "" : "/") +
								scriptName.substr(0, scriptName.find_last_of('/'));
	//req.cgiVars[PATH_TRANSLATED] = ;
	req.cgiVars[QUERY_STRING] = req.queryString;
	//req.cgiVars[REMOTE_ADDR] = ;
	req.cgiVars[REMOTE_HOST] = req.getNVPValue("Host", req.headers);
	req.cgiVars[REQUEST_METHOD] = req.method;
	req.cgiVars[REQUEST_URI] = req.uri;
	req.cgiVars[SCRIPT_FILENAME] = requestPath;
	req.cgiVars[SCRIPT_NAME] = scriptName;
	//req.cgiVars[SERVER_NAME] = ;
	//req.cgiVars[SERVER_PORT] = ;
	req.cgiVars[SERVER_PROTOCOL] = "HTTP";
}