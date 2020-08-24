#ifndef _TRAIL_H_
#define _TRAIL_H_

#include "drawable.h"
#include <list>

namespace BEAST {

/**
 * \addtogroup framework
 * @{
 */

const unsigned int TRAIL_LENGTH = 30;

class Trail : public Drawable, public std::list<Vector2D>
{
public:
	Trail(float r = 1.0f, float g = 1.0f, float b = 1.0f):
	  Drawable(), trailWidth(2.0){SetColour(r, g, b);}
	virtual ~Trail(){}

	virtual void	Draw(){}
	virtual void	Display();
	virtual void	Update();

private:
	double			trailWidth;
};

/**
 * @}
 */

} // namespace BEAST

#endif
