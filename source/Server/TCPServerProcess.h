/*----==== TCPSERVERPROCESS.H ====----
	Author:	Jeff Kiah
	Date:	10/30/2010
	Rev:	9/16/2011
------------------------------------*/

#pragma once

#include "../Process/ProcessManager.h"
#include "../Event/EventListener.h"
#include "TCPTypes.h"

///// STRUCTURES /////

/*=====================================================================
class TCPServerProcess
=====================================================================*/
class TCPServerProcess : public CProcess
{
	friend class TCPServerProcessListener;

	private:
		///// STRUCTURES /////
		/*=====================================================================
		class TCPServerProcessListener
		=====================================================================*/
		class TCPServerProcessListener : public EventListener {
			friend class TCPServerProcess;
			private:
				///// VARIABLES /////
				TCPServerProcess &mSvrProc;

				///// FUNCTIONS /////
				//bool handleDigitalSwitchCreateEvent(const EventPtr &ePtr);

			public:
				explicit TCPServerProcessListener(const string &name, TCPServerProcess &svrProc);
		};

		///// VARIABLES /////
		TCPServerPtr				mServerPtr;
		TCPServerProcessListener	mServerListener;
		TCPServerOptionsPtr			mOptions;

	public:
		///// FUNCTIONS /////
		virtual void onUpdate(float deltaMillis);
		virtual void onInitialize();
		virtual void onFinish();
		virtual void onTogglePause();

		explicit TCPServerProcess(const TCPServerOptionsPtr &options);
};