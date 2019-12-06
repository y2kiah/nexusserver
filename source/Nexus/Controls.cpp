/*----==== CONTROLS.CPP ====----
	Author: Jeffrey Kiah
	Orig.Date: 11/10/2010
	Rev.Date:  11/10/2010
------------------------------*/

#pragma once

#include "Controls.h"

// class Control
Control::~Control()
{}

// class DigitalSwitch
DigitalSwitch::DigitalSwitch(short id, uchar numPins, const vector<uchar> &pins, bool offEvent,
							 char activeLevel, bool useInternalPullup, short debounceDelay) :
	Control(id), mNumPins(numPins), mPins(pins), mOffEvent(offEvent),
	mActiveLevel(activeLevel), mUseInternalPullup(useInternalPullup),
	mDebounceDelay(debounceDelay)
{}
