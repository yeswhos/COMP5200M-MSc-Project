/**
 * \file animatmonitor.h
 * \author Tom Carden
 * \author David Gordon
 */

#ifndef _ANIMAT_MONITOR_H_
#define _ANIMAT_MONITOR_H_

#include <vector>
#include <algorithm>

#include "drawable.h"

namespace BEAST { 

/**
 * \addtogroup framework
 * @{
 */

class Animat;

using std::vector;

const int MONITOR_BARHEIGHT = 25;

class AnimatMonitor : public Drawable, public vector<Animat*>
{
public:
	AnimatMonitor(){}
	virtual ~AnimatMonitor(){}

	virtual void	Display();

	void			SetVisible(bool v) { visible = v; }
	bool			IsVisible()const { return visible; }

private:
	int				widthSoFar,
					heightSoFar;

	void			DrawBars(Animat*);

	static bool		visible;
};

/**
 * @}
 */

} // namespace BEAST

#endif
