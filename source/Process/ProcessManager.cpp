/*----==== PROCESSMANAGER.CPP ====----
	Author:		Jeff Kiah
	Orig.Date	04/20/2007
	Rev.Date	06/14/2009
------------------------------------*/

#include "ProcessManager.h"

////////// class ProcessManager //////////

bool ProcessManager::isProcessActive(const string &procName)
{
	ProcessList::const_iterator i, end = mProcessList.end();
	for (i = mProcessList.begin(); i != end; ++i) {
		if (_stricmp((*i)->name().c_str(), procName.c_str()) == 0) return true;
	}
	return false;
}

void ProcessManager::attach(const CProcessPtr &procPtr)
{
	mProcessList.push_back(procPtr);
	procPtr->setAttached();
}

void ProcessManager::detach(const CProcessPtr &procPtr)
{
	procPtr->setAttached(false);
	debugPrintf("ProcessManager: \"%s\" process detached: %i running\n", procPtr->name().c_str(), mProcessList.size()-1);
	mProcessList.remove(procPtr); // this is expensive - o(n) - and needs to be addressed
}

void ProcessManager::updateProcesses(float deltaMillis)
{
	ProcessList::iterator i = mProcessList.begin(), end = mProcessList.end();

	while (i != end) {
		CProcessPtr &p = (*i);
		++i;
		
		// **NOTE** all of the functionality here could also be done in onFinish overrides,
		// but this provides a built-in mechanism for kicking off dependant processes
		if (p->isFinished()) {
			// check for a child process and add if exists
			CProcessPtr pNext(p->getNextProcess());
			if (pNext) {	// if the process has a child
				// erase the reference to the child process residing in the current one
				p->setNextProcess(CProcessPtr((CProcess *)NULL));
				attach(pNext);	// and kick off the new process
			}
			detach(p);		// then detach the current process

		} else if (p->isActive() && !p->isPaused()) {
			p->update(deltaMillis);
		}
	}
}

/*---------------------------------------------------------------------
	destroys all processes in the list
---------------------------------------------------------------------*/
void ProcessManager::clear()
{
	mProcessList.clear();
}
