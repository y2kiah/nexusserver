 /*----==== APPLICATION.CPP ====----
	Author:	Jeff Kiah
	Date:	11/18/2010
	Rev:	11/18/2010
---------------------------------*/

#include "Application.h"
#include "../Event/EventManager.h"
#include "../Process/ProcessManager.h"
#include "../Resource/ResCache.h"
#include "../Win32/HighPerfTimer.h"
#include "../Server/TCPServerProcess.h"
#include "../Server/TCPServerOptions.h"
#include "../Server/HTTPRequestParser.h"
#include "../Server/HTTPRequestHandler.h"
#include "../Server/NexusMessageParser.h"
#include "../Server/NexusMessageHandler.h"
//#include "../Scripting/ScriptManager_Lua.h"
#include "../Resource/ZipFile.h"
#include "../Resource/FileSystemSource.h"

///// class Application /////

void Application::process()
{
	float deltaMS = mUpdateTimer->stop();
	mUpdateTimer->start();
	//if (!isPaused()) {
		update(deltaMS);
	//}
}

void Application::update(float deltaMillis)
{
	mEventMgr->notifyQueued(0);
	mProcMgr->updateProcesses(deltaMillis);
}

bool Application::init()
{
	// Load/parse config file
	if (!mConfig.load()) {
		return false;
	}

	mUpdateTimer = new HighPerfTimer();
	mUpdateTimer->start();
	mEventMgr = new EventManager();
	mProcMgr = new ProcessManager();
	// set up resource caches
	uint availableSysMemMB = mConfig.webCacheMB + mConfig.projectCacheMB + mConfig.scriptCacheMB;
	mResCacheMgr = new ResCacheManager(availableSysMemMB, 0);
	mResCacheMgr->createCache(ResCache_Web, mConfig.webCacheMB);
	mResCacheMgr->createCache(ResCache_Project, mConfig.projectCacheMB);
	mResCacheMgr->createCache(ResCache_Script, mConfig.scriptCacheMB);

	// init the admin http server
	if (!initAdminServer()) {
		return false;
	}

	// Load project files
	
	// Create projects

	// Create sim server processes for projects
	TCPServerOptionsPtr o = TCPServerOptions::create("ProjectServer",20000,
								&NexusMessageParser::create,
								&NexusMessageHandler::create,
								512, KeepAlive);
	CProcessPtr serverProcPtr(new TCPServerProcess(o)); // keep connection alive, 512 char buffer
	mProcMgr->attach(serverProcPtr);

	return true;
}

bool Application::initAdminServer()
{
	const char *adminDocRoot = "wwwroot";
	
	if (mConfig.adminUseZip) {
		// Set up admin server resource file
		ResSourcePtr srcPtr(new ZipFile(L"wwwroot/admin/admin.zip"));
		if (srcPtr->open()) {
			mResCacheMgr->registerSource(adminDocRoot, srcPtr);
		} else {
			debugPrintf("Error: wwwroot/admin/admin.zip not found\n");
			return false;
		}
	} else {
		// Set up admin server file source
		ResSourcePtr srcPtr(new FileSystemSource("wwwroot\\"));
		if (srcPtr->open()) {
			mResCacheMgr->registerSource(adminDocRoot, srcPtr);
		} else {
			return false;
		}
	}

	// Create http server process for administration page
	TCPServerOptionsPtr o = TCPServerOptions::create("AdminServer",mConfig.adminPort,
								&HTTPRequestParser::create,
								boost::bind(&HTTPRequestHandler::create, adminDocRoot),
								8192, CloseAfterMessage);
	CProcessPtr adminServerProcPtr(new TCPServerProcess(o));
	mProcMgr->attach(adminServerProcPtr);

	return true;
}

void Application::deInit()
{
	mProcMgr->clear();
	//delete mLuaMgr;
	delete mResCacheMgr;
	delete mProcMgr;
	delete mEventMgr;
	delete mUpdateTimer;
}