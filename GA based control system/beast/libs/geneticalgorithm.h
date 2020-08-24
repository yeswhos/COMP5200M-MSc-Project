/**
 * \file geneticalgorithm.h
 * \author David Gordon
 */

#ifndef _GENETIC_ALGORITHM_H_
#define _GENETIC_ALGORITHM_H_

#include <vector>
#include <map>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stack>
#include <typeinfo>
#include <functional>

#include "random.h"
#include "serialfuncs.h"

namespace BEAST {

/**
 * \addtogroup biosystems Biosystems-Related Classes
 * This collection of classes implement a range of biosystems algorithms and
 * control systems including a multi-purpose genetic algorithm and two neural
 * nets. There are also classes for using these controllers in the simulation
 * environment.
 * @{
 */

/**
 * \par
 * A functor which may be initialised with max and min values, and then
 * returns a uniformly distributed random number between those values.
 * MutationOperator may be adapted in a number of ways to suit different GA
 * requirements:
 * \par
 * The template can be initialised as any numeric type and will return good
 * results for real values, usable results for integer types (remember to add
 * one to the max limit to take account of the lack of proper rounding.)
 * \par
 * The template can be specialised to provide an alternative default mutation
 * operator for whichever type you are using for your genotype, e.g. you might
 * want to make a gaussian distributed float mutation operator.
 * \par
 * Alternatively you could do away with MutationOperator entirely and specify
 * a different functor type when initialising your GA (the GA defaults to a
 * mutation operator of a type matching the GA's gene type)
 */
template <typename T>
struct MutationOperator
{
	/// The maximum and minimum random values can be optionally specified
	MutationOperator(T min = -1.0, T max = 1.0):minimum(min), maximum(max){}
	/// Performs a random mutation on the argument and returns the result
	T operator()(T t) { return t + randval(maximum - minimum) + minimum; }

	T minimum,	///< Minimum mutation
	  maximum;	///< Maximum mutation
};

template <typename T>
struct NormalMutator
{
	/// The maximum and minimum random values can be optionally specified
	NormalMutator(T m = 0.0, T s = 0.5):mean(m), sd(s){}
	/// Performs a random mutation on the argument and returns the result
	T operator()(T t) { return normrand(mean, sd); }

	T mean,	///< Mean
	  sd;	///< Standard deviation
};

/**
 * Output operator for the MutationOperator function object. Note that in
 * order for this function to work, there must exist an output operator for
 * the mutation operator's type.
 * \see IMPLEMENT_IOSTREAM_CAST
 * \see IMPLEMENT_IOSTREAM_BINARY_CONVERSION
 */
template <typename T>
std::ostream& operator<<(std::ostream& out, const MutationOperator<T>& m)
{
	out << m.minimum << " " << m.maximum << std::endl;
	// MdK: 02-01-2007, added return statement
	return out;
}

/**
 * Input operator for the MutationOperator function object. Note that in
 * order for this function to work, there must exist an input operator for
 * the mutation operator's type.
 * \see IMPLEMENT_IOSTREAM_CAST
 * \see IMPLEMENT_IOSTREAM_BINARY_CONVERSION
 */
template <typename T>
std::istream& operator>>(std::istream& in, MutationOperator<T>& m)
{
	in >> m.minimum >> m.maximum;
	// MdK: 02-01-2007, added return statement
	return in;
}

/**
 * \par
 * The Evolver class is an abstract base class from which you may derive the
 * objects which will comprise your population. The best approach is to use
 * multiple inheritance and create an evolvable subclass of whatever it is you
 * want to use the GA on.
 * \par
 * The type specified must match the type specified in the first template
 * parameter of your GA, otherwise it won't work. You are not limited to basic
 * types for your genes, any class may be used, but you will need to provide
 * a suitable mutation operator for that class.
 */
template <typename T>
class Evolver
{
public:

	typedef T						gene_type;
	typedef typename std::vector<T>	genotype_type;

	virtual ~Evolver(){}

	virtual genotype_type GetGenotype()const = 0;	///< Returns the genotype.
	virtual void		SetGenotype(const genotype_type&) = 0;	///< Sets the genotype.
	virtual float		GetFitness()const = 0;	///< Returns the fitness.
	
	/** Stores current fitness, overload this for one way of resetting
	 * individuals' internal fitness scores each assessment (another way might
	 * be e.g. to overload Init).
	 * \note If you are using only one fitness score per individual you do not
	 * need to call StoreFitness, the GA will simply get the fitness from the
	 * fitness function */
	virtual void		StoreFitness() { GAFitnessScores.push_back(GetFitness()); }
	std::vector<float>	GAFitnessScores;	///< A list of previous scores.
	float				GAProbability;		///< Used by GeneticAlgorithm.
	float				GAFixedFitness;		///< Used by GeneticAlgorithm.

	genotype_type		PSOBestSolution;
	float				PSOBestFitness;
};

/// The different options for selection are enumerated here.
enum GASelectionType {
	GA_ROULETTE = 0,	///< Fitness proportional selection
	GA_RANK,			///< Rank proportional selection
	GA_TOURNAMENT		///< Tournament selection
};

/// Assorted float parameters, set using GeneticAlgorithm::SetParameter
enum GAFltParamType {
	GA_TOURNAMENT_PARAM, ///< Probability of fittest winning in tournament selection
	GA_RANK_SPRESSURE,	///< Selection pressure for rank selection, [1.0:2.0]
	GA_EXPONENT			///< An expontial modifier for rank and roulette probabilities
};

/// Assorted integer parameters, set using GeneticAlgorithm::SetParameter
enum GAIntParamType {
	GA_TOURNAMENT_SIZE	///< Number of individuals per round of tournament selection
};

/// Use to set the printing style when using GA's << operator
enum GAPrintStyleType {
	GA_PARAMETERS = 1,	///< Display the current parameters
	GA_CURRENT = 2,		///< Display details of current generation
	GA_GENERATION = 4,	///< Display all members of current generation
	GA_HISTORY = 8		///< Show average and best fitnesses of each generation
};

/// The method by which fitness is decided when individuals have multiple scores
enum GAFitnessMethodType {
	GA_BEST_FITNESS,	///< The best fitness score is used
	GA_WORST_FITNESS,	///< The worst fitness score is used
	GA_MEAN_FITNESS,	///< The average fitness is used
	GA_TOTAL_FITNESS	///< The total fitness is used
};

/// Sets the method by which fitness scores are adjusted before selection
enum GAFitnessFixType	{
	GA_IGNORE,			///< Leave them as they are
	GA_CLAMP,			///< Minimum fitness is clamped at 0
	GA_FIX				///< Worst score set to 0, others scaled up
};

template <class EVO, class MUTFUNC> class GeneticAlgorithm;
/// The GA's output operator
template <class EVO, class MUTFUNC>
std::ostream& operator<< (std::ostream&, const GeneticAlgorithm<EVO, MUTFUNC>&);
/// The GA's input operator
template <class EVO, class MUTFUNC>
std::istream& operator>> (std::istream&, const GeneticAlgorithm<EVO, MUTFUNC>&);

/**
 * The GeneticAlgorithm class provides functionality to cover a range of GA
 * methods, and may be extended to incorporate other approaches. The class is
 * completely generic, in order to allow the widest possible application.
 * The first template parameter is the type of the genes (e.g. int, float).
 * The second is a class which must provide the methods exposed by the
 * \par
 * Evolver ABC, ideally inherited from an Evolver of the same templated type.
 * The third template parameter is the type of the mutation operator and will
 * usually be fine as the default type of MutationOperator<T>, which is a
 * MutationOperator function object with the same type as the genes.
 * Phew!
 */
template <class EVO, class MUTFUNC = MutationOperator<typename EVO::gene_type> >
class GeneticAlgorithm
{
public:
	/// A typedef for the gene type (e.g. float), adapted from EVO
	typedef typename EVO::gene_type GENE;
	/// A typedef for the genotype type (e.g. vector<float>), adapted from EVO
	typedef typename EVO::genotype_type GENOTYPE;
	/// An iterator type for the population
	typedef typename std::vector<EVO*>::iterator PopIter;
	/// A const iterator type for the population
	typedef typename std::vector<EVO*>::const_iterator PopConstIter;
	/// An iterator type for the chromosomes
	typedef typename std::vector<GENE>::iterator ChromIter;

	GeneticAlgorithm(float crossover = 0.7f, float mutation = 0.01f, int popSize = 0);
	virtual ~GeneticAlgorithm();

	void			SetPopulation(const std::vector<EVO*>& p){ population = p; }
	std::vector<EVO*>&	GetPopulation()		 			{ return outputPopulation; }
	std::vector<EVO*>	GetPopulationCopy()const;
	PopIter			begin() { return outputPopulation.begin(); }
	PopIter			end() { return outputPopulation.end(); }

	/** Sets the rate of crossover
	 * \param c A float in the range [0,1] */
	void			SetCrossover(float c)				{ crossover = c; }
	/** Sets the rate of mutation
	 * \param m A float in the range [0,1] */
	void			SetMutation(float m)				{ mutation = m; }
	/** Sets the selection operator
	 * \see GASelectionType */
	void			SetSelection(GASelectionType s) 	{ selection = s; }
	/** Sets the mutation function, which may be any C function which takes as
	 * its argument one variable of the gene type and returns a variable of the
	 * gene type. */
	void			SetMutationFunction(GENE (*ptr)(GENE))	{ mutFuncPtr = ptr; }
	/** Sets the mutation function to a function object, the type of which
	 * should also be specified in the template constructor for the GA. */
	void			SetMutationFunction(MUTFUNC obj)	{ mutFuncObj = obj; }
	/** Sets the elitism value, which decides how many of the fittest individuals
	 * go through to the next generation unchanged.
	 * \param e An integer in the range [0,population size] */
	void			SetElitism(int e)					{ elitism = e; }
	/** Sets the subelitism value, which decides how many of the least fit
	 * individuals are barred from reproducing.
	 * \param s An integer in the range [0,population size] */
	void			SetSubelitism(int s)				{ subelitism = s; }
	/** Sets the number of points of crossover.
	 * \param p An integer >= 0 (0 results in no crossover) */
	void			SetCrossoverPoints(int p)			{ crossoverPoints = p; }
	/** Sets the method by which fitness scores are used.
	 * \see GAFitnessMethodType */
	void			SetFitnessMethod(GAFitnessMethodType f)	{ fitnessMethod = f; }
	/** Sets the method by which fitness scores < 0 are treated.
	 * \see GAFitnessFixType */
	void			SetFitnessFix(GAFitnessFixType f)	{ fitnessFix = f; }

	/** Set to true if the GA is responsible for deleting the old population
	 * objects. */
	void			SetOwnsData(bool b)					{ ownsData = b; }
	/** Sets a real-valued parameter
	 * \param p The parameter to set
	 * \param f The value to set it to.
	 * \see GAFltParamType */
	void			SetParameter(GAFltParamType p, float f) { fltParams[p] = f; }
	/** Sets an integer-valued parameter
	 * \param p The parameter to set
	 * \param n The value to set it to.
	 * \see GAIntParamType */
	void			SetParameter(GAIntParamType p, int n) { intParams[p] = n; }
	/** Sets the output style of the GA's ToString method
	 * \see GAPrintStyleType */
	void			SetPrintStyle(int p)				{ printStyle = p; }

	virtual void	Generate();
	void			ReGenerate();

	/** \return The number of Generations so far */
	int				GetGenerations()const				{ return generations; }
	std::vector<float>	GetAvgFitnessHistory()const		{ return bestFitnessRecord; }
// Mdk: 02-02-2007, disabled after consultation with David Gordon
//	std::vector<float>	GetBestFitnessHistory()const	{ return bestFitnessHistory; }
	std::vector<GENE>	GetBestCurrentGenome()const		{ return bestCurrentGenome; }
	std::vector<GENE>	GetBestEverGenome()const		{ return bestEverGenome; }
	float			GetBestCurrentFitness()const		{ return bestFitness; }
	float			GetBestEverFitness()				{ return bestEverFitness; }

	std::string		ToString()const;
	std::string		GetCSV(char separator = ',')const;

	void			CleanUp();

	void			Unserialise(std::istream&);
	void			Serialise(std::ostream&)const;

protected:
	// Methods used in creation of a new generation
	void			CalcStats();
	void 			Setup();
	void 			FixFitness();
	float			GetFitness(EVO* i);
	void			SelectParentGenotype(GENOTYPE&);
	void			CrossoverGenotypes(GENOTYPE&, GENOTYPE&);
	void			MutateGenotype(GENOTYPE&);

	// Selection methods
	void			SelectProbability(GENOTYPE&);
	void			SelectEven(GENOTYPE&);
	void			SelectTournament(GENOTYPE&);

	/// A little function object to enable us to sort the population by fitness
	template <class _EVO>
	struct evo_sort {
		bool operator()(_EVO*const & p1, _EVO*const & p2)
		{ return p1->GAFixedFitness < p2->GAFixedFitness;}
	};

private:
	std::vector<EVO*>	population;		///< Stores the input (working) population
	std::vector<EVO*>	outputPopulation;///< Stores the newly generated population

	// These attributes define the behaviour of the algorithm
	float			crossover;			///< The rate of crossover
	float			mutation;			///< The rate of mutation
	GASelectionType selection;			///< The selection method
	int				outputPopSize;		///< Size of population to produce
	int				elitism;			///< Number to go through unchanged
	int				subelitism;			///< Number barred from reproduction
	int				crossoverPoints;	///< Number of crossover points (default 1)
	GAFitnessMethodType fitnessMethod;	///< Selection method for multiple fitnesses
	GAFitnessFixType fitnessFix;		///< What to do with fitness scores < 0

	std::map<GAFltParamType,float>	fltParams;	///< float parameters
	std::map<GAIntParamType,int>	intParams;	///< integer parameters
	bool			ownsData;			///< Whether the GA is responsible for deletion
	int				printStyle;			///< The current printing style

	// Attributes for storing the algorithm's history
	int					generations;		///< Number of generations so far
	std::vector<float>	averageFitnessRecord;	///< Average fitness of each generation
	std::vector<float>	bestFitnessRecord;	///< Best fitness of each generation
	float				bestEverFitness;	///< Best fitness score so far
	std::vector<GENE>	bestEverGenome;		///< A copy of the best candidate ever
	std::vector<GENE>	bestCurrentGenome;	///< The best candidate of this generation

	// Data used during creation of a new generation
	int				inputPopSize;		///< The size of the input population
	int				chromoLength;		///< Chromosome length, calculated from first individual
	float			totalFitness;		///< The total unscaled fitness of the whole population
	float			bestFitness;		///< The best fitness in the current generation
	float			totalFixedFitness;	///< Total fitness after scaling
	float			worstFitness;		///< Lowest fitness score in the whole population
	float			totalProbability;	///< Total of probability distribution calculation

	/// The mutation function, either a unary function object...
	MUTFUNC			mutFuncObj;
	/// ... or a pointer to a unary function
	GENE			(*mutFuncPtr)(GENE);
};


/**
 * Constructor: sets default parameters:
 * elitism and subelitism 0, crossover points 1, tournament parameter 0.75,
 * tournament size 2, rank pressure 1.5, exponent 1.0
 */
template <class EVO, class MUTFUNC>
GeneticAlgorithm<EVO, MUTFUNC>::GeneticAlgorithm(float c, float m, int p):
crossover(c), mutation(m), selection(GA_ROULETTE), outputPopSize(p),
elitism(0), subelitism(0), crossoverPoints(1), fitnessMethod(GA_MEAN_FITNESS),
fitnessFix(GA_IGNORE),
ownsData(false), printStyle(GA_CURRENT),
generations(0), bestFitness(0.0f), mutFuncPtr(NULL)
{
	fltParams[GA_TOURNAMENT_PARAM] = 0.75f;
	fltParams[GA_RANK_SPRESSURE] = 1.5f;
	fltParams[GA_EXPONENT] = 1.0f;

	intParams[GA_TOURNAMENT_SIZE] = 2;
}

template <class EVO, class MUTFUNC>
GeneticAlgorithm<EVO, MUTFUNC>::~GeneticAlgorithm()
{
	// CleanUp();
}


/**
 * Returns duplicates of the output population which won't be deleted by the
 * GA if it does a CleanUp()
 */
template <class EVO, class MUTFUNC>
std::vector<EVO*> GeneticAlgorithm<EVO, MUTFUNC>::GetPopulationCopy()const
{
	std::vector<EVO*> popCopy;

	for (PopConstIter i = outputPopulation.begin(); i != outputPopulation.end(); ++i) {
		popCopy.push_back(new EVO(**i));
	}

	return popCopy;
}


/**
 * The generation function: this is where it all happens.
 * First the total, best, worst and average fitnesses are calculated with
 * CalcStats(). Then fitness scaling and probability distributions are
 * done by Setup(). Elitism and subelitism are dealt with and then the
 * new population is generated using the current selection method,
 * crossover parameters and mutation operator.
 */
template <class EVO, class MUTFUNC>
void GeneticAlgorithm<EVO, MUTFUNC>::Generate()
{
	CalcStats();
	Setup();

	std::vector<GENE> chromo1, chromo2;
	EVO *evo;

	int i = 0, j;
	PopIter iter = population.begin();

	if (elitism + subelitism < inputPopSize - 2) {
		// First, remove the worst <subelitism> individuals from the population to
		// ensure they never get to pass on their rubbish genes.
		for (; i < subelitism; ++i) {
			population.pop_back();
		}

		// Next, fill the first <elitism> slots in the new generation with the
		// top individuals from the input population
		for (i = 0; i < elitism && iter != population.end(); ++i, ++iter) {
			evo = new EVO;
			evo->SetGenotype((*iter)->GetGenotype());
			outputPopulation.push_back(evo);
		}
	}

	/** \todo Account for odd numbered output population sizes */
	for (; i < outputPopSize / 2; ++i) {
		SelectParentGenotype(chromo1);
		SelectParentGenotype(chromo2);
		for (j=0; j<crossoverPoints; ++j) {
			if (randval(1.0) < crossover) CrossoverGenotypes(chromo1, chromo2);
		}
		MutateGenotype(chromo1);
		MutateGenotype(chromo2);
		evo = new EVO;
		evo->SetGenotype(chromo1);
		outputPopulation.push_back(evo);
		evo = new EVO;
		evo->SetGenotype(chromo2);
		outputPopulation.push_back(evo);
	}

	if (ownsData) {
		// Delete the input population
		for (PopIter i = population.begin(); i != population.end(); ++i) {
			delete *i;
		}
	}
}


/**
 * Calls the Generate() method and copies the output population into the input
 */
template <class EVO, class MUTFUNC>
void GeneticAlgorithm<EVO, MUTFUNC>::ReGenerate()
{
	Generate();
	population = outputPopulation;
}

/**
 * Calculates some statistics used in some of the selection methods and also
 * stored by the class for data collection
 */
template <class EVO, class MUTFUNC>
void GeneticAlgorithm<EVO, MUTFUNC>::CalcStats()
{
	// First find out how many we're dealing with
	inputPopSize = static_cast<int>(population.size());

	// Now calculate total fitness and find best fitness
	float f;	// just a temp storage value

	PopIter iter = population.begin();
	EVO* bestEvoSoFar = *iter;
	totalFitness = bestFitness = worstFitness = GetFitness(*iter);

	for (++iter; iter != population.end(); ++iter) {
		f = GetFitness(*iter);

		if (f > bestFitness) {
			bestEvoSoFar = *iter;
			bestFitness = f;
		}
		else if (f < worstFitness) {
			worstFitness = f;
		}

		totalFitness += f;
	}

	// Store some statistics
	++generations;
	averageFitnessRecord.push_back(totalFitness / static_cast<float>(inputPopSize));
	bestFitnessRecord.push_back(bestFitness);
	bestCurrentGenome = bestEvoSoFar->GetGenotype();
	if (bestFitness > bestEverFitness) {
		bestEverFitness = bestFitness;
		bestEverGenome = bestCurrentGenome;
	}
}

/**
 * Prepares the GA for the next epoch. Output population is cleared, input
 * population is sorted by fitness, population probabilities are set according
 * to the selection method in use, the chromosome length is determined, etc.
 */
template <class EVO, class MUTFUNC>
void GeneticAlgorithm<EVO, MUTFUNC>::Setup()
{
	outputPopulation.clear();
	if (outputPopSize == 0) outputPopSize = static_cast<int>(population.size());
	/** TODO: perhaps make chromoLength the shortest of any given pair? */
	chromoLength = static_cast<int>(population.front()->GetGenotype().size());

	FixFitness();

	// Elitism, subelitism and rank selection require population to be
	// sorted by fitness (best-worst)
	sort(population.rbegin(), population.rend(), evo_sort<EVO>());

	// Probability distribution Setup:
	// Each individual has a public attribute, GAProbability, which is used
	// to store probability distribution data. In the following code this
	// attribute is precalculated in order to speed up the selection process.

	totalProbability = 0;

	// Roulette wheel selection:
	// Each individual's GAProbability attribute is primed with a value
	// corresponding to their fitness compared to the overall population.
	if (selection == GA_ROULETTE) {
		for (PopIter i = population.begin(); i != population.end(); ++i) {
			(*i)->GAProbability = static_cast<float>(pow((*i)->GAFixedFitness / totalFixedFitness,
														 fltParams[GA_EXPONENT]));
			totalProbability += (*i)->GAProbability;
		}
	}

	// Rank selection:
	// The relative rank of the invidual is used, preventing one or two
	// overwhelmingly fit individuals in a population from dominating the next
	// generation. So if in a population of 3 individuals you had scores of
	// (2, 3, 400), rank selection would convert these to (0.167, 0.333, 0.5).
	if (selection == GA_RANK) {
		float sp = fltParams[GA_RANK_SPRESSURE];
		float fltInputPopSize = static_cast<float>(inputPopSize);

		PopIter it = population.begin();
		for (int i = inputPopSize; it != population.end(); --i, ++it) {
			// This handy ranking equation ensures that with no exponent
			// (GA_EXPONENT==1) the total probability is 1, useful for
			// debugging.
			(*it)->GAProbability = (2.0f - sp + 2.0f * (sp - 1.0f)
								 * static_cast<float>(i - 1)
								 / static_cast<float>(inputPopSize - 1))
								 / fltInputPopSize;
			(*it)->GAProbability = static_cast<float>(pow((*it)->GAProbability,fltParams[GA_EXPONENT]));
			totalProbability += (*it)->GAProbability;
		}
	}
}

/**
 * Adjusts the fitness according to
 */
template <class EVO, class MUTFUNC>
void GeneticAlgorithm<EVO, MUTFUNC>::FixFitness()
{
	float f;
	totalFixedFitness = 0;

	for (PopIter i = population.begin(); i != population.end(); ++i) {
		f = GetFitness(*i);
		switch (fitnessFix) {
		case GA_FIX: f -= worstFitness; break;
		case GA_CLAMP: f = (f < 0 ? 0 : f); break;
		case GA_IGNORE: break;
		}
		(*i)->GAFixedFitness = f;

		totalFixedFitness += f;
	}
}

/**
 * Calculates the fitness score to be used by the GA from the stored fitness
 * scores in the EVO object. If no scores have been stored, the output from
 * EVO::GetFitness is returned.
 */
template <class EVO, class MUTFUNC>
float GeneticAlgorithm<EVO, MUTFUNC>::GetFitness(EVO* i)
{
	using namespace std;

	if (i->GAFitnessScores.empty()) {
		return i->GetFitness();
	}

	switch (fitnessMethod) {
	case GA_BEST_FITNESS:
		return *max_element(i->GAFitnessScores.begin(),
						   i->GAFitnessScores.end());
	case GA_WORST_FITNESS:
		return *min_element(i->GAFitnessScores.begin(),
						   i->GAFitnessScores.end());
	case GA_MEAN_FITNESS:
		return accumulate(i->GAFitnessScores.begin(),
						  i->GAFitnessScores.end(), 0.0f)
			 / static_cast<float>(i->GAFitnessScores.size());
	case GA_TOTAL_FITNESS:
		return accumulate(i->GAFitnessScores.begin(),
						  i->GAFitnessScores.end(), 0.0f);
	default:
		return 0.0f;
	}
}

/**
 * Depending on the selection procedure chosen, a genotype is taken from the
 * population and returned by reference.
 */
template <class EVO, class MUTFUNC>
void GeneticAlgorithm<EVO, MUTFUNC>::SelectParentGenotype(GENOTYPE& chromo)
{
	switch (selection) {
	case GA_ROULETTE:
	case GA_RANK:			SelectProbability(chromo);	break;
	case GA_TOURNAMENT:		SelectTournament(chromo);	break;
	}
}


/**
 * Roulette and Rank Selection
 * Having done the Setup function (above), each individual has a
 * probability score, derived from their rank or fitness.
 * Imagine all the probability scores of the population as a big pie
 * chart, printed on a roulette wheel. Now imagine numbers around the
 * edge, starting at 0, going up to the total probability of selection.
 * We pick a random number between 0 and 1, and call it slice.
 * This is where our roulette ball will land.
 */
template <class EVO, class MUTFUNC>
void GeneticAlgorithm<EVO, MUTFUNC>::SelectProbability(GENOTYPE& chromo)
{
	float slice = randval(totalProbability);

	PopIter i = population.begin();
	for (; i != population.end(); ++i) {
		slice -= (*i)->GAProbability;
		if (slice < 0) break;
	}

	if (i != population.end()) {
		chromo = (*i)->GetGenotype();
	}
	else {
		chromo = (*(i - 1))->GetGenotype();
	}
}


/**
 * Tournament Selection
 * The method implemented here is an amalgamation of two slightly different
 * approaches. GA_TOURNAMENT_SIZE individuals are selected at random from
 * the population. With GA_TOURNAMENT_PARAM probability, the fittest
 * individual is picked, otherwise a random individual (perhaps still the
 * fittest) is chosen from the tournament.
 */
template <class EVO, class MUTFUNC>
void GeneticAlgorithm<EVO, MUTFUNC>::SelectTournament(GENOTYPE& chromo)
{
	std::vector<EVO*> tournament;

	// Pick GA_TOURNAMENT_SIZE individuals at random
	for (int i=0; i<intParams[GA_TOURNAMENT_SIZE]; ++i) {
		tournament.push_back(population[irand(inputPopSize)]);
	}

	// Decide the winner...
	EVO* winner;
	// Depending on GA_TOURNAMENT_PARAM,
	if (randval(1.0f) < fltParams[GA_TOURNAMENT_PARAM]) {
		// The fittest individual is chosen:
		winner = *(max_element(tournament.begin(), tournament.end(), evo_sort<EVO>()));
	}
	else {
		// Or an invidiual is chosen at random
		winner = tournament[irand(intParams[GA_TOURNAMENT_SIZE])];
	}

	chromo = winner->GetGenotype();
}

/**
 * This method simply takes two chromosomes, mum and dad, and swaps them over
 * at a random point along the length. This, therefore, is the sexual part of
 * the genetic algorithm
 */
template <class EVO, class MUTFUNC>
void GeneticAlgorithm<EVO, MUTFUNC>::CrossoverGenotypes(GENOTYPE& mum, GENOTYPE& dad)
{
	int crossoverPoint = static_cast<int>(randval(1.0f) * chromoLength);
	swap_ranges(mum.begin() + crossoverPoint, mum.end(), dad.begin() + crossoverPoint);
}

/**
 * While crossover simulates the effect of sexual reproduction within a
 * population, mutation artificially reproduces the effects of transcription
 * errors in the replication of DNA.
 */
template <class EVO, class MUTFUNC>
void GeneticAlgorithm<EVO, MUTFUNC>::MutateGenotype(GENOTYPE& chromo)
{
	for (ChromIter i= chromo.begin(); i != chromo.end(); ++i) {
		if (randval(1.0f) < mutation) {
			// If a mutation function has been set, use that, otherwise use a
			// function object.
			*i = (mutFuncPtr == NULL) ? mutFuncObj(*i) : mutFuncPtr(*i);
		}
	}
}

/**
 * Returns a string containing various details about the GA's current state,
 * depending on what has been set with SetPrintStyle. Options include:
 * GA_PARAMETERS: print the current parameters
 * GA_CURRENT: print stats for the current generation
 * GA_GENERATION: output the current generation
 * GA_HISTORY: print the history of average and best fitness
 * \todo store time data
 */
template <class EVO, class MUTFUNC>
std::string GeneticAlgorithm<EVO, MUTFUNC>::ToString()const
{
	using namespace std;

	ostringstream out;

	if ((printStyle & GA_PARAMETERS) != 0) {
		out << "Crossover:       " << setw(6) << crossover << "  "
			<< "Mutation:        " << setw(6) << mutation << endl
			<< "Selection type:  ";
		switch (selection) {
		case GA_ROULETTE:
			out << "roulette wheel selection" << endl;
			break;
		case GA_RANK:
			out << "rank selection" << endl;
			break;
		case GA_TOURNAMENT:
			out << "tournament selection" << endl
				<< "Tournament size: " << setw(6)
				<< intParams.find(GA_TOURNAMENT_SIZE)->second << "  "
				<< "Chance of win:   " << setw(6)
				<< fltParams.find(GA_TOURNAMENT_PARAM)->second << endl;
			break;
		}
		out << "Elitism:         " << setw(6) << elitism << "  "
			<< "Sub-elitism:     " << setw(6) << subelitism << endl
			<< "Output population size: " << outputPopSize << endl << endl;
	}

	if ((printStyle & GA_CURRENT) != 0) {
		out << "Generation: " << setw(6) << generations << "   " << "Average fitness: "
			<< setw(8) << totalFitness / static_cast<float>(inputPopSize) << "   "
			<< "Best fitness: " << setw(8) << bestFitness << endl;
	}

	if ((printStyle & GA_GENERATION) != 0) {
		int i = 1;
		ChromIter ch;
		vector<GENE> chromo;
		for (PopConstIter iter = population.begin(); iter != population.end(); ++iter, ++i) {
			out << setw(3) << i << ": ";
			chromo = (*iter)->GetGenotype();
			for (ch = chromo.begin(); ch != chromo.end(); ++ ch) {
				out << setw(2) << *ch << " ";
			}
			out << " Fitness: " << setw(6) << (*iter)->GetFitness() << endl;
		}
		out << endl;
	}

	if ((printStyle & GA_HISTORY) != 0) {
		out << "  Gen   |   Avg   |  Best" << endl;
		vector<float>::const_iterator avgIter = averageFitnessRecord.begin();
		vector<float>::const_iterator bestIter = bestFitnessRecord.begin();
		for (int i = 0; avgIter != averageFitnessRecord.end();
		++i, ++avgIter, ++bestIter) {
			out << setw(8) << i << "  "
				<< setw(8) << *avgIter << "  "
				<< setw(8) << *bestIter
				<< endl;
		}
	}

	return out.str();
}

/**
 * Returns a string containing a simple CSV table with average and best
 * fitness for every generation so far. The default separator is "," but
 * a different one may be specified.
 */
template <class EVO, class MUTFUNC>
std::string GeneticAlgorithm<EVO, MUTFUNC>::GetCSV(char separator)const
{
	using namespace std;

	ostringstream out;

	vector<float>::const_iterator avgIter = averageFitnessRecord.begin();
	vector<float>::const_iterator bestIter = bestFitnessRecord.begin();

	out << "Average fitness" << separator << "Best fitness" << endl;

	for (; avgIter != averageFitnessRecord.end(); ++avgIter, ++bestIter) {
		out << *avgIter << separator << *bestIter << endl;
	}

	return out.str();
}

/**
 * Deletes the objects comprising the input and output populations. Only
 * called if the GeneticAlgorithm has ownership of its data.
 * \see SetOwnsData
 */
template <class EVO, class MUTFUNC>
void GeneticAlgorithm<EVO, MUTFUNC>::CleanUp()
{
	using namespace std;
	for_each(population.begin(), population.end(), deleter<EVO>());
	for_each(outputPopulation.begin(), outputPopulation.end(), deleter<EVO>());
}


IMPLEMENT_IOSTREAM_CAST(GASelectionType, int)
IMPLEMENT_IOSTREAM_CAST(GAFltParamType, int)
IMPLEMENT_IOSTREAM_CAST(GAIntParamType, int)
IMPLEMENT_IOSTREAM_CAST(GAPrintStyleType, int)
IMPLEMENT_IOSTREAM_CAST(GAFitnessMethodType, int)
IMPLEMENT_IOSTREAM_CAST(GAFitnessFixType, int)

template <class EVO, class MUTFUNC>
void GeneticAlgorithm<EVO, MUTFUNC>::Serialise(std::ostream& out)const
{
	std::string name = add_slashes(std::string("GeneticAlgorithm_")
								   + typeid(EVO).name());

	out << name << "\n"
		<< crossover << "\n"
		<< mutation << "\n"
		<< selection << "\n"
		<< outputPopSize << "\n"
		<< elitism << "\n"
		<< subelitism << "\n"
		<< crossoverPoints << "\n"
		<< fitnessMethod << "\n"
		<< fitnessFix << "\n"
		<< fltParams
		<< intParams
		<< (ownsData ? "ownsdata" : "not-ownsdata") << "\n"
		<< printStyle << "\n"
		<< generations << "\n"
		<< averageFitnessRecord
		<< bestFitnessRecord
		<< bestEverFitness << "\n"
		<< bestEverGenome
		<< bestCurrentGenome;
}


template <class EVO, class MUTFUNC>
void GeneticAlgorithm<EVO, MUTFUNC>::Unserialise(std::istream& in)
{
	std::string name, expName = add_slashes(std::string("GeneticAlgorithm_")
											+ typeid(EVO).name());
	in >> name;
	if (name != expName) {
		throw SerialException(SERIAL_ERROR_WRONG_TYPE, strip_slashes(name),
							  "This object is type " + strip_slashes(expName));
	}

	in >> crossover
	   >> mutation
	   >> selection
	   >> outputPopSize
	   >> elitism
	   >> subelitism
	   >> crossoverPoints
	   >> fitnessMethod
	   >> fitnessFix
	   >> fltParams
	   >> intParams
	   >> switcher("ownsdata", ownsData)
	   >> printStyle
	   >> generations
	   >> averageFitnessRecord
	   >> bestFitnessRecord
	   >> bestEverFitness
	   >> bestEverGenome
	   >> bestCurrentGenome;
}

template <class EVO, class MUTFUNC>
std::ostream& operator<<(std::ostream& out, const GeneticAlgorithm<EVO, MUTFUNC>& ga)
{
	ga.Serialise(out);
	return out;
}

template <class EVO, class MUTFUNC>
std::istream& operator>>(std::istream& in, GeneticAlgorithm<EVO, MUTFUNC>& ga)
{
	ga.Unserialise(in);
	return in;
}


/**
 * Specialised MutationOperator for bool, simply NOT's its input.
 * This is mostly for a demonstration, although bool would be an easy way to
 * implement binary genotypes, you would be better advised to specialise the
 * int type of MutationOperator since the native integer type is faster on
 * most systems.
 */
template <>
struct MutationOperator<bool>
{
	MutationOperator(){}
	MutationOperator(bool, bool){}
	bool operator()(bool b) { return !b; }
};

/**
 * A type flag for the GAVariant data type.
 * \see GAVariant
 */
enum GAVariantType {
	GAV_INT,
	GAV_FLOAT,
	GAV_DOUBLE,
	GAV_CHAR,
	GAV_BOOL
};

/**
 * This is a general purpose data type which takes five basic data types:
 * int, float, double, char and bool. A union is used to ensure that GAVariant
 * never takes up more space than the size of the largest datatype plus the
 * size of one enumeration type.
 */
struct GAVariant
{
	/// Constructs GAVariant from an int, doubles up as default constructor
	GAVariant(int v = 0):	value(v),type(GAV_INT){}
	/// Constructs GAVariant from a float
	GAVariant(float v):	value(v),type(GAV_FLOAT){}
	/// Constructs GAVariant from a double
	GAVariant(double v):value(v),type(GAV_DOUBLE){}
	/// Constructs GAVariant from a char
	GAVariant(char v):	value(v),type(GAV_CHAR){}
	/// Constructs GAVariant from a bool
	GAVariant(bool v):	value(v),type(GAV_BOOL){}

	/// Union of five data types for GAVariant
	union VariantData {
		VariantData(int v):i(v){}
		VariantData(float v):f(v){}
		VariantData(double v):d(v){}
		VariantData(char v):c(v){}
		VariantData(bool v):b(v){}

		int		i;
		float	f;
		double	d;
		char	c;
		bool	b;
	} value;

	/// A flag signifying the type of data currently held.
	GAVariantType type;

	/// int typecast operator
	operator int()const
	{
		switch (type) {
		case GAV_INT: return value.i;
		case GAV_FLOAT: return static_cast<int>(value.f);
		case GAV_DOUBLE: return static_cast<int>(value.d);
		case GAV_CHAR: return static_cast<int>(value.c);
		case GAV_BOOL: return static_cast<int>(value.b);
		}
		return 0;
	}

	/// float typecast operator
	operator float()const
	{
		switch (type) {
		case GAV_INT: return static_cast<float>(value.i);
		case GAV_FLOAT: return value.f;
		case GAV_DOUBLE: return static_cast<float>(value.d);
		case GAV_CHAR: return static_cast<float>(value.c);
		case GAV_BOOL: return static_cast<float>(value.b);
		}
		return 0.0f;
	}

	/// double typecast operator
	operator double()const
	{
		switch (type) {
		case GAV_INT: return static_cast<double>(value.i);
		case GAV_FLOAT: return static_cast<double>(value.f);
		case GAV_DOUBLE: return value.d;
		case GAV_CHAR: return static_cast<double>(value.c);
		case GAV_BOOL: return static_cast<double>(value.b);
		}
		return 0.0;
	}

	/// char typecast operator
	operator char()const
	{
		switch (type) {
		case GAV_INT: return static_cast<char>(value.i);
		case GAV_FLOAT: return static_cast<char>(value.f);
		case GAV_DOUBLE: return static_cast<char>(value.d);
		case GAV_CHAR: return value.c;
		case GAV_BOOL: return static_cast<char>(value.b);
		}
		return '\0';
	}

	/// bool typecast operator
	operator bool()const
	{
		switch (type) {
		case GAV_INT: return value.i != 0;
		case GAV_FLOAT: return value.f != 0.0f;
		case GAV_DOUBLE: return value.d != 0.0;
		case GAV_CHAR: return value.c != 0;
		case GAV_BOOL: return value.b;
		}
		return false;
	}
};

inline std::ostream& operator<<(std::ostream& out, const GAVariant& v)
{
	out << "GAVariant\n";
	switch (v.type) {
	case GAV_INT: return out << "int " << v.value.i << "\n";
	case GAV_FLOAT: return out << "float " << v.value.f << "\n";
	case GAV_DOUBLE: return out << "double " << v.value.d << "\n";
	case GAV_CHAR: return out << "char " << v.value.c << "\n";
	case GAV_BOOL: return out << "bool " << v.value.b << "\n";
	}
	return out;
}

inline std::istream& operator>>(std::istream& in, GAVariant& v)
{
	std::string name;
	in >> name;
	if (name != "GAVariant") {
		throw SerialException(SERIAL_ERROR_DATA_MISMATCH, "",
							  "Expected GAVariant but got " + name);
	}

	std::string type;
	in >> type;
	if (type == "int") { v.type = GAV_INT;  return in >> v.value.i; }
	if (type == "float") { v.type = GAV_FLOAT;  return in >> v.value.f; }
	if (type == "double") { v.type = GAV_DOUBLE;  return in >> v.value.d; }
	if (type == "char") { v.type = GAV_CHAR;  return in >> v.value.c; }
	if (type == "bool") { v.type = GAV_BOOL;  return in >> v.value.b; }

	return in;
}

/**
 * This specialised mutation operator provides the facilities of the basic
 * MutationOperator for GAVariant. Five separate MutationOperator objects are
 * created, one for each data type (int, float, double, char, bool). These
 * mutation operators are initialised with the same min and max mutations but
 * can be accessed individually and reassigned via the public members which
 * store them.
 * \todo serialise/unserialise
 */
template <>
struct MutationOperator<GAVariant>
{
	/// Constructor, casts inputs to different types for sub-operators
	MutationOperator(GAVariant min = -1.0, GAVariant max = 1.0):
	intMutator(static_cast<int>(min), static_cast<int>(max)),
	floatMutator(static_cast<float>(min), static_cast<float>(max)),
	doubleMutator(static_cast<double>(min), static_cast<double>(max)),
	charMutator(static_cast<char>(min), static_cast<char>(max)),
	boolMutator(static_cast<bool>(min), static_cast<bool>(max))
	{}

	/// Calls the mutation operator for the correct type.
	GAVariant operator()(GAVariant in)
	{
		switch (in.type) {
		case GAV_INT: return intMutator(in.value.i);
		case GAV_FLOAT: return floatMutator(in.value.f);
		case GAV_DOUBLE: return doubleMutator(in.value.d);
		case GAV_CHAR: return charMutator(in.value.c);
		case GAV_BOOL: return boolMutator(in.value.b);
		}
		return GAVariant();
	}

	MutationOperator<int>		intMutator;		///< Mutation operator for ints
	MutationOperator<float>		floatMutator;	///< Mutation operator for floats
	MutationOperator<double>	doubleMutator;	///< Mutation operator for doubles
	MutationOperator<char>		charMutator;	///< Mutation operator for chars
	MutationOperator<bool>		boolMutator;	///< Mutation operator for bools
};

/**
 * @}
 */

} // namespace BEAST

#endif
