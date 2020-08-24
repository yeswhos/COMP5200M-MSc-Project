/**
 * \file world.h
 * \author Tom Carden
 * \addtogroup framework
 */

#ifndef _WORLD_H_
#define _WORLD_H_

#include <vector>
#include <deque>
#include <algorithm>
#include <functional>

#include "vector2d.h"
#include "vector3d.h"
#include "animat.h"
#include "animatmonitor.h"
#include "worldobject.h"
#include "collisions.h"
#include "random.h"
#include "sensorfunctors.h"

#define SIZE 512

namespace BEAST {

/**
 * \addtogroup framework
 * @{
 */

// Forward declaration for Animat container
class Animat;

const double WORLD_WIDTH = 800.0;
const double WORLD_HEIGHT = 600.0;

/** An enumeration type for specifying which elements of the world are to be
 * displayed.
 * \see World::Toggle
 */
enum WorldDisplayType {
	DISPLAY_NONE = 0,			///< Nothing is displayed at all.
	DISPLAY_ANIMATS = 1,		///< Animats are displayed.
	DISPLAY_WORLDOBJECTS = 2,	///< WorldObjects are displayed.
	DISPLAY_TRAILS = 4,			///< Trails are displayed.
	DISPLAY_SENSORS = 8,		///< Sensors are displayed.
	DISPLAY_COLLISIONS = 16,	///< Collisions are displayed.
	DISPLAY_MONITOR = 32,		///< The monitor is displayed.
	DISPLAY_ALL = 65535			///< Everything is displayed.
};

enum WorldDimension {
  TWO = 0,
  THREE = 1
};

/**
 * This is where it all happens: World contains pointers to every object in
 * the simulation environment and allows those objects to interact with each
 * other, and then be displayed.
 */
class World
{
	/// The container type for Animats.
	typedef std::vector<Animat*> AnimatContainer;
	/// The container type for WorldObjects.
	typedef	std::vector<WorldObject*> WorldObjectContainer;
	/// The mutable iterator type for Animats.
	typedef AnimatContainer::iterator AnimatIter;
	/// The mutable iterator type for WorldObjects
	typedef	WorldObjectContainer::iterator WorldObjectIter;

public:
	World();
	~World(){};

	void				Init();
	void				InitGL()const;
	void				Add(Animat* r);
	void				Add(WorldObject* r);
	/// Adds a collision to the collisions object.
	void				AddCollision(Vector2D pv) { collisions.push_back(pv); }
	

	/** Adds a vector containing pointers to objects of the specified type
	 * to the world. The vector may be a type derived from vector, e.g.
	 * Population or Group.
	 * \param T The type of objects to be added, must be derived from Animat
	 * or WorldObject.
	 * \param v A reference to an input vector of pointers to the specified type. */
	template <typename T> 
	void Add(const vector<T*>& v)
	{
		using std::vector;

		Animat* ptra;
		WorldObject* ptrw;
		for (typename vector<T*>::const_iterator i = v.begin(); i != v.end(); ++i) {
			ptra = dynamic_cast<Animat*>(*i);
			if (ptra != NULL) {
				Add(ptra);
				continue;
			}

			ptrw = dynamic_cast<WorldObject*>(*i);
			if (ptrw != NULL) {
				Add(ptrw);
			}
		}
	}

	/** Removes all objects of the specified type from the World and inserts
	 * them into the vector.
	 * \warning Will not work during World::Update - only use between assessments.
	 * \param T The type of objects to remove.
	 * \param v A reference to a vector of pointers to the specified type. */
	template <typename T>
	void Remove(vector<T*>& v)
	{
		using namespace std;

		if (updateInProgress) return;

		T* ptr;
		WorldObjectIter i = 
		     remove_if
		     (
		          worldobjects.begin(), 
			  worldobjects.end(), 
			  bind2nd(ptr_fun(IsKindOf<T, WorldObject>), ptr)
		     );

		for (WorldObjectIter ii = i; ii != worldobjects.end(); ++ii) {
			v.push_back(static_cast<T*>(*ii));
		}
		worldobjects.erase(i, worldobjects.end());

		AnimatIter j = 
                     remove_if
                     (
			  animats.begin(), 
			  animats.end(), 
			  bind2nd(ptr_fun(IsKindOf<T, Animat>), ptr)
		     );
                // David Gordon: ii = i should be changed into ii = j
		// Mdk: implemented 02-01-2007
		for (AnimatIter ii = j; ii != animats.end(); ++ii) {
			v.push_back(static_cast<T*>(*ii));
		}

		// David Gordon: i should be j
		// MdK: implemented 02-01-2007
		animats.erase(j, animats.end());
	}

	/**
	 * Removes all objects of the specified type from the World.
	 * \warning Will not work during World::Update - only use between assessments.
	 */
	template <typename T>
	void Remove()
	{
		using namespace std;

		if (updateInProgress) return;

		worldobjects.erase(	remove_if(worldobjects.begin(), 
									  worldobjects.end(), 
									  MatchKindOf<T>()),
							worldobjects.end());

		animats.erase(		remove_if(animats.begin(), 
									  animats.end(), 
									  MatchKindOf<T>()),
							animats.end());

		monitor.erase(		remove_if(monitor.begin(),
									  monitor.end(),
									  MatchKindOf<T>()),
						    monitor.end());
	}

	// lifecycle functions
	void				Display();
	void				Update();
	void				CleanUp();

	// events
	void				OnMouseLDown(int x, int y);
	void				OnMouseRDown(int x, int y);
	void				OnMouseLUp(int x, int y);
	void				OnMouseRUp(int x, int y);
	void				OnMouseMove(int x, int y);
	void				OnSelectNext();
	void				OnSelectPrevious();

	// helpers
	inline Vector2D		Centre();	///< Returns the centre coordinates
	inline Vector2D		RandomLocation()const;	///< Returns a random coordinate

	// Mutators:
	void				SetWidth(double w)			{ disp.width = w; }
	void				SetHeight(double h)			{ disp.height = h; }
	void				SetWindow(int w, int h)		{ disp.winWidth = w; disp.winHeight = h; }
	void				Toggle(WorldDisplayType t)	{ disp.config ^= t; }
	void				SetColour(const float c[3])	{ SetColour(c[0], c[1], c[2]); }
	inline void			SetColour(float r, float g, float b);

	// accessors
	/// Returns the current width of the world.
	double				GetWidth()const		{ return disp.width; }
	/// Returns the current height of the world.
	double				GetHeight()const	{ return disp.height; }
	/// Returns the current width of the world.
	int					GetWinWidth()const	{ return static_cast<int>(disp.winWidth); }
	/// Returns the current height of the world.
	int					GetWinHeight()const	{ return static_cast<int>(disp.winHeight); }
	/// Returns the current display configuration.
	int					GetDispConfig()const{ return disp.config; }
	/// Returns true if the World is currently performing an update.
	bool				IsUpdating()const	{ return updateInProgress; }
	/** Finds every object in the World of the specified type and adds it to a
	 * vector.
	 * \param T The type of objects to get.
	 * \param v A reference to a vector of pointers to objects of the specified type. */
	template <typename T> void Get(vector<T*>& v)
	{
		using std::vector;

		T* ptr;
		for (AnimatIter i = animats.begin(); i != animats.end(); ++i) {
			ptr = dynamic_cast<T*>(*i);
			if (ptr != NULL) {
				v.push_back(ptr);
			}
		}

		for (WorldObjectIter i = worldobjects.begin(); i != worldobjects.end(); ++i) {
			ptr = dynamic_cast<T*>(*i);
			if (ptr != NULL) {
				v.push_back(ptr);
			}
		}
	}
	WorldObject*		GetSelected()const			{ return mouse.selected; }

	/** Provides direct access to the start of the animat container.
	 * \warning Should not be used when IsUpdating is true. */
	AnimatContainer::const_iterator AnimatsBegin()const { return animats.begin(); }
	/** Provides direct access to the end of the animat container.
	 * \warning Should not be used when IsUpdating is true. */
	AnimatContainer::const_iterator AnimatsEnd()const { return animats.end(); }
	/** Provides direct access to the start of the worldobject container.
	 * \warning Should not be used when IsUpdating is true. */
	WorldObjectContainer::const_iterator WorldObjectsBegin()const { return worldobjects.begin(); }
	/** Provides direct access to the end of the worldobject container.
	 * \warning Should not be used when IsUpdating is true. */
	WorldObjectContainer::const_iterator WorldObjectsEnd()const { return worldobjects.end(); }
	
	void        World2D();
	void        World3D();
	void        SetWXKLeft(int k);
	void        SetWXKRight(int k);
	void        SetWXKUp(int k);
	void        SetWXKDown(int k);
	void        OnKeyDown(int k, char kChar, bool shift);
	void        OnKeyUp(int k, char kChar, bool shift);
	int         GetWorldDimensions();

private:
	AnimatContainer		animats, animatQueue;
	WorldObjectContainer worldobjects, worldobjectQueue;

	Collisions			collisions;
	AnimatMonitor		monitor;

	bool				updateInProgress;

	struct PointerInfo {
		PointerInfo():left(false),right(false),current(NULL),selected(NULL){}
		Vector2D		location;
		Vector2D		staticLocation;
		bool			left;
		bool			right;
		WorldObject*	current;
		WorldObject*	selected;
	} mouse;

	struct DisplayInfo {
		DisplayInfo():
			width(WORLD_WIDTH),height(WORLD_HEIGHT),
			winWidth(WORLD_WIDTH), winHeight(WORLD_HEIGHT),
			config(DISPLAY_ALL){}
        double			width;
		double			height;
		double			winWidth;
		double			winHeight;
		int				config;
		float			colour[3];
		int       dimension;
	} disp;
	
	struct KeyInfo {
    KeyInfo():left(false),right(false),up(false),down(false),add(false),sub(false){}
    bool left;
    bool right;
    bool up;
    bool down;
    bool add;
    bool sub;
    int wxLeft;
    int wxRight;
    int wxUp;
    int wxDown;
  } key;

	Vector2D			WindowXY(int x, int y);
	void DrawObjects();
	inline void			UpdateMouse();
	inline void			UpdateQueues();
	void MoveEye();
	Vector3D eye;
	Vector3D look;
	Vector3D up;
};

Vector2D World::Centre()
{
	return Vector2D(disp.width / 2.0, disp.height / 2.0);
}

Vector2D World::RandomLocation()const {
	return Vector2D(disp.width * static_cast<double>(rand()-1)/static_cast<double>(RAND_MAX),
					disp.height * static_cast<double>(rand()-1)/static_cast<double>(RAND_MAX));
}

void World::SetColour(float r, float g, float b)
{
	disp.colour[0] = r;
	disp.colour[1] = g;
	disp.colour[2] = b;
	glClearColor(r, g, b, 1.0f);
}

/**
 * @}
 */

} // namespace BEAST

#endif
