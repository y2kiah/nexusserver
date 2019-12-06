/*----==== APPLICATION.H ====----
	Author:	Jeff Kiah
	Date:	10/28/2010
	Rev:	9/24/2011
-------------------------------*/

#pragma once

#include "Config.h"

class HighPerfTimer;
class EventManager;
class ProcessManager;
class ResCacheManager;

class Application {
	private:
		// Variables
		HighPerfTimer		*mUpdateTimer;
		EventManager		*mEventMgr;
		ProcessManager		*mProcMgr;
		ResCacheManager		*mResCacheMgr;
		AppConfig			mConfig;

		// Functions
		bool initAdminServer();

	public:
		void process();
		void update(float deltaMillis);
		bool init();
		void deInit();

		explicit Application() :
			mUpdateTimer(0), mEventMgr(0), mProcMgr(0), mResCacheMgr(0),
			mConfig(L"app.config")
		{}

		~Application()
		{
			deInit();
		}
};