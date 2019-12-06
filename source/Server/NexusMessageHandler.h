#pragma once

#include <vector>
#include "TCPTypes.h"
#include "Message.h"

using std::vector;

class NexusMessageHandler : public MessageHandler
{
	private:
		string mReply;
		BufferList mReplyBuffers;
		explicit NexusMessageHandler() {}

	public:
		virtual void handleMessage(MessageParser *parser)
		{
			NexusMessageParser &rp = *(reinterpret_cast<NexusMessageParser*>(parser));
			mReply.assign(rp.getMessage().str()); // echo message back to sender
		}

		virtual bool hasReply() const { return (mReply.length() > 0); }
		virtual string getReply() const { return mReply; }
		
		virtual const BufferList &getReplyBuffers()
		{
			mReplyBuffers.clear();
			mReplyBuffers.push_back(boost::asio::buffer(mReply));
			return mReplyBuffers;
		}

		virtual void setBadRequest() {} // do nothing (for now)

		static HandlerPtr create()
		{
			HandlerPtr h(new NexusMessageHandler());
			return h;
		}
};