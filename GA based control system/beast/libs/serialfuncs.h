/**
 * \file serialfuncs.h
 * A bunch of methods, templates and classes for performing quick stream
 * insertion and extraction, include this file if you need to serialise your
 * objects' data.
 * \author David Gordon
 * \addtogroup serialisation
 */

#ifndef _SERIALFUNCS_H
#define _SERIALFUNCS_H

#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <sstream>
#include <vector>
#include <map>

namespace BEAST {

/**
 * \addtogroup serialisation Serialisation Utilities
 * Currently, the simplest way to store data is by serialisation - simply
 * saving and loading object data as unformatted text. To make it easier for
 * you to implement serialisation for your own classes, a range of functions,
 * classes and macros are provided.
 * @{
 */

std::string add_slashes(const std::string& str);
std::string strip_slashes(const std::string& str);

/// Enumerates the different types of errors encountered in serialisation.
enum SerialErrorType {
	SERIAL_ERROR_UNKNOWN,		///< An unknown problem.
	SERIAL_ERROR_BAD_FILE,		///< Unable to open or write to the file.
	SERIAL_ERROR_WRONG_TYPE,	///< Incoming data has the wrong type label.
	SERIAL_ERROR_UNKNOWN_TYPE,	///< Incoming data has an unknown type label.
	SERIAL_ERROR_DATA_MISMATCH	///< The wrong type of data seems to be coming in.
};

/**
 * Since exceptions have an undesirable overhead, they have not been used
 * elsewhere in the simulation environment for reasons of speed. Loading and
 * saving, however, is not speed-critical and the problems encountered suit
 * exception handling well. If in the future a more complete exception
 * framework is added, SerialException should be incorporated into that.
 */
struct SerialException
{
	SerialException(SerialErrorType e, std::string n = "", std::string msg = ""):
		error(e), name(n), message(msg){}

	std::string		ToString()const;

	SerialErrorType error;
	std::string		name;
	std::string		message;
};

/**
 * This is a function object which can be used for copying from an iterator
 * when the number of input values is unknown. Best constructed using the
 * extract helper function
 * \see extract
 */
template <class _Iterator>
struct extractor
{
	typedef typename _Iterator::value_type _ValueType;
	extractor(_Iterator& Iter):iter(Iter){}
	_ValueType operator()()
	{
		tempVal = *iter;
		++iter;
		return tempVal;
	}
	_Iterator iter;
	_ValueType tempVal;
};

/**
 * Constructs and returns an extractor of the correct type. For example, to
 * copy from a vector of unknown length into a range of known length:
 * <code>
 * vector<int> target(30);
 * vector<int> source(rand(100) + 30);
 * generate(target.begin(), target.end(), extract(source.begin()));
 * </code>
 * \see extractor
 */
template <class _Iterator>
extractor<_Iterator> extract(_Iterator& Iter)
{
	return extractor<_Iterator>(Iter);
}

/**
 * An algorithm similar to copy, but where the start and end of the target
 * range is specified rather than the source range. For example, to copy
 * from a vector of unknown length into a range of known length:
 * <code>
 * vector<int> target(30);
 * vector<int> source(rand(100) + 30);
 * copy_from(target.begin(), target.end(), source.begin());
 * </code>
 * \see extract for another way of doing exactly the same thing.
 */
template<class _InIt, class _OutIt>
_InIt copy_from(_OutIt _First, _OutIt _Last, _InIt _Src)
{	// copy [_First, _Last) from [_Src, ...)
	for (; _First != _Last; ++_Src, ++_First)
		*_First = *_Src;
	return (_Src);
}

/**
 * Although extract and copy_from should, with the help of istream_iterator,
 * be able to fill ranges from input streams, certain difficulties with
 * istreams arise which make the copy_from_istream function useful. Simply
 * specify the start and end iterators of the range to be filled and an
 * input stream to use as the source.
 */
template<class _OutIt>
std::istream& copy_from_istream(_OutIt _First, _OutIt _Last, std::istream& in)
{	// copy [_First, _Last) from in
	for (; _First != _Last; ++_First)
		in >> *_First;
	return in;
}

/**
 * The switcher is useful when configuring bools from string data. E.g. to set
 * <code>bool isTrue</code> according to a string coming from a stream:
 * <code>
 * cin >> switcher("isTrue", isTrue);
 * </code>
 */
struct switcher
{
	switcher(std::string s, bool& b):name(s),option(b){}
	std::string name;
	bool& option;
};

/**
 * Input operator for the switcher helper object. Note that unlike most input
 * operators, this one does not take a reference to the switcher, since the
 * switcher is intended for instantiation at the time of input.
 */
inline std::istream& operator>>(std::istream& in, switcher s)
{
	std::string compare;
	in >> compare;
	s.option = (compare == s.name);
	return in;
}

/**
 * A generic output operator for vectors, requires the vector's contents type
 * to have its own << operator. The size is output, followed by each entry,
 * separated by spaces.
 */
template <typename _Ty, typename _Ax>
inline std::ostream& operator<<(std::ostream& out, const std::vector<_Ty, _Ax>& v)
{
	out << "vector "
		<< static_cast<int>(v.size()) << " ";
	std::copy(v.begin(), v.end(), std::ostream_iterator<_Ty>(out, " "));
	return out;
}

/**
 * A generic input operator for vectors, requires the vector's contents type
 * to have its own >> operator. The size is input, followed by each entry.
 */
template <typename _Ty, typename _Ax>
inline std::istream& operator>>(std::istream& in, std::vector<_Ty, _Ax>& v)
{
	std::string name;
	in >> name;
	if (name != "vector") {
		throw SerialException(SERIAL_ERROR_DATA_MISMATCH, "", 
							  "Expected vector but got " + name);
	}
	int size;
	in >> size;
	v.resize(size);
	copy_from_istream(v.begin(), v.end(), in);
	return in;
}

/**
 * A specialised output operator for maps with key type string, which uses
 * add_slashes to encode the string.
 * \see add_slashes
 */
template <typename _Ty, typename _Pr, typename _Alloc>
inline std::ostream& operator<<(std::ostream& out, const std::map<std::string, _Ty, _Pr, _Alloc>& m)
{
	using namespace std;

	out << "map_string "
		<< static_cast<int>(m.size()) << "\n";
	typename map<string, _Ty, _Pr, _Alloc>::const_iterator i = m.begin();

	for (; i != m.end(); ++i) {
		out << add_slashes(i->first) << " " << i->second << "\n";
	}

	return out;
}

/**
 * A specialised input operator for maps with key type string, which uses
 * strip_slashes to decode the string.
 */
template <typename _Ty, typename _Pr, typename _Alloc>
inline std::istream& operator>>(std::istream& in, const std::map<std::string, _Ty, _Pr, _Alloc>& m)
{
	int size;
	std::string name;
	_Ty val;

	in >> name;
	if (name != "map_string") {
		throw SerialException(SERIAL_ERROR_DATA_MISMATCH, "", 
							  "Expected map_string but got " + name);
	}

	in >> size;
	m.clear();

	for (int i=0; i < size; ++i) {
		in >> name >> val;
		m[strip_slashes(name)] = val;
	}

	return in;
}

/**
 * A generic output operator for maps, requires the map's contents to have its
 * own << operator. The size is output, followed by each key and value,
 * separated by spaces.
 */
template <typename _Kty, typename _Ty, typename _Pr, typename _Alloc>
inline std::ostream& operator<<(std::ostream& out, const std::map<_Kty, _Ty, _Pr, _Alloc>& m)
{
  // MdK: 02-01-2007, added std to endl
	out << "map "
	    << static_cast<int>(m.size()) << std::endl;
	typename std::map<_Kty, _Ty, _Pr, _Alloc>::const_iterator i = m.begin();

	for (; i != m.end(); ++i) {
		out << i->first << " " << i->second << std::endl;
	}
	return out;
}

/**
 * A generic input operator for maps, requires the map's contents to have its
 * own >> operator. The size is read, followed by each key and value.
 */
template <typename _Kty, typename _Ty, typename _Pr, typename _Alloc>
inline std::istream& operator>>(std::istream& in, std::map<_Kty, _Ty, _Pr, _Alloc>& m)
{
	std::string name;
	in >> name;
	if (name != "map") {
		throw SerialException(SERIAL_ERROR_DATA_MISMATCH, "", 
							  "Expected map but got " + name);
	}

	int size;
	_Kty key;
	_Ty val;

	in >> size;
	m.clear();

	for (int i=0; i < size; ++i) {
		in >> key >> val;
		m[key] = val;
	}

	return in;
}

/**
 * An inline reinterpret cast which may be helpful in outputting enumeration
 * types to streams. Probaly better to use IMPLEMENT_IOSTREAM_CAST or ideally,
 * write your own input/output operators for your types.
 */
template <typename T1, typename T2>
inline T1& stream_convert(T2& input)
{
	return *reinterpret_cast<T1*>(&input);
}

/**
 * Use this macro to create a custom output operator which simply casts _Ty to
 * the type specified by _Cast. Useful for converting enum types to ints which
 * may then be input and output in the usual way.
 */
#define IMPLEMENT_IOSTREAM_CAST(_Ty,_Cast)					\
inline std::ostream& operator<<(std::ostream& out, _Ty& i)	\
{															\
	out << static_cast<_Cast>(i);							\
	return out;												\
}															\
															\
inline std::istream& operator>>(std::istream& in, _Ty& i)	\
{															\
	_Cast input;											\
	in >> input;											\
	i = static_cast<_Ty>(input);							\
	return in;												\
}

/**
 * This macro is intended as a quick solution to the problem of encoding
 * structs and classes into output streams. A binary conversion is performed,
 * meaning that any pointers and complex data types (e.g. vectors) will be
 * rendered nonsensical to the input operator. Only for use on simple structs
 * and classes containing non-pointer types. Also, output produced by these
 * functions is unlikely to transfer between platforms due to differing number
 * formats. This is only a temporary solution and should be replaced with
 * special input/output operators.
 */
#define IMPLEMENT_IOSTREAM_BINARY_CONVERSION(_Ty)			\
inline std::ostream& operator<<(std::ostream& out, _Ty& i)	\
{															\
	out.write(reinterpret_cast<char*>(&i), sizeof(i));		\
	return out;												\
}															\
															\
inline std::istream& operator>>(std::istream& in, _Ty& i)	\
{															\
	in.read(reinterpret_cast<char*>(&i), sizeof(i));		\
	return in;												\
}

/**
 * Use this macro to add basic serialisation functionality to your derived
 * classes by simply serialising under a new name, but using the parent
 * serialisation methods.
 */
#define IMPLEMENT_SERIALISATION(_Name, _Parent)						\
public:																\
	void Serialise(std::ostream& out)const							\
	{																\
		out << _Name << "\n";										\
		_Parent::Serialise(out);									\
	}																\
																	\
	void Unserialise(std::istream& in)								\
	{																\
		std::string name;											\
		in >> name;													\
		if (name != _Name) {										\
			throw SerialException(SERIAL_ERROR_WRONG_TYPE, name,	\
					std::string("This object is type ") + _Name);	\
		}															\
		_Parent::Unserialise(in);									\
	}

#define IMPLEMENT_LOADER(_Name, _Type) \
	Unserialiser::Instance().Add(_Name, new ObjLoader<_Type>());


/**
 * @}
 */

} // namespace BEAST

#endif
