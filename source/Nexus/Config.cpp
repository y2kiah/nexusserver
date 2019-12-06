/*----==== CONFIG.CPP ====----
	Author:	Jeff Kiah
	Date:	11/18/2010
	Rev:	11/18/2010
----------------------------*/

#include "Config.h"
#include <fstream>
#include <boost\serialization\vector.hpp>
#include <boost\archive\xml_iarchive.hpp>
#include <boost\archive\xml_oarchive.hpp>
#include "../Utility/Typedefs.h"

///// class AppConfig /////

bool AppConfig::load()
{
	try {
		std::ifstream ifs(filename);
		if (!ifs.good()) {
			// if the file isn't found, write it
			if (save()) {
				// and try one more time
				ifs.open(filename);
				if (!ifs.good()) {
					return false;
				}
			}
		}
		boost::archive::xml_iarchive ia(ifs);
		ia >> boost::serialization::make_nvp("NexusServer", *this);
		return true;
	} catch (char *str) {
		debugPrintf("Exception raised: %s\n", str);
		return false;
	}
}

bool AppConfig::save()
{
	try {
		std::ofstream ofs(filename, std::ios::trunc);
		if (!ofs.good()) {
			return false;
		}
		boost::archive::xml_oarchive oa(ofs);
		oa << boost::serialization::make_nvp("NexusServer", *this);
		return true;
	} catch (char *str) {
		debugPrintf("Exception raised: %s\n", str);
		return false;
	}
}

bool AppConfig::checkConfigIntegrity() const
{
	return true;
}
