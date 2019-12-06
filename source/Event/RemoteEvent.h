/*----==== REMOTEEVENT.H ====----
	Author: Jeffrey Kiah
	Orig.Date: 11/17/2010
	Rev.Date:  11/17/2010
-------------------------------*/

#pragma once

//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/tracking.hpp>
//#include "../Utility/Serialization.h" // using Boost.Serialization instead
#include "Event.h"

/*=============================================================================
class RemoteEvent
	This is the base class for any event with a registered event type of
	RemoteCallableEvent. If you don't intend for your event to be fired
	remotely, just inherit from Event instead. This requires a new constructor
	signature to be implemented in the derived class (const AnyVars &). For
	this event type, the AnyVars list can be assumed to have a single member
	that can be interpreted as an Archive type as described in Serialization.h.
=============================================================================*/
class RemoteEvent : public Event {//, public ISerializable {
	private:
		// Boost.Serialization stuff
		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			//ar & type();
			ar & BOOST_SERIALIZATION_NVP(mTargetClientId);
		}

	protected:
		int mTargetClientId; // unique id of the client being targeted, value of -1 used for broadcast to all clients

	public:
		// Derived implementations can call these base class functions if targetID is desired in byte stream
		//virtual bool serialize(Archive &ar) const { ar << mTargetId; return true; }
		//virtual bool deserialize(Archive &ar) { ar >> mTargetId; return true; }

		int targetClientId() const { return mTargetClientId; }
		bool isBroadcast() const { return (mTargetClientId == -1); }

		/*---------------------------------------------------------------------
			This constructor would be called by the remote manager for an event
			fired remotely.
		---------------------------------------------------------------------*/
		explicit RemoteEvent(int targetClientId) :
			Event(), mTargetClientId(targetClientId)
		{}
		virtual ~RemoteEvent() {}
};

// elminate serialization overhead at the cost of
// never being able to increase the version
BOOST_CLASS_IMPLEMENTATION(RemoteEvent, boost::serialization::object_serializable);

// eliminate object tracking (even if serialized through a pointer)
// at the risk of a programming error creating duplicate objects.
BOOST_CLASS_TRACKING(RemoteEvent, boost::serialization::track_never)