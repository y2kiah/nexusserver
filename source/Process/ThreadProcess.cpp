/*----==== THREADPROCESS.CPP ====----
	Author:		Jeff Kiah
	Orig.Date	05/18/2009
	Rev.Date	05/25/2009
-----------------------------------*/

#include "ThreadProcess.h"

////////// class ThreadProcess //////////

/*---------------------------------------------------------------------
	This begins thread execution by contructing a new thread object,
	then calling swap to copy it into the member variable.
	**NOTE**
	A derived class can override this function if necessary, but MUST
	explicitly call this base class version of the function within it,
	or perform the same tasks in its own implementation.
---------------------------------------------------------------------*/
void ThreadProcess::onInitialize()
{
	thread startThread(&ThreadProcess::threadProc, this);
	mThread.swap(startThread);
	mThreadID = mThread.get_id();
	debugPrintf("ThreadProcess: \"%s\" started\n", name().c_str());
}

// Constructor
ThreadProcess::ThreadProcess(const string &name) :
	CProcess(name, CProcess_Run_AlwaysRun, CProcess_Queue_Single),
	mThread(), // construct a Not-a-Thread object, onInitialize() will swap in the running thread
	mThreadID(mThread.get_id()),
	mKillThread(false)
{}

// Destructor
ThreadProcess::~ThreadProcess()
{
	if (!isFinished()) {
		debugPrintf("ThreadProcess: \"%s\" destructor: finish() not called. Possible orphaned thread!\n", name().c_str());
	}
}
