/**
 * \file unserialiser.cc
 * Implementation of Unserialiser.
 * \author David Gordon
 */

#include "unserialiser.h"

using namespace std;

namespace BEAST {

/**
 * Constructor, sets up built-in Unserialisable types.
 */
Unserialiser::Unserialiser()
{
	(*this)["WorldObject"]	= new ObjLoader<WorldObject>();
	(*this)["Animat"]		= new ObjLoader<Animat>();
	(*this)["FFNAnimat"]	= new ObjLoader<FFNAnimat>();
	(*this)["DNNAnimat"]	= new ObjLoader<DNNAnimat>();
	(*this)["Wall"]			= new ObjLoader<Wall>();
}

/**
 * Destructor, deletes all the member function objects.
 */
Unserialiser::~Unserialiser()
{
	for_each(begin(), end(), 
			 call_on_mem(&pair<const string, ObjLoaderBase*>::second,
						 deleter<ObjLoaderBase>()));
}

/**
 * Adds a new ObjLoader functor to the map, deleting any existing one.
 */
void Unserialiser::Add(std::string name, ObjLoaderBase* func)
{
	iterator i = find(name);
	if (i != end()) {
		delete i->second;
	}
	(*this)[name] = func;
}


/**
 * Calling the Unserialiser as a function object will return a pointer to the
 * next serialised object on the specified istream, or NULL if none is found.
 */
WorldObject* Unserialiser::operator()(std::istream& in)
{
	string name;
	in >> name;
	for (int i=0; i < static_cast<int>(name.length()) + 1; ++i) in.unget();

	iterator func = find(name);

	if (func == end()) return NULL;
	return (*func->second)(in);
}

}
