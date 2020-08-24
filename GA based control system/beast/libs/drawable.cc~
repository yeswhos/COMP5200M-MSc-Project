/**
 * \file drawable.cc
 * Implementation of Drawable
 */

#include "drawable.h"
#include "world.h"

namespace BEAST {

/**
 * Initialises the GL display list and figures out the effective radius of
 * non-circular objects.
 * \see Drawable::Draw
 */
void Drawable::Init() {
	// allocate a free Display list index, deleting any existing
	// display list first
	if (displaylist != 0) glDeleteLists(displaylist, 1);
	displaylist = glGenLists(1);

	/* define the Display list: */
	glNewList(displaylist, GL_COMPILE);
	{
		Draw();
	}
	glEndList();

	if (!circular) {
		for (vector<Vector2D>::iterator i(edges.begin());
			i != edges.end(); ++i) {
			if (i->GetLengthSquared() > radiusSquared) SetRadius(i->GetLength());
		}
	}

	return;
}

/**
 * Default display method used on nearly all objects. Positions and orients
 * the modelview matrix and calls the object's Render method.
 * \see Drawable::Render
 */
void Drawable::Display()
{
	if (!visible) return;

	glPushMatrix();
	glTranslated(location.x, location.y, 0);
	glRotated(rad2deg(orientation), 0.0, 0.0, 1.0);
	Render();
	glPopMatrix();
	return;
}

/**
 * By default simply calls the display list for this object, but may be
 * overridden to display dynamically changing objects.
 */
void Drawable::Render()
{
	glCallList(displaylist);
}


/**
 * Draws a simple shape according to the Drawable's shape, size and colour
 */
void Drawable::Draw()
{
	float sides = circular ? 15.0f : static_cast<float>(edges.size()),
		  pos, f = 0;

	if (circular) {
		glBegin(GL_POLYGON);
			for (;f<sides;++f) {
				pos = f / sides;
				glColor4f(colour[0] * (1 - pos * pos),
						  colour[1] * (1 - pos * pos),
						  colour[2] * (1 - pos * pos),
						  colour[3]);
				glVertex2d(GetRadius() * static_cast<float>(sin(pos * TWOPI)), GetRadius() * static_cast<float>(cos(pos * TWOPI)));
			}
		glEnd();
	}
	else {
		glBegin(GL_POLYGON);
		for (vector<Vector2D>::iterator i(edges.begin());
			 i != edges.end(); ++i, ++f) {
			pos = f / sides;
			glColor4f(colour[0] * (1 - pos * pos), 
					  colour[1] * (1 - pos * pos),
					  colour[2] * (1 - pos * pos),
					  colour[3]);
			glVertex2d(i->x, i->y);
		}
		glEnd();
	}

	return;
}

using namespace std;

/**
 * Outputs the object's data to a stream.
 * \see Drawable::Unserialise
 */
void Drawable::Serialise(ostream& out)const
{
	out << "Drawable\n"
		<< location
		<< orientation << endl
		<< radius << endl
		<< (circular ? "circle" : "polygon") << endl
		<< colour[0] << endl
		<< colour[1] << endl
		<< colour[2] << endl
		<< colour[3] << endl
		<< (visible ? "visible" : "invisible") << endl
		<< edges;
}

/** 
 * Sets up the object from a stream
 * \see Drawable::Serialise
 */
void Drawable::Unserialise(istream& in)
{
	string name;
	in >> name;
	if (name != "Drawable") {
		throw SerialException(SERIAL_ERROR_WRONG_TYPE, name, 
							  "This object is type Drawable");
	}

	in >> location
	   >> orientation
	   >> radius
	   >> switcher("circle", circular)
	   >> colour[0]
	   >> colour[1]
	   >> colour[2]
	   >> colour[3] 
	   >> switcher("visible", visible)
	   >> edges;

   SetRadius(radius);	// and sort out radiusSquared in the process.
}

ostream& operator<< (ostream& out, const Drawable& d)
{
	d.Serialise(out);
	return out;
}

istream& operator>> (istream& in, Drawable& d)
{
	d.Unserialise(in);
	return in;
}

} // namespace BEAST
