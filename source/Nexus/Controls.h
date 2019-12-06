/*----==== CONTROLS.H ====----
	Author: Jeffrey Kiah
	Orig.Date: 11/10/2010
	Rev.Date:  11/10/2010
----------------------------*/

#pragma once

#include <vector>
//#include "../Utility/Serialization.h"
#include "../Utility/Typedefs.h"

using std::vector;

class Control {
	protected:
		// Variables
		int mId;

	public:
		// Functions
		int id() const { return mId; }

		explicit Control(int id) : mId(id) {}
		virtual ~Control() = 0;
};

///// class DigitalSwitch /////
class DigitalSwitch : public Control {
	private:
		// Variables
		uchar mNumPins;			// total number of switch positions needing a pin
		vector<uchar> mPins;	// pin numbers
		bool mOffEvent;			// Whether the control has an off event or not
		char mActiveLevel;		// Whether pins are active HIGH or active LOW. When using pullup resistor, this should be active LOW
		bool mUseInternalPullup;// Use the internal pullup resistor on each pin or not
		short mDebounceDelay;	// Time in ms of the debounce delay. Default is 50ms.
		short mActivePin;		// Current active position index
    
	public:
		// Functions
		explicit DigitalSwitch(short id, uchar numPins, const vector<uchar> &pins, bool offEvent = true,
								char activeLevel = 0, bool useInternalPullup = false, short debounceDelay = 50);
};

///// class AnalogControl /////
class AnalogControl : public Control {
	private:
		// Variables
		uchar mPin;				// pin number
		uchar mChangeThreshold;	// minimum reading change to trigger an event message
		short mInterval;		// minimum time interval between sending event messages

	public:
		// Functions
		explicit AnalogControl(short id, uchar pin, uchar changeThreshold = 1, short interval = 50);
		~AnalogControl() {}
};

///// class Encoder /////
class Encoder : public Control {
	public:
		enum EncoderType : uchar {
			EncoderType_Incremental = 0,
			EncoderType_Absolute
		};
	private:
		// Variables
		uchar mPinA; // interrupt pin
		uchar mPinB;
		uchar mInterruptNumber;
		uchar mBits; // 2 or 4
		short mInterval;
		bool mGrayCode;
		EncoderType mEncoderType;
		bool mUseInternalPullup;// Use the internal pullup resistor on each pin or not

	public:
		// Functions
		explicit Encoder(short id, uchar pinA, uchar pinB, uchar interruptNumber, short interval = 50,
						 bool grayCode = false, EncoderType encoderType = EncoderType_Incremental,
						 bool useInternalPullup = false);
		~Encoder() {}
};

///// class ShiftRegister /////
class ShiftRegister : public Control {
};

///// class KeyMatrix /////
class KeyMatrix : public Control {
	private:
		// Variables
		uchar mNumRows;
		uchar mNumCols;
		vector<uchar> mRowPins;
		vector<uchar> mColPins;
		short mDebounceDelay;
    
	public:
		// Functions
		explicit KeyMatrix(short id, uchar numRows, uchar numCols, const vector<uchar> &rowPins,
							const vector<uchar> &colPins, short debounceDelay = 50);
		~KeyMatrix() {}
};