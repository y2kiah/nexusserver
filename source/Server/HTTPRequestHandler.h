#pragma once

#include <boost/noncopyable.hpp>
#include "TCPTypes.h"
#include "Message.h"
#include "HTTPReply.h"

class HTTPRequest;

class HTTPRequestHandler : public MessageHandler, private boost::noncopyable
{
	private:
		// Variables
		string		mDocRoot; // Resource Source name containing the web documents
		HTTPReply	mReply;

		// Functions
		// Handle a request and produce a reply
		void handleRequest(HTTPRequest &req);

		// get environment variables from request and into the cgi list
		void getCGIVars(HTTPRequest &req, const string &scriptName,
						const string &requestPath);

		explicit HTTPRequestHandler(const string &docRoot) :
			mDocRoot(docRoot)
		{}

	public:
		virtual void handleMessage(MessageParser *parser);

		virtual bool hasReply() const { return true; }

		virtual string getReply() const
		{
			return mReply.toString();
		}

		virtual const BufferList &getReplyBuffers()
		{
			return mReply.toBuffers();
		}

		virtual void setBadRequest()
		{
			// if the status is already set to something else, it was done in the parser
			// so we should not override it here and return a 500 code
			if (!mReply.statusSet()) {
				mReply = HTTPReply::stockReply(HTTPReply::bad_request);
			}
		}

		void setStockReply(HTTPReply::StatusType status)
		{
			mReply = HTTPReply::stockReply(status);
		}
		
		static HandlerPtr create(const string &docRoot)
		{
			HandlerPtr h(new HTTPRequestHandler(docRoot));
			return h;
		}

		virtual ~HTTPRequestHandler() {}
};
