/**
 * \file animat.h
 * Interface of the Animat class and associated constants.
 * #include this file if you are deriving an Animat with a unique control
 * system (overloaded Control method) which you are writing from scratch. If
 * you are working with neural nets you may find it more useful to start with
 * FFNAnimat and DNNAnimat which come with their own neural nets and automatic
 * configuration methods.
 * \author Tom Carden
 * \author David Gordon
 * \see neuralanimat.h
 * \see FFNAnimat
 * \see DNNAnimat
 */

#ifndef _ANIMAT_H
#define _ANIMAT_H

#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>

#include "worldobject.h"
#include "sensorbase.h"
#include "trail.h"
#include "utilities.h"

namespace BEAST { 

/**
 * \addtogroup framework
 * @{
 */

const double ANIMAT_RADIUS = 5.0;		///< Animat's default radius.
const double ANIMAT_MAX_SPEED = 100.0;	///< Animat's default maximum speed.
const double ANIMAT_MIN_SPEED = -50.0;	///< Animat's default minimum speed.
const double ANIMAT_MAX_ROTATE = TWOPI; ///< The default max rotation/frame.
const double ANIMAT_DRAG = 50.0;		///< An arbitrary friction value.
const double ANIMAT_ACCEL = 5000.0;		///< AN arbitrary acceleration value.
const double ANIMAT_TIMESTEP = 0.05;	///< The default time step.
const int	 ANIMAT_PARTS = 4;			///< The number of different colours.

/// Enumeration type for the different coloured parts of the Animat.
enum AnimatPartType {
	ANIMAT_BODY,
	ANIMAT_CENTRE,
	ANIMAT_ARROW,
	ANIMAT_WHEEL
};

// Forward declarations for friend declarations
class AnimatMonitor;
class SelfSensor;

/**
 * Animats can move around and interact with other objects in the world.
 * Unlike a plain WorldObject, Animats have a Control method which allows
 * them to be customised with new control systems. The Animat class will
 * rarely be of any use on its own, rather it should be used as a base
 * class. Examples of classes derived from Animat include FFNAnimat and
 * DNNAnimat.
 * \see Animat::Control
 * \see WorldObject
 * \see FFNAnimat
 * \see DNNAnimat
 */
class Animat : public WorldObject
{
protected:
	/// A typedef for the Animat's sensor container, sensors.
	typedef std::map<std::string, Sensor*> SensorContainer;
	/// A typedef for the SensorContainer's iterator.
	typedef SensorContainer::iterator SensorIter;
	/// A typedef for the Animat's control container, controls.
	typedef std::map<std::string, float> ControlContainer;
	/// A typedef for the ControlContainer's iterator.
	typedef ControlContainer::iterator ControlIter;

	friend class AnimatMonitor;	///< To give AnimatMonitor access to sensor output.
	friend class SelfSensor;		///< To give SelfSensor quick access to all info.

public:
	// Construction and destruction:
	Animat();
	virtual ~Animat();

	// Lifecycle methods:
	virtual void	Init();
	void			Add(std::string name, Sensor* s);
	/**
	 * Allows an Animat to have access to a sensor without taking
	 * ownership of it.
	 * \param name The name of the sensor (unique to this Animat).
	 * \param s A pointer to the sensor.
	 * \see Animat::Add
	 */
	void			Share(std::string name, Sensor* s) { sensors[name] = s; }

	virtual void	Update();
	/**
	 * Override this method to provide your own control method.
	 */
	virtual void	Control(){}
	
	virtual void	Interact(Animat*);
	virtual void	Interact(WorldObject*);
	virtual bool	IsTouching(const WorldObject*)const;
	virtual void	OnCollision(WorldObject* r){}

	void			SensorInteract(WorldObject* other);

	virtual void	Display();
	virtual void	Draw();

	// Accessors:
	Vector2D		GetVelocity()const			{ return velocity; }
	double			GetMaxSpeed()const			{ return maxSpeed; }
	double			GetMinSpeed()const			{ return minSpeed; }
	double			GetMaxRotateSpeed()const	{ return maxTurn; }
	/// Returns the global Animat count, Animat::numAnimats
	static int		GetNumAnimats()				{ return numAnimats; }
	double			GetTimeStep()const			{ return timeStep; }
	double			GetDistanceTravelled()const	{ return distanceTravelled; }
	double			GetPowerUsed()const			{ return powerUsed; }

	// Mutators:
	/// Sets the Animat's starting location.
	void			SetStartLocation(const Vector2D& l){ startLocation = l; }
	void			SetStartOrientation(double o){ startOrientation = o; }
	static void		SetTimeStep(double t)		{ timeStep = t; }

	virtual void	Serialise(std::ostream&)const;
	virtual void	Unserialise(std::istream&);

	void			SetCollisionPoint(const Vector2D& v)const	{ collisionPoint = v; }
	void			SetCollisionNormal(const Vector2D& v)const	{ collisionNormal = v; }

	auto_property<Vector2D, Vector2D&> Velocity;
	auto_property<double> MinSpeed;
	auto_property<double> MaxSpeed;
	auto_property<double> MaxTurn;
	// TODO: read/write only properties
	auto_property<double, double, double> DistanceTravelled;
	auto_property<double, double, double> PowerUsed;

protected:
	/// Sets the Animat's velocity.
	void			SetVelocity(const Vector2D& pv)	{ velocity = pv; }
	/// Sets the X component of the Animat's velocity.
	void			SetVelocityX(double x)		{ velocity.x = x; }
	/// Sets the Y component of the Animat's velocity.
	void			SetVelocityY(double y)		{ velocity.y = y; }
	/// Adds a value (possibly negative) to the Animat's velocity.
	void			AddVelocity(const Vector2D& v){ velocity = v; }

	/// Sets the Animat's maximum speed.
	void			SetMaxSpeed(double s)		{ maxSpeed = s; }
	/// Sets the Animat's minimum speed.
	void			SetMinSpeed(double s)		{ minSpeed = s; }

	/// Sets the specified part of the Animat to the specified colour.
	inline void		SetColour(AnimatPartType, const float*);
	/// Sets the specified part of the Animat to the specified colour.
	inline void		SetColour(AnimatPartType, float, float, float, float a = 1.0f);

	const SensorContainer& GetSensors()const	{ return sensors; }
	SensorContainer& GetSensors()				{ return sensors; }
	auto_indexed_pointer_property<SensorContainer, const SensorContainer&, void> Sensors;
	const ControlContainer& GetControls()const	{ return controls; }
	ControlContainer& GetControls()				{ return controls; }
	auto_indexed_property<ControlContainer, const ControlContainer&, void> Controls;
	
	GLfloat			colours[4][4];	///< Animat's various body colours

private:
	/**
	 * Stores pointers to the Animat's sensors. Don't add sensors to this
	 * directly, use Add and Share instead.
	 * \see Animat::Add
	 * \see Animat::Share
	 */
	SensorContainer sensors;
	/**
	 * Stores the results of the Control method, e.g. "left" and "right",
	 * which are defined by default, are the speeds of the Animat's two
	 * motors. You may add any entries you wish.
	 */
	ControlContainer controls;

	Vector2D		velocity;		///< The velocity, stored as a vector.
	double			minSpeed,		///< Animat's maximum speed.
					maxSpeed,		///< Animat's maximum speed.
					maxTurn;		///< Animat's maximum rotation per frame.

	//GLfloat			colours[4][4];	///< Animat's various body colours

	Vector2D		startLocation;	///< The initial location, stored for Reset.
	double			startOrientation;///< Initial orientation, stored for Reset.
			
	double 			distanceTravelled,///< Useful in determining fitness scores.
					powerUsed;		///< The total control activation.
	
	Trail			trail;			///< Responsible for drawing the trail.

	mutable Vector2D collisionPoint;///< Point of contact for collision detection
	mutable Vector2D collisionNormal;///< Normal to the side collided with

	static double	timeStep;		///< Granularity of movement.

	static int		numAnimats;		///< They can count themselves
};

/**
 * Sets the colour of the specified
 */
void Animat::SetColour(AnimatPartType part, const float* col)
{
	SetColour(part, col[0], col[1], col[2], col[3]);
}

void Animat::SetColour(AnimatPartType part, float r, float g, float b, float a)
{
	if (part > ANIMAT_PARTS) return;

	colours[part][0] = r;
	colours[part][1] = g;
	colours[part][2] = b;
	colours[part][3] = a;
}

/**
 * @}
 */

}

#endif

