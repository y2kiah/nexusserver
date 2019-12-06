/*----==== HIGHPERFTIMER.H ====----
	Author:		Jeff Kiah
	Orig.Date:	12/01/2007
	Rev.Date:	05/22/2009
---------------------------------*/

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN	// defined in project settings
#endif

#include <Windows.h>
#include <cmath>
#include <boost/noncopyable.hpp>
#include "../Utility/Typedefs.h"

///// DEFINES /////

#define DISCREPANCY_MS_CHECK	100

///// STRUCTURES /////

/*=============================================================================
class HighPerfTimer
=============================================================================*/
class HighPerfTimer : private boost::noncopyable {
	private:
		///// VARIABLES /////

		// Static
		static __int64	sTimerFreq;
		static float	sSecondsPerCount;
		static float	sMillisecondsPerCount;
		#ifdef _DEBUG
		static bool		sInitialized;	// in debug mode asserts make sure static functions aren't called before init
		#endif

		// Instance
		__int64		mStartCounts, mStopCounts, mCountsPassed; // set by QPC for high res timing
		DWORD		mStartTickCount, mStopTickCount; // set by GetTickCount to check for QPC jumps
		float		mMillisecondsPassed;
		float		mSecondsPassed;

	public:
		///// FUNCTIONS /////

		// Static
		#ifdef _DEBUG
		static bool		initialized() { return sInitialized; }
		#endif
		static __int64	timerFreq() { return sTimerFreq; }
		static float	secondsPerCount() { return sSecondsPerCount; }
		static __int64	queryCounts() {
							_ASSERTE(sInitialized);
							__int64 now = 0;
							QueryPerformanceCounter((LARGE_INTEGER *)&now);
							return now;
						}
		static __int64	countsSince(__int64 startCounts) {
							_ASSERTE(sInitialized);
							__int64 now = 0;
							QueryPerformanceCounter((LARGE_INTEGER *)&now);
							return now - startCounts;
						}
		static float	secondsSince(__int64 startCounts) {
							_ASSERTE(sInitialized);
							__int64 now = 0;
							QueryPerformanceCounter((LARGE_INTEGER *)&now);
							return static_cast<float>(now - startCounts) * sSecondsPerCount;
						}
		static float	secondsBetween(__int64 startCounts, __int64 stopCounts) {
							_ASSERTE(sInitialized);
							return static_cast<float>(stopCounts - startCounts) * sSecondsPerCount;
						}
		static bool		initHighPerfTimer();

		// Instance
		__int64		startCounts() const			{ return mStartCounts; }
		__int64		stopCounts() const			{ return mStopCounts; }
		__int64		countsPassed() const		{ return mCountsPassed; }
		float		millisecondsPassed() const	{ return mMillisecondsPassed; }
		float		secondsPassed() const		{ return mSecondsPassed; }

		//float		getDeltaT() {
						/*_ASSERTE(sInitialized);
						
						__int64 currCounts;
						DWORD currTicks;
						QueryPerformanceCounter((LARGE_INTEGER *)&currCounts);
						currTicks = GetTickCount();
						
						__int64 perfDelta = ((currCounts - prevCounts) * 1000) / sTimerFreq;
						static_cast<DWORD>((perfDelta * 1000) / sTimerFreq);
						DWORD tickDelta = currTicks - prevTicks;*/
						//return 0; // dummy
					//}
		void		start() {
						_ASSERTE(sInitialized);
						QueryPerformanceCounter((LARGE_INTEGER *)&mStartCounts);
						mStartTickCount = GetTickCount();
						mStopCounts = mStartCounts;
						mStopTickCount = mStartTickCount;
						mCountsPassed = 0;
						mMillisecondsPassed = 0.0f;
						mSecondsPassed = 0.0f;
					}
		float		stop() {
						_ASSERTE(sInitialized);
						// query the current counts from QPC and GetTickCount
						QueryPerformanceCounter((LARGE_INTEGER *)&mStopCounts);
						mStopTickCount = GetTickCount();
						// get time passed since start() according to QPC and GetTickCount
						mCountsPassed = mStopCounts - mStartCounts;
						mMillisecondsPassed = (mCountsPassed * sMillisecondsPerCount);
						DWORD ticksPassed = mStopTickCount - mStartTickCount;
						// find the difference between the two clocks
						float diff = mMillisecondsPassed - ticksPassed;
						if (abs(diff) > DISCREPANCY_MS_CHECK) { // check for discrepancy > X ms
							// if the discrepancy is large, QPC probably skipped so we should trust GetTickCount
							debugPrintf("Timer::stop: QPC discrepency detected (difference %ims)\n", diff);
							mMillisecondsPassed = static_cast<float>(ticksPassed);
							mSecondsPassed = ticksPassed * 0.001f;
						} else {
							mSecondsPassed = static_cast<float>(mCountsPassed) * sSecondsPerCount;
						}
						return mMillisecondsPassed;
					}
		void		reset() {
						mStartCounts = mStopCounts = mCountsPassed = 0;
						mStartTickCount = mStopTickCount = 0;
						mMillisecondsPassed = mSecondsPassed = 0.0f;
					}
		__int64		currentCounts() const {
						_ASSERTE(sInitialized);
						__int64 now = 0;
						QueryPerformanceCounter((LARGE_INTEGER *)&now);
						return now - mStartCounts;
					}
		float		currentSeconds() const {
						_ASSERTE(sInitialized);
						__int64 now = 0;
						QueryPerformanceCounter((LARGE_INTEGER *)&now);
						return static_cast<float>(now - mStartCounts) * sSecondsPerCount;
					}

		// Constructor

		explicit HighPerfTimer() :
			mStartCounts(0), mStopCounts(0), mCountsPassed(0), mMillisecondsPassed(0.0f),
			mSecondsPassed(0.0f), mStartTickCount(0), mStopTickCount(0)
		{}
};