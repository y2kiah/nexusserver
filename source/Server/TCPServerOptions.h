#pragma once

#include "TCPTypes.h"

// Structs
class TCPServerOptions : private boost::noncopyable
{
	public:
		// Variables
		string					name;
		unsigned short			port;
		unsigned int			connectionBufferSize;
		TCPConnectionSettings	connDefault;
		CreateParserFuncPtr		createParser;
		CreateHandlerFuncPtr	createHandler;

		// Functions
		static TCPServerOptionsPtr create(const string &_name, unsigned short _port,
										  const CreateParserFuncPtr &_createParser,
										  const CreateHandlerFuncPtr &_createHandler,
										  unsigned int _connectionBufferSize = DFLT_BUFFER_SIZE,
										  TCPConnectionSettings _connDefault = CloseAfterMessage
										 )
		{
			TCPServerOptionsPtr p(new TCPServerOptions(_name, _port, _connectionBufferSize, _connDefault,
													   _createParser, _createHandler));
			return p;
		}
	private:
		explicit TCPServerOptions(const string &_name, unsigned short _port,
								  unsigned int _connectionBufferSize,
								  TCPConnectionSettings _connDefault,
								  const CreateParserFuncPtr &_createParser,
								  const CreateHandlerFuncPtr &_createHandler) :
			name(_name), port(_port), createParser(_createParser), createHandler(_createHandler),
			connectionBufferSize(_connectionBufferSize), connDefault(_connDefault)
		{}
};