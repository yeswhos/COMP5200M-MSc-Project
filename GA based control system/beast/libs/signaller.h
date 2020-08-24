/**
 * \file signaller.h
 * This file defines a class which can be multiply inherited with Animat to
 * create a signalling Animat, and some associated sensor functors which
 * \author David Gordon
 * \addtogroup biosystems
 */

#ifndef _SIGNALLER_H
#define _SIGNALLER_H

#include <map>

#include "sensorbase.h"
#include "sensorfunctors.h"

namespace BEAST {

/**
 * \addtogroup biosystems
 * @{
 */ 

/**
 * A general-purpose class for modelling signallers with discrete signal and
 * state types. Signallers can be made to keep track of signalling costs and
 * are compatible with their own signal sensor functions (if multiply
 * inherited with WorldObject).
 * Each Signaller template maintains its own static costs map, so costs are
 * shared between signallers with the same templated types.
 */
template <typename _State, typename _Signal, typename _Cost = float>
class Signaller
{
public:
	typedef _State state_type;	 ///< A typedef for use in creating sensors
	typedef _Signal signal_type; ///< A typedef for use in creating sensors
	typedef _Cost cost_type;	 ///< A typedef for use in creating sensors

	Signaller(){}
	~Signaller(){}

	/// Resets the Signaller by putting the total cost back to 0.
	void			Reset() { totalCost = static_cast<_Cost>(0); }
	inline void		Randomise(int numStates, int numSignals);

	/// Adds the current signalling cost to the total signalling cost so far.
	void			PushCost()			{ totalCost += GetCost(); }

	/// Returns the internal state of the signaller.
	_State			GetState()const		{ return state; }
	/// Returns the current signal.
	_Signal			GetSignal()const	{ return signals.find(state)->second; }
	/// Returns the signal for the specified state.
	_Signal			GetSignal(_State s)const { return signals.find(s)->second; }
	/// Returns the current signalling cost.
	_Cost			GetCost()const		{ return costs.find(state)->second.find(GetSignal())->second; }
	/// Returns the signalling cost for the specified state/signal
	static _Cost	GetCost(_State st, _Signal si) { return costs[st][si]; }
	/// Returns the total signalling cost so far.
	_Cost			GetTotalCost()const	{ return totalCost; }

	/// Sets the internal state of the signaller.
	void			SetState(_State s)	{ state = s; }
	/// Sets up the signals for each state.
	void			SetSignal(_State st, _Signal si) { signals[st] = si; }
	/// Sets up costs associated with signalling.
	static void		SetCost(_State st, _Signal si, _Cost co) { costs[st][si] = co; }

private:
	_Cost			totalCost;			///< The total signalling cost so far.

	_State state;						///< Internal state of the signaller.
	std::map<_State, _Signal> signals;	///< Maps internal states to signals.

	/// Costs of each state/signal.
	static typename std::map<_State, std::map<_Signal, _Cost> > costs;
};

template <typename _State, typename _Signal, typename _Cost>
typename std::map<_State, std::map<_Signal, _Cost> > 
Signaller<_State, _Signal, _Cost>::costs = std::map<_State, std::map<_Signal, _Cost> >();

/**
 * Randomises the signaller so that each possible internal state has a random
 * signal associated with it. Also sets the signaller to a random state value.
 */
template <typename _State, typename _Signal, typename _Cost>
void Signaller<_State, _Signal, _Cost>::Randomise(int numStates, int numSignals)
{
	using namespace std;

	signals.clear();

	for (int i=0; i<numStates; ++i) {
		signals[static_cast<_State>(i)] = static_cast<_Signal>(irand(numSignals));
	}

	state = static_cast<_State>(irand(numStates));
}


/**
 * Sensor evaluation functor: returns the signal of the nearest individual.
 * \ingroup sensors
 */
template <typename _State, typename _Signal, typename _Cost = float>
class EvalNearestSignal : public EvalNearest
{
public:
	EvalNearestSignal(WorldObject* o, double range):EvalNearest(o, range){}
	virtual ~EvalNearestSignal(){}

	/**
	 * Returns the signal number (as a double) of the nearest signaller, or
	 * 0.0 if no signaller was found (or the signaller is signalling 0)
	 */
	virtual double GetOutput()const
	{
		Signaller<_State, _Signal, _Cost>* s;
		if (IsKindOf(bestCandidate, s)) {
			return static_cast<double>(s->GetSignal());
		}
		return 0.0;
	}
};

/**
 * Constructs and returns a pointer to a sensor which will return the signal
 * of the nearest Signaller of the specified type.
 * \param T The type of object to detect (must be derived from Signaller).
 * \param highestSignal The highest value a signal might take, for scaling.
 * \ingroup sensors
 */
template <class T>
Sensor* NearestSignalSensor(int highestSignal)
{
	Sensor* s = new Sensor(Vector2D(0.0, 0.0), 0.0);
	s->SetMatchingFunction(new MatchKindOf<T>);
	s->SetEvaluationFunction(new EvalNearestSignal<typename T::state_type, 
												   typename T::signal_type,
												   typename T::cost_type>(s, 1000.0));
	s->SetScalingFunction(new ScaleLinear(0.0, static_cast<double>(highestSignal), -1.0, 1.0));
	
	return s;
}

/**
 * @}
 */ 

} // namespace BEAST

#endif

