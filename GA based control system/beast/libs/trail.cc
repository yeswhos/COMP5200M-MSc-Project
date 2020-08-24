#include "trail.h"

namespace BEAST {

void Trail::Update()
{
	while (size() > TRAIL_LENGTH) pop_front();
}

void Trail::Display()
{
	if (!IsVisible()) return;

	float f=0;

	glLineWidth(static_cast<GLfloat>(trailWidth));
	glEnable(GL_BLEND);
	glBegin(GL_LINE_STRIP);
	{
		for (iterator i = begin(); i != end(); ++i, ++f) {
			glColor4f(GetColour(0), GetColour(1), GetColour(2), f / size());
			glVertex2d(i->GetX(), i->GetY());
		} // for each trail point
	}
	glEnd();
	glDisable(GL_BLEND);
}

} // namespace BEAST
