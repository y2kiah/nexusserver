/*----==== SERIALIZATION.H ====----
	Author: Jeffrey Kiah
	Orig.Date: 11/09/2010
	Rev.Date:  11/09/2010
---------------------------------*/

#pragma once

#include <memory>

class ArchiveImpl;
typedef std::shared_ptr<ArchiveImpl>	ArchiveImplPtr;

// TO-DO:	make an NVP macro like Boost.Serialization has that will pick up the variable name and value for some archive impl's,
//			but can ignore the name part for archive impl's that don't need it. Then all serializations could use the macro on
//			each value whether the name is to be used or not

class Archive {
	private:
		ArchiveImplPtr mpImpl;	// implementation of functions called by << and >> here can be swapped out
								// this allows archiving with different behaviors based on implementation attached
	public:
		void setImpl(const ArchiveImplPtr &pImpl) { mpImpl = pImpl; }
		//getImplType() const;	// RTTI for implementation incase you need to check it in serialization functions to do
								// different serializations for different archives (e.g. network message vs. store to disk)

		template <typename T>
		Archive & operator<<(const T &val) {
			_ASSERTE(mpImpl.get() && "Archive implementation is null");
			mpImpl->write(val);
		}

		template <typename T>
		Archive & operator>>(T &val) {
			_ASSERTE(mpImpl.get() && "Archive implementation is null");
			mpImpl->read(val);
		}

		explicit Archive(const ArchiveImplPtr &pImpl) :
			mpImpl(pImpl)
		{}
};

class ISerializable {
	public:
		virtual bool serialize(Archive &ar) const = 0;
		virtual bool deserialize(Archive &ar) = 0;
		
		virtual ~ISerializable() {}
};