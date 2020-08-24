/**
 * \file unserialiser.h
 * Interface of a class and related functors for unserialising unknown types.
 * \author David Gordon
 * \addtogroup serialisation
 */

#ifndef _UNSERIALISER_H
#define _UNSERIALISER_H

#include <map>
#include <functional>
#include <iostream>

#include "utilities.h"
#include "serialfuncs.h"
#include "neuralanimat.h" // Includes all the other stuff we need

namespace BEAST {

/**
 * \addtogroup serialisation
 * @{
 */

/// A simple abstract base class for ObjLoader functors
struct ObjLoaderBase : public std::unary_function<std::istream&, WorldObject*>
{
	virtual ~ObjLoaderBase(){}
	virtual WorldObject* operator()(std::istream& in) = 0;
};

/// A functor for recreating templated object types using serialisation.
template <typename _Type>
struct ObjLoader : public ObjLoaderBase
{
	virtual ~ObjLoader(){}

	virtual WorldObject* operator()(std::istream& in)
	{
		_Type* obj = new _Type;
		in >> *obj;
		return dynamic_cast<WorldObject*>(obj);
	}
};

/**
 * This class is available for unserialising objects from streams, without
 * knowing which type of object is to be unserialised - the type is determined
 * from the header of the stream. Since it's a singleton object, there is only
 * ever one reference to it at a time. To access that reference, use 
 * Unserialiser::Instance().
 */
class Unserialiser : public std::map<std::string, ObjLoaderBase*>,
					 public std::unary_function<WorldObject*, std::istream&>
{
public:
	/// Returns a reference to the one and only Unserialiser object.
	static Unserialiser& Instance()
	{
		static Unserialiser theInstance;
		return theInstance;
	}
	
	void			Add(std::string name, ObjLoaderBase* func);

	WorldObject*	operator()(std::istream& in);

private:
	Unserialiser();
	~Unserialiser();
};

/**
 * @}
 */

} // namespace BEAST

#endif
