/*----==== TCPSERVER.CPP ====----
	Author:	Jeff Kiah
	Date:	10/04/2010
	Rev:	9/16/2011
-------------------------------*/

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "../Utility/Typedefs.h"
#include "TCPServer.h"
#include "TCPConnection.h"
#include "TCPServerOptions.h"

using namespace boost::asio;

///// class TCPServer /////

void TCPServer::startAccept()
{
	// create the new connection instance
	TCPConnectionPtr cn(TCPConnection::create(mAcceptor.get_io_service(), shared_from_this(), mOptions));
	// start listening to accept the next connection into this instance
	mAcceptor.async_accept(cn->getSocket(),
						   boost::bind(&TCPServer::handleAccept, this, cn, placeholders::error));
}

void TCPServer::handleAccept(const TCPConnectionPtr &cn, const error_code &error)
{
    if (!error) {
		mConnectionList.insert(cn); // add the accepted connection to the list
		cn->start(); // start handling the connection
		debugPrintf("\n%s: \"%u\" connection accepted: %s\n",
			mOptions->name.c_str(), cn->id(),
			cn->getSocket().remote_endpoint().address().to_string().c_str());
		startAccept(); // start accepting a new connection
    } else {
		debugPrintf("\n%s: error accepting connection: %s\n",
			mOptions->name.c_str(), error.message().c_str());
	}
}

// Run the server's io_service loop
void TCPServer::run()
{
	if (mRunning) { return; }
	startAccept();
	mRunning = true;
	debugPrintf("\n%s: server running...\n", mOptions->name.c_str());
}

// Server's main processing loop, run once per frame
void TCPServer::tick()
{
	if (!mRunning) { return; }
	mIOService.poll();
}

// Stop the server
void TCPServer::stop()
{
	if (!mRunning) { return; }
	// The server is stopped by cancelling all outstanding asynchronous operations
	mAcceptor.close();
	std::for_each(mConnectionList.begin(), mConnectionList.end(),
				  bind(&TCPConnection::stop, _1));
	mConnectionList.clear();
	mIOService.run();
	mRunning = false;
	debugPrintf("\n%s: server stopped\n", mOptions->name.c_str());
}

// Stop one connection
void TCPServer::close(const TCPConnectionPtr &cn)
{
	mConnectionList.erase(cn);
	cn->stop();
}

// Constructor
TCPServer::TCPServer(const TCPServerOptionsPtr &options) :
	mRunning(false),
	mIOService(),
	mAcceptor(mIOService, tcp::endpoint(tcp::v4(), options->port), true),
	mOptions(options)
{}

// Destructor
TCPServer::~TCPServer()
{
	stop();
}