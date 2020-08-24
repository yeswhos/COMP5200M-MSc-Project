/**
 * \file vector3d.h
 * \addtogroup framework
 * author: Joseph Shaw
 *
 * Basically, took Vector2D class into the third dimension :)
 */

#ifndef _VECTOR3D_H_
#define _VECTOR3D_H_

#include <iostream>
#include <cmath>
#include <float.h>

#include "utilities.h"
#include "random.h"
#include "serialfuncs.h"

namespace BEAST {

/**
 * \addtogroup framework
 * @{
 */


/**
 * A class for representing three-dimensional vectors and coordinates. All
 * usual operators are overloaded and other features are provided.
 */
class Vector3D  
{
public:
	double x, y, z;

// Constructors & destructor...

	Vector3D():x(0.0),y(0.0),z(0.0){}
	Vector3D(double X, double Y, double Z):x(X),y(Y),z(Z){}

	// destructor
	~Vector3D() {};

// Overloaded operators...
	inline Vector3D		operator+(const Vector3D&)const;
	inline Vector3D		operator-(const Vector3D&)const;
	inline Vector3D		operator*(double)const;

	inline Vector3D		&operator+=(const Vector3D&); 
	inline Vector3D		&operator-=(const Vector3D&);
	inline Vector3D		&operator*=(double);

	inline Vector3D		operator-()const;

	inline bool			operator==(const Vector3D&)const;  // equality
	inline bool			operator!=(const Vector3D&)const;	

// Mutators...
	inline void			SetX(double X) { x = X; }
	inline void			SetY(double Y) { y = Y; }
	inline void     SetZ(double Z) { z = Z; }

	inline void			SetEuclidean(double,double,double);

	inline void			SetLength(double);
	inline bool			incLength(double);
	inline bool			decLength(double);

	inline void			normalise();		   // convert to a unit vector
	inline void			rotate(double, Vector3D);

	inline Vector3D		rotation(double, Vector3D)const; 

// Accessors...
	double				GetX()const { return x; }
	double				GetY()const { return y; }
	double        GetZ()const { return z; }

	inline double		GetLength()const;
	inline double		GetLengthSquared()const;

	inline Vector3D		GetReciprocal()const; // return reciprocal vector
	inline Vector3D		GetNormalised()const; // return unit vector form

	inline double		dot(const Vector3D&)const;
  inline Vector3D   cross(const Vector3D&)const;

	inline void			Serialise(std::ostream&)const;
	inline void			Unserialise(std::istream&);
};

// non-member helper functions
inline Vector3D V3MultMatrix(Vector3D, double[16]);
inline Vector3D Rotate(Vector3D, double, Vector3D);

Vector3D V3MultMatrix(Vector3D v, double matrix[16])
//
// Performs matrix multiplication on given 4D vector
//
// IN:
//    v   Vector to be mulitplied with matrix
//    matrix    A 4x4 matrix to be multiplied with v
// OUT:
//    v   Matrix multiplication result
// Returns:
//    none
//
{
  double temp[4] = { 0.0, 0.0, 0.0, 1.0 };
  for (int i = 0; i < 4; i++)
  {
    int pos = 4 * i;
    temp[i] = v.x * matrix[pos] + v.y * matrix[++pos] + v.z * matrix[++pos] + 1.0 * matrix[++pos];
  }
  
  return Vector3D(temp[0], temp[1], temp[2]);
}

Vector3D Rotate(Vector3D vec, double a, Vector3D axis)
{
  double c = cos(a);
  double s = sin(a);
  axis.normalise();
  double x = axis.x;
  double y = axis.y;
  double z = axis.z;

  double matrix[16] = { x*2*(1-c)+c, x*y*(1-c)-z*s, x*z*(1-c)+y*s, 0.0,
                        y*x*(1-c)+z*s, y*2*(1-c)+c, y*z*(1-c)-x*s, 0.0,
                        x*z*(1-c)-y*s, y*z*(1-c)+x*s, z*2*(1-c)+c, 0.0,
                        0.0, 0.0, 0.0, 1.0 };

  return V3MultMatrix(vec, matrix);
} 

/// Returns true if this vector is equal to other
bool Vector3D::operator==(const Vector3D& other)const
{
	return (x == other.x && y == other.y && z == other.z);
}

/// Returns true if this vector is not equal to other
bool Vector3D::operator!=(const Vector3D& other)const
{
	return (x != other.x || y != other.y || z != other.z);	
}

/// Returns this vector multiplied by l.
Vector3D Vector3D::operator*(double l)const
{
	return Vector3D(x * l, y * l, z * l);
}

/// Returns a vector multiplied by a double.
inline Vector3D operator*(double l, const Vector3D& v)
{
	return v * l;
}

/// Multiplies this vector's length by the specified value.
Vector3D& Vector3D::operator*=(double l)
{
	x *= l;
	y *= l;
	z *= l;
	return *this;
}

/// Returns this vector plus other.
Vector3D Vector3D::operator+(const Vector3D& other)const
{
	return Vector3D(x + other.x, y + other.y, z + other.z);
}

/// Adds other to this vector.
Vector3D& Vector3D::operator +=(const Vector3D& other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

/// Returns this vector minus other.
Vector3D Vector3D::operator-(const Vector3D& other)const {
	return Vector3D(x - other.x, y - other.y, z - other.z);
}	

/// Subtracts other from this vector.
Vector3D& Vector3D::operator -=(const Vector3D& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

/// Negates both values of the vector.
Vector3D Vector3D::operator-()const
{
	return Vector3D(-x, -y, -z);
}

/// Converts the vector into a unit vector with the same angle.
void Vector3D::normalise()
{
	if (x == 0.0 && y == 0.0 && z == 0.0) {
		x = 1.0;
		return;
	}
	double invLength = 1 / GetLength();
	x *= invLength;
	y *= invLength;
	z *= invLength;
}

/// Rotates the vector by the specified number of radians.
void Vector3D::rotate(double a, Vector3D axis)
{
	Vector3D vec = Rotate(*this, a, axis);

  x = vec.x;
  y = vec.y;
  z = vec.z;
}

/// Returns the vector rotated by the specified number of radians.
Vector3D Vector3D::rotation(double a, Vector3D axis)const
{

	return Rotate(*this, a, axis);
}


/// Sets the length of the vector.
void Vector3D::SetLength(double l)
{
	normalise();
	operator*=(l);
}


/// Quick way of setting X and Y of vector at the same time.
void Vector3D::SetEuclidean(double X, double Y, double Z)
{
	x = X;
	y = Y;
  z = Z;
}

/// Returns the opposite vector.
Vector3D Vector3D::GetReciprocal()const
{
	return Vector3D(-x, -y, -z);
}

/// Returns a unit vector with the same angle as the current vector.
Vector3D Vector3D::GetNormalised()const
{
	if (x == 0.0 && y == 0.0 && z == 0.0) {
		return Vector3D(1.0,0.0,0.0);
	}
	double invLength = 1 / GetLength();
	return Vector3D(invLength * x, invLength * y, invLength * z);
}

/// Returns the dot product of the vector with other
double Vector3D::dot(const Vector3D& other)const
{
	return x * other.x + y * other.y + z * other.z;
}


Vector3D Vector3D::cross(const Vector3D& other)const
{
  return Vector3D(y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x);
}

/// Returns the length of the vector, if possible use GetLengthSquared instead.
double Vector3D::GetLength()const
{
	return sqrt(GetLengthSquared());
}

/// Returns the square of the vector's length, useful for quicker comparisons.
double Vector3D::GetLengthSquared()const
{
	return x * x + y * y + z * z;
}



/// Writes a Vector2D to an output stream.
void Vector3D::Serialise(std::ostream& out)const
{
	out << "Vector3D " << x << " " << y << " " << z << "\n";
}

/// Reads a Vector3D from an input stream.
void Vector3D::Unserialise(std::istream& in)
{
	std::string name;
	in >> name;
	if (name != "Vector3D") {
		throw SerialException(SERIAL_ERROR_DATA_MISMATCH, "", 
							  "Expected Vector3D but got " + name);
	}

	in >> x >> y;
}

/**
 * Output operator for Vector3D
 * \relates Vector3D
 */
inline std::ostream& operator<< (std::ostream& out, const Vector3D& v)
{
	v.Serialise(out);
	return out;
}

/**
 * Input operator for Vector3D
 * \relates Vector3D
 */
inline std::istream& operator>> (std::istream& in, Vector3D& v)
{
	v.Unserialise(in);
	return in;
}

/**
 * @}
 */

} // namespace BEAST

#endif

