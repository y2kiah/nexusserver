/*----==== CONFIG.H ====----
	Author:	Jeff Kiah
	Date:	11/18/2010
	Rev:	11/18/2010
--------------------------*/

#pragma once

#include <string>
#include <vector>
#include <boost/serialization/nvp.hpp>

using std::string;
using std::wstring;
using std::vector;

///// STRUCTURES /////

class AppConfig {
	public:
		///// DEFINITIONS /////
		struct ProjectOptions {
			string	name;
			wstring	filename;
			int		port;
			bool	enabled;

			// Boost.Serialization stuff
			template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
			{
				ar & BOOST_SERIALIZATION_NVP(name);
				ar & BOOST_SERIALIZATION_NVP(filename);
				ar & BOOST_SERIALIZATION_NVP(port);
				ar & BOOST_SERIALIZATION_NVP(enabled);
			}
		};
		typedef vector<ProjectOptions> ProjOptionsList;

		///// VARIABLES /////
		wstring			filename;
		ProjOptionsList	projects;
		int				adminPort;
		bool			adminUseZip;
		int				webCacheMB;
		int				projectCacheMB;
		int				scriptCacheMB;

		///// FUNCTIONS /////
		bool load();	// load settings from file
		bool save();	// save settings to file
		bool checkConfigIntegrity() const; // check for config data errors and warnings

		explicit AppConfig(const wstring &_filename) :
			filename(_filename), adminPort(8080), adminUseZip(true),
			webCacheMB(128), projectCacheMB(128), scriptCacheMB(32)
		{}

	private:
		// Boost.Serialization stuff
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(projects);
			ar & BOOST_SERIALIZATION_NVP(adminPort);
			ar & BOOST_SERIALIZATION_NVP(adminUseZip);
			ar & BOOST_SERIALIZATION_NVP(webCacheMB);
			ar & BOOST_SERIALIZATION_NVP(projectCacheMB);
			ar & BOOST_SERIALIZATION_NVP(scriptCacheMB);
		}
};
