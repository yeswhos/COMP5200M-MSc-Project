#include "animatmonitor.h"
#include "animat.h"
#include "world.h"

namespace BEAST {

bool AnimatMonitor::visible = true;

void AnimatMonitor::Display()
{
	if (visible) {
		widthSoFar = heightSoFar = 0;
		for (vector<Animat*>::iterator i = begin(); i != end(); ++i) {
			DrawBars(*i);
		}
	}
}

void AnimatMonitor::DrawBars(Animat* aa)
{
	int numBars = 2; // already counted motors

	glLineWidth(4.0);

	glPushMatrix();

	// left motor (numBars++)
	glBegin(GL_LINE_STRIP);
	{
		glColor4fv(ColourPalette[COLOUR_BLUE]);
		glVertex2d(10 + widthSoFar, heightSoFar + 10 + MONITOR_BARHEIGHT);

		glColor4fv(ColourPalette[COLOUR_GREEN]);
		glVertex2d(10 + widthSoFar, heightSoFar + 10 + MONITOR_BARHEIGHT * (1 + aa->controls["left"]));
	}
	glEnd();

	// right motor (numBars++)
	glBegin(GL_LINE_STRIP);
	{
		glColor4fv(ColourPalette[COLOUR_BLUE]);
		glVertex2d(20 + widthSoFar, heightSoFar + 10 + MONITOR_BARHEIGHT);
		glColor4fv(ColourPalette[COLOUR_RED]);
		glVertex2d(20 + widthSoFar, heightSoFar + 10 + MONITOR_BARHEIGHT * (1 + aa->controls["right"]));
	}
	glEnd();

	// sensors:
	int j = 0;
	for (Animat::SensorIter iter(aa->sensors.begin()); iter != aa->sensors.end(); ++iter, ++j) {
		glBegin(GL_LINE_STRIP);
		{
			glColor4fv(ColourPalette[COLOUR_BLUE]);
			glVertex2d(30 + (10 * j) + widthSoFar, heightSoFar + 10 + MONITOR_BARHEIGHT);
			glColor4fv(ColourPalette[COLOUR_WHITE]);
			glVertex2d(30 + (10 * j) + widthSoFar, heightSoFar + 10 + MONITOR_BARHEIGHT * (1 + iter->second->GetOutput()));
		}
		glEnd();
		++numBars;
	}

	// axes:
	glLineWidth(1.0);
	glColor4fv(ColourPalette[COLOUR_BLACK]);
	glBegin(GL_LINES);
	{
		// vertical right
		glVertex2d((numBars * 10) + widthSoFar + 5, heightSoFar + 10);
		glVertex2d((numBars * 10) + widthSoFar + 5, heightSoFar + 10 + (MONITOR_BARHEIGHT * 2));
		// vertical left
		glVertex2d(widthSoFar + 5, heightSoFar + 10);
		glVertex2d(widthSoFar + 5, heightSoFar + 10 + (MONITOR_BARHEIGHT * 2));
		// bottom
		glVertex2d((numBars * 10) + widthSoFar + 5, heightSoFar + 10);
		glVertex2d(widthSoFar + 5, heightSoFar + 10);
		// midpoint
		glVertex2d((numBars * 10) + widthSoFar + 5, heightSoFar + 10 + MONITOR_BARHEIGHT);
		glVertex2d(widthSoFar + 5, heightSoFar +  10 + MONITOR_BARHEIGHT);
		// top
		glVertex2d((numBars * 10) + widthSoFar + 5, heightSoFar + 10 + (MONITOR_BARHEIGHT * 2));
		glVertex2d(widthSoFar + 5, heightSoFar + 10 + (MONITOR_BARHEIGHT * 2));
	}
	glEnd();

	glLineWidth(1.0);
	glColor4fv(aa->GetColour());
	glBegin(GL_LINES);
	{
		// bottom (colour coded)
		glVertex2d((numBars * 10) + widthSoFar + 5, heightSoFar + 10);
		glVertex2d(widthSoFar + 5, heightSoFar + 10);
	}
	glEnd();
	
	widthSoFar += static_cast<int>(20 + aa->sensors.size() * 10);

	if (widthSoFar + (10 * (2 + (int)aa->sensors.size())) > GetWorld().GetWidth()) {
		heightSoFar += (2 * MONITOR_BARHEIGHT) + 5;
		widthSoFar = 0;
	}

	return;
}

} // namespace BEAST
