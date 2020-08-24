/**
 * \file vector2d.h
 * \addtogroup framework
 */

#ifndef _VECTOR2D_H_
#define _VECTOR2D_H_

#include <iostream>
#include <cmath>
#include <float.h>

#include "utilities.h"
#include "random.h"
#include "serialfuncs.h"

/// pi defined to a much greater accuracy than will ever be needed
#define PI 3.14159265358979323846
/// A handy define for 2 x pi
#define TWOPI 6.28318530717958647693

namespace BEAST {

/**
 * \addtogroup framework
 * @{
 */

// non-member helper functions
inline double deg2rad(double);
inline double rad2deg(double);

/**
 * A class for representing two-dimensional vectors and coordinates. All
 * usual operators are overloaded and other features are provided.
 */
class Vector2D  
{
public:
	double x, y;

// Constructors & destructor...

	Vector2D():x(0.0),y(0.0){}
	Vector2D(double X, double Y):x(X),y(Y){}
	Vector2D(double X, double Y, double l, double a):
		x(l * cos(a) + X),   y(l * sin(a) + Y){}
	Vector2D(const Vector2D &v, double l, double a):
		x(l * cos(a) + v.x), y(l * sin(a) + v.y){}

	// destructor
	~Vector2D() {};

// Overloaded operators...
	inline Vector2D		operator+(const Vector2D&)const;
	inline Vector2D		operator-(const Vector2D&)const;
	inline Vector2D		operator*(double)const;

	inline Vector2D		&operator+=(const Vector2D&); 
	inline Vector2D		&operator-=(const Vector2D&);
	inline Vector2D		&operator*=(double);

	inline Vector2D		operator-()const;

	inline bool			operator==(const Vector2D&)const;  // equality
	inline bool			operator!=(const Vector2D&)const;	

// Mutators...
	inline void			SetX(double X) { x = X; }
	inline void			SetY(double Y) { y = Y; }

	inline void			SetPolarCoordinates(double,double);	// length & angle
	inline void			SetCartesian(double,double);	// standard x,y 

	inline void			SetLength(double);
	inline bool			incLength(double);
	inline bool			decLength(double);

	inline void			SetAngle(double);

	inline void			normalise();		   // convert to a unit vector
	inline void			rotate(double);		   // rotate the Vector2D
	inline Vector2D		rotation(double)const; // rotate and return the rotated vector

// Accessors...
	double				GetX()const { return x; }
	double				GetY()const { return y; }

	inline double		GetLength()const;
	inline double		GetLengthSquared()const;
	inline double		GetAngle()const;
	inline double		GetGradient()const;

	inline Vector2D		GetReciprocal()const; // return reciprocal vector
	inline Vector2D		GetNormalised()const; // return unit vector form
	inline Vector2D		GetPerpendicular()const;

	inline double		dot(const Vector2D&)const;

	inline void			Serialise(std::ostream&)const;
	inline void			Unserialise(std::istream&);
};

/// Returns true if this vector is equal to other
bool Vector2D::operator==(const Vector2D& other)const
{
	return (x == other.x && y == other.y);
}

/// Returns true if this vector is not equal to other
bool Vector2D::operator!=(const Vector2D& other)const
{
	return (x != other.x || y != other.y);	
}

/// Returns this vector multiplied by l.
Vector2D Vector2D::operator*(double l)const
{
	return Vector2D(x * l, y * l);
}

/// Returns a vector multiplied by a double.
inline Vector2D operator*(double l, const Vector2D& v)
{
	return v * l;
}

/// Multiplies this vector's length by the specified value.
Vector2D& Vector2D::operator*=(double l)
{
	x *= l;
	y *= l;
	return *this;
}

/// Returns this vector plus other.
Vector2D Vector2D::operator+(const Vector2D& other)const
{
	return Vector2D(x + other.x, y + other.y);
}

/// Adds other to this vector.
Vector2D& Vector2D::operator +=(const Vector2D& other)
{
	x += other.x;
	y += other.y;
	return *this;
}

/// Returns this vector minus other.
Vector2D Vector2D::operator-(const Vector2D& other)const {
	return Vector2D(x - other.x, y - other.y);
}	

/// Subtracts other from this vector.
Vector2D& Vector2D::operator -=(const Vector2D& other)
{
	x -= other.x;
	y -= other.y;
	return *this;
}

/// Negates both values of the vector.
Vector2D Vector2D::operator-()const
{
	return Vector2D(-x, -y);
}

/// Converts the vector into a unit vector with the same angle.
void Vector2D::normalise()
{
	if (x == 0.0 && y == 0.0) {
		y = 1.0;
		return;
	}
	double invLength = 1 / GetLength();
	x *= invLength;
	y *= invLength;
}

/// Rotates the vector by the specified number of radians.
void Vector2D::rotate(double a)
{
	double m1 = cos(a);
	double m2 = sin(a);
	x = m1 * x - m2 * y;
	y = m1 * y + m2 * x; // -m2 and m1 stand in for m3 and m4
}

/// Returns the vector rotated by the specified number of radians.
Vector2D Vector2D::rotation(double a)const
{
	double m1 = cos(a);
	double m2 = sin(a);
	return Vector2D(m1 * x - m2 * y, m1 * y + m2 * x); // -m2 and m1 stand in for m3 and m4
}

/** Sets the vector up using polar coordinates.
 * \param l The vector's new length.
 * \param a The vector's new angle.
 */
void Vector2D::SetPolarCoordinates(double l,double a)
{
	x = l * cos(a);
	y = l * sin(a);
}

/// Sets the length of the vector.
void Vector2D::SetLength(double l)
{
	normalise();
	operator*=(l);
}

/// Sets the angle of the vector.
void Vector2D::SetAngle(double a)
{
	*this = Vector2D(0.0, 0.0, GetLength(), a);
}

/// Quick way of setting X and Y of vector at the same time.
void Vector2D::SetCartesian(double X, double Y)
{
	x = X;
	y = Y;
}

/// Returns the opposite vector.
Vector2D Vector2D::GetReciprocal()const
{
	return Vector2D(-x, -y);
}

/// Returns a unit vector with the same angle as the current vector.
Vector2D Vector2D::GetNormalised()const
{
	if (x == 0.0 && y == 0.0) {
		return Vector2D(0.0,1.0);
	}
	double invLength = 1 / GetLength();
	return Vector2D(invLength * x, invLength * y);
}

/// Returns the dot product of the vector with other
double Vector2D::dot(const Vector2D& other)const
{
	return x * other.x + y * other.y;
}

/// Returns the length of the vector, if possible use GetLengthSquared instead.
double Vector2D::GetLength()const
{
	return sqrt(GetLengthSquared());
}

/// Returns the square of the vector's length, useful for quicker comparisons.
double Vector2D::GetLengthSquared()const
{
	return x * x + y * y;
}

/// Returns the angle of the vector in radians.
double Vector2D::GetAngle()const
{
	if		(x==0)	return y>=0 ? PI/2 : -PI/2;
	else if (x > 0) return atan(y/x);
	else			return atan(y/x) + PI;
}

/// Returns the gradient of the vector.
double Vector2D::GetGradient()const
{
	return x == 0 ? DBL_MAX : y / x;

}

/// Returns the perpendicular to the vector/
Vector2D Vector2D::GetPerpendicular()const
{
	return Vector2D(-y,x);
}

/// Writes a Vector2D to an output stream.
void Vector2D::Serialise(std::ostream& out)const
{
	out << "Vector2D " << x << " " << y << "\n";
}

/// Reads a Vector2D from an input stream.
void Vector2D::Unserialise(std::istream& in)
{
	std::string name;
	in >> name;
	if (name != "Vector2D") {
		throw SerialException(SERIAL_ERROR_DATA_MISMATCH, "", 
							  "Expected Vector2D but got " + name);
	}

	in >> x >> y;
}

// Non-member functions...

/// Converts degrees to radians.
double deg2rad(double angle) {
	return angle/360.0 * TWOPI;
}

/// Converts radians to degrees.
double rad2deg(double angle) {
	double result = (angle/TWOPI) * 360.0;
	return result;
}

/**
 * Creates a Vector2D object using old PolarVector syntax.
 * \param l The length of the vector
 * \param a The angle of the vector anticlockwise from due east.
 * \deprecated Probably of no use to anyone unless they have a strange
 * fascination with polar coordinates.
 */
inline Vector2D PolarVector(double l, double a)
{
	return Vector2D(0.0, 0.0, l, a);
}

/**
 * Output operator for Vector2D
 * \relates Vector2D
 */
inline std::ostream& operator<< (std::ostream& out, const Vector2D& v)
{
	v.Serialise(out);
	return out;
}

/**
 * Input operator for Vector2D
 * \relates Vector2D
 */
inline std::istream& operator>> (std::istream& in, Vector2D& v)
{
	v.Unserialise(in);
	return in;
}

/**
 * @}
 */

} // namespace BEAST

#endif

