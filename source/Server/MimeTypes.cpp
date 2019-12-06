#include "mimetypes.h"

namespace MimeTypes {

	struct Mapping {
		const char *extension;
		const char *mime_type;
	} mappings[] =
	{
		{ "gif", "image/gif" },
		{ "htm", "text/html" },
		{ "html","text/html" },
		{ "jpg", "image/jpeg" },
		{ "png", "image/png" },
		{ "ico", "image/vnd.microsoft.icon" },
		{ "luap","text/html" },
		{ "css", "text/css" },
		{ "js",  "application/javascript" },
		{ 0, 0 } // Marks end of list.
	};

	string extension_to_type(const string &extension)
	{
		for (Mapping *m = mappings; m->extension; ++m) {
			if (m->extension == extension) {
				return m->mime_type;
			}
		}
		return "text/plain";
	}

}