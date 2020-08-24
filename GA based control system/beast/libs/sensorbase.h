/**
 * \file sensorbase.h
 * All basic sensor objects are defined in this file, but no sensor functors,
 * or helper functions appear here. If you want to include sensors in your
 * simulation, include the file sensor.h instead.
 * \author David Gordon
 * \addtogroup sensors
 */

#ifndef _SENSORBASE_H
#define _SENSORBASE_H

#include <cmath>
#include <functional>
#include <string>

#include "worldobject.h"

namespace BEAST {

/**
 * \addtogroup sensors
 * @{
 */
	
/**
 * Abstract base class for matching functors. Classes inherited from
 * SensorMatchFunction must implement operator()(WorldObject*).
 */
struct SensorMatchFunction : public std::unary_function<WorldObject*, bool>
{
	virtual 		~SensorMatchFunction(){}
	virtual void 	Reset(){};
	virtual bool 	operator()(WorldObject*) = 0;
};

/**
 * Abstract base class for evaluation functors. Classes inherited from
 * SensorEvalFunction must implement operator()(WorldObject*, const Vector2d&)
 * and double GetOutput()const.
 */
struct SensorEvalFunction : public std::binary_function<WorldObject*, const Vector2D&, void>
{
	virtual ~SensorEvalFunction(){}
	virtual void 	Reset(){};
	virtual void 	operator()(WorldObject*, const Vector2D& nearestPoint) = 0;
	virtual double 	GetOutput()const = 0;

};

/**
 * Abstract base class for scaling functors. Classes inherited from
 * SensorScaleFunction must implement double operator()(double).
 */
struct SensorScaleFunction : public std::unary_function<double, double>
{
	virtual 		~SensorScaleFunction(){}
	virtual void 	Reset(){}
	virtual double 	operator()(double) = 0;
};


const float SENSOR_ALPHA = 0.1f; ///< Transparency value for Sensors.

// Forward declaration for SetOwner and member data
class Animat;

/**
 * The Sensor class is the base class for all the different types of sensor:
 * TouchSensor, SelfSensor, AreaSensor and BeamSensor. The basic Sensor class
 * imposes no area restriction and so will allow its owner to detect any
 * object in the world.
 */
class Sensor : public WorldObject
{
public:
	Sensor(Vector2D l = Vector2D(0.0, 0.0), double o = 0.0);
	virtual ~Sensor();

	virtual void	Init();
	virtual void 	Update();
	virtual void 	Interact(WorldObject*);
	virtual void	Display(){}
	virtual double 	GetOutput()const;

	void			SetOwner(Animat* owner) { myOwner = owner; }
	Animat*			GetOwner()const { return myOwner; }

	void			SetMatchingFunction(SensorMatchFunction* func);
	void			SetEvaluationFunction(SensorEvalFunction* func);
	void			SetScalingFunction(SensorScaleFunction* func);

protected:
	Animat* 		myOwner;		///< pointer to owner Animat, NULL if none
	Vector2D		relLocation;	///< relative location to Animat
	double			relOrientation;	///< relative orientation to Animat

	SensorMatchFunction*	MatchFunc;
	SensorEvalFunction*		EvalFunc;
	SensorScaleFunction*	ScaleFunc;
};

/**
 * An enumeration type for SelfSensor, used to specify which feature of the
 * sensor's owner is to be returned by GetOutput().
 */
enum SelfSensorType {
	SELF_SENSOR_X,		///< Returns the x coordinate of the owner
	SELF_SENSOR_Y,		///< Returns the y coordinate of the owner
	SELF_SENSOR_ANGLE,	///< Returns the owner's orientation
	SELF_SENSOR_CONTROL	///< Returns a control value, specified in constructor
};

/**
 * The SelfSensor is used to detect information about its owner. An Animat can
 * use a SelfSensor to get information on its location and the state of its
 * controls.
 */
class SelfSensor : public Sensor
{
public:
	/// Constructor, specifying type and optionally a control to watch
	SelfSensor(SelfSensorType t, std::string ctrl = ""):
	  myType(t), controlName(ctrl){}
	virtual ~SelfSensor(){}

	virtual void Update(){}
	virtual void Interact(WorldObject*){}
	virtual double GetOutput()const;

protected:
	SelfSensorType	myType;		///< The type of SelfSensor
	std::string		controlName;///< From Animat controls , e.g. "left"
};

/**
 * Detects objects within an area specified by the size and shape of the
 * AreaSensor. Currenly only detects objects when their location is within
 * the area (i.e. just touching the area won't trip the sensor).
 */
class AreaSensor : public Sensor
{
public:
	AreaSensor(Vector2D l, double o):Sensor(l, o){}
	virtual ~AreaSensor(){}

	virtual void	Interact(WorldObject* other);
};

/**
 * Detects objects which are touching the sensor's owner.
 * \see Sensor
 */
class TouchSensor : public Sensor
{
public:
	TouchSensor():Sensor(Vector2D(0.0,0.0), 0.0){}
	virtual ~TouchSensor(){}

	virtual void	Init();
	virtual void	Interact(WorldObject* other);
};

const double BEAM_SENSOR_SCOPE = PI/4;	///< The default scope for BeamSensor
const double BEAM_SENSOR_RANGE = 250.0; ///< The default range for BeamSensor
const float BEAM_DRAW_QUALITY = 0.1f;	///< The default draw quality for BeamSensor

/**
 * BeamSensors can really be three distinct kinds of sensor:
 * - Lasers, which just detect objects a certain distance away in a straight
 * line from the sensor's origin.
 * - Scoped sensors, which detect objects a within a certain range and a
 * specified angle.
 * - Unidirectional sensors which detect objects a certain distance away at
 * any angle.
 * The three types of sensor are achieved by specifying scopes of 0, [0 < TWOPI]
 * and TWOPI respectively.
 * Note that BeamSensors are the most computationally expensive sensors, so if
 * you can substitute another kind of Sensor, do so.
 * \see Sensor
 */
class BeamSensor : public Sensor
{
public:
	BeamSensor(double s = BEAM_SENSOR_SCOPE, double r = BEAM_SENSOR_RANGE,
			   Vector2D l = Vector2D(0.0, 0.0), double o = 0,
			   Animat* owner = NULL):
	  Sensor(l, o), scope(limit(0.0, TWOPI, s)), range(r), drawScale(1.0f),
	  drawFixed(false), wrapping(true){}
	virtual ~BeamSensor(){}

	virtual void	Update();
	virtual void 	Interact(WorldObject* other);
	void 			_Interact(WorldObject* other);

	virtual void 	Display();
	void 			_Display();
	virtual void 	Draw();

	double			GetScope()const			{ return scope; }
	double			GetRange()const			{ return range; }

	void			SetDrawScale(float d) 	{ drawScale = d; }
	void			SetDrawFixed(bool f) 	{ drawFixed = f; }
	void			SetWrapping(bool w)		{ wrapping = w; }

protected:
	double 	scope;		///< width of the beam in radians
	double 	range;		///< Sets maximum distance

	float 	drawScale;	///< Scaling factor used in Display
	bool	drawFixed;	///< Whether to scale display according to output

	bool 	InScope(const Vector2D& vec);

private:
	bool	wrapping,
			wrapLeft,
			wrapBottom,
			wrapRight,
			wrapTop;
};

/**
 * @}
 */

} // namespace BEAST

#endif

