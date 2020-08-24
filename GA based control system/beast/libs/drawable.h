/**
 * \file drawable.h
 * Include this file if you wish to create scenery or other non-interactive
 * objects which appear in the world.
 * Everything that appears in the world is a Drawable, and all WorldObjects
 * and Animats are derived from it. 
 * \author Tom Carden
 * \author David Gordon
 */

#ifndef _DRAWABLE_H_
#define _DRAWABLE_H_

#ifdef _WIN32
	#include <windows.h>
#endif

#include <vector>

#include <GL/gl.h> 
#include <GL/glu.h> 
#include "vector2d.h"
#include "colours.h"
#include "serialfuncs.h"
#include "utilities.h" // For properties

#define This (*this)

/**
 * The namespace for everything in the simulation environment.
 */
namespace BEAST {

/**
 * \addtogroup framework
 * @{
 */

class World;

const double DRAWABLE_RADIUS = 50.0;	///< The default diameter for drawables.

class Drawable
{
public:
	Drawable(Vector2D p = Vector2D(0.0,0.0), double o = 0.0,
			 double r = DRAWABLE_RADIUS):
	  Location(location),
	  Orientation(orientation),
	  Radius(radius),
	  RadiusSquared(radiusSquared),
	  Visible(visible),
	  Circular(circular),

	  location(p),orientation(bound(0.0, TWOPI, o)), radius(r),
	  radiusSquared(r * r), circular(true), displaylist(0), visible(true),
	  myWorld(NULL)

	  {colour[0] = 0.5; colour[1] = 0.5; colour[2] = 0.5;
	   MyWorld.init(this, &Drawable::GetWorld, &Drawable::SetWorld);}

	Drawable(Vector2D p, double o, std::vector<Vector2D> e):
	  Location(location),
	  Orientation(orientation),
	  Radius(radius),
	  RadiusSquared(radiusSquared),
	  Visible(visible),
	  Circular(circular),

	  location(p),orientation(bound(0.0, TWOPI, o)), radius(0.0),
	  radiusSquared(0.0), edges(e), circular(false), displaylist(0),
	  visible(true), myWorld(NULL)

	  {colour[0] = 0.5; colour[1] = 0.5; colour[2] = 0.5;
	   MyWorld.init(this, &Drawable::GetWorld, &Drawable::SetWorld);
	   if (edges.size() > 0) edges.push_back(edges.front()); }

	virtual ~Drawable() { if (displaylist != 0) glDeleteLists(displaylist, 1); }

	virtual void	Init();
	virtual void	Display();
	virtual void	Render();
	virtual void	Draw();

	const float*	GetColour()const			{ return colour; }
	float			GetColour(int i)const		{ return colour[i]; }
	inline void		SetColour(float, float, float, float = 1.0f);
	inline void		SetColour(const float*);

	Vector2D		GetLocation()const	{return location;}
	/// Sets the Drawable's location.
	void			SetLocation(const Vector2D& pv)		{ location = pv; }
	/// Sets the Drawable's location.
	void			SetLocation(double x, double y)		{ location.SetCartesian(x,y); }
	/// Sets the X component of the Drawable's location.
	void			SetLocationX(double x)		{ location.x = x; }
	/// Sets the Y component of the Drawable's location.
	void			SetLocationY(double y)		{ location.y = y; }
	void			OffsetLocation(const Vector2D& pv)	{ location += pv; }
	void			OffsetLocation(double x, double y)
					{location.SetCartesian(location.x + x, location.y + y); }

	double			GetOrientation()const		{ return orientation; }
	/// Sets the Drawable's orientation.
	void			SetOrientation(double o)	{ orientation = o; }
	void			OffsetOrientation(double o)	{ orientation += o; }

	double			GetDiameter()const	{ return radius + radius; }
	double			GetRadius()const	{ return radius; }
	double			GetRadiusSquared()const { return radiusSquared; }
	void			SetRadius(double r) { radius = r; radiusSquared = r * r; }
	bool			IsCircular()const	{ return circular; }
	bool			IsPolygon()const	{ return !circular; }

	bool			IsVisible()const	{ return visible; }
	void			SetVisible(bool v)	{ visible = v; }

	World&			GetWorld()const		{ return *myWorld; }
	void			SetWorld(World* p)	{ myWorld = p; }
	void			SetWorld(World& p)	{ myWorld = &p; }

	virtual void	Serialise(std::ostream&)const;
	virtual void	Unserialise(std::istream&);

	auto_property<Vector2D, const Vector2D&>
							Location;
	auto_property<double>	Orientation;
	auto_property<double>	Radius;
	auto_property<double>	RadiusSquared;
	auto_property<bool>		Visible;
	auto_property<bool>		Circular;
	property<Drawable, World, World&> MyWorld;

protected:
	std::vector<Vector2D>& GetEdges()	{ return edges; }
	void			SetEdges(const std::vector<Vector2D>& e) { edges = e; }

	GLuint			GetDisplayList()const{ return displaylist; }

private:
	Vector2D		location;		///< Relative to 0,0 in myWorld, the bottom left corner.
	double			orientation;  	///< In radians going anti-clockwise, east is zero for Vector2D compatibility.
	double			radius;			///< The object's radius
	double			radiusSquared;	///< The square of the radius, for quicker calculations
	std::vector<Vector2D> edges;	///< Edges are defined as vectors of points relative to this object's location.
	bool			circular;		///< True for circular objects, false for polygons
	float			colour[4];		///< RGBA, although alpha channel is hardly used and defaults to 1.0.

	GLuint			displaylist;	///< This object's GL display list.

	bool			visible;		///< Whether this object should be displayed.

	World*			myWorld;		///< A pointer to a World.
	
};

std::ostream& operator<< (std::ostream& out, const Drawable& ffn);
std::istream& operator>> (std::istream& in, Drawable& ffn);

void Drawable::SetColour(float r, float g, float b, float a) {
	colour[0] = r;	colour[1] = g;
	colour[2] = b;	colour[3] = a;
}

void Drawable::SetColour(const float* col)
{
	colour[0] = col[0];	colour[1] = col[1];
	colour[2] = col[2];	colour[3] = col[3];
}

/**
 * @}
 */

} // namespace BEAST

#endif

