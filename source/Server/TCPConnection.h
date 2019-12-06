#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio/streambuf.hpp>
#include "TCPTypes.h"

using boost::asio::io_service;
using boost::asio::ip::tcp;
using boost::asio::streambuf;
using boost::system::error_code;

// Structures

class TCPConnection : public boost::enable_shared_from_this<TCPConnection>,
					  private boost::noncopyable
{
	friend class TCPServer; // allow access to private stop()
	
	public:
		// Functions
		tcp::socket & getSocket() { return mSocket; }
		HandlerPtr & getHandler() { return mHandler; }
		unsigned int id() const { return mId; }

		void start();
		void writeReply();

		static TCPConnectionPtr create(io_service &ioService, const TCPServerPtr &server,
									   const TCPServerOptionsPtr &options);

	private:
		// Variables
		tcp::socket			mSocket;
		streambuf			mBuffer;
		TCPServerWeakPtr	mServer;
		TCPServerOptionsPtr	mOptions;
		ParserPtr			mParser;
		HandlerPtr			mHandler;
		unsigned int		mId;
		static unsigned int	nextId;

		// Functions
		void stop();
		void handleRead(const error_code &error, size_t bytesTransferred);
		void handleWrite(const error_code &error, size_t bytesTransferred);

		// Constructor
		explicit TCPConnection(io_service &ioService, const TCPServerPtr &server, const TCPServerOptionsPtr &options,
							   const ParserPtr &parser, const HandlerPtr &handler) : 
			mSocket(ioService), mServer(server), mOptions(options), mParser(parser), mHandler(handler), mId(nextId++)
		{}
};