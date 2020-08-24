#ifndef _SENSORFUNCTIONS_H
#define _SENSORFUNCTIONS_H

#include <typeinfo>
#include "random.h"

namespace BEAST {

/**
 * \addtogroup sensors
 * @{
 */

/**
 * Identifies objects belonging to hierarchies, so if defined with Cheese,
 * will return true for objects of type Cheese, or derived classes such as
 * Cheddar and Gruyère.
 */
template <class _ObjectType>
struct MatchKindOf : public SensorMatchFunction
{
	virtual bool operator()(WorldObject* obj)
	{
		return dynamic_cast<_ObjectType*>(obj) != NULL;
	}
};

/**
 * Identifies exact object types, so if defined with Cheese, will return true
 * only for Cheese, and false for Cheddar and Gruyère.
 */
template <class _ObjectType>
struct MatchExact : public SensorMatchFunction
{
	virtual bool operator()(WorldObject* obj)
	{
		return typeid(*obj) == typeid(_ObjectType);
	}
};

/**
 * Identifies one particular object and returns true only for that object.
 */
struct MatchSpecific : public SensorMatchFunction
{
	MatchSpecific(WorldObject* obj):target(obj){}

	virtual bool operator()(WorldObject* obj)
	{
		return obj == target;
	}

	WorldObject* target;
};

/**
 * Chains any number of matching functions together such that should any of
 * them be true for the object being matched, MatchComposeOr will return true.
 * The first two functors may be added in the constructor. If more are needed,
 * they may be added using push_back. Because MatchComposeOr is derived from
 * std::vector, its contents may be manipulated just like any other vector.
 */
struct MatchComposeOr : public SensorMatchFunction, public std::vector<SensorMatchFunction*>
{
	MatchComposeOr(){}
	MatchComposeOr(SensorMatchFunction* first, SensorMatchFunction* second)
	{
		push_back(first);
		push_back(second);
	}

	virtual ~MatchComposeOr()
	{
		for (iterator i = begin(); i != end(); ++i) {
			delete *i;
		}
	}

	virtual bool operator()(WorldObject* obj)
	{
		for (iterator i = begin(); i != end(); ++i) {
			if ((**i)(obj)) return true;
		}
		return false;
	}
};

/**
 * Chains any number of matching functions together such that only if all of
 * them are true for the object being matched, MatchComposeAnd will return
 * true.
 * The first two functors may be added in the constructor. If more are needed,
 * they may be added using push_back. Because MatchComposeAnd is derived from
 * std::vector, its contents may be manipulated just like any other vector.
 */
struct MatchComposeAnd : public SensorMatchFunction, public std::vector<SensorMatchFunction*>
{
	MatchComposeAnd(){}
	MatchComposeAnd(SensorMatchFunction* first, SensorMatchFunction* second)
	{
		push_back(first);
		push_back(second);
	}

	virtual ~MatchComposeAnd()
	{
		for (iterator i = begin(); i != end(); ++i) {
			delete *i;
		}
	}

	virtual bool operator()(WorldObject* obj)
	{
		for (iterator i = begin(); i != end(); ++i) {
			if (!(**i)(obj)) return false;
		}
		return true;
	}
};

/**
 * Allows any unary predicate to be adapted for use as a matching function. 
 * This includes all unary functors described by the STL and any binary
 * functors after adaptation with bind1st or bind2nd. The ptr_fun adapter
 * may be used to turn any function into a scaling functor.
 */
template <class _Functor>
struct MatchAdapter : SensorMatchFunction
{
	MatchAdapter(_Functor f):functor(f){}

	virtual bool operator()(WorldObject* input)
	{
		return functor(input);
	}

	_Functor functor;
};

/**
 * A helper function for creating MatchAdapter functors.
 */
template <class _Functor>
MatchAdapter<_Functor>* MatchAdapt(_Functor f)
{
	return new MatchAdapter<_Functor>(f);
}

/**
 * Keeps a tally of the nearest point passed in and returns it with GetOutput.
 * Also keeps a pointer to the nearest candidate and a copy of the nearest
 * point on that candidate, this data can be accessed by an adapter such as
 * EvalXDist and EvalAngle.
 */
class EvalNearest : public SensorEvalFunction
{
public:
	EvalNearest(WorldObject* o, double range):owner(o), range(range),
		nearestSoFar(range), bestCandidate(NULL){}
	virtual ~EvalNearest(){}

	virtual void Reset()
	{
		bestCandidate = NULL;
		nearestSoFar = range;
	}
		
	virtual void operator()(WorldObject* obj, const Vector2D& loc)
	{	
		distance = (owner->GetLocation() - loc).GetLength();
		if (distance < nearestSoFar) {
			nearestSoFar = distance;
			bestCandidate = obj;
			bestCandidateVec = loc;
		}
	}
	
	virtual double GetOutput()const
	{	
		return nearestSoFar;
	}

	WorldObject* owner;

	double range;
	double nearestSoFar;
	WorldObject* bestCandidate;
	Vector2D bestCandidateVec;

private:
	double distance;
};

/**
 * Returns the vertical distance to the nearest target. Most effective when
 * coupled with EvalNearestYDist.
 */
class EvalNearestXDist : public EvalNearest
{
public:
	EvalNearestXDist(WorldObject* o, double range):EvalNearest(o, range){}
	virtual ~EvalNearestXDist(){}

	virtual double GetOutput()const
	{
		return bestCandidateVec.x - owner->GetLocation().x;
	}
};

/**
 * Returns the horizontal distance to the nearest target. Most effective when
 * coupled with EvalNearestXDist.
 */
class EvalNearestYDist : public EvalNearest
{
public:
	EvalNearestYDist(WorldObject* o, double range):EvalNearest(o, range){}
	virtual ~EvalNearestYDist(){}

	virtual double GetOutput()const
	{
		return bestCandidateVec.y - owner->GetLocation().y;
	}
};

/**
 * Returns the absolute x position of the nearest target. Most effective when
 * coupled with EvalNearestAbsY.
 */
class EvalNearestAbsX : public EvalNearest
{
public:
	EvalNearestAbsX(WorldObject* o, double range):EvalNearest(o, range){}
	virtual ~EvalNearestAbsX(){}

	virtual double GetOutput()const
	{
		return bestCandidateVec.x;
	}
};

/**
 * Returns the absolute y position of the nearest target. Most effective when
 * coupled with EvalNearestAbsX.
 */
class EvalNearestAbsY : public EvalNearest
{
public:
	EvalNearestAbsY(WorldObject* o, double range):EvalNearest(o, range){}
	virtual ~EvalNearestAbsY(){}

	virtual double GetOutput()const
	{
		return bestCandidateVec.y;
	}
};

/**
 * Returns the normalised angle to the nearest target
 */
class EvalNearestAngle : public EvalNearest
{
public:
	EvalNearestAngle(WorldObject* o, double range):EvalNearest(o, range){}
	virtual ~EvalNearestAngle(){}

	virtual double GetOutput()const
	{
		return bound(-PI, PI, (bestCandidateVec - owner->GetLocation()).GetAngle() - owner->GetOrientation());
	}
};

/*
 * Keeps a total of every time it's called per round. Starting count may
 * be defined but defaults to 0.
 */
class EvalCount : public SensorEvalFunction
{
public:
	EvalCount(int start = 0):startingCount(start), numberSoFar(0){}
	virtual ~EvalCount(){}

	virtual void Reset()
	{	numberSoFar = 0;
	}

	virtual void operator()(WorldObject*, const Vector2D& loc)
	{	++numberSoFar;
	}

	virtual double GetOutput()const
	{	return static_cast<double>(numberSoFar + startingCount); }

private:		
	const int startingCount;
	int numberSoFar;
};

/**
 * ScaleCompose allows the chaining of two scaling functions together, such
 * the output of a ScaleCompose functor is the result of second(first(input)),
 * where first and second are the arguments in ScaleCompose's constructor.
 * For example, to create a function which scales the input from [0:50] to [0:1]
 * and then adds random noise between -0.5 and +0.5:
 * s->SetScalingFunction(new ScaleCompose(new ScaleLinear(50.0), 
 * new ScaleNoise(-0.5, 0.5)));
 * To compose more complex functions, instances of ScaleCompose may be nested.
 * ScaleCompose is responsible for deleting its child functions.
 */
struct ScaleCompose : public SensorScaleFunction
{
	ScaleCompose(SensorScaleFunction* f, SensorScaleFunction* s):first(f), second(s){}

	virtual ~ScaleCompose()
	{
		delete first;
		delete second;
	}

	virtual double operator()(double input)
	{
		return (*second)((*first)(input));
	}

private:
	SensorScaleFunction *first, *second;
};

/**
 * A simple linear scaling function which defaults to an input scale between
 * 0 and a defined maximum, scaling to an output range between 0 and 1. Any
 * input and output range can be defined, including inverted ranges with
 * min > max, which can invert the output.
 */
struct ScaleLinear : public SensorScaleFunction
{
	ScaleLinear(double range):inMin(0.0), inMax(range), outMin(0.0), outMax(1.0){}
	ScaleLinear(double inputMinimum, double inputMaximum, 
				 double outputMinimum, double outputMaximum):
	  inMin(inputMinimum), inMax(inputMaximum), 
	  outMin(outputMinimum), outMax(outputMaximum){}

	virtual double operator()(double input)
	{	
		return (input - inMin) / (inMax - inMin) * (outMax - outMin) + outMin;
	}

	double inMin, inMax, outMin, outMax;
};

/**
 * Returns the absolute value of the input, as for the std::abs function.
 */
struct ScaleAbs : public SensorScaleFunction
{
	virtual double operator()(double input)
	{
		return input >= 0.0 ? input : -input;
	}
};

/**
 * ScaleThreshold takes values: threshold, min and max and returns min if
 * input < threshold, or max if input >= threshold. Min and max default to
 * 0 and 1.
 */
struct ScaleThreshold : SensorScaleFunction
{
	ScaleThreshold(double t, double a = 0.0, double b = 1.0):threshold(t), minimum(a), maximum(b){}

	virtual double operator()(double input)
	{	
		return input < threshold ? minimum : maximum;
	}

	double threshold, minimum, maximum;
};

/**
 * ScaleNoise adds uniform random noise to its input. Minimum and maximum
 * values may be specified, but default to [-0.1:0.1]
 */
struct ScaleNoise : SensorScaleFunction
{
	ScaleNoise(double min = -0.1, double max = 0.1):minimum(min), maximum(max){}

	virtual double operator()(double input)
	{
		return input + randval(maximum - minimum) + minimum;
	}

	double minimum, maximum;
};

/**
 * Allows any unary functor to be adapted for use as a scaling function. This
 * includes all unary functors described by the STL and any binary functors
 * after adaptation with bind1st or bind2nd. The ptr_fun adapter may be used
 * to turn any function into a scaling functor.
 */
template <class _Functor>
struct ScaleAdapter : SensorScaleFunction
{
	explicit ScaleAdapter(_Functor f):functor(f){}

	virtual double operator()(double input)
	{
		return functor(input);
	}

	_Functor functor;
};

/**
 * A helper function for creating ScaleAdapter functors.
 * \relates ScaleAdapter
 */
template <class _Functor>
ScaleAdapter<_Functor>* ScaleAdapt(_Functor f)
{
	return new ScaleAdapter<_Functor>(f);
}

/**
 * @}
 */

} // namespace BEAST

#endif
