/*----==== PROCESSMANAGER.H ====----
	Author:		Jeff Kiah
	Orig.Date:	04/20/2007
	Rev.Date:	06/14/2009
----------------------------------*/

#pragma once

#include <string>
#include <list>
#include <bitset>
#include <boost/noncopyable.hpp>
#include <memory>
#include "../Utility/Typedefs.h"
#include "../Utility/Singleton.h"

using std::string;
using std::list;
using std::bitset;
using std::shared_ptr;

///// DEFINITIONS /////

#define	procMgr		ProcessManager::instance()

// ** NOTE **
// The settings controlled by these enums are not implemented yet

enum CProcessQueueMode : uchar {
	CProcess_Queue_Multiple = 0,	// will allow multiple processes of same type in the list
	CProcess_Queue_Single,			// only allow 1 process of type in the list at a time
	CProcess_Queue_Single_Replace	// upon submission, will end and replace a process of same type
};

enum CProcessRunMode : uchar {
	CProcess_Run_AlwaysRun = 0,		// will run the process every frame regardless of time
	CProcess_Run_CanDelay			// if max frame time exceeded, will run in a later frame
};

class CProcess;
typedef shared_ptr<CProcess>	CProcessPtr;
typedef list<CProcessPtr>		ProcessList;

///// STRUCTURES /////

/*=============================================================================
class CProcess
=============================================================================*/
class CProcess : private boost::noncopyable {
	friend class ConcurrentManager;

	protected:
		///// VARIABLES /////
		bitset<5>			mProcessFlags;
		CProcessRunMode		mRunMode;
		CProcessQueueMode	mQueueMode;
		CProcessPtr			mNext;
		const string		mName;

		///// DEFINITIONS /////
		enum CProcessFlagBits : size_t {
			bitFinished = 0,
			bitActive,
			bitPaused,
			bitInitialized,
			bitAttached
		};

		///// FUNCTIONS /////
		/*---------------------------------------------------------------------
			As part of a cooperative multitasking system, these functions are
			responsible for returning control to the main loop without hogging
			CPU. These are pure virtual and must be overridden in derived
			classes.
		---------------------------------------------------------------------*/
		virtual void	onUpdate(float deltaMillis) = 0;
		virtual void	onInitialize() = 0;
		virtual void	onFinish() = 0;
		virtual void	onTogglePause() = 0;

	public:
		/*---------------------------------------------------------------------
			This function is called to perform the main task of the process. If
			the process has not been initialized, it will be prior to calling
			onUpdate() for the first time.
		---------------------------------------------------------------------*/
		void	update(float deltaMillis) {
			if (!isInitialized()) {
				onInitialize();
				setInitialized();
			}
			onUpdate(deltaMillis);
		}

		// Getters and setters
		bool	isFinished() const	{ return mProcessFlags[bitFinished]; }
		void	finish()			{ mProcessFlags[bitFinished] = true; onFinish(); }

		bool	isActive() const	{ return mProcessFlags[bitActive]; }
		void	setActive(bool b = true) { mProcessFlags[bitActive] = b; }

		bool	isAttached() const	{ return mProcessFlags[bitAttached]; }
		void	setAttached(bool b = true) { mProcessFlags[bitAttached] = b; }

		bool	isPaused() const	{ return mProcessFlags[bitPaused]; }
		void	togglePause()		{ mProcessFlags.flip(bitPaused); onTogglePause(); }
		
		bool	isInitialized() const { return mProcessFlags[bitInitialized]; }
		void	setInitialized()	{ mProcessFlags[bitInitialized] = true; }

		const CProcessPtr & getNextProcess() const { return mNext; }
		void	setNextProcess(const CProcessPtr &procPtr) {
			_ASSERTE((procPtr.get() != this) && "Don't chain a process to itself!");
			if (procPtr.get() != this) { mNext = procPtr; }
		}
		
		const string &	name() const { return mName; }

		// Constructor / destructor
		explicit CProcess(const string &name,
						  CProcessRunMode runMode = CProcess_Run_CanDelay,
						  CProcessQueueMode queueMode = CProcess_Queue_Multiple) :
			mName(name), mRunMode(runMode), mQueueMode(queueMode),
			mProcessFlags(0), mNext(CProcessPtr((CProcess *)NULL))
		{
			mProcessFlags[bitActive] = true;
		}
		virtual ~CProcess() {}
};

/*=============================================================================
class ProcessManager
=============================================================================*/
class ProcessManager : public Singleton<ProcessManager> {
	private:
		///// VARIABLES /////
		ProcessList		mProcessList;

		// also need a multimap for random searches for processes and detaching

		///// FUNCTIONS /////
		void	detach(const CProcessPtr &procPtr);

	public:
		bool	isProcessActive(const string &procName);
		bool	hasProcesses() const	{ return !mProcessList.empty(); }

		void	attach(const CProcessPtr &procPtr);

		void	updateProcesses(float deltaMillis);

		/*---------------------------------------------------------------------
			destroys all processes in the list
		---------------------------------------------------------------------*/
		void	clear();

		explicit ProcessManager() : Singleton<ProcessManager>(*this) {}
		~ProcessManager() {}
};