#pragma once

#include <string>
#include <memory>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/function.hpp>

#define DFLT_BUFFER_SIZE	512

using std::string;
using std::set;
using boost::function;

class TCPServer;
class TCPConnection;
class TCPServerOptions;
class MessageParser;
class MessageHandler;

typedef boost::shared_ptr<TCPServer>		TCPServerPtr;
typedef boost::weak_ptr<TCPServer>			TCPServerWeakPtr;
typedef boost::shared_ptr<TCPConnection>	TCPConnectionPtr;
typedef boost::weak_ptr<TCPConnection>		TCPConnectionWeakPtr;
typedef set<TCPConnectionPtr>				TCPConnectionList;
typedef std::shared_ptr<TCPServerOptions>	TCPServerOptionsPtr;
typedef std::shared_ptr<TCPServerOptions>	TCPServerOptionsPtr;
typedef std::shared_ptr<MessageParser>		ParserPtr;
typedef std::shared_ptr<MessageHandler>		HandlerPtr;
typedef function<ParserPtr()>				CreateParserFuncPtr;
typedef function<HandlerPtr()>				CreateHandlerFuncPtr;

enum TCPConnectionSettings {
	KeepAlive = 0,
	CloseAfterMessage
};
