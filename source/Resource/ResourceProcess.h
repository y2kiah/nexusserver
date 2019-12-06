/*----==== RESOURCEPROCESS.H ====----
	Author:		Jeff Kiah
	Orig.Date:	05/25/2009
	Rev.Date:	06/10/2009
-----------------------------------*/

#pragma once

#include <string>
#include "../Process/ThreadProcess.h"
#include "../Event/EventListener.h"

using std::string;
using boost::checked_array_deleter;

class IResourceSource;

///// STRUCTURES /////

/*=====================================================================
class AsyncLoadEvent
=====================================================================*/
class AsyncLoadEvent : public Event {
	public:
		typedef shared_ptr<IResourceSource>		ResSourcePtr;

		///// VARIABLES /////
		static const string sEventType;

		string			mResName;		// the file to load from the source object
		string			mSourceName;	// the name of the ResourceSource
		ResSourcePtr	mSourcePtr;		// shared_ptr to the ResourceSource

		///// FUNCTIONS /////
		const string &	type() const { return sEventType; }
		//void	serialize(ostream &out) const {}
		//void	deserialize(istream &in) {}
		//void	buildScriptData();

		// Constructor / destructor
		explicit AsyncLoadEvent(const string &resName, const string &sourceName,
								const ResSourcePtr &sourcePtr) :
			Event(),
			//ScriptableEvent(),
			mResName(resName),
			mSourceName(sourceName),
			mSourcePtr(sourcePtr)
		{}
		//explicit AsyncLoadEvent(const AnyVars &eventData);
		virtual ~AsyncLoadEvent() {}
};

/*=====================================================================
class AsyncLoadDoneEvent
=====================================================================*/
class AsyncLoadDoneEvent : public Event {
	public:
		typedef shared_ptr<char>	BufferPtr; // use checked_array_deleter<char> to ensure delete[] called

		///// VARIABLES /////
		static const string sEventType;
		bool		mSuccess;		// true if decompression successful
		int			mSize;			// size of the buffer array
		string		mResName;		// the resource path
		string		mSourceName;	// the name of the ResourceSource
		BufferPtr	mDataPtr;		// the buffer containing data

		///// FUNCTIONS /////
		const string &	type() const { return sEventType; }
		//void	serialize(ostream &out) const {}
		//void	deserialize(istream &in) {}

		// Constructor / destructor
		explicit AsyncLoadDoneEvent(const string &resName, const string &sourceName,
									const BufferPtr &bPtr, int size, bool success = true) :
			Event(),
			mResName(resName), mSourceName(sourceName), mDataPtr(bPtr),
			mSize(size), mSuccess(success)
		{}
		virtual ~AsyncLoadDoneEvent() {}
};

/*=============================================================================
class AsyncLoadProcess
=============================================================================*/
class AsyncLoadProcess : public ThreadProcess {
	private:
		///// DEFINITIONS /////
		typedef	hash_map<string, int>	ThreadIndexMap;

		///// STRUCTURES /////
		/*=====================================================================
		class AsyncLoadListener
		=====================================================================*/
		class AsyncLoadListener : public EventListener {
			friend class AsyncLoadProcess;
			private:
				IEventHandlerPtr	mAsyncLoadHandler;	// we need to hang on to a shared_ptr to the
														// handler for access to its stored queue
			public:
				explicit AsyncLoadListener() :
					EventListener("AsyncLoadListener"),
					mAsyncLoadHandler(new ThreadEventHandler())
				{
					registerEventHandler(AsyncLoadEvent::sEventType, mAsyncLoadHandler, 1);
					registerEventHandler(sAsyncLoadShutdownEvent, mAsyncLoadHandler, 1);
				}
		};

		///// VARIABLES /////
		AsyncLoadListener	mEventListener;
		ThreadIndexMap		mSourceThreadIndexMap;	// for each ResSource, the threadIndex assigned to this thread

		///// FUNCTIONS /////
		void onUpdate(float deltaMillis) {}

		void threadProc();

	public:
		static const string sAsyncLoadShutdownEvent;

		explicit AsyncLoadProcess(const string &name);
		~AsyncLoadProcess();
};