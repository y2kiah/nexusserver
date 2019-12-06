/*----==== RESOURCEPROCESS.CPP ====----
	Author:		Jeff Kiah
	Orig.Date:	05/25/2009
	Rev.Date:	06/10/2009
-------------------------------------*/

#include "ResourceProcess.h"
#include "../Event/EventManager.h"
#include "../Event/RegisteredEvents.h"
#include "ZipFile.h"

////////// class AsyncLoadEvent //////////

const string AsyncLoadEvent::sEventType("SYS_RES_ASYNCLOAD");

/*void AsyncLoadEvent::buildScriptData()
{
	mScriptData.clear();
	mScriptData.push_back(AnyVarsValue("mZipFileName", mZipFileName));
	mScriptData.push_back(AnyVarsValue("mFileName", mFileName));
	mScriptDataBuilt = true;
}

AsyncLoadEvent::AsyncLoadEvent(const AnyVars &eventData) :
	ScriptableEvent(eventData)
{
	for (AnyVars::const_iterator i = eventData.begin(); i != eventData.end(); ++i) {
		try {
			if (_stricmp(i->first.c_str(), "mZipFileName") == 0) {
				mZipFileName = any_cast<wstring>(i->second);
			} else if (_stricmp(i->first.c_str(), "mFileName") == 0) {
				mFileName = any_cast<string>(i->second);
			}
		} catch (const boost::bad_any_cast &ex) {
			// nothing happens with a bad datatype in release build, silently ignores
			debugPrintf("AsyncLoadEvent: bad_any_cast \"%s\"\n", ex.what());
		}
	}
}*/

////////// class AsyncLoadDoneEvent //////////

const string AsyncLoadDoneEvent::sEventType("SYS_RES_ASYNCLOAD_DONE");

////////// class AsyncLoadProcess //////////

const string AsyncLoadProcess::sAsyncLoadShutdownEvent("SYS_RES_ASYNCLOAD_SHUTDOWN");

void AsyncLoadProcess::threadProc()
{
	while (!threadKilled()) {
		ThreadEventHandler &h = *((ThreadEventHandler *)mEventListener.mAsyncLoadHandler.get());
		EventPtr ePtr;
		h.mEventQueue.waitPop(ePtr);
		// condition variable causes the process to sit idle until an event is in the queue,
		// so a shutdown event could wake the thread and then exit. If load events
		// are still queued, threadKilled() returning true could also cause an exit
		if (ePtr->type() == sAsyncLoadShutdownEvent) break;

		// if it's not a shutdown event, we know it's a decompression event
		AsyncLoadEvent &e = *(static_cast<AsyncLoadEvent*>(ePtr.get()));

		int threadIndex = -1;
		// find the threadIndex in our source map, or call getNewThreadIndex if it doesn't exist yet
		ThreadIndexMap::const_iterator i = mSourceThreadIndexMap.find(e.mSourceName);
		if (i == mSourceThreadIndexMap.end()) {	// not found in the hash_map
			threadIndex = e.mSourcePtr->getNewThreadIndex();	// request a threadIndex from the ResourceSource
			mSourceThreadIndexMap[e.mSourceName] = threadIndex;	// store in the hash_map for future reference
		} else {
			threadIndex = i->second;	// found in map, get the stored threadIndex
		}
		debugPrintf("GOT THREADINDEX = %i\n\n", threadIndex); // TEMP

		bool success = false;
		BufferPtr dataPtr((char *)0);
		int size = 0;
		// threadIndex -1 means there was an error opening the file
		if (threadIndex != -1) {
			// load from source
			size = e.mSourcePtr->getResource(e.mResName, dataPtr, threadIndex);
			if (size) {
				success = true;
				debugPrintf("%s: async load \"%s\": success=%i\n", name().c_str(), e.mResName.c_str(), success);
			}
		}
		// send async load result event
		EventPtr doneEventPtr(new AsyncLoadDoneEvent(e.mResName, e.mSourceName, dataPtr, size, success));
		eventMgr.raiseThreadSafe(doneEventPtr);
	}
}

AsyncLoadProcess::AsyncLoadProcess(const string &name) :
	ThreadProcess(name)
{
	// register the decompress event
	eventMgr.registerEventType(AsyncLoadEvent::sEventType,
							 //RegEventPtr(new ScriptCallableCodeEvent<AsyncLoadEvent>(EventDataType_NotEmpty)));
							 RegEventPtr(new CodeOnlyEvent(EventDataType_NotEmpty)));
	// register the decompress done event
	eventMgr.registerEventType(AsyncLoadDoneEvent::sEventType,
							 RegEventPtr(new CodeOnlyEvent(EventDataType_NotEmpty)));
	// register the exit thread event
	eventMgr.registerEventType(sAsyncLoadShutdownEvent,
							 RegEventPtr(new CodeOnlyEvent(EventDataType_Empty)));
}

AsyncLoadProcess::~AsyncLoadProcess()
{
	killThread();	// request thread to shut down
	// send shutdown event to wake up the thread and allow it to exit incase it's idle
	eventMgr.trigger(sAsyncLoadShutdownEvent);
	finish();	// ensures the main thread will wait for thread to join
}
