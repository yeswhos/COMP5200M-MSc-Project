
/**
 * \file simulation.h
 * \author David Gordon
 * The Simulation class is really a framework of classes providing the 
 * facilities for implementing a range of different types of simulation.
 * SimObject is an interface class for the Group class, which simply
 * adds objects to the Simulation, and the Population class which provides
 * handles GA functionality, and the insertion of groups of Animats (or other
 * evolvable WorldObjects into the World.
 */

#ifndef _SIMULATION_H
#define _SIMULATION_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <sstream>
#include <iostream>
#include <fstream>
#include <typeinfo>

#include "utilities.h"
#include "world.h"
#include "animat.h"
#include "serialfuncs.h"
#include "unserialiser.h"

namespace BEAST {

/**
 * \addtogroup framework Simulation Framework
 * These classes wrap the interfaces of the GeneticAlgorithm and simulation
 * environment to provide an easy way to set up a range of common simulation
 * types. They also provide an interface to the GUI.
 * @{
 */

/**
 * An abstract base class for the Population template, allowing populations
 * with different templated types to be represented in Simulation.
 */
class SimObject
{
public:
	/// Virtual destructor.
	virtual ~SimObject(){}

	/// This method may be overidden to do processing between assessments.
	virtual void	BeginAssessment(){ AddToWorld(); }
	virtual void	EndAssessment(){}
	/// This method may be overidden to do processing between generations.
	virtual void	BeginGeneration(){}
	virtual void	EndGeneration(){}
	/// This method may be overidden to do processing between runs.
	virtual void	BeginRun(){}
	virtual void	EndRun(){}

	/// Calls the world's Add method on the contents of this object.
	virtual void	AddToWorld() = 0;
	/// Outputs object to a stream.
	virtual void	Serialise(std::ostream&)const{}
	/// Inputs object from a stream.
	virtual void	Unserialise(std::istream&){}
	/// Serialises the object to the specified file.
	virtual bool	Save(const char*)const;
	/// Unserialises the object from the specified file.
	virtual bool	Load(const char*);

	/// Returns the world in which the SimObject resides.
	World&			GetWorld()const		{ return *myWorld; }
	/// Sets the world used in AddToWorld
	void			SetWorld(World* w) { myWorld = w; }

	virtual std::string ToString()const{ return ""; }

private:
	/// A pointer to the World this Population belongs to
	World*			myWorld;
};

/** Output operator for all objects derived from SimObject. \relates SimObject */
std::ostream& operator<<(std::ostream& out, const SimObject& obj);
/** Input operator for all objects derived from SimObject. \relates SimObject */
std::istream& operator>>(std::istream& in, SimObject& obj);

/**
 * A simple class which creates and maintains a vector of objects of the
 * specified type and adds them to the world each round.
 * Note that Group is also responsible for deleting the objects it contains,
 * if you attempt to delete any of the objects in a Group yourself, there
 * will likely be segmentation faults.
 * \param _ObjType The type of objects to create.
 * \see Population
 * \see SimObject
 */
template <class _ObjType>
class Group : public SimObject, public std::vector<_ObjType*>
{
public:
	typedef _ObjType object_type;
	
	Group(int s = 0);
	/**
	 * Destructor - deletes all the objects in the group.
	 */
	virtual ~Group()
	{
		std::for_each
		(
		     this->begin(), 
		     this->end(), 
		     deleter<_ObjType>()
		 );
	}

	virtual void	AddToWorld();

	/**
	 * Calls the specified member function on each object
	 * in the Group.
	 */
	void ForEach(void (_ObjType::*method)())
	{	for (typename Group<_ObjType>::iterator i = this->begin(); i != this->end(); ++i)
			((*i)->*method)();
	}

	/**
	 * Calls the specified member function on each object
	 * in the group, passing an argument.
	 */
	template <typename _Arg>
	void ForEach(void (_ObjType::*method)(_Arg), _Arg arg)
	{	for (typename Group<_ObjType>::iterator i = this->begin(); i != this->end(); ++i)
			((*i)->*method)(arg);
	}

	/**
	 * Calls the specified member function on each object
	 * in the Group. The member function may be one returning
	 * a result, but the result will be discarded.
	 */
	template <typename _Result>
	void ForEach(_Result (_ObjType::*method)())
	{	for (typename Group<_ObjType>::iterator i = this->begin(); i != this->end(); ++i)
			((*i)->*method)();
	}

	/**
	 * Calls the specified member function on each object
	 * in the group, passing an argument. The member function
	 * may be one returning a result, but the result will be
	 * discarded.
	 */
	template <typename _Result, typename _Arg>
	void ForEach(_Result (_ObjType::*method)(_Arg), _Arg arg)
	{	for (typename Group<_ObjType>::iterator i = this->begin(); i != this->end(); ++i)
			((*i)->*method)(arg);
	}

	virtual void	Serialise(std::ostream&)const;
	virtual void	Unserialise(std::istream&);
};

/// Used in Simulation::ToString to specify what is to be output.
enum SimPrintStyleType
{
	SIM_PRINT_STATUS,		///< One-line description of simulation position.
	SIM_PRINT_ASSESSMENT,	///< Output at end of assessment.
	SIM_PRINT_GENERATION,	///< Output at end of generation.
	SIM_PRINT_RUN,			///< Output at end of run.
	SIM_PRINT_COMPLETE		///< Output at completion of simulation.
};

/**
 * The basic Simulation framework which must be derived to set up simulations.
 * This class cannot itself be instantiated. To derive your own Simulation:
 * - Create a new class, publicly derived from Simulation.
 * - Specify your simulation contents (e.g. Group and Population classes) as
 * member data.
 * - Create a public constructor which initialises the member data, then adds
 * each object using Simulation::Add.
 * - Perform any setup of the World, the Simulation or the contents in the
 * constructor.
 * - If you need to perform actions every epoch, override the virtual method,
 * Init.
 */
class Simulation
{
public:
	/// Constructor, creates an empty simulation.
	Simulation():
		runs(1), generations(0), assessments(1), timeSteps(1000),
		logStream(NULL){}

	virtual ~Simulation(){}

	virtual void	Init();
	virtual bool	Update();
	void			Display();
	std::string		ToString(SimPrintStyleType s = SIM_PRINT_STATUS)const;

	/// Adds a Population, Group or other SimObject to the world.
	void			Add(std::string name, SimObject& pop) { contents[name] = &pop; }

	void			ResetRun();
	void			ResetGeneration();
	void			ResetAssessment();

	/** Sets the number of runs for this simulation. Has no effect if the
	 * number of generations has not been specified using SetGenerations. */
	void			SetRuns(int r)			{ runs = r; }
	/** Sets the number of generations per run, if unset generations are
	 * unlimited. */
	void			SetGenerations(int g)	{ generations = g; }
	/// Sets the number of assessments per generation, default is one.
	void			SetAssessments(int a)	{ assessments = a; }
	/// Sets the number of time steps per assessment, default is 1000.
	void			SetTimeSteps(int t)		{ timeSteps = t; }

	/// Sets the output log stream.
	void			SetLogStream(std::ostream& o)
					{ logStream = &o; WorldObject::SetLogStream(o); }

	/// Checks if the specified simulation object is in the simulation.
	bool			HasSimObject(std::string n) { return contents.find(n) != contents.end(); }
	/// Gets the specified simulation object.
	SimObject&		GetSimObject(std::string n) { return *contents[n]; }
	/// Gets the current run.
	int				GetRun()const			{ return run; }
	/// Gets the current generation.
	int				GetGeneration()const	{ return generation; }
	/// Gets the current assessment.
	int				GetAssessment()const	{ return assessment; }
	/// Gets the current timestep.
	int				GetTimeStep()const		{ return timeStep; }

	/// Gets the total runs.
	int				GetTotalRuns()const		{ return runs; }
	/// Gets the total generations.
	int				GetTotalGenerations()const { return generations; }
		/// Gets the total assessments.
	int				GetTotalAssessments()const { return assessments; }
	/// Gets the total time steps.
	int				GetTotalTimeSteps()const{ return timeSteps; }

	///	Gets the log stream.
	std::ostream&	GetLogStream()			{ return *logStream; }

	/// Gets the World object for this simulation.
	World&			GetWorld()				{ return theWorld; }

	const std::map<std::string, SimObject*>&
					GetContents()const		{ return contents; }

protected:
	void			Generate();
	void			TimeUp();

	virtual void	BeginAssessment();
	virtual void	EndAssessment();
	virtual void	BeginGeneration();
	virtual void	EndGeneration();
	virtual void	BeginRun();
	virtual void	EndRun();

	std::map<std::string, SimObject*>&
					GetContents()			{ return contents; }

private:
	/**
	 * The World object for this simulation. Simulations only use one World at a
	 * time, but might be extended to use multiple worlds.
	 */
	World			theWorld;
	/// Pointers to every simulation object.
	std::map<std::string, SimObject*> contents;

	int				runs;			///< Number of whole simulation repetitions.
	int				generations;	///< Number of generations (e.g. GA runs).
	int				assessments;	///< Number of assessments per generation.
	int				timeSteps;		///< Number of time steps per assessment.

	int				run;			///< Current run.
	int				generation;		///< Current generation.
	int				assessment;		///< Current assessment.
	int				timeStep;		///< Current timestep.
	bool			complete;		///< True when simulation is over.

	std::ostream*	logStream;		///< Log output stream.
};

/**
 * Sets the Group up with a number of objects of the native type.
 * \param s The number of objects to be created.
 */
template <class _ObjType>
Group<_ObjType>::Group(int s): std::vector<_ObjType*>(s)
{
	std::generate
        (
            this->begin(), 
	    this->end(), 
	    creator<_ObjType>()
	);
}

/// Adds the contents of the Group to the World.
template <class _ObjType>
void Group<_ObjType>::AddToWorld()
{
	GetWorld().Add(*this);
}


/**
 * \todo Finish!
 */
template <class _ObjType>
void Group<_ObjType>::Serialise(std::ostream& out)const
{
	std::string name = add_slashes(std::string("Group_")
								   + typeid(_ObjType).name());
	out << name << "\n"
		<< static_cast<int>(this->size()) << "\n";

	typename Group<_ObjType>::const_iterator i = this->begin();
	for (; i != this->end(); ++i) {
		out << **i;
	}
}

/**
 * \todo Add general object unserialiser
 */
template <class _ObjType>
void Group<_ObjType>::Unserialise(std::istream& in)
{
	std::string name, expName = add_slashes(std::string("Group_")
											+ typeid(_ObjType).name());
	in >> name;
	if (name != expName) {
		throw SerialException(SERIAL_ERROR_WRONG_TYPE, strip_slashes(name), 
							  "This object is type " + strip_slashes(expName));
	}

	/** \todo Rather than removing only objects from this group, all objects
	 * with the same type as this group are removed. */
//	GetWorld().Remove<_ObjType>();	// disabled because of some error with g++
	GetWorld().CleanUp();
	std::for_each
        (
            this->begin(), 
	    this->end(), 
	    deleter<_ObjType>()
	);
	this->clear();
	
	int size;
	in >> size;

	WorldObject* wobj;
	_ObjType* obj;

	for (int i=0; i < size; ++i) {
		wobj = Unserialiser::Instance()(in);
		obj = dynamic_cast<_ObjType*>(wobj);
		if (obj != NULL) {
			this->push_back(obj);
		}
	}
}

/*
 * @}
 */

} // namespace BEAST

// Population is included down here instead of including simulation.h at the
// top of population.h, so that by including simulation.h the Population
// template becomes available rather than vice-versa.
#include "population.h"

#endif
