/**
 * \file animat.cc
 * Implementation of the Animat class.
 */

#include "animat.h"
#include "world.h"

using namespace std;

namespace BEAST {

int Animat::numAnimats = 0;
double Animat::timeStep = ANIMAT_TIMESTEP;

/**
 * Constructor for Animat, sets most values to 0 or default. Sets up controls
 * for motors, "left" and "right", stores the start location and increments
 * the global Animat counter, Animat::numAnimats. Finally, gives the Animat
 * a random colour.
 */
Animat::Animat():
WorldObject (Vector2D(), 0.0, ANIMAT_RADIUS, true),
Velocity(velocity),
MinSpeed(minSpeed),
MaxSpeed(maxSpeed),
MaxTurn(maxTurn),
DistanceTravelled(distanceTravelled),
PowerUsed(powerUsed),
Sensors(sensors),
Controls(controls),

velocity(0,0), minSpeed(ANIMAT_MIN_SPEED), maxSpeed(ANIMAT_MAX_SPEED),
maxTurn(ANIMAT_MAX_ROTATE), distanceTravelled(0.0), powerUsed(0.0)
{
	controls["left"] = 0.0;
	controls["right"] = 0.0;
	SetStartLocation(GetLocation());
	SetStartOrientation(GetOrientation());

	++numAnimats;
	Drawable::SetColour(random_colour());

	colours[ANIMAT_BODY][0] = GetColour(0);
	colours[ANIMAT_BODY][1] = GetColour(1);
	colours[ANIMAT_BODY][2] = GetColour(2);
	colours[ANIMAT_BODY][3] = GetColour(3);

	colours[ANIMAT_CENTRE][0] = colours[ANIMAT_CENTRE][1] = 
	colours[ANIMAT_CENTRE][2] = colours[ANIMAT_CENTRE][3] = 1.0f;

	colours[ANIMAT_ARROW][0] = colours[ANIMAT_ARROW][1] =
	colours[ANIMAT_ARROW][2] = 0.0f;
	colours[ANIMAT_ARROW][3] = 1.0f;

	colours[ANIMAT_WHEEL][0] = colours[ANIMAT_WHEEL][1] = 
	colours[ANIMAT_WHEEL][2] = 0.1f;
	colours[ANIMAT_WHEEL][3] = 1.0f;
}

/**
 * Destructor: decrements Animat::numAnimats and deletes all the sensors
 * which are owned by this Animat.
 */
Animat::~Animat()
{
	--numAnimats;

	for(SensorIter i = sensors.begin(); i != sensors.end(); ++i) {
		if (i->second->GetOwner() == this) delete i->second;
	}
}

/**
 * Sets the trail colour to match the animat and calls WorldObject::Init()
 */
void Animat::Init()
{
	if (IsInitRandom()) {
		velocity = Vector2D(0.0, 0.0, 1.0, randval(TWOPI));
		SetOrientation(GetVelocity().GetAngle());
	}
	else {
		SetLocation(startLocation);
		SetOrientation(startOrientation);
		SetVelocity(Vector2D(0.0,0.0));
	}

	controls["left"] = controls["right"] = 0.0;
	trail.clear();

	trail.SetColour(GetColour(0), GetColour(1), GetColour(2));
	for (SensorIter j = sensors.begin(); j != sensors.end(); ++j) {
		j->second->Init();
	}

	WorldObject::Init();
}

/**
 * Adds named sensors to the Animat's sensor container, and sets the owner
 * to this Animat.
 * \param name The name of the sensor (unique to animat).
 * \param s A pointer to the sensor.
 */
void Animat::Add(string name, Sensor* s)
{
	if (sensors.find(name) != sensors.end() 
		&& sensors[name]->GetOwner() == this) {
		delete sensors[name];
	}
	sensors[name] = s;
	s->SetOwner(this);
}

/**
 * Updates the animat's position according to velocity and performs updates on
 * members (sensors and trail). Note that the recommended way of influencing
 * the behaviour of Animat subclasses is to overload the Control method of
 * Animat, rather than Update (since Control is called at an opportune moment
 * from within Update).
 */
void Animat::Update()
{
	WorldObject::Update();

	Control();

	OffsetOrientation(maxTurn * (controls["left"] - controls["right"]) 
						   * timeStep);

	velocity += Vector2D(0.0, 0.0, 
						 (maxSpeed - minSpeed)
							 * ((controls["left"] + controls["right"])/2.0) 
							 + minSpeed,
						 GetOrientation());

	if (maxSpeed > 0.0) {
		velocity -= velocity * (1.0 / maxSpeed) * ANIMAT_DRAG; // "drag"
	}
		
	if (velocity.GetLengthSquared() > maxSpeed * maxSpeed) {
		velocity.SetLength(maxSpeed);
	}

	OffsetLocation(velocity * timeStep);

	// Here we handle wrapping and clear the trail to ensure that no lines are
	// drawn across the display as the Animat zaps from side to side.
	while (GetLocation().x < 0) 
		{ SetLocationX(GetLocation().x + GetWorld().GetWidth()); trail.clear(); }
	while (GetLocation().x >= GetWorld().GetWidth())
		{ SetLocationX(GetLocation().x - GetWorld().GetWidth()); trail.clear(); }
	while (GetLocation().y < 0) 
		{ SetLocationY(GetLocation().y + GetWorld().GetHeight()); trail.clear(); }
	while (GetLocation().y >= GetWorld().GetHeight()) 
		{ SetLocationY(GetLocation().y - GetWorld().GetHeight()); trail.clear(); }

	for (SensorIter j = sensors.begin(); j != sensors.end(); ++j) {
		j->second->Update();
	}

	distanceTravelled += velocity.GetLength() * timeStep;

	for (ControlIter i = controls.begin(); i != controls.end(); ++i) {
		powerUsed += ((maxSpeed - minSpeed) * fabs(i->second) + minSpeed) * timeStep;
	}

	trail.push_back(GetLocation());
	trail.Update();
}

/**
 * Processes collisions with other animats, including rudimentary physics
 * (sticky collisions). Also calls onCollide event and sensorInteract on both
 * animats.
 * \param other A pointer to the Animat we're interacting with.
 */
void Animat::Interact(Animat* other)
{
	// Sensors go first because we don't want things bouncing away to not be sensed.
	SensorInteract(other);
	other->SensorInteract(this);
	
	if (IsTouching(other)) {
		if (other->IsSolid()) {
			Vector2D averageVelocity = (velocity + other->GetVelocity()) * 0.5;
			Vector2D vecToOther = other->GetLocation() - GetLocation();
			double minDistance = GetRadius() + other->GetRadius();
			SetVelocity(averageVelocity);
			OffsetLocation(vecToOther.GetReciprocal().GetNormalised()
						   * (minDistance - vecToOther.GetLength()));

			other->SetVelocity(averageVelocity);
			other->OffsetLocation(vecToOther.GetNormalised()
								  * (minDistance - vecToOther.GetLength()));
		}

		GetWorld().AddCollision(collisionPoint);

		OnCollision(other);
		other->OnCollision(this);
	}

	WorldObject::Interact(other);
}

/**
 * Processes collisions with non-animats, including displacement, sensors and
 * collision events
 * \param other A pointer to the WorldObject we're interacting with.
 */
void Animat::Interact(WorldObject* other)
{
	SensorInteract(other);

	if (IsTouching(other)) {
		if (other->IsSolid()) {
			OffsetLocation(collisionNormal * (GetRadius() - (GetLocation() - collisionPoint).GetLength()));
		}
		GetWorld().AddCollision(collisionPoint);

		OnCollision(other);
		other->OnCollision(this);
	}

	WorldObject::Interact(other);
}

/**
 * Passes a WorldObject to all the sensors in turn.
 * \param other A pointer to the WorldObject being sensed.
 */
void Animat::SensorInteract(WorldObject* other)
{
	for (SensorIter j = sensors.begin(); j != sensors.end(); ++j) {
		j->second->Interact(other);
	}
}

/**
 * Returns true when animat is touching other, and Sets the collision point to
 * a point on the animat. Also Sets the colision normal via GetNearestPoint.
 * \param other A pointer to the WorldObject we're checking.
 */
bool Animat::IsTouching(const WorldObject* other)const
{
	Vector2D vecToOther = other->GetLocation() - GetLocation();
	double minDistance = GetRadius() + other->GetRadius();
	
	if (vecToOther.GetLengthSquared() > minDistance * minDistance) return false;
	
	collisionPoint = other->GetNearestPoint(GetLocation(), collisionNormal);
	return (other->IsCircular() || IsInside(collisionPoint));
}

/**
 * Implementing Drawable - draws the Animat in the correct colour.
 */
void Animat::Draw()
{
	float colTemp[4] = { GetColour(0), GetColour(1), GetColour(2), GetColour(3) };

	Drawable::SetColour(colours[ANIMAT_BODY]);
	Drawable::Draw();		// Borrow the nice shaded effect from drawable
	Drawable::SetColour(colTemp);

	glColor4fv(colours[ANIMAT_CENTRE]);
	GLUquadricObj* Disk = gluNewQuadric();
	gluQuadricDrawStyle(Disk, GLU_FILL);
	gluDisk(Disk, 0, (GetRadius() / 0.85) - 4.0, 20, 1); // body
	gluDeleteQuadric(Disk);

	glColor4fv(colours[ANIMAT_ARROW]);
	glLineWidth(1.0);
	glBegin(GL_LINE_STRIP);
	{
		glVertex2d(0.0, GetRadius() / 2.0);
		glVertex2d(GetRadius() / 1.5, 0.0);
		glVertex2d(0.0, GetRadius() / -2.0);
	}
	glEnd(); // direction arrow

	glColor4fv(colours[ANIMAT_WHEEL]);
	glLineWidth(4.0);
	glBegin(GL_LINE_STRIP);
	{
		glVertex2d(GetRadius() / -2.0, 2.0 - GetRadius());
		glVertex2d(GetRadius() /  2.0, 2.0 - GetRadius());
	}
	glEnd(); // right wheel

	glColor4fv(colours[ANIMAT_WHEEL]);
	glLineWidth(4.0);
	glBegin(GL_LINE_STRIP);
	{
		glVertex2d(GetRadius() / -2.0, GetRadius() - 2.0);
		glVertex2d(GetRadius() /  2.0, GetRadius() - 2.0);
	}
	glEnd(); // left wheel
}

/**
 * Displays the Animat along with its sensors and trail, depending on the
 * configuration of myWorld.
 * \see World::SetDispConfig
 * \see World::GetDispConfig
 * \see World::WorldDisplayType
 */
void Animat::Display()
{
	if ((GetWorld().GetDispConfig() & DISPLAY_SENSORS) != 0) {
		for (SensorIter j = sensors.begin(); j != sensors.end(); j++) {
			j->second->Display();
		}
	}

	if ((GetWorld().GetDispConfig() & DISPLAY_TRAILS) != 0) trail.Display();
	WorldObject::Display();
}

/**
 * Outputs the Animat's data to a stream.
 * \param out Reference to an output stream.
 */
void Animat::Serialise(ostream& out)const
{
	out << "Animat\n";
	WorldObject::Serialise(out);
	out << controls
		<< velocity
		<< maxSpeed << "\n"
		<< minSpeed << "\n"
		<< maxTurn << "\n"
		<< startLocation
		<< startOrientation << "\n"
		<< distanceTravelled << "\n"
		<< powerUsed << "\n";
}

/**
 * Inputs the Animat's data from a stream.
 * \param in Reference to an input stream.
 */
void Animat::Unserialise(istream& in)
{
	string name;
	in >> name;
	if (name != "Animat") {
		throw SerialException(SERIAL_ERROR_WRONG_TYPE, name, 
							  "This object is type Animat");
	}

	WorldObject::Unserialise(in);

	in >> controls
	   >> velocity
	   >> maxSpeed
	   >> minSpeed
	   >> maxTurn
	   >> startLocation
	   >> startOrientation
	   >> distanceTravelled
	   >> powerUsed;
}

} // namespace BEAST

