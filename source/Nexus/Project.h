/*----==== PROJECT.H ====----
	Author: Jeffrey Kiah
	Orig.Date: 11/10/2010
	Rev.Date:  11/10/2010
---------------------------*/

#pragma once

#include <vector>
#include <memory>

using std::vector;
using std::shared_ptr;
using std::weak_ptr;

class Project;
class HardwareInterface;
class SoftwareInterface;
class Control;
class Instrument;
typedef weak_ptr<Project>				ProjectWeakPtr;
typedef shared_ptr<HardwareInterface>	HwIntPtr;
typedef shared_ptr<SoftwareInterface>	SwIntPtr;
typedef shared_ptr<Control>				ControlPtr;
typedef shared_ptr<Instrument>			InstrumentPtr;
typedef vector<HwIntPtr>				HwIntList;
typedef vector<SwIntPtr>				SwIntList;
typedef vector<ControlPtr>				ControlList;
typedef vector<InstrumentPtr>			InstrumentList;

class Project {
	private:
		HwIntList mHwInts;
		SwIntList mSwInts;
};

class HardwareInterface {
	private:
		ControlList		mControls;
		InstrumentList	mInstruments;
		ProjectWeakPtr	mProject;

	public:
};

// Client.h

class Client {
	// client id (ip address, etc.)
};

class HardwareInterfaceClient : public Client {
	private:
		HwIntPtr	mHwInt;
};

class SoftwareInterfaceClient : public Client {
	private:
		SwIntPtr	mSwInt;
};