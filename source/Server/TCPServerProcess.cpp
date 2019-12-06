/*----==== TCPSERVERPROCESS.CPP ====----
	Author:	Jeff Kiah
	Date:	11/05/2010
	Rev:	9/16/2011
--------------------------------------*/

#include "TCPServerProcess.h"
#include "TCPServer.h"
#include "TCPServerOptions.h"
#include "../Nexus/ControlEvents.h"

///// class TCPServerProcess /////

void TCPServerProcess::onUpdate(float deltaMillis)
{
	mServerPtr->tick();
}

void TCPServerProcess::onInitialize()
{
	mServerPtr = TCPServer::create(mOptions);
	mServerPtr->run();

	debugPrintf("%s: process initialized\n", name().c_str());
}

void TCPServerProcess::onFinish()
{
	mServerPtr->stop();
	mServerPtr.reset();

	debugPrintf("%s: process finished\n", name().c_str());
}

void TCPServerProcess::onTogglePause()
{
	if (isPaused()) {
		debugPrintf("%s: process paused\n", name().c_str());
	} else {
		debugPrintf("%s: process unpaused\n", name().c_str());
	}
}

TCPServerProcess::TCPServerProcess(const TCPServerOptionsPtr &options) :
	CProcess(options->name + "Process", CProcess_Run_CanDelay, CProcess_Queue_Multiple),
	mServerListener(options->name + "Listener", *this),
	mOptions(options)
{}

///// class TCPServerProcessListener /////

/*---------------------------------------------------------------------
	Handles event by serializing with the socket's "Archive" object and
	passing the bytes over the socket
---------------------------------------------------------------------*/
/*bool TCPServerProcess::TCPServerProcessListener::handleDigitalSwitchCreateEvent(const EventPtr &ePtr)
{
	//DigitalSwitchCreateEvent<TCPTextStream> &e = *(static_cast<DigitalSwitchCreateEvent<TCPTextStream>*>(ePtr.get()));
	//e.serialize(mSvrProc.mServerPtr->getClient() );
	//mServerPtr

	// here we need to decide which client the event is routed to, so we should first test that the event is a RemoteEvent or similar with a specific target ID
	// then we use that property of the event to find the client, and the Archive type that the client is using (text or binary), then we know which way to
	// serialize the event and send the byte stream along to the client for sending
	return true;
}*/

TCPServerProcess::TCPServerProcessListener::TCPServerProcessListener(const string &name, TCPServerProcess &svrProc) :
	EventListener(name), mSvrProc(svrProc)
{
	// register listeners with manager
	//IEventHandlerPtr p(new EventHandler<TCPServerProcessListener>(this, &TCPServerProcessListener::handleDigitalSwitchCreateEvent));
	//registerEventHandler(DigitalSwitchCreateEvent::sEventType, p, 1);
}