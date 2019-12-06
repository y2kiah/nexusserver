/*----==== HIGHPERFTIMER.CPP ====----
	Author:	Jeffrey Kiah
	Date:	12/1/2007
-----------------------------------*/

#include "HighPerfTimer.h"

///// STATIC VARIABLES /////

__int64 HighPerfTimer::sTimerFreq = 0;
float HighPerfTimer::sSecondsPerCount;
float HighPerfTimer::sMillisecondsPerCount;
#ifdef _DEBUG
bool HighPerfTimer::sInitialized = false;
#endif

///// STATIC FUNCTIONS /////

bool HighPerfTimer::initHighPerfTimer()
{
	// get high performance counter frequency
	BOOL result = QueryPerformanceFrequency((LARGE_INTEGER *)&sTimerFreq);
	if (result == 0 || sTimerFreq == 0) {
		debugPrintf("Timer::initTimer: QueryPerformanceFrequency failed (error %d)\n", GetLastError());
		return false;
	}

	sSecondsPerCount = 1.0f / static_cast<float>(sTimerFreq);
	sMillisecondsPerCount = sSecondsPerCount * 1000.0f;

	// test counter function
	__int64 dummy = 0;
	result = QueryPerformanceCounter((LARGE_INTEGER *)&dummy);
	if (result == 0) {
		debugPrintf("Timer::initTimer: QueryPerformanceCounter failed (error %d)\n", GetLastError());
		return false;
	}

	#ifdef _DEBUG
	sInitialized = true;
	#endif

	return true;
}