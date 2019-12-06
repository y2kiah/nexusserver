#pragma once

#include <string>

using std::string;

namespace MimeTypes {

/// Convert a file extension into a MIME type.
string extension_to_type(const string &extension);

}