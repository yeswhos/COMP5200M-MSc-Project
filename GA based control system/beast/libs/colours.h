/**
 * \file colours.h
 * A handy include which provides a bunch of colours.
 * \author Tom Carden
 * \author David Gordon
 */

#ifndef _COLOURS_H
#define _COLOURS_H

#include "random.h"

namespace BEAST { 

/**
 * \addtogroup utilities
 * @{
 */
		
/// An enumeration type for colours.
enum ColourType {
	COLOUR_BLACK,	///< Black
	COLOUR_WHITE,	///< White
	COLOUR_GREEN,	///< Green
	COLOUR_BLUE,	///< You get the idea
	COLOUR_RED,
	COLOUR_PURPLE,
	COLOUR_DARK_PURPLE,
	COLOUR_YELLOW,
	COLOUR_LILAC,
	COLOUR_BROWN,
	COLOUR_LIGHT_GREY,
	COLOUR_DARK_GREY,
	COLOUR_MID_GREY,
	COLOUR_ORANGE,
	COLOUR_PINK,
	COLOUR_SELECTION
};

/// A global colour pallete. Could probably do with many more colours.
const float ColourPalette[][4] =
{
	{	0.0f, 0.0f, 0.0f, 1.0f	},	// black
	{	1.0f, 1.0f, 1.0f, 1.0f	},	// white
	{	0.2f, 0.8f, 0.2f, 1.0f	},	// green
	{	0.2f, 0.2f, 0.8f, 1.0f	},	// blue
	{	0.8f, 0.2f, 0.2f, 1.0f	},	// red
	{	0.5f, 0.3f, 0.7f, 1.0f	},	// purple
	{	0.2f, 0.0f, 0.4f, 1.0f	},	// dark purple
	{	0.8f, 0.8f, 0.2f, 1.0f	},	// yellow
	{	0.8f, 0.5f, 0.9f, 1.0f	},	// lilac
	{	0.4f, 0.3f, 0.1f, 1.0f	},	// brown
	{	0.8f, 0.8f, 0.8f, 1.0f	},	// light grey
	{	0.3f, 0.3f, 0.3f, 1.0f	},	// dark grey
	{	0.5f, 0.5f, 0.5f, 1.0f	},	// mid grey
	{	0.9f, 0.9f, 0.1f, 1.0f	},	// orange
	{	1.0f, 0.8f, 0.8f, 1.0f	},	// pink
	{	0.5f, 0.5f, 1.0f, 0.5f	}	// selected
};

/// Returns a random colour, all set for input to glColour4fv
inline const float* random_colour()
{
	static float colour[4];
	
	colour[0] = randval(1.0f);
	colour[1] = randval(1.0f);
	colour[2] = randval(1.0f);
	colour[3] = randval(1.0f);
	
	return colour;
}

/**
 * @}
 */

} // namespace BEAST

#endif

