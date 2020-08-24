/**
 * \file collisions.h Draws collisions in the World
 * \author Tom Carden
 */

#ifndef _COLLISION_H_
#define _COLLISION_H_

#include <list>

#include "drawable.h"

namespace BEAST { 

/**
 * \addtogroup framework
 * @{
 */

const unsigned int MAX_COLLISIONS = 200;

class Collisions : public Drawable, public std::list<Vector2D>
{
public:
	// constructors, destructor...
	// specify nothing, use defaults for the const attributes...
	Collisions(): Drawable() {}
	virtual ~Collisions() {}
	
	virtual void	Draw(){}
	virtual void	Update();
	virtual void	Display();

};

/**
 * @}
 */

} // namespace BEAST

#endif

