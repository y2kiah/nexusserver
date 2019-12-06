/*----==== TCPSTREAM.H ====----
	Author: Jeffrey Kiah
	Orig.Date: 11/15/2010
	Rev.Date:  11/15/2010
---------------------------------*/

#pragma once

#include <iostream>
#include <vector>
#include <stdio.h>
#include <boost/asio/streambuf.hpp>

using boost::asio::streambuf;
using std::iostream;
using std::vector;

/*=====================================================================
class TCPTextStream
=====================================================================*/
class TCPTextStream {
	friend class TCPConnection;
	private:
		streambuf	mBuffer;
		iostream	ios;

	public:
		template <typename T>
		TCPTextStream & operator<<(const vector<T> &val);
		template <typename T>
		TCPTextStream & operator>>(vector<T> &val);

		template <typename T>
		TCPTextStream & operator<<(const T &val)
		{
			ios << val;
			return *this;
		}

		template <typename T>
		TCPTextStream & operator>>(T &val)
		{
			ios >> val;
			return *this;
		}

		explicit TCPTextStream() :
			ios(&mBuffer)
		{}
};

/*=====================================================================
class TCPBinaryStream
=====================================================================*/
class TCPBinaryStream {
	friend class TCPConnection;
	private:
		streambuf	mBuffer;
		iostream	ios;

	public:
		template <typename T>
		TCPBinaryStream & operator<<(const T &val)
		{
			switch (sizeof(val)) {
				case 2: ios.write(htons(val)); break;
				case 4: ios.write(htonl(val)); break;
				default: ios.write(val);
			}
			return *this;
		}

		template <typename T>
		TCPBinaryStream & operator>>(T &val)
		{
			uint s = sizeof(val);
			switch (s) {
				case 2: ios.read(ntohs(val), s); break;
				case 4: ios.read(ntohl(val), s); break;
				default: ios.read(val, s);
			}
			return *this;
		}

		explicit TCPBinaryStream() :
			ios(&mBuffer) // std::ios::binary ??
		{}
};

///// FUNCTIONS /////

// class TCPTextStream

template <typename T>
TCPTextStream & TCPTextStream::operator<<(const vector<T> &val)
{
	for (auto i = val.cbegin(); i != val.cend(); ++i) {
		ios << *i;
	}
	return *this;
}

template <typename T>
TCPTextStream & TCPTextStream::operator>>(vector<T> &val)
{
	for (auto i = val.begin(); i != val.end(); ++i) {
		ios >> *i;
	}
	return *this;
}