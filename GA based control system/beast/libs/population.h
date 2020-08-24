#ifndef _POPULATION_H
#define _POPULATION_H

#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>

#include "world.h"
#include "geneticalgorithm.h"
#include "serialfuncs.h"
#include "simulation.h"

namespace BEAST {

/**
 * \addtogroup framework
 * @{
 */

/**
 * This class is derived from Group and adds a managed GA which is
 * automatically run on the whole Population every epoch.
 * \param _Ind The type of objects to create, must inherit from Evolver.
 * \param _MutFunc If your GA uses a custom mutation operator, it must also be
 * specified here.
 * \see GeneticAlgorithm
 * \see Evolver
 * \see Group
 */
template <class _Ind, class _MutFunc = MutationOperator<typename _Ind::gene_type> >
class Population : public Group<_Ind>
{
public:
	Population(int s, GeneticAlgorithm<_Ind, _MutFunc>& ga):
		Group<_Ind>(s),GA(ga),teamSize(-1),numClones(1){}
	virtual ~Population(){}

	virtual void	BeginAssessment();
	virtual void	EndAssessment();
	virtual void	BeginGeneration();
	virtual void	EndGeneration();
	virtual void	BeginRun();
	virtual void	EndRun();
	virtual void	AddToWorld();

	/// Decides how many individuals will go into assessments.
	void			SetTeamSize(int n)	{ teamSize = n; }
	/** Allows you to specify the number of clones to be made of each
	 * individual in each assessment. If you want to assess clones of
	 * only one individual, set the team size to one. */
	void			SetClones(int n)	{ numClones = n; }

	/// Returns the current team.
	const std::vector<_Ind*>& GetTeam()const	{ return team; }
	
	/// Outputs data about the current generation, currently only from the GA.
	virtual std::string ToString()const
	{
		return GA.ToString();
	}
	
	virtual void	Serialise(std::ostream&)const;
	virtual void	Unserialise(std::istream&);

	//virtual void printPlot()const;

	GeneticAlgorithm<_Ind, _MutFunc>& GA;

private:
	int				teamSize;
	int				numClones;

	typename Population<_Ind, _MutFunc>::iterator current;
	std::vector<_Ind*> team;

	/**
	* Used by Population to create a clone of an individual.
	* \see Population::BeginAssessment
	*/
	struct Clone : public std::unary_function<_Ind*, _Ind*>
	{
		_Ind* operator()(_Ind* in)
		{
			_Ind* out = new _Ind(*in);
			out->SetGenotype(in->GetGenotype());
			return out;
		}
	};

	/**
	* Merges two individuals such that the resulting individual's fitness scores
	* will contain all the scores of both individuals.
	* \see Population::EndAssessment
	*/
	struct UnClone : public std::binary_function<_Ind*, _Ind*, _Ind*>
	{
		_Ind* operator()(_Ind* in1, _Ind* in2)
		{
			in1->GAFitnessScores.push_back(in2->GetFitness());
			delete in2;

			return in1;
		}
	};
};

/**
 * Adds either the whole population, or the currently selected team to the
 * world.
 */
template <class _Ind, class _MutFunc>
void Population<_Ind, _MutFunc>::AddToWorld()
{
	if (teamSize == -1) {
		this->GetWorld().Add(*this);
	}
	else {
		this->GetWorld().Add(team);
	}
}

/**
 * This method is called at the beginning of the assessment and sets up teams
 * if required.
 */
template <class _Ind, class _MutFunc>
void Population<_Ind, _MutFunc>::BeginAssessment()
{
	if (teamSize != -1) {
		team.clear();
		for (int i = 0; i < teamSize; ++i) {
			if (++current == this->end()) 
			      current = this->begin();
			team.push_back(*current);
		}
	}

	using namespace std;
	// If numClones > 1, clones are made:
	for (int i=1; i<numClones; ++i) {
		team.resize(team.size() + teamSize);
		transform
		(
		    team.begin(), 
		    team.begin() + teamSize, 
		    team.begin() + i * teamSize, 
		    Clone()
                );
	}

	Group<_Ind>::BeginAssessment();
}

/**
 * This method is called at the end of the assessment and causes each
 * individual's fitness score to be stored.
 */
template <class _Ind, class _MutFunc>
void Population<_Ind, _MutFunc>::EndAssessment()
{
	using namespace std;

	if (teamSize != -1)
		for_each(team.begin(), team.end(), mem_fun(&_Ind::StoreFitness));
	else
		for_each
		(
		     this->begin(), 
		     this->end(), 
		     mem_fun(&_Ind::StoreFitness)
		);

	for (int i=1; i<numClones; ++i) {
		transform(team.begin(), team.begin() + teamSize,
				  team.end() - teamSize, team.begin(), UnClone());
		if (static_cast<int>(team.size()) > teamSize) {
			team.resize(team.size() - teamSize);
		}
	}

}

/**
 * This method is called at the beginning of the generation and
 */
template <class _Ind, class _MutFunc>
void Population<_Ind, _MutFunc>::BeginGeneration()
{
	current = this->begin();
}

/**
 * Performs GA management, by copying the Population into the GA, running the
 * GA on the population and then retrieving the new generation from the GA.
 * \see GeneticAlgorithm
 * \see GeneticAlgorithm::Generate
 */
template <class _Ind, class _MutFunc>
void Population<_Ind, _MutFunc>::EndGeneration()
{
	GA.SetPopulation(*this);
	GA.Generate();

	for_each
        (
	     this->begin(), 
	     this->end(), 
	     deleter<_Ind>()
	);
	this->clear();
	copy(GA.begin(), GA.end(), inserter(*this, this->begin()));
}

/**
 * This method is called at the beginning of the run and ensures that the
 * contents of the population has been reset.
 */
template <class _Ind, class _MutFunc>
void Population<_Ind, _MutFunc>::BeginRun()
{
	std::for_each
	(
	     this->begin(), 
	     this->end(), 
	     deleter<_Ind>()
	);
	std::generate
        (
	     this->begin(), 
	     this->end(), 
	     creator<_Ind>()
	);
}

/**
 * This method is called at the end of the run and currently doesn't do
 * anything at all.
 */
template <class _Ind, class _MutFunc>
void Population<_Ind, _MutFunc>::EndRun()
{
}

/**
 * Copies the Population to a stream. Note that only the GA data and the
 * genotypes of the population are stored.
 * \see Population::Unserialise
 */
template <class _Ind, class _MutFunc>
void Population<_Ind, _MutFunc>::Serialise(std::ostream& out)const
{
	using namespace std;

	std::string name = add_slashes(std::string("Population_") 
								   + typeid(_Ind).name());
	out << name << "\n"
		<< GA
		<< static_cast<int>(this->size()) << "\n";

	typename Population<_Ind, _MutFunc>::const_iterator i = this->begin();
	
	for (; i != this->end(); ++i) {
		out << (*i)->GetGenotype();
	}
}

/**
 * Converts the data produced by Serialise back into a population.
 * \see Population::Serialise
 */
template <class _Ind, class _MutFunc>
void Population<_Ind, _MutFunc>::Unserialise(std::istream& in)
{
	std::string name, expName = add_slashes(std::string("Population_") 
											+ typeid(_Ind).name());
	in >> name;
	if (name != expName) {
		throw SerialException(SERIAL_ERROR_WRONG_TYPE, strip_slashes(name), 
							  "This object is type " + strip_slashes(expName));
	}

	int s;
	typename _Ind::genotype_type g;

	in >> GA >> s;

	std::for_each
	(
	    this->begin(), 
	    this->end(), 
	    deleter<_Ind>()
	);

	this->clear();
	this->resize(s);

	typename Population<_Ind, _MutFunc>::iterator i = this->begin();
	
	for (; i != this->end(); ++i) {
		in >> g;
		*i = new _Ind;
		(*i)->SetGenotype(g);
	}
}

/**
 * @}
 */

} // namespace BEAST

#endif

