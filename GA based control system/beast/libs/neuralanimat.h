/**
 * \file neuralanimat.h
 * The basic Animat comes with no control system, so in the course of deriving
 * a new type of Animat, a control system must be added. Two useful control
 * systems are the FeedForwardNet and DynamicalNet classes. FFNAnimat and
 * DNNAnimat provide Animats with these control systems built-in and
 * automatically configured from the Animat's sensors and controls. Two other
 * classes, EvoFFNAnimat and EvoDNNAnimat are provided as evolvable versions
 * in case the only data contained in the genotype is the ANN configuration.
 * \addtogroup biosystems
 */

#ifndef _NEURALANIMAT_H_
#define _NEURALANIMAT_H_

#include "animat.h"
#include "geneticalgorithm.h"
#include "feedforwardnet.h"
#include "dynamicalnet.h"
#include "serialfuncs.h"

namespace BEAST { 

/**
 * \addtogroup biosystems 
 * 
 */

/**
 * An Animat with a built-in feed-forward network which is automatically
 * configured depending on the Animat's sensor and control configuration.
 * \see EvoFFNAnimat for an evolvable version.
 */
class FFNAnimat : public Animat
{
public:
	FFNAnimat(): myBrain(NULL), ownBrain(true)
	{ Brain.init(this, &FFNAnimat::GetBrain, &FFNAnimat::SetBrain); }	///< Constructor
	virtual ~FFNAnimat();			///< Destructor

	void				InitFFN(int hidden = -1, int inputs = -1, int outputs = -1);
	virtual void		Control();

	virtual void		Serialise(std::ostream&)const;
	virtual void		Unserialise(std::istream&);

	void				SetBrain(FeedForwardNet&);
	const FeedForwardNet& GetBrain()const		{ return *myBrain; }
	bool				IsOwnBrain()const		{ return ownBrain; }

	property<FFNAnimat, FeedForwardNet, FeedForwardNet&, const FeedForwardNet&>
						Brain;

protected:
	FeedForwardNet&		GetBrain()				{ return *myBrain; }

private:
	FeedForwardNet*		myBrain;	///< Pointer to FFNAnimat's neural net.
	bool				ownBrain;
};

/**
 * An evolvable version of FFNAnimat with GetGenotype/SetGenotype methods
 * already set up.
 * \see FFNAnimat
 */
class EvoFFNAnimat : public FFNAnimat, public Evolver<float>
{
public:
	virtual ~EvoFFNAnimat(){}
	virtual void		SetGenotype(const std::vector<float>& g) { GetBrain().SetConfiguration(g); }
	virtual genotype_type GetGenotype()const { return GetBrain().GetConfiguration(); }
};

/**
 * An Animat with a built-in dynamical network which is automatically
 * configured depending on the Animat's sensor and control configuration.
 * \see EvoDNNAnimat for an evolvable version.
 * \todo Review brain ownership/destructor
 */
class DNNAnimat : public Animat
{
public:
	DNNAnimat(): myBrain(NULL), ownBrain(true)
	{ Brain.init(this, &DNNAnimat::GetBrain, &DNNAnimat::SetBrain); }
	virtual ~DNNAnimat();
	void				InitDNN(int hidden = -1, int inputs = -1, int outputs = -1,
								bool multiInputs = true, bool multiOutputs = false);
	virtual void		Control();

	virtual void		Serialise(std::ostream&)const;
	virtual void		Unserialise(std::istream&);

	void				SetBrain(DynamicalNet& brain);

	const DynamicalNet&	GetBrain()const				{ return *myBrain; }
	bool				IsOwnBrain()const			{ return ownBrain; }	

	property<DNNAnimat, DynamicalNet, DynamicalNet&, const DynamicalNet&>
						Brain;
						

protected:
	DynamicalNet&		GetBrain()				{ return *myBrain; }

private:
	DynamicalNet*		myBrain;
	bool				ownBrain;
};

/**
 * An evolvable version of DNNAnimat with GetGenotype/SetGenotype methods
 * already set up.
 * \see DNNAnimat
 */
class EvoDNNAnimat : public DNNAnimat, public Evolver<float>
{
public:
	virtual ~EvoDNNAnimat(){}
	virtual void		SetGenotype(const std::vector<float>& g) { GetBrain().SetConfiguration(g); }
	virtual genotype_type GetGenotype()const { return GetBrain().GetConfiguration(); }
};

}

#endif
