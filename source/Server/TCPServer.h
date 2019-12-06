/*----==== TCPSERVER.H ====----
	Author:	Jeff Kiah
	Date:	10/04/2010
	Rev:	9/13/2011
-----------------------------*/

#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "TCPTypes.h"

using boost::asio::io_service;
using boost::asio::ip::tcp;
using boost::system::error_code;

class TCPServer : public boost::enable_shared_from_this<TCPServer>,
				  private boost::noncopyable
{
	private:
		// Variables
		bool					mRunning;
		io_service				mIOService;
		tcp::acceptor			mAcceptor;
		TCPConnectionList		mConnectionList;
		TCPServerOptionsPtr		mOptions;

		// Functions
		// start accepting a new connection
		void startAccept();

		// handle accepting a new connection
		void handleAccept(const TCPConnectionPtr &cn, const error_code &error);

		// handle stopping the server
		void handleStop();

		// Constructor
		explicit TCPServer(const TCPServerOptionsPtr &options);

	public:
		// Run the server's io_service loop
		void run();

		// Server's main processing loop, run once per frame
		void tick();

		// Stop the server
		void stop();

		// Close one connection
		void close(const TCPConnectionPtr &cn);

		// Accessors
		bool isRunning() const { return mRunning; }
		const TCPServerOptions &getOptions() const { return *(mOptions.get()); }

		// Create server instance
		static TCPServerPtr create(const TCPServerOptionsPtr &options)
		{
			TCPServerPtr sp(new TCPServer(options));
			return sp;
		}

		// Destructor
		~TCPServer();
};
