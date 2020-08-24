/**
 * \file worldobject.cc
 * The implementation of WorldObject.
 */

#include "worldobject.h"
#include "world.h"

namespace BEAST {

std::ostream* WorldObject::logStream;	///< Pointer to global output stream.

/**
 * Used to provide default arguments for methods optionally returning a
 * Vector2D. Must not be used to store any data!
 */
Vector2D WorldObject::VECTOR2D_FAKE_REFERENCE = Vector2D();

/**
 * Initialises WorldObject by calculating edges if required, and setting a
 * random location if initRandom is set.
 */
void WorldObject::Init()
{
	if (!IsCircular()) CalcAbsoluteEdges();
	if (initRandom) {
		SetLocation(GetWorld().RandomLocation());
		SetOrientation(randval(TWOPI));
	}

	dead = false;

	Drawable::Init();
}

/**
 * Calls the one-way interact method of this and another WorldObject. See
 * UniInteract for the use of one-way interactions.
 * \param other The WorldObject we're interacting with.
 */
void WorldObject::Interact(WorldObject* other)
{
	UniInteract(other);
	other->UniInteract(this);
}

/**
 * Returns true if the specified point is inside this object.
 * \param vecTest The point being tested.
 * \return True if vecTest is inside this object.
 */
bool WorldObject::IsInside(const Vector2D& vecTest)const
{
	if (IsCircular()) {
		if ((vecTest - GetLocation()).GetLengthSquared() <= GetRadiusSquared()) {
			return true;
		}
		return false;
	}

	Vector2D vecOutside(vecTest.x + GetRadius() + 1.0, vecTest.y);
	bool inside = false;

	for (vector<Vector2D>::const_iterator i(absoluteEdges.begin());
		 i != absoluteEdges.end() - 1; ++i) {
		if (i->y >= vecTest.y && (i+1)->y < vecTest.y
			|| i->y < vecTest.y && (i+1)->y >= vecTest.y) {

			if (CalcIntersect(*i, *(i+1), vecTest, vecOutside)) {
				inside = !inside;
			}
		}
	}

	return inside;
}

/**
 * Returns the nearest point on this object to the argument, and also returns
 * the collision normal by reference.
 * \param vec The point we are comparing.
 * \param collisionNormal The normal between the nearest side to vec and vec.
 * \return The nearest point on this object to vec.
 */
Vector2D WorldObject::GetNearestPoint(const Vector2D& vec, 
									  Vector2D& collisionNormal)const
{
	if (IsCircular()) {
		collisionNormal = (vec - GetLocation()).GetNormalised();
		return GetLocation() + collisionNormal * GetRadius();
	}

	bool sideFound = false;
	Vector2D l1, l2;
	vector<Vector2D>::const_iterator i = absoluteEdges.begin();

	// Find the closest side on the polygon to vec
	for (; i != absoluteEdges.end() - 1; ++i) {
		if (CalcIntersect(*i, *(i+1), vec, GetLocation())) {
			l1 = *i;
			l2 = *(i+1);
			sideFound = true;
			break;
		}
	}

	// If no side was found, the point is inside so just return the same point
	if (!sideFound) {
		collisionNormal = (vec - GetLocation()).GetNormalised();
		return vec;
	}
	
	// Now we find the nearest point on the line l1-l2 to vec
	collisionNormal = (l2 - l1).GetPerpendicular().GetNormalised();

	return GetNearestPointOnLine(vec, l1, l2);
}
	
/**
 * Finds the nearest point on the line l1-l2 to vec and returns that point.
 * \param vec The point we're looking for the nearest point to.
 * \param l1 The first vertex of the line.
 * \param l2 The second vertex of the line.
 * \return The nearest point on the line to vec.
 */
Vector2D WorldObject::GetNearestPointOnLine(const Vector2D& vec, const Vector2D& l1, const Vector2D& l2)
{
	Vector2D A = vec - l1;	// l1 has to be subtracted from everything because
	Vector2D B = l2	 - l1;	// it's acting as our new origin

	double theta = PI/2 - (B.GetAngle() - A.GetAngle());
	double distAlongSide = A.GetLength() * sin(theta);

	if (distAlongSide > B.GetLength()) {
		return l2;
	}
	else if (distAlongSide <= 0.0) {
		return l1;
	}
	else {
		return l1 + B.GetNormalised() * distAlongSide;
	}
}

/**
 * Returns true if the line defined by the two inputs intersects with this
 * object at some point, and can also return the intersection by reference.
 * \param l1 The first point of the line to be tested.
 * \param l2 The second piont of the line to be tested.
 * \param intersection The point of intersection, returned by reference.
 * \return True if the line intersects this object, false if not.
 */
bool WorldObject::Intersects(const Vector2D& l1, const Vector2D& l2, Vector2D& intersection)const
{
	if (IsCircular()) {
		Vector2D nearPoint(GetNearestPointOnLine(GetLocation(), l1, l2));
		double distToLine = (GetLocation() - nearPoint).GetLength();

		if (GetRadiusSquared() >= distToLine * distToLine) {
			if (&intersection != &VECTOR2D_FAKE_REFERENCE) {
				double l = sqrt(GetRadiusSquared() - distToLine*distToLine);
				Vector2D l1_nearPoint = nearPoint - l1;
				l1_nearPoint.SetLength(l1_nearPoint.GetLength() - l);
				intersection = l1 + l1_nearPoint;
			}
			return true;
		}
		return false;
	}
	else {
		Vector2D vec;
		bool found = false;
	
		for (vector<Vector2D>::const_iterator i(absoluteEdges.begin());
			i != absoluteEdges.end() - 1; ++i) {
			if (CalcIntersect(*i, *(i+1), l1, l2, vec)) {
				if (!found) {
					found = true;
					intersection = vec;
				}
				else {
					if ((vec-l1).GetLengthSquared() < (intersection-l1).GetLengthSquared()) {
						intersection = vec;
					}
				}
			};
		}
		return found;
	}
	
	return false;
}

/**
 * Calculates a vector of absolute edge coordinates from the coordinates in
 * the WorldObject's edges vector, which are relative to the object's
 * location.
 */
void WorldObject::CalcAbsoluteEdges()
{
	vector<Vector2D>::const_iterator i = GetEdges().begin();
	vector<Vector2D>::iterator j = absoluteEdges.begin();

	double m1 = cos(GetOrientation());	// Two values used to make up a
	double m2 = sin(GetOrientation());	// 2x2 rotation matrix

	for (; i != GetEdges().end(); ++i, ++j) {
		j->x = GetLocation().x + (m1 * i->x - m2 * i->y); // m2 and m1 stand
		j->y = GetLocation().y + (m1 * i->y + m2 * i->x); // in for m3 and m4
	}
}

/*
 * Takes four Vector2D coordinates describing two lines as input, and returns
 * true if the line of the first two coordinates intersects that of the second
 * two.
 * \param a1 First vertex of first line.
 * \param a2 Second vertex of first line.
 * \param b1 First vertex of second line.
 * \param b2 Second versted of second line.
 * \param r Point of intersection,
 * \return True if the lines intersect, false if not.
 */
bool WorldObject::CalcIntersect(const Vector2D& a1, const Vector2D& a2,
								const Vector2D& b1, const Vector2D& b2,
								Vector2D& r)
{
	// Equation of a line is Y = Grad * X + YInt
	double dAGrad = (a2 - a1).GetGradient();	// Gradients
	double dBGrad = (b2 - b1).GetGradient();
	double dAYInt = a1.y - dAGrad * a1.x;	// Y intersections
	double dBYInt = b1.y - dBGrad * b1.x;

	if (dAGrad == dBGrad) return false;		// If they're parallel give up

	if (dAGrad == DBL_MAX) {	// If a is vertical...
		r = Vector2D(a1.x, dBGrad * a1.x + dBYInt);
		return (((r.y <= a1.y && r.y >= a2.y) || (r.y <= a2.y && r.y >= a1.y))
			 && ((r.y <= b1.y && r.y >= b2.y) || (r.y <= b2.y && r.y >= b1.y))
			 && ((r.x <= b1.x && r.x >= b2.x) || (r.x <= b2.x && r.x >= b1.x)));
		return false;
	}
	if (dBGrad == DBL_MAX) {	// If b is vertical...
		r = Vector2D(b1.x, dAGrad * b1.x + dAYInt);
		return (((r.y <= a1.y && r.y >= a2.y) || (r.y <= a2.y && r.y >= a1.y))
			 && ((r.y <= b1.y && r.y >= b2.y) || (r.y <= b2.y && r.y >= b1.y))
			 && ((r.x <= a1.x && r.x >= a2.x) || (r.x <= a2.x && r.x >= a1.x)));
		return false;
	}

	// Find the intersections
	r = Vector2D(-(dAYInt - dBYInt) / (dAGrad - dBGrad),0);
	if (&r != &VECTOR2D_FAKE_REFERENCE) r.y = dAGrad * r.x + dAYInt;

	// Only need to check one axis for bounding
	if	   (((r.x <= a1.x && r.x >= a2.x) || (r.x <= a2.x && r.x >= a1.x))
		 && ((r.x <= b1.x && r.x >= b2.x) || (r.x <= b2.x && r.x >= b1.x)))
		return true;
	return false;
}

using namespace std;

/**
 * Outputs the object's data to a stream.
 * \see WorldObject::Unserialise
 * \see Drawable::Serialise
 */
void WorldObject::Serialise(ostream& out)const
{
	out << "WorldObject\n";
	Drawable::Serialise(out);
	out	<< (solid ? "solid" : "non-solid") << endl 
		<< (dead ? "dead" : "alive") << endl
		<< (initRandom ? "initRandom" : "initRegular") << endl;
}

/** 
 * Sets up the object from a stream
 * \see WorldObject::Serialise
 * \see Drawable::Unserialise
 */
void WorldObject::Unserialise(istream& in)
{
	string name;
	in >> name;
	if (name != "WorldObject") {
		throw SerialException(SERIAL_ERROR_WRONG_TYPE, name,
							  "This object is type WorldObject");
	}

	Drawable::Unserialise(in);

	in >> switcher("solid", solid) 
	   >> switcher("dead", dead)
	   >> switcher("initRandom", initRandom);

	absoluteEdges = GetEdges();
}

/// Returns a vector of points on a wall using width and height
vector<Vector2D> Wall::GetSides(double w, double h)
{
	std::vector<Vector2D> sides;
	sides.push_back(Vector2D(h/-2.0, w/-2.0));
	sides.push_back(Vector2D(h/-2.0, w/ 2.0));
	sides.push_back(Vector2D(h/ 2.0, w/ 2.0));
	sides.push_back(Vector2D(h/ 2.0, w/-2.0));
	return sides;
}

} // namespace BEAST
