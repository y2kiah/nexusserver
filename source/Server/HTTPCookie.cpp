#include "HTTPCookie.h"
#include <sstream>
#include "../Utility/Typedefs.h"

using std::ostringstream;

string HTTPCookie::toString() const
{
	const char *delim = "; ";

	ostringstream oss;
	oss << name << "=" << value;
	if (!domain.empty()) { oss << delim << "Domain=" << domain; }
	if (!path.empty()) { oss << delim << "Path=" << path; }
	if (!expires.is_not_a_date_time()) {
		ostringstream expString;
		expString << expires.date().day_of_week() << ", " << expires.date().day() << "-" << expires.date().month() << "-" << expires.date().year()
			<< " " << expires.time_of_day() << " GMT";
		oss << delim << "Expires=" << expString.str();
	}
	if (httpOnly) { oss << delim << "HttpOnly"; }
	return oss.str();
}