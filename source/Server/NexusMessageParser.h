/*----==== NEXUSMESSAGEPARSER.H ====----
	Author:	Jeff Kiah
	Date:	10/27/2010
	Rev:	9/13/2011
--------------------------------------*/

#pragma once

#include <sstream>
#include <boost/noncopyable.hpp>
#include "TCPTypes.h"
#include "Message.h"

class NexusRequestHeader {
	public:
		short	eventID;	// event ID
		char	version;	// event message version
		short	length;		// length of data in bytes

		explicit NexusRequestHeader(short _eventID, char _version, short _length) :
			eventID(_eventID), version(_version), length(_length)
		{}
};

class NexusMessageParser : public MessageParser, private boost::noncopyable
{
	private:
		std::stringstream	mMsg;
		bool				mMsgInProgress;
		bool				mMsgTerminating;

		explicit NexusMessageParser() :
			mMsgInProgress(false), mMsgTerminating(false)
		{}

	public:
		// Structures
		struct MsgSpecialChars {
			char msgPrefix;
			char msgTerminator;
			char msgTerminator2;
			char nvpSeparator;
			char nvpDelimiter;
			char nvpPrefix;
			char nvpStringDelimiter;
		};

		// Variables
		static MsgSpecialChars sSpecials;

		// Functions
		virtual void reset() {}
		virtual tribool collectMessage(std::istream &is, unsigned int size, TCPConnection *cn);
		virtual const std::stringstream &getMessage() const
		{
			return mMsg;
		}
		
		void fireEventFromRemote(const char *eventType, std::ostream &os, bool raise);

		static ParserPtr create()
		{
			ParserPtr p(new NexusMessageParser());
			return p;
		}
};

///// DEFINES /////

/*
enum NVPParameterType {
  NVPParameterType_Integer = 0,
  NVPParameterType_Decimal,
  NVPParameterType_String,
  NVPParameterType_BitFlags
};

///// STRUCTURES /////

///// class Parameter /////
class Parameter {
  public:
    // Variables
    String name;
   
    // Functions
    bool parse(const String &str, int from, int to);
    
    virtual NVPParameterType type() const = 0;
    virtual void sendNVPString(Client &) const = 0;
    virtual bool validateAndConvert(const char *) = 0;
    
    explicit Parameter(const String &name) :
      name(name)
    {}
    virtual ~Parameter() {}
};

///// class NVPParameter /////
template <typename T, NVPParameterType TType>
class NVPParameter : public Parameter {
  public:
    // Variables
    T value;
  
    // Functions
    NVPParameterType type() const { return TType; }
    void sendNVPString(Client &client) const;
    bool validateAndConvert(const char *strData);
    
    // Constructor/Destructor
    NVPParameter(const String &name, T defaultValue) :
      Parameter(name), value(defaultValue)
    {}
    ~NVPParameter() {}
};

typedef NVPParameter<int, NVPParameterType_Integer>   NVPIntegerParam;
typedef NVPParameter<float, NVPParameterType_Decimal> NVPDecimalParam;
typedef NVPParameter<String, NVPParameterType_String> NVPStringParam;
typedef NVPParameter<int, NVPParameterType_BitFlags>  NVPBitFlagsParam;

template <typename T, NVPParameterType TType>
void NVPParameter<T, TType>::sendNVPString(Client &client) const
{
  client.print(name);
  client.write(NVP_SEPARATOR);
  if (type() == NVPParameterType_String) {
    client.write(NVP_STRING_DELIMITER);
  }
  client.print(value);
  if (type() == NVPParameterType_String) {
    client.write(NVP_STRING_DELIMITER);
  }
}

///// class NVPMessage /////
class NVPMessage {
  private:
    Parameter **mParams;
    
  public:
    const String name;
    const byte numParams;

    // Functions
    Parameter *getParam(byte i)
    {
      if (i < 0 || i >= numParams) { return 0; }
      return mParams[i];
    }
    
    bool addParameter(byte paramIndex, Parameter *param);    
    void sendMessage(Client &client) const;
    bool parse(const String &msg, int from);
  
    explicit NVPMessage(const String &name, byte numParams) :
      name(name), numParams(numParams), mParams(0)
    {
      mParams = new Parameter*[numParams];
      memset(mParams, 0, sizeof(mParams));
    }
    ~NVPMessage()
    {
      for (byte p = 0; p < numParams; ++p) {
        delete mParams[p];
      }
      delete [] mParams;
    }
};*/