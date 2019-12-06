/*----==== NEXUSMESSAGEPARSER.CPP ====----
	Author:	Jeff Kiah
	Date:	10/27/2010
	Rev:	9/16/2011
----------------------------------------*/

#include <string>
#include <istream>
#include "NexusMessageParser.h"
#include "TCPServer.h"
#include "TCPConnection.h"
#include "../Utility/Typedefs.h"

#define DFLT_MSG_PREFIX				'/'
#define DFLT_MSG_TERMINATOR			'\r'
#define DFLT_MSG_TERMINATOR2		'\n'
#define DFLT_NVP_SEPARATOR			'='
#define DFLT_NVP_DELIMITER			'&'
#define DFLT_NVP_PREFIX				'?'
#define DFLT_NVP_STRING_DELIMITER	'"'

// Static Variables
NexusMessageParser::MsgSpecialChars NexusMessageParser::sSpecials =
	{DFLT_MSG_PREFIX,DFLT_MSG_TERMINATOR,DFLT_MSG_TERMINATOR2,DFLT_NVP_SEPARATOR,
	 DFLT_NVP_DELIMITER,DFLT_NVP_PREFIX,DFLT_NVP_STRING_DELIMITER};

// Functions
tribool NexusMessageParser::collectMessage(std::istream &is, unsigned int size, TCPConnection *cn)
{
	for (unsigned int i = 0; i < size; ++i) {
		char c;
		is.get(c);

		// Begin new message
		if (c == sSpecials.msgPrefix && !mMsgInProgress) {
			mMsg.str(string()); // clear the buffer
			mMsgInProgress = true;
			mMsgTerminating = false;

		// End of message
		} else if (c == sSpecials.msgTerminator && mMsgInProgress) {
			mMsgTerminating = true;

		} else if (c == sSpecials.msgTerminator2 && mMsgTerminating) {
			mMsgInProgress = false;
			mMsgTerminating = false;
			// log the message
			debugPrintf("%s sent: %s\n",
						cn->getSocket().remote_endpoint().address().to_string().c_str(),
						mMsg.str().c_str());
			return true;

		} else if (mMsgInProgress) {
			mMsg << c;
			mMsgTerminating = false;
		}
	}
	return boost::indeterminate;
}