//
// collision.cc
//

#include "collisions.h"

namespace BEAST {

void Collisions::Update()
{
	while (size() > MAX_COLLISIONS) pop_front();
}

void Collisions::Display()
{
	if (!IsVisible()) return;

	GLUquadricObj* Disk;
	Disk = gluNewQuadric();
	gluQuadricDrawStyle(Disk, GLU_FILL);
	glColor4f(0.9f, 0.9f, 0.1f, 0.05f);
	glEnable(GL_BLEND);
	
	for (iterator i = begin(); i != end(); ++i) {
		glPushMatrix();
		glTranslated(i->GetX(), i->GetY(), 0);
		gluDisk(Disk, 0, 3, 10, 1);
		glPopMatrix();		
	}
	
	glDisable(GL_BLEND);
	gluDeleteQuadric(Disk);
	
	return;
}

} // namespace BEAST
