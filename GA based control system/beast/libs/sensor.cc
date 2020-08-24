#include "sensorbase.h"
#include "animat.h"
#include "world.h"

namespace BEAST {

/**
 * Constructor for the basic Sensor class
 */
Sensor::Sensor(Vector2D l, double o):
WorldObject(l, o), myOwner(NULL), relLocation(l), relOrientation(o),
MatchFunc(NULL), EvalFunc(NULL), ScaleFunc(NULL)
{
	if (myOwner != NULL) {
		SetLocation(myOwner->GetLocation() 
				 + relLocation.rotation(myOwner->GetOrientation()));
		SetOrientation(myOwner->GetOrientation() + relOrientation);
	}
}

/**
 * Destructor: deletes the sensor's functors, if present.
 * \todo Shared functors?
 */
Sensor::~Sensor()
{
	if (MatchFunc != NULL) delete MatchFunc;
	if (EvalFunc != NULL)  delete EvalFunc;
	if (ScaleFunc != NULL) delete ScaleFunc;
}

void Sensor::Init()
{
	Update();
	WorldObject::Init();
}

/**
 * Resets the sensor ready for the next round of tests.
 */
void Sensor::Update()
{
	EvalFunc->Reset();
	if (myOwner != NULL) {
		SetLocation(relLocation.rotation(myOwner->GetOrientation()) 
				 + myOwner->GetLocation());
		SetOrientation(relOrientation + myOwner->GetOrientation());
	}
}

/**
 * Calls the sensor's matching function on the WorldObject, then if it's a
 * match, calls the evaluation function.
 */
void Sensor::Interact(WorldObject* other)
{
	if ((*MatchFunc)(other)) (*EvalFunc)(other, other->GetLocation());
}

/**
 * Returns the sensor's output for this round. The scaling function is applied
 * to the output of the evaluation function to get the result, thus the output
 * might be divided by the range to yield a value [0:1], or randomly adjusted
 * to simulate noise.
 */
double Sensor::GetOutput()const
{
	return (*ScaleFunc)(EvalFunc->GetOutput());
}

/**
 * Sets the matching function, deleting the old one if appropriate.
 */ 
void Sensor::SetMatchingFunction(SensorMatchFunction* func)
{
	if (MatchFunc != NULL) delete MatchFunc;
	MatchFunc = func;
}

/**
 * Sets the evaluation function, deleting the old one if appropriate.
 */ 
void Sensor::SetEvaluationFunction(SensorEvalFunction* func)
{
	if (EvalFunc != NULL) delete EvalFunc;
	EvalFunc = func;
}

/**
 * Sets the scaling function, deleting the old one if appropriate.
 */ 
void Sensor::SetScalingFunction(SensorScaleFunction* func)
{
	if (ScaleFunc != NULL) delete ScaleFunc;
	ScaleFunc = func;
}

/**
 * Returns the SelfSensor's output, which comes directly from the
 * sensor's owner. Currently supports sensing of x/y location,
 * angle, or control output.
 * \todo Sensing of fitness, perhaps adjusting SelfSensor into a
 * template which can be made to sense any detail about the owner.
 */
double SelfSensor::GetOutput()const
{
	if (myOwner == NULL) return 0.0;

	switch (myType) {
	case SELF_SENSOR_X:
		return myOwner->GetLocation().x / myOwner->GetWorld().GetWidth();
	case SELF_SENSOR_Y:
		return myOwner->GetLocation().y / myOwner->GetWorld().GetHeight();
	case SELF_SENSOR_ANGLE:
		return myOwner->GetOrientation() / TWOPI;
	case SELF_SENSOR_CONTROL:
		return myOwner->GetControls()[controlName];
	}

	return 0.0;
}


/**
 * Checks if the WorldObject is the correct type using MatchFunc, then checks
 * if the object's centre is inside the AreaSensor and calls the EvalFunc.
 */
void AreaSensor::Interact(WorldObject* other)
{
	Vector2D vec = other->GetNearestPoint(this);
	if ((*MatchFunc)(other) && IsInside(vec)) {
		(*EvalFunc)(other, vec);
	}
}

/**
 * Initialises sensor radius to match that of owner.
 */
void TouchSensor::Init()
{
	if (myOwner != NULL) {
		SetRadius(myOwner->GetRadius());
	}
	Sensor::Init();
}


/**
 * Checks if the WorldObject is the correct type using MatchFunc, then checks
 * if the sensor's owner is touching the WorldObject and calls the EvalFunc.
 */
void TouchSensor::Interact(WorldObject* other)
{
	if ((*MatchFunc)(other) && myOwner->IsTouching(other)) {
		(*EvalFunc)(other, other->GetNearestPoint(this));
	}
}

/**
 * A wrapper for _Display (the real display method) for handling wrapping.
 */
void BeamSensor::Display()
{
	if (!IsVisible()) return;

	_Display();

	if (!wrapping) return;

	static double temp;

	if (wrapLeft) {
		temp = GetLocation().x;
		SetLocationX(temp + myOwner->GetWorld().GetWidth());
		_Display();
		SetLocationX(temp);
	}
	if (wrapBottom) {
		temp = GetLocation().y;
		SetLocationY(temp + myOwner->GetWorld().GetHeight());
		_Display();
		SetLocationY(temp);
	}
	if (wrapRight) {
		temp = GetLocation().x;
		SetLocationX(temp - myOwner->GetWorld().GetWidth());
		_Display();
		SetLocationX(temp);
	}
	if (wrapTop) {
		temp = GetLocation().y;
		SetLocationY(temp - myOwner->GetWorld().GetHeight());
		_Display();
		SetLocationY(temp);
	}
}

/**
 * Positions the matrix according to the location of the ownerAnimat and
 * draws the sensor's display list.
 */
void BeamSensor::_Display()
{
	glPushMatrix();
	glTranslated(GetLocation().x, GetLocation().y, 0);
	glRotated(rad2deg(GetOrientation()), 0.0, 0.0, 1.0);
	glScaled(1.0 - (drawFixed ? 0.0 : GetOutput()),
			 1.0 - (drawFixed ? 0.0 : GetOutput()),
			 1.0);
	glCallList(this->GetDisplayList());
	glPopMatrix();
}

/**
 * Draws an alpha-blended line, segment or circle depending on the scope
 * of the sensor. The number of points in the circle is determined by the
 * scope and range of the sensor.
 */
void BeamSensor::Draw()
{
	glEnable(GL_BLEND);

	if (scope == 0.0) {
		glBegin(GL_LINES);
		{
			glLineWidth(1.0f);
			glColor4f(GetColour(0), GetColour(1), GetColour(2), SENSOR_ALPHA);
			glVertex2d(0.0, 0.0);
			glColor4f(GetColour(0), GetColour(1), GetColour(2), SENSOR_ALPHA * 2.0f);
			glVertex2d(range, 0.0);
		}
		glEnd();
	}
	else {
		int numArcPoints =static_cast<int>(scope*range*BEAM_DRAW_QUALITY);
		double angle = scope / -2.0;

		glBegin(GL_TRIANGLE_FAN);
		{
			glColor4f(GetColour(0), GetColour(1), GetColour(2), 0.0f);
			glVertex2d(0.0, 0.0);
			glColor4f(GetColour(0), GetColour(1), GetColour(2), SENSOR_ALPHA);
			for (int i = 0; i < numArcPoints; ++i) {
				glVertex2d(range * cos(angle), range * sin(angle));
				angle += scope / static_cast<float>(numArcPoints - 1);
			}
			angle = (scope / 2.0);
			glVertex2d(range * cos(angle), (range * sin(angle)));
		}
		glEnd();
	}

	glDisable(GL_BLEND);

	return;
}

/**
 * Checks if the sensor is wrapping, then sets wrap locations accordingly.
 */
void BeamSensor::Update()
{
	if (wrapping) {
		wrapLeft = (GetLocation().x - range < 0);
		wrapBottom = (GetLocation().y - range < 0);
		wrapRight = (GetLocation().x + range > myOwner->GetWorld().GetWidth());
		wrapTop = (GetLocation().y + range > myOwner->GetWorld().GetHeight());
	}

	Sensor::Update();
}

/**
 * A wrapper for _Interact (the real interaction method) for handling wrapping.
 */
void BeamSensor::Interact(WorldObject* other)
{
	// If other doesn't pass MatchingFunctor, or the sensor's range is set to
	// 0, there's nothing to do
	if (!(*MatchFunc)(other) || range == 0.0) return;

	_Interact(other);

	if (!wrapping) return;

	static double temp;

	if (wrapLeft) {
		temp = GetLocation().x;
		SetLocationX(temp + myOwner->GetWorld().GetWidth());
		_Interact(other);
		SetLocationX(temp);
	}
	if (wrapBottom) {
		temp = GetLocation().y;
		SetLocationY(temp + myOwner->GetWorld().GetHeight());
		_Interact(other);
		SetLocationY(temp);
	}
	if (wrapRight) {
		temp = GetLocation().x;
		SetLocationX(temp - myOwner->GetWorld().GetWidth());
		_Interact(other);
		SetLocationX(temp);
	}
	if (wrapTop) {
		temp = GetLocation().y;
		SetLocationY(temp - myOwner->GetWorld().GetHeight());
		_Interact(other);
		SetLocationY(temp);
	}
}

/**
 * Uses a number of collision detection functions to locate the nearest point
 * of the nearest object in scope. Automatic optimisation in the case of 360
 * degree scope.
 */
void BeamSensor::_Interact(WorldObject* other)
{
	// First find the nearest point on other to the sensor's origin
	Vector2D vecTest = other->GetNearestPoint(GetLocation());
	double distanceToCurrent = (GetLocation() - vecTest).GetLength();
	
	// If the nearest point on other is within scope, we have a winner
	// If distanceToCurrent is 0 the sensor is likely inside the object.
	// Without the distanceToCurrent == 0 test, the sensor detects the
	// inside edges.
	if (InScope(vecTest) || distanceToCurrent == 0.0f) {
		(*EvalFunc)(other, vecTest);
	}
	else {
		// Find the two edges of the beam and check intersections with
		// shape. The "scope > 0.0" part ensures we only do one test if
		// the sensor is a laser.
		if (other->Intersects(	GetLocation(),
								Vector2D(GetLocation(),range,GetOrientation()-scope*0.5),
								vecTest))
		{
			(*EvalFunc)(other, vecTest);
		}
		// Even if the first test was successful, we test again in case
		// both edges of the beam are intersecting the shape.
		if (scope > 0.0 &&
			other->Intersects(	GetLocation(),
								Vector2D(GetLocation(),range,GetOrientation()+scope*0.5),
								vecTest))
		{
			(*EvalFunc)(other, vecTest);
		}
	}
}

/**
 * Checks to see if a point is within the current testing angle of the sensor
 */
bool BeamSensor::InScope(const Vector2D& vec)
{
	if (scope >= TWOPI) return true;

	double angleToOther = (vec - GetLocation()).GetAngle() + scope * 0.5;
	double angleDifference = angleToOther - GetOrientation();
	
	while (angleDifference < 0) angleDifference += TWOPI;
	while (angleDifference >= TWOPI) angleDifference -= TWOPI;
	
	return angleDifference <= scope;
}

} // namespace BEAST

