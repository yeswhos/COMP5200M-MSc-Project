/**
 * \file psoalgorithm.h
 * \author David Gordon
 * \addtogroup biosystems
 */

#ifndef _PSO_ALGORITHM_H_
#define _PSO_ALGORITHM_H_

#include "geneticalgorithm.h"

namespace BEAST { 

/**
 * \addtogroup biosystems
 * @{
 */

template <class EVO>
class PSOAlgorithm : public GeneticAlgorithm<EVO, MutationOperator<typename EVO::gene_type> >
{
public:
	PSOAlgorithm(){}
	virtual ~PSOAlgorithm(){}

	virtual void	Generate();
	EVO*			Fly(EVO*);
};

template <class EVO>
void PSOAlgorithm<EVO>::Generate()
{
	CalcStats();
	Setup();

	// Ought to be an STL transform
	for (PopIter i = population.begin(); i != population.end(); ++i) {
		outputPopulation.push_back(Fly(*i));
	}

}

template <class EVO>
EVO* PSOAlgorithm<EVO>::Fly(EVO* e)
{
	if (e->PSOBestSolution.empty() 
		|| e->GAFixedFitness > e->PSOBestFitness) {
		e->PSOBestSolution = e->GetGenotype();
		e->PSOBestFitness = e->GAFixedFitness;
	}

	GENOTYPE	newSolution,
				currSolution = e->GetGenotype();

	ChromIter	pCurr = currSolution.begin(),
				pBest = e->PSOBestSolution.begin(),
				gBest = bestCurrentGenome.begin();

	for (; pCurr != currSolution.end(); ++pCurr, ++pBest, ++gBest) {
		newSolution.push_back(*pCurr 
		  + randval(static_cast<GENE>(2)) * (*pBest - *pCurr)
		  + randval(static_cast<GENE>(2)) * (*gBest - *pCurr));
	}

	EVO* evo = new EVO;
	evo->SetGenotype(newSolution);
	evo->PSOBestSolution = e->PSOBestSolution;
	evo->PSOBestFitness = e->PSOBestFitness;

	return evo;
}

/**
 * @}
 */

} // namespace BEAST

#endif
