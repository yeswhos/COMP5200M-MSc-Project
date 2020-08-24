using namespace std;
using namespace BEAST;

class WorldObject3D : public WorldObject
{

private:

  GLuint			displaylist3D;

public:

  virtual void Init()
  {
    WorldObject::Init();
    
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
    if (!IsCircular())
    {
      Draw();
      return;
    }
    
    glColor4fv(GetColour());
    
    GLUquadricObj* quadric = gluNewQuadric();
    
    glPushMatrix();
      glTranslatef(0.0, 0.0, This.Radius);
      gluSphere(quadric, This.Radius, 20, 20);
    glPopMatrix();
    
    gluDeleteQuadric(quadric);
  }


};
