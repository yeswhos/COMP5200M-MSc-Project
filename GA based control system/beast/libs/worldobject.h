/*
* \file worldobject.h
* \author Tom Carden
* \author David Gordon
* \addtogroup framework
* Everything interactive in the simulation environment is derived from 
* WorldObject. Include this file if you wish to create your own
* walls, scenery and other interactive objects.
*/

#ifndef _WORLDOBJECT_H
#define _WORLDOBJECT_H

#include <cmath>
#include <iostream>
#include <string>

#include "vector2d.h"
#include "drawable.h"

namespace BEAST {

/**
 * \addtogroup framework
 * @{
 */

// Forward declaration for myWorld pointer
class World;

/**
 * The base class for everything that makes a difference in the world,
 * including Animats, Sensors and all types of scenery and interactive object.
 * WorldObject provides many overridable methods which ensure that just about
 * any kind of thing can be represented in the simulation environment.
 * Of particular importance are Init, Update, Interact and OnCollision, the
 * main methods used in making a useful simulation object.
 * WorldObject also handles some collision detection and provides methods for
 * detecting if other objects are touching this one.
 */
class WorldObject : public Drawable
{
public:
	WorldObject(const Vector2D& l = Vector2D(0,0), double o = 0.0,
				double d = DRAWABLE_RADIUS, bool so = false):
		Drawable(l, o, d),
		Solid(solid),
		Dead(dead),
		InitRandom(initRandom),
		Moveable(moveable),
		Selectable(selectable),
		
		solid(so), dead(false), initRandom(false),
		moveable(true), selectable(true)
		{
			LogStream.init(this, &WorldObject::GetLogStream, &WorldObject::_SetLogStream);
		}

	WorldObject(const Vector2D& l, double o, std::vector<Vector2D> e,
				bool so = false):
		Drawable(l, o, e),
		Solid(solid),
		Dead(dead),
		InitRandom(initRandom),
		Moveable(moveable),
		Selectable(selectable),
		solid(so), dead(false), initRandom(false),
		moveable(true), selectable(true),
		absoluteEdges(GetEdges())
		{
			LogStream.init(this, &WorldObject::GetLogStream, &WorldObject::_SetLogStream);
		}

	~WorldObject() {}

	virtual void	Init();

	virtual void	Update(){ SetOrientation(bound(0.0, TWOPI, GetOrientation())); }
	virtual void	Interact(WorldObject*);
	virtual void	UniInteract(WorldObject*){}
	virtual void	OnCollision(WorldObject* r){}

	virtual bool	IsInside(const Vector2D& vec)const;
	Vector2D		GetNearestPoint(const WorldObject* i) 
					{ return GetNearestPoint(i->GetLocation()); }
	Vector2D		GetNearestPoint(const Vector2D&, 
									Vector2D& r=VECTOR2D_FAKE_REFERENCE)const;
	bool 			Intersects(const Vector2D& l1, const Vector2D& l2,
							   Vector2D& r = VECTOR2D_FAKE_REFERENCE)const;

	virtual void	OnClick(){}
	virtual void	OnSelect(){}

	bool			IsSolid()const			{ return solid; }
	bool			IsDead()const			{ return dead; }
	bool			IsInitRandom()const		{ return initRandom; }
	bool			IsMoveable()const		{ return moveable; }
	bool			IsSelectable()const		{ return selectable; }

	void			SetSolid(bool s)		{ solid = s; }
	void			SetDead(bool d)			{ dead = d; }
	void			SetInitRandom(bool r)	{ initRandom = r; }
	void			SetMoveable(bool m)		{ moveable = m; }
	void			SetSelectable(bool s)	{ selectable = s; }

	void			_SetLogStream(std::ostream& o) { logStream = &o; }
	static void		SetLogStream(std::ostream& o) { logStream = &o; }
	std::ostream&	GetLogStream()const		{ return *logStream; }

	/// Returns basic information about this object as a string.
	virtual std::string ToString()const { return ""; }

	virtual void	Serialise(std::ostream&)const;
	virtual void	Unserialise(std::istream&);

	auto_property<bool> Solid;
	auto_property<bool> Dead;
	auto_property<bool> InitRandom;
	auto_property<bool> Moveable;
	auto_property<bool> Selectable;

	property<WorldObject, std::ostream, std::ostream&> LogStream;

private:
	bool			solid;		///< Whether other objects hit or pass through.
	bool			dead;		///< True when object needs to be removed.
	bool			initRandom;	///< Initialise at a random position.
	bool			moveable;	///< Whether object can be moved by the mouse.
	bool			selectable;	///< Whether object can be selected.

	static bool		CalcIntersect(const Vector2D&, const Vector2D&,
								  const Vector2D&, const Vector2D&,
								  Vector2D& r = VECTOR2D_FAKE_REFERENCE);

	static Vector2D	GetNearestPointOnLine(const Vector2D&,
										  const Vector2D&,
										  const Vector2D&);
	

	/// The absolute positions of the edges, calculated each frame.
	std::vector<Vector2D> absoluteEdges;

	void			CalcAbsoluteEdges();
	
	static std::ostream* logStream;	///< Pointer to global output stream.
	static Vector2D	VECTOR2D_FAKE_REFERENCE; ///< Dummy parameter
};

/**
 * This is a handy class for putting the most common type of obstacle - 
 * walls - into the world.
 */
class Wall : public WorldObject
{
public:
	/**
	 * Constructs a rectangular grey WorldObject with the specified properties.
	 * \param pos The x,y location of the centre of the wall.
	 * \param w The width of the wall
	 * \param h The height of the wall
	 * \param o The orientation of the wall (defaults to vertical - 90 deg).
	 */
	Wall(Vector2D pos = Vector2D(), double w = 50.0, double h = 50.0, double o = PI/2):
	WorldObject(pos, o, GetSides(w, h), true)
	{
		SetMoveable(false);
		SetSelectable(false);
	}

	static std::vector<Vector2D> GetSides(double w, double h);

	IMPLEMENT_SERIALISATION("Wall", WorldObject)
};

/**
 * @}
 */

} // namespace BEAST

#endif
