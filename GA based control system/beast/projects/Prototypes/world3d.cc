#ifndef _WORLD3D_
#define _WORLD3D_


#include "beast.h"

namespace BEAST {

class World3D : public World
{

public:

  virtual void InitGL()
  {

	  glMatrixMode(GL_PROJECTION);
	  glLoadIdentity();
	  gluPerspective(45.0, 1.0, 0.1, 1000.0);
	  //gluOrtho2D(0, disp.width, 0, disp.height);
	  glMatrixMode(GL_MODELVIEW);
	  glBlendFunc(GL_SRC_ALPHA, GL_ONE); // for sensors
	  gluLookAt(400.0, 400.0, 500.0, 400.0, 400.0, 0.0, 0.0, 1.0, 0.0);
  }

};

}

#endif
