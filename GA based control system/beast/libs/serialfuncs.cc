#include "serialfuncs.h"

using namespace std;

namespace BEAST {

/**
 * Replaces spaces with underscores and adds backslashes to other characters
 * which might be interpreted as white space by an input stream.
 * \see strip_slashes
 */
string add_slashes(const string& str)
{
	string output;

	string::const_iterator i = str.begin();

	for (; i != str.end(); ++i) {
		switch (*i) {
		case '\n':
			output.push_back('\\');
			output.push_back('n');
			continue;
		case '\r':
			output.push_back('\\');
			output.push_back('r');
			continue;
		case ' ': 
			output.push_back('_');
			continue;
		case '_':
			output.push_back('\\');
			output.push_back('_');
			continue;
		case '\t':
			output.push_back('\\');
			output.push_back('t');
			continue;
		case '\\':
			output.push_back('\\');
			output.push_back('\\');
			continue;
		}
		output.push_back(*i);
	}

	return output;
}

/**
 * Removes the slashes added by add_slashes and reinstates the original
 * string.
 * \see add_slashes
 */
string strip_slashes(const string& str)
{
	string output;

	string::const_iterator i = str.begin();

	for (; i != str.end(); ++i) {
		switch (*i) {
		case '_':
			output.push_back(' ');
			continue;
		case '\\':
			++i;
			if (i == str.end()) {
				output.push_back('\\');
			}
			else {
				switch (*i) {
					case '\\':
						output.push_back('\\');
						continue;
					case '_':
						output.push_back('_');
						continue;
					case 't':
						output.push_back('\t');
						continue;
					case 'n':
						output.push_back('\n');
						continue;
					case 'r':
						output.push_back('\r');
						continue;
				}
			}
		}
		output.push_back(*i);
	}

	return output;
}

/// Returns a textual description of the exception.
string SerialException::ToString()const
{
	ostringstream out;
	switch (error) {
	case SERIAL_ERROR_UNKNOWN:
		out << "There was an unknown problem.";
		break;
	case SERIAL_ERROR_BAD_FILE:
		out << "There was a problem accessing the file " << name << ".";
		break;
	case SERIAL_ERROR_UNKNOWN_TYPE:
		out << "The type " << name << " is unknown.";
		break;
	case SERIAL_ERROR_WRONG_TYPE:
		out << "The type " << name << " is not the same as this object.";
		break;
	case SERIAL_ERROR_DATA_MISMATCH:
		out << "The data is either corrupt or belongs to a different version of this object.";
		break;
	}

	if (message.size() != 0) {
		out << "\n" << message;
	}

	return out.str();
}

} // namespace BEAST
