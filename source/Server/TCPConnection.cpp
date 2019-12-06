#include "TCPConnection.h"
#include <iostream>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>
#include "TCPServer.h"
#include "TCPServerOptions.h"
#include "Message.h"
#include "../Utility/Typedefs.h"

using namespace boost::asio;
using std::iostream;

///// class TCPConnection /////

unsigned int TCPConnection::nextId = 0;

void TCPConnection::start()
{
	error_code ec;
	mSocket.set_option(tcp::no_delay(true), ec);

	mSocket.async_read_some(mBuffer.prepare(mOptions->connectionBufferSize),
							boost::bind(&TCPConnection::handleRead, shared_from_this(),
							placeholders::error, placeholders::bytes_transferred));
	
	// start a process to identify the connection as a unique client, then create
	// a new client in the client list which refers to this connection
	// the client's ID will then be used to identify the connection throughout the server

/*	mMessage = "TEST";

	async_write(mSocket, buffer(mMessage),
				boost::bind(&TCPConnection::handleWrite, shared_from_this(),	// using boost::bind instead of std::bind
							placeholders::error,								// to work with placeholders, resolves
							placeholders::bytes_transferred));					// compiler errors
*/
}

void TCPConnection::stop()
{
	if (mSocket.is_open()) {
		debugPrintf("%s: \"%u\" connection closed: %s\n",
			mOptions->name.c_str(), mId,
			mSocket.remote_endpoint().address().to_string().c_str());
		mSocket.close();
	}
}

void TCPConnection::writeReply()
{
	async_write(mSocket, mHandler->getReplyBuffers(),
				bind(&TCPConnection::handleWrite, shared_from_this(),
					placeholders::error, placeholders::bytes_transferred));
	//debugPrintf("\n\"%u\" sent:\n%s\n", mId, mHandler->getReply().c_str()); // TEMP
}

void TCPConnection::handleRead(const error_code &error, size_t bytesTransferred)
{
	if (!error) {
		mBuffer.commit(bytesTransferred);
		iostream ios(&mBuffer);

		//debugPrintf("\n\"%u\" collecting message\n", mId);
		boost::tribool result =
			mParser->collectMessage(ios, bytesTransferred, this);

		if (result) { // parsed a message successfully
			//debugPrintf("\n\"%u\" message received\n", mId, mParser->getMessage().str().c_str());
			mHandler->handleMessage(mParser.get());
			
			if (mHandler->hasReply()) {
				writeReply();
			}
			mParser->reset();

		} else if (!result) { // parsed a complete but invalid message
			mHandler->setBadRequest();
			writeReply();
		}
		
		// continue receiving data, connection does not die
		if (indeterminate(result) || mOptions->connDefault == KeepAlive) {
			mSocket.async_read_some(mBuffer.prepare(mOptions->connectionBufferSize),
				boost::bind(&TCPConnection::handleRead, shared_from_this(),
					placeholders::error, placeholders::bytes_transferred));
		} else {
			// Initiate graceful connection closure
			error_code ec;
			mSocket.shutdown(tcp::socket::shutdown_both, ec);
			TCPServerPtr server(mServer);
			server->close(shared_from_this());
		}
	} else if (error != error::operation_aborted) {
		debugPrintf("\n\"%u\" error \"%s\"\n", mId, error.message().c_str());
		TCPServerPtr server(mServer);

		server->close(shared_from_this());
	}
}

void TCPConnection::handleWrite(const error_code &error, size_t bytesTransferred)
{
	/*if (!error) {
		// Initiate graceful connection closure
		error_code ec;
		mSocket.shutdown(tcp::socket::shutdown_both, ec);
		debugPrintf("\n\"%u\" closing socket due to error: %s\n", mId, error.message().c_str());
	}

	if (error != error::operation_aborted) {
		TCPServerPtr server(mServer);
		server->close(shared_from_this());
		debugPrintf("\n\"%u\" operation aborted\n", mId);
	}*/

	if (error) {
		// Close connection
		error_code ec;
		mSocket.shutdown(tcp::socket::shutdown_both, ec);
		TCPServerPtr server(mServer);
		server->close(shared_from_this());
		debugPrintf("\n\"%u\" closing socket due to error: %s\n", mId, error.message().c_str());
	}
}

TCPConnectionPtr TCPConnection::create(io_service &ioService, const TCPServerPtr &server,
									   const TCPServerOptionsPtr &options)
{
	// get a parser and handler instance for the next connection
	ParserPtr parser(options->createParser());
	HandlerPtr handler(options->createHandler());

	TCPConnectionPtr cp(new TCPConnection(ioService, server, options, parser, handler));
	return cp;
}