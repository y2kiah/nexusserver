/*----==== CONTROLEVENTS.CPP ====----
	Author: Jeffrey Kiah
	Orig.Date: 11/10/2010
	Rev.Date:  11/10/2010
-----------------------------------*/

#include "ControlEvents.h"
#include "../Event/RegisteredEvents.h"

#include <boost/archive/text_oarchive.hpp> // TEMP

// register event types with manager, call once on application startup
void registerEventTypes()
{
	// Outgoing events (server to client)
	eventMgr.registerEventType(DigitalSwitchCreateEvent::sEventType,
								RegEventPtr(new CodeOnlyEvent(EventDataType_NotEmpty)));

	eventMgr.registerEventType(AnalogControlCreateEvent::sEventType,
								RegEventPtr(new CodeOnlyEvent(EventDataType_NotEmpty)));

	eventMgr.registerEventType(EncoderCreateEvent::sEventType,
								RegEventPtr(new CodeOnlyEvent(EventDataType_NotEmpty)));

	eventMgr.registerEventType(KeyMatrixCreateEvent::sEventType,
								RegEventPtr(new CodeOnlyEvent(EventDataType_NotEmpty)));

	// Incoming events (client to server)
	// use RemoteCallableEvent as the registered type
	eventMgr.registerEventType(DigitalSwitchEvent::sEventType,
								RegEventPtr(new RemoteCallableEvent<DigitalSwitchEvent, boost::archive::text_oarchive>(
													EventDataType_NotEmpty)));

	eventMgr.registerEventType(AnalogControlEvent::sEventType,
								RegEventPtr(new RemoteCallableEvent<AnalogControlEvent, boost::archive::text_oarchive>(
													EventDataType_NotEmpty)));

	eventMgr.registerEventType(EncoderEvent::sEventType,
								RegEventPtr(new RemoteCallableEvent<EncoderEvent, boost::archive::text_oarchive>(
													EventDataType_NotEmpty)));

	eventMgr.registerEventType(KeyMatrixEvent::sEventType,
								RegEventPtr(new RemoteCallableEvent<KeyMatrixEvent, boost::archive::text_oarchive>(
													EventDataType_NotEmpty)));
}

// class DigitalSwitchCreateEvent

const string DigitalSwitchCreateEvent::sEventType("createDigitalSwitch");

DigitalSwitchCreateEvent::DigitalSwitchCreateEvent(int targetClientId) :
	RemoteEvent(targetClientId), mNumPins(2), mPins(2), mOffEvent(true), mActiveLevel(0),
	mUseInternalPullup(false), mDebounceDelay(50)
{}

// class DigitalSwitchEvent
const string DigitalSwitchEvent::sEventType("ds");

DigitalSwitchEvent::DigitalSwitchEvent(int targetClientId) :
	RemoteEvent(targetClientId), mControlId(0), mActivePos(0)
{}

// class AnalogControlCreateEvent

const string AnalogControlCreateEvent::sEventType("createAnalogControl");

AnalogControlCreateEvent::AnalogControlCreateEvent(int targetClientId) :
	RemoteEvent(targetClientId), mPin(0), mChangeThreshold(1), mInterval(50)
{}

// class AnalogControlEvent
const string AnalogControlEvent::sEventType("an");

AnalogControlEvent::AnalogControlEvent(int targetClientId) :
	RemoteEvent(targetClientId), mControlId(0), mRawValue(0)
{}

// class EncoderCreateEvent

const string EncoderCreateEvent::sEventType("createEncoder");

EncoderCreateEvent::EncoderCreateEvent(int targetClientId) :
	RemoteEvent(targetClientId), mPinA(0), mPinB(0), mInterruptNumber(1), mBits(2),
	mInterval(50), mGrayCode(false), mUseInternalPullup(false)
{}

// class EncoderEvent
const string EncoderEvent::sEventType("en");

EncoderEvent::EncoderEvent(int targetClientId) :
	RemoteEvent(targetClientId), mControlId(0), mValue(0)
{}

// class KeyMatrixCreateEvent

const string KeyMatrixCreateEvent::sEventType("createKeyMatrix");

KeyMatrixCreateEvent::KeyMatrixCreateEvent(int targetClientId) :
	RemoteEvent(targetClientId), mNumRows(8), mNumCols(8),
	mRowPins(8), mColPins(8), mDebounceDelay(50)
{}

// class KeyMatrixEvent
const string KeyMatrixEvent::sEventType("km");

KeyMatrixEvent::KeyMatrixEvent(int targetClientId) :
	RemoteEvent(targetClientId), mControlId(0), mKeyIndex(0), mPosition(0)
{}
