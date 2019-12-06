/*----==== CONTROLEVENTS.H ====----
	Author: Jeffrey Kiah
	Orig.Date: 11/10/2010
	Rev.Date:  11/10/2010
---------------------------------*/

#pragma once

#include <vector>
#include "../Event/RemoteEvent.h"
#include "Controls.h"
//#include "../Utility/Serialization.h"

using std::vector;

void registerEventTypes();

///// STRUCTURES /////

// Digital Switch Events

/*=====================================================================
class DigitalSwitchCreateEvent
=====================================================================*/
class DigitalSwitchCreateEvent : public RemoteEvent {
	private:
		// Boost.Serialization stuff
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RemoteEvent);
			ar & BOOST_SERIALIZATION_NVP(mNumPins);
			ar & BOOST_SERIALIZATION_NVP(mPins);
			ar & BOOST_SERIALIZATION_NVP(mOffEvent);
			ar & BOOST_SERIALIZATION_NVP(mActiveLevel);
			ar & BOOST_SERIALIZATION_NVP(mUseInternalPullup);
			ar & BOOST_SERIALIZATION_NVP(mDebounceDelay);
		}

	public:
		///// VARIABLES /////
		static const string sEventType;

		uchar mNumPins;
		vector<uchar> mPins;
		bool mOffEvent;
		char mActiveLevel;
		bool mUseInternalPullup;
		short mDebounceDelay;

		///// FUNCTIONS /////
		const string &	type() const { return sEventType; }

		// Serialization
		/*bool serialize(Archive &ar) const
		{
			ar << mNumPins << mPins << mOffEvent << mActiveLevel <<
				mUseInternalPullup << mDebounceDelay;
			return true;
		}

		bool deserialize(Archive &ar)
		{
			ar >> mNumPins >> mPins >> mOffEvent >> mActiveLevel >>
				mUseInternalPullup >> mDebounceDelay;
			return true;
		}*/

		// Constructor / destructor
		explicit DigitalSwitchCreateEvent(int targetClientId);
		virtual ~DigitalSwitchCreateEvent() {}
};

BOOST_CLASS_IMPLEMENTATION(DigitalSwitchCreateEvent, boost::serialization::object_serializable);
BOOST_CLASS_TRACKING(DigitalSwitchCreateEvent, boost::serialization::track_never)

/*=====================================================================
class DigitalSwitchEvent
=====================================================================*/
class DigitalSwitchEvent : public RemoteEvent {
	private:
		// Boost.Serialization stuff
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RemoteEvent);
			ar & BOOST_SERIALIZATION_NVP(mControlId);
			ar & BOOST_SERIALIZATION_NVP(mActivePos);
		}

	public:
		///// VARIABLES /////
		static const string sEventType;

		short mControlId;	// switch id, index to control array
		uchar mActivePos;	// active switch position, index to pin array, off position is max+1

		///// FUNCTIONS /////
		const string &	type() const { return sEventType; }

		// Constructor / destructor
		explicit DigitalSwitchEvent(int targetClientId);
		virtual ~DigitalSwitchEvent() {}
};

BOOST_CLASS_IMPLEMENTATION(DigitalSwitchEvent, boost::serialization::object_serializable);
BOOST_CLASS_TRACKING(DigitalSwitchEvent, boost::serialization::track_never)

// Analog Control Events

/*=====================================================================
class AnalogControlCreateEvent
=====================================================================*/
class AnalogControlCreateEvent : public RemoteEvent {
	private:
		// Boost.Serialization stuff
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RemoteEvent);
			ar & BOOST_SERIALIZATION_NVP(mPin);
			ar & BOOST_SERIALIZATION_NVP(mChangeThreshold);
			ar & BOOST_SERIALIZATION_NVP(mInterval);
		}

	public:
		///// VARIABLES /////
		static const string sEventType;

		uchar mPin;
		uchar mChangeThreshold;
		short mInterval;

		///// FUNCTIONS /////
		const string &	type() const { return sEventType; }

		// Constructor / destructor
		explicit AnalogControlCreateEvent(int targetClientId);
		virtual ~AnalogControlCreateEvent() {}
};

BOOST_CLASS_IMPLEMENTATION(AnalogControlCreateEvent, boost::serialization::object_serializable);
BOOST_CLASS_TRACKING(AnalogControlCreateEvent, boost::serialization::track_never)

/*=====================================================================
class AnalogControlEvent
=====================================================================*/
class AnalogControlEvent : public RemoteEvent {
	private:
		// Boost.Serialization stuff
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RemoteEvent);
			ar & BOOST_SERIALIZATION_NVP(mControlId);
			ar & BOOST_SERIALIZATION_NVP(mRawValue);
		}

	public:
		///// VARIABLES /////
		static const string sEventType;

		short mControlId;	// switch id, index to control array
		short mRawValue;	// value read from the pin, unmapped

		///// FUNCTIONS /////
		const string &	type() const { return sEventType; }

		// Constructor / destructor
		explicit AnalogControlEvent(int targetClientId);
		virtual ~AnalogControlEvent() {}
};

BOOST_CLASS_IMPLEMENTATION(AnalogControlEvent, boost::serialization::object_serializable);
BOOST_CLASS_TRACKING(AnalogControlEvent, boost::serialization::track_never)

// Encoder Events

/*=====================================================================
class EncoderCreateEvent
=====================================================================*/
class EncoderCreateEvent : public RemoteEvent {
	private:
		// Boost.Serialization stuff
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RemoteEvent);
			ar & BOOST_SERIALIZATION_NVP(mPinA);
			ar & BOOST_SERIALIZATION_NVP(mPinB);
			ar & BOOST_SERIALIZATION_NVP(mInterruptNumber);
			ar & BOOST_SERIALIZATION_NVP(mBits);
			ar & BOOST_SERIALIZATION_NVP(mInterval);
			ar & BOOST_SERIALIZATION_NVP(mGrayCode);
			ar & BOOST_SERIALIZATION_NVP(mEncoderType);
			ar & BOOST_SERIALIZATION_NVP(mUseInternalPullup);
		}

	public:
		///// VARIABLES /////
		static const string sEventType;

		uchar mPinA;
		uchar mPinB;
		uchar mInterruptNumber;
		uchar mBits;
		short mInterval;
		bool mGrayCode;
		Encoder::EncoderType mEncoderType;
		bool mUseInternalPullup;

		///// FUNCTIONS /////
		const string &	type() const { return sEventType; }

		// Constructor / destructor
		explicit EncoderCreateEvent(int targetClientId);
		virtual ~EncoderCreateEvent() {}
};

BOOST_CLASS_IMPLEMENTATION(EncoderCreateEvent, boost::serialization::object_serializable);
BOOST_CLASS_TRACKING(EncoderCreateEvent, boost::serialization::track_never)

/*=====================================================================
class EncoderEvent
=====================================================================*/
class EncoderEvent : public RemoteEvent {
	private:
		// Boost.Serialization stuff
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RemoteEvent);
			ar & BOOST_SERIALIZATION_NVP(mControlId);
			ar & BOOST_SERIALIZATION_NVP(mValue);
		}

	public:
		///// VARIABLES /////
		static const string sEventType;

		short mControlId;	// switch id, index to control array
		short mValue;		// relative change or absolute value

		///// FUNCTIONS /////
		const string &	type() const { return sEventType; }

		// Constructor / destructor
		explicit EncoderEvent(int targetClientId);
		virtual ~EncoderEvent() {}
};

BOOST_CLASS_IMPLEMENTATION(EncoderEvent, boost::serialization::object_serializable);
BOOST_CLASS_TRACKING(EncoderEvent, boost::serialization::track_never)

// KeyMatrix Events

/*=====================================================================
class KeyMatrixCreateEvent
=====================================================================*/
class KeyMatrixCreateEvent : public RemoteEvent {
	private:
		// Boost.Serialization stuff
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RemoteEvent);
			ar & BOOST_SERIALIZATION_NVP(mNumRows);
			ar & BOOST_SERIALIZATION_NVP(mNumCols);
			ar & BOOST_SERIALIZATION_NVP(mRowPins);
			ar & BOOST_SERIALIZATION_NVP(mColPins);
			ar & BOOST_SERIALIZATION_NVP(mDebounceDelay);
		}

	public:
		///// VARIABLES /////
		static const string sEventType;

		uchar mNumRows;
		uchar mNumCols;
		vector<uchar> mRowPins;
		vector<uchar> mColPins;
		short mDebounceDelay;

		///// FUNCTIONS /////
		const string &	type() const { return sEventType; }

		// Constructor / destructor
		explicit KeyMatrixCreateEvent(int targetClientId);
		virtual ~KeyMatrixCreateEvent() {}
};

BOOST_CLASS_IMPLEMENTATION(KeyMatrixCreateEvent, boost::serialization::object_serializable);
BOOST_CLASS_TRACKING(KeyMatrixCreateEvent, boost::serialization::track_never)

/*=====================================================================
class KeyMatrixEvent
=====================================================================*/
class KeyMatrixEvent : public RemoteEvent {
	private:
		// Boost.Serialization stuff
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RemoteEvent);
			ar & BOOST_SERIALIZATION_NVP(mControlId);
			ar & BOOST_SERIALIZATION_NVP(mKeyIndex);
			ar & BOOST_SERIALIZATION_NVP(mPosition);
		}

	public:
		///// VARIABLES /////
		static const string sEventType;

		short mControlId;	// switch id, index to control array
		short mKeyIndex;	// row * numRows + col
		uchar mPosition;	// 0 = down, 1 = up

		///// FUNCTIONS /////
		const string &	type() const { return sEventType; }

		// Constructor / destructor
		explicit KeyMatrixEvent(int targetClientId);
		virtual ~KeyMatrixEvent() {}
};

BOOST_CLASS_IMPLEMENTATION(KeyMatrixEvent, boost::serialization::object_serializable);
BOOST_CLASS_TRACKING(KeyMatrixEvent, boost::serialization::track_never)
