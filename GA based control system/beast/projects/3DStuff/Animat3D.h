using namespace std;
using namespace BEAST;

class Animat3D : public Animat
{

private:

  GLuint			displaylist3D;

public:

  virtual void Init()
  {
    Animat::Init();
    
    displaylist3D = glGenLists(2);
    
    /* define the Display list: */
	  glNewList(displaylist3D, GL_COMPILE);
	  {
		  Draw3D();
	  }
	  glEndList();

	  
  }
  
  
  virtual void Render()
  {
    if (GetWorld().GetWorldDimensions() == THREE)
      glCallList(displaylist3D);
    else
      WorldObject::Render();
  }
  
  
  virtual void Draw3D()
  {
    GLUquadricObj* quadric = gluNewQuadric();
    gluQuadricNormals(quadric, GLU_SMOOTH);
    float colTemp[4] = { GetColour(0), GetColour(1), GetColour(2), GetColour(3) };

	  //Drawable::SetColour(colours[ANIMAT_BODY]);
	  //Drawable::Draw();		// Borrow the nice shaded effect from drawable
	  //Drawable::SetColour(colTemp);

	  //glColor4fv(colours[ANIMAT_CENTRE]);
	  glPushMatrix();
	    //glTranslatef(0.0, 0.0, GetRadius() / 2);
	    gluCylinder(quadric, GetRadius(), GetRadius(), GetRadius(), 20, 20); // body
	    glTranslatef(0.0, 0.0, GetRadius());
	    gluDisk(quadric, 0.0, GetRadius(), 20, 20);
	  glPopMatrix();

    glDisable(GL_DEPTH_TEST);
	  glColor4fv(colours[ANIMAT_ARROW]);
	  glPushMatrix();
	    glTranslatef(0.0, 0.0, GetRadius());
	    glBegin(GL_QUAD_STRIP);
	    {
	      glNormal3f(0.0, 0.0, 1.0);
	      glVertex2d(0.0, GetRadius() / 2.0 + 0.5);
	      glVertex2d(0.0, GetRadius() / 2.0 - 0.5);
	    
	      glVertex2d(GetRadius() / 1.5 + 0.5, 0.0);
	      glVertex2d(GetRadius() / 1.5 - 0.5, 0.0);
	      
	      glVertex2d(0.0, GetRadius() / -2.0 - 0.5);
	      glVertex2d(0.0, GetRadius() / -2.0 + 0.5);
	    
	    }
	    glEnd(); // direction arrow
	  glPopMatrix();
	  glEnable(GL_DEPTH_TEST);

	  glColor4fv(colours[ANIMAT_WHEEL]);
	  glPushMatrix();
	    
	    glTranslatef(0.0, GetRadius(), GetRadius() / 2.0);
	    glRotatef(-90.0, 1.0, 0.0, 0.0);
		  gluCylinder(quadric, GetRadius() / 2, GetRadius() / 2, 2.0, 20, 20);
		  glTranslatef(0.0, 0.0, 2.0); //z in line with global x
		  gluDisk(quadric, 0.0, GetRadius() / 2, 20, 20);
	  glPopMatrix();

	  glPushMatrix();
	    
	    glTranslatef(0.0, -GetRadius(), GetRadius() / 2.0);
	    glRotatef(90.0, 1.0, 0.0, 0.0);
		  gluCylinder(quadric, GetRadius() / 2, GetRadius() / 2, 2.0, 20, 20);
		  glTranslatef(0.0, 0.0, 2.0); //z in line with global x
		  gluDisk(quadric, 0.0, GetRadius() / 2, 20, 20);
	  glPopMatrix();
	  
	  gluDeleteQuadric(quadric);

  }


};
