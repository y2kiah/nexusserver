#pragma once

#include <vector>
#include <string>
#include <boost/logic/tribool.hpp>
#include <boost/asio/buffer.hpp>

using std::vector;
using std::string;
using boost::tribool;

class TCPConnection;

typedef vector<boost::asio::const_buffer> BufferList;

class MessageParser {
	public:
		virtual void reset() = 0;
		virtual tribool collectMessage(std::istream &is, unsigned int size, TCPConnection *cn) = 0;
		virtual const std::stringstream &getMessage() const = 0;
};

class MessageHandler {
	public:
		virtual void handleMessage(MessageParser *parser) = 0;
		virtual bool hasReply() const = 0;
		virtual string getReply() const = 0;
		virtual const BufferList &getReplyBuffers() = 0;
		virtual void setBadRequest() = 0;
};