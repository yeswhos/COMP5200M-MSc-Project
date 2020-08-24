/*
 * \file world.cc
 * The implementation of World.
 */

#include "world.h"
#include "animat.h"

using namespace std;

namespace BEAST {

/**
 * Constructor, simply configures the world's monitor and collisions classes.
 */
World::World(): updateInProgress(false)
{
	monitor.SetWorld(this);
	collisions.SetWorld(this);
	SetColour(ColourPalette[COLOUR_DARK_PURPLE]);
	
	eye = Vector3D(0.5 * disp.width, disp.height, 100.0);
  look = Vector3D(disp.width / 2.0, disp.height / 2.0, 0.0);
  up = Vector3D(0.0, 0.0, 1.0);
}

/**
 * Calls Init on every Animat and WorldObject in the World. Usually called at
 * the start of a simulation, to allow objects to be set up correctly (e.g.
 * defining display lists and performing configuration which can't be done til
 * an object knows which World it's in.
 * \see WorldObject::Init
 * \see Animat::Init
 */
void World::Init()
{
	for_each(worldobjects.begin(), worldobjects.end(), mem_fun(&WorldObject::Init));
	for_each(animats.begin(), animats.end(), mem_fun(&Animat::Init));
}

/**
 * Sets up GL with the correct background colour, projection mode and blend
 * function.
 */
void World::InitGL()const
{
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  
  //lights - for 3D mode
  glEnable(GL_COLOR_MATERIAL); //use glColor functions to change material properties
  //light parameters
  GLfloat global_ambient[] = { 0.3, 0.3, 0.3, 1.0 };
  GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };
  
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
  GLfloat position[] = { 0.0, disp.height / 2, disp.width / 2, 1.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, position);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
  glEnable(GL_LIGHT0);
  
  glShadeModel(GL_SMOOTH); //gouraud shading
  
	glClearColor(disp.colour[0], disp.colour[1], disp.colour[2], 1.0f);
  
  
	glMatrixMode(GL_PROJECTION);
  gluOrtho2D(0, disp.width, 0, disp.height);
	//glFrustum(0.0, 100000.0, 0.0, 80000.0, -0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE); // for sensors
}	

/**
 * Called every frame and responsible for calling Update on every WorldObject
 * and Animat in the World.
 * \see WorldObject::Update
 * \see Animat::Update
 */
void World::Update()
{
	updateInProgress = true;

	UpdateMouse();

	// Update every object in the world.
	for_each(worldobjects.begin(), worldobjects.end(), mem_fun(&WorldObject::Update));
	for_each(animats.begin(), animats.end(), mem_fun(&Animat::Update));

	worldobjects.erase(
		remove_if(worldobjects.begin(), worldobjects.end(),
				  mem_fun(&WorldObject::IsDead)),
		worldobjects.end());
	animats.erase(
		remove_if(animats.begin(), animats.end(), 
				  mem_fun(&Animat::IsDead)),
		animats.end());

	if (!animats.empty()) {
		// Interactions:
		AnimatIter j, k;

		AnimatIter first = animats.begin();
		AnimatIter last = animats.end();
		AnimatIter secondlast = animats.end() - 1;

		// Each Animat interacts with each WorldObject:
		for (WorldObjectIter i = worldobjects.begin(); i != worldobjects.end(); ++i)
			for (j = first; j != last; ++j)
				(*j)->Interact(*i);

		// Each Animat interacts with each Animat.
		for (j = first; j != secondlast; ++j)
			for (k = ++first; k != last; ++k)
				(*j)->Interact(*k);
	}

	collisions.Update();
	updateInProgress = false;
	UpdateQueues();
}

/**
 * Adds an Animat to the World's animat container, set's the Animat's world to
 * this one and adds a pointer to the Animat to the monitor object.
 * \param r A pointer to the Animat to be added.
 */
void World::Add(Animat* r)
{
	if (!updateInProgress) animats.push_back(r);
	else animatQueue.push_back(r);
	r->SetWorld(this);
	monitor.push_back(r);
}

/**
 * Adds a WorldObject to the World's worldobject container and set's the
 * WorldObject's world to this one.
 * \param r A pointer to the WorldObject to be added.
 */
void World::Add(WorldObject* r)
{
	if (!updateInProgress) worldobjects.push_back(r);
	else worldobjectQueue.push_back(r);
	r->SetWorld(this);
}

/**
 * Clears all containers in this World.
 */
void World::CleanUp()
{
	animats.clear();
	worldobjects.clear();
	collisions.clear();
	monitor.clear();
	mouse.current = NULL;
	mouse.selected = NULL;
}

/**
 * Calls the Display method of every object in the world, depending on this
 * World's DisplayInfo struct.
 * \see DisplayInfo
 * \see WorldDisplayType
 */
void World::Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	/*
	 * For 3D world must be drawn differently in order to get depth testing
	 * done for Animats and WorldObjects, but to disable depth testing
	 * on collisions and mouse selections.
	 *
	 *
	 */
	if (disp.dimension == THREE) {
	
	  glViewport (0, 0, disp.winWidth, disp.winHeight);
	  SetColour(1.0, 1.0, 1.0);
	  
	  //camera stuff
	  //glTranslatef(100.0, 0.0, 0.0);
    MoveEye();
	  gluLookAt(eye.x, eye.y, eye.z, look.x, look.y, look.z, up.x, up.y, up.z);

	  glColor4fv(ColourPalette[COLOUR_DARK_PURPLE]);
    
    glEnable(GL_LIGHTING);
    //table second - want everything to appear on top of this
	  glBegin(GL_QUADS);
	    glNormal3f(0.0, 0.0, 1.0);
	    glVertex3f(0.0, 0.0, 0.0);
	    glVertex3f(0.0, disp.height, 0.0);
	    glVertex3f(disp.width, disp.height, 0.0);
	    glVertex3f(disp.width, 0.0, 0.0);
	    
	  glEnd();
	  glDisable(GL_LIGHTING);
	    
	  //next - collisions
	  if ((disp.config & DISPLAY_COLLISIONS) != 0) collisions.Display();
	  
	  //look at location marker
	  glColor4fv(ColourPalette[COLOUR_BLACK]);
	  glBegin(GL_QUADS);
	    glVertex3f(look.x - 1, look.y + 10, 0.0);
	    glVertex3f(look.x + 1, look.y + 10, 0.0);
	    glVertex3f(look.x + 1, look.y - 10, 0.0);
	    glVertex3f(look.x - 1, look.y - 10, 0.0);
	    glVertex3f(look.x - 10, look.y - 1, 0.0);
	    glVertex3f(look.x - 10, look.y + 1, 0.0);
	    glVertex3f(look.x + 10, look.y + 1, 0.0);
	    glVertex3f(look.x + 10, look.y - 1, 0.0);
	  glEnd();
	  
	  //finally, mouse choices
	  if (mouse.selected != NULL) {
		  glColor4fv(ColourPalette[COLOUR_SELECTION]);
		  Vector2D pos = mouse.selected->GetLocation();
		  glEnable(GL_BLEND);
		  glPushMatrix();
			  GLUquadricObj* Disk;
			  Disk = gluNewQuadric();
			  gluQuadricDrawStyle(Disk, GLU_FILL);
			  glTranslated(pos.x, pos.y, 0.0);
			  gluDisk(Disk, 0, mouse.selected->GetRadius() + 5.0, 16, 1);
			  gluDeleteQuadric(Disk);
		  glPopMatrix();
		  glDisable(GL_BLEND);
		}
		
		//then just do world objects and animats using depth testing
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		
		if ((disp.config & DISPLAY_WORLDOBJECTS) != 0) {
		for_each(worldobjects.begin(), worldobjects.end(),
				 mem_fun(&WorldObject::Display));
	  }
	  if ((disp.config & DISPLAY_ANIMATS) != 0) {
		  for_each(animats.begin(), animats.end(), mem_fun(&Animat::Display));
	  }
	  
	  glDisable(GL_LIGHTING);
	  glDisable(GL_DEPTH_TEST);
	  
	  
	}
	else
	{
	
	  SetColour(ColourPalette[COLOUR_DARK_PURPLE]);

	  if ((disp.config & DISPLAY_WORLDOBJECTS) != 0) {
		  for_each(worldobjects.begin(), worldobjects.end(),
		  		 mem_fun(&WorldObject::Display));
	  }
	  if ((disp.config & DISPLAY_ANIMATS) != 0) {
		  for_each(animats.begin(), animats.end(), mem_fun(&Animat::Display));
	  }
	
	  if (mouse.selected != NULL) {
		  glColor4fv(ColourPalette[COLOUR_SELECTION]);
		  Vector2D pos = mouse.selected->GetLocation();
		  glEnable(GL_BLEND);
		  glPushMatrix();
		  	GLUquadricObj* Disk;
		  	Disk = gluNewQuadric();
		  	gluQuadricDrawStyle(Disk, GLU_FILL);
		  	glTranslated(pos.x, pos.y, 0.0);
		  	gluDisk(Disk, 0, mouse.selected->GetRadius() + 5.0, 16, 1);
		  	gluDeleteQuadric(Disk);
		  glPopMatrix();
		  glDisable(GL_BLEND);
	  }

	  if ((disp.config & DISPLAY_COLLISIONS) != 0) collisions.Display();
	  if ((disp.config & DISPLAY_MONITOR) != 0) monitor.Display();
	}
}

void World::DrawObjects()
{
  if ((disp.config & DISPLAY_WORLDOBJECTS) != 0) {
		//for_each(worldobjects.begin(), worldobjects.end(),
				 //mem_fun(&WorldObject::Display));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity();
		gluLookAt(eye.x, eye.y, eye.z, look.x, look.y, look.z, up.x, up.y, up.z);
	  for (int i = 1; i <= worldobjects.size(); i++)
	  {
	    //cout << i << endl;
	    glLoadName(i);
	    worldobjects[i - 1]->Display();
	  }
	  
	  glFlush();
	}
	
	
	//if ((disp.config & DISPLAY_ANIMATS) != 0) {
		//for_each(animats.begin(), animats.end(), mem_fun(&Animat::Display));
	//}
}

void World::OnMouseLDown(int x, int y)
{
	if (mouse.right) return;

	mouse.left = true;
	mouse.location = WindowXY(x, y);
	
	if (disp.dimension == THREE)
	{
	  GLuint selectBuf[SIZE];
    GLint hits;
    GLint viewport[4];
  
    glGetIntegerv (GL_VIEWPORT, viewport);
    glSelectBuffer (SIZE, selectBuf);
    glRenderMode(GL_SELECT);
  
    glInitNames();
    glPushName(0);
  
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();
    /*  create 5x5 pixel picking region near cursor location	*/
    gluPickMatrix ((GLdouble) x, (GLdouble) (viewport[3] - y), 
                  5.0, 5.0, viewport);
    //gluOrtho2D (-2.0, 2.0, -2.0, 2.0);
    if (disp.width <= disp.height)
	    gluPerspective(45.0, disp.winHeight / disp.winWidth, 0.1, 10000.0);
    else
      gluPerspective(45.0, disp.winWidth / disp.winHeight, 0.1, 10000.0);
    
    glMatrixMode(GL_MODELVIEW);
    DrawObjects();
    
    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();
    glFlush ();
    
    hits = glRenderMode (GL_RENDER);
    glMatrixMode (GL_MODELVIEW);

    
    //cout << hits << endl;
    
    unsigned int i, j;
    GLint ii, jj, names, *ptr;
    
    //printf ("hits = %d\n", hits);
    ptr = (GLint *) selectBuf; 
    
    if (hits > 0)
    {
      
      WorldObject* closestHit;
   
      WorldObject* far = new WorldObject(Vector2D(10000, 10000));
      closestHit = far;
      Vector3D v = Vector3D(closestHit->GetLocation().x, closestHit->GetLocation().y, 0.0) - eye;
      double closestLen = v.GetLength();
      for (i = 0; i < hits; i++) {	/*  for each hit  */
        names = *ptr;
	      ptr+=3;
        for (j = 0; j < names; j++) { /*  for each name */
          Vector3D vNew = Vector3D(worldobjects[*ptr - 1]->GetLocation().x, worldobjects[*ptr - 1]->GetLocation().y, 0.0) - eye;
          double l = vNew.GetLength();
          if (l < closestLen)
          {
            closestLen = l;
            closestHit = worldobjects[*ptr - 1];
          }
          //cout << *ptr << endl;
          ptr++;
        }
      }
      delete far;
      
    
      //WorldObject* obj = worldobjects[*ptr - 1];
      //obj->OnClick();
      //obj->OnSelect();
      //if (obj->IsMoveable()) mouse.current = obj;
      if (closestHit->IsMoveable()) mouse.current = closestHit;
      if (closestHit->IsSelectable()) mouse.selected = closestHit;
    }

    //Display();
    //glutPostRedisplay();
  }
  else
  {

	  for (AnimatIter i = animats.begin(); i != animats.end(); ++i) {
		  if ((*i)->IsInside(mouse.location)) {
			  (*i)->OnClick();
			  if ((*i)->IsMoveable()) mouse.current = *i;
			  if ((*i)->IsSelectable()) mouse.selected = *i;
			  if ((*i)->IsMoveable() || (*i)->IsSelectable()) break;
		  }
	  }
	  if (mouse.current == NULL) {
		  for (WorldObjectIter i = worldobjects.begin();
			  i != worldobjects.end(); ++i) {
			  if ((*i)->IsInside(mouse.location)) {
				  (*i)->OnClick();
				  if ((*i)->IsMoveable()) mouse.current = *i;
				  if ((*i)->IsSelectable()) mouse.selected = *i;
				  if ((*i)->IsMoveable() || (*i)->IsSelectable()) break;
			  }
		  }
	  }
	}
	if (mouse.current == NULL) mouse.selected = NULL;
	else mouse.current->OnSelect();
}

void World::OnMouseRDown(int x, int y)
{
	mouse.right = true;

	if (mouse.current != NULL) {
		mouse.staticLocation = mouse.location;
	}
}

void World::OnMouseLUp(int x, int y)
{
	mouse.left = false;
	mouse.location = WindowXY(x, y);

	if (mouse.current != NULL) {
		if (!mouse.right && disp.dimension == TWO) {
			mouse.current->SetLocation(mouse.location);
		}
		mouse.current = NULL;
	}
}

void World::OnMouseRUp(int x, int y)
{
	mouse.right = false;

	if (mouse.left) {
		mouse.current = NULL;
	}
}

void World::OnMouseMove(int x, int y)
{
    
    
  if (disp.dimension == THREE && mouse.right == true)
  {
    Vector2D newLoc = WindowXY(x, y);

    Vector2D diff = newLoc - mouse.location;
    double direction = diff.GetAngle();
    Vector3D temp = eye - look;
    Vector3D rotateZ = Vector3D(0.0, 0.0, 1.0);
    Vector3D norm = rotateZ.cross(look - eye);
    float circ = 2 * PI * temp.GetLength();
    //cout << temp.GetLength() << endl;
    float move = 0.02;
    
    if (direction >= 0.25*PI && direction <= 0.75*PI)
    {
      move = -move;
      temp.rotate(move, norm);
    }
    else if (direction >= 1.25*PI || direction <= -0.25*PI)
    {
      temp.rotate(move, norm);
    }
    else if (direction < 1.25*PI && direction > 0.75*PI)
    {
      
      temp.rotate(move, rotateZ);
    }
    else if (direction > -0.25*PI || direction < 0.25*PI)
    {
      
      move = -move;
      temp.rotate(move, rotateZ);
    }
    
    
    eye = temp + look;
    mouse.location = newLoc;
  }
  else
	  mouse.location = WindowXY(x, y);
}

void World::OnSelectNext()
{
	AnimatIter a = animats.end();

	if (mouse.selected != NULL) {
		a = find(animats.begin(), animats.end(), mouse.selected);
	}

	if (a == animats.end()) {
		a = find_if(animats.begin(), animats.end(), mem_fun(&WorldObject::IsSelectable));
	}

	if (a != animats.end()) {
		AnimatIter b = a;
		++a;
		for (; a != b; ++a) {
			if (a == animats.end()) a = animats.begin();
			if ((*a)->IsSelectable()) break;
		}
		mouse.selected = *a;
		mouse.selected->OnSelect();
	}
	else {
		mouse.selected = NULL;
		return;
	}
}

void World::OnSelectPrevious()
{
	AnimatContainer::reverse_iterator a = animats.rend();

	if (mouse.selected != NULL) {
		a = find(animats.rbegin(), animats.rend(), mouse.selected);
	}

	if (a == animats.rend()) {
		a = find_if(animats.rbegin(), animats.rend(), mem_fun(&WorldObject::IsSelectable));
	}

	if (a != animats.rend()) {
		AnimatContainer::reverse_iterator b = a;
		++a;
		for (; a != b; ++a) {
			if (a == animats.rend()) a = animats.rbegin();
			if ((*a)->IsSelectable()) break;
		}
		mouse.selected = *a;
	}
	else {
		mouse.selected = NULL;
		return;
	}
}

Vector2D World::WindowXY(int x, int y)
{
	return Vector2D((static_cast<double>(x) / disp.winWidth) * disp.width, 
				  ((disp.winHeight - static_cast<double>(y)) / disp.winHeight) * disp.height);
}

void World::UpdateMouse()
{
	if (mouse.current == NULL) return;

	if (mouse.left && disp.dimension == TWO) {
		if (mouse.right) {
			mouse.current->SetOrientation((mouse.location - mouse.current->GetLocation()).GetAngle());
			mouse.current->SetLocation(mouse.staticLocation);
		}
		else{
			mouse.current->SetLocation(mouse.location);
		}
	} 
	return;	
}

void World::UpdateQueues()
{
	copy(animatQueue.begin(), animatQueue.end(),
		 inserter(animats, animats.end()));
	animatQueue.clear();

	copy(worldobjectQueue.begin(), worldobjectQueue.end(),
		 inserter(worldobjects, worldobjects.end()));
	worldobjectQueue.clear();
}

void World::World2D()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, disp.width, 0, disp.height);
	glMatrixMode(GL_MODELVIEW);
	disp.dimension = TWO;
	
	Display();
}

void World::World3D()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
	
	if (disp.width <= disp.height)
	  gluPerspective(45.0, disp.winHeight / disp.winWidth, 0.1, 10000.0);
  else
    gluPerspective(45.0, disp.winWidth / disp.winHeight, 0.1, 10000.0);
  
  glMatrixMode(GL_MODELVIEW);
  disp.dimension = THREE;
  
  Display();
}

void World::SetWXKLeft(int k)
{
  key.wxLeft = k;
}

void World::SetWXKRight(int k)
{
  key.wxRight = k;
}
	
void World::SetWXKUp(int k)
{
  key.wxUp = k;
}

void World::SetWXKDown(int k)
{
  key.wxDown = k;
}
	
void World::OnKeyDown(int k, char c, bool shift)
{
  if (k == key.wxLeft)
    key.left = true;
  else if (k == key.wxRight)
    key.right = true;
  else if (k == key.wxUp)
    key.up = true;
  else if (k == key.wxDown)
    key.down = true;
  else if (c == '=' && shift || c == '+')
    key.add = true;
  else if (c == '-' && !shift)
    key.sub = true;
}
	
void World::OnKeyUp(int k, char c, bool shift)
{
  if (k == key.wxLeft)
    key.left = false;
  else if (k == key.wxRight)
    key.right = false;
  else if (k == key.wxUp)
    key.up = false;
  else if (k == key.wxDown)
    key.down = false;
  else if (c == '=' || c == '+')
    key.add = false;
  else if (c == '-')
    key.sub = false;
}

int World::GetWorldDimensions()
{
  return disp.dimension;
}

void World::MoveEye()
{
  if (key.left && !key.right)
  {

    Vector3D c2 = Vector3D(0.0, 0.0, 1.0);

    Vector3D norm = c2.cross(look - eye);
    norm.normalise();

    eye += norm * 6;
    look += norm * 6;
    //Vector3D norm = look.cross(cross2);

  }
  else if (!key.left && key.right)
  {
    Vector3D c2 = Vector3D(0.0, 0.0, 1.0);

    Vector3D norm = c2.cross(look - eye);
    norm.normalise();

    eye -= norm * 6;
    look -= norm * 6;
  }
  
  if (key.up && !key.down)
  {
    Vector2D dir = Vector2D(look.x - eye.x, look.y - eye.y);
    
    dir.normalise();
    double xChange = dir.x * 6;
    double yChange = dir.y * 6;
    eye.x += xChange;
    eye.y += yChange;

    look.x += xChange;
    look.y += yChange;
  }
  else if (!key.up && key.down)
  {
    Vector2D dir = Vector2D(look.x - eye.x, look.y - eye.y);
    
    dir.normalise();;
    double xChange = dir.x * 6;
    double yChange = dir.y * 6;
    eye.x -= xChange;
    eye.y -= yChange;

    look.x -= xChange;
    look.y -= yChange;
  }

  if (key.add && !key.sub)
  {
    Vector3D dir = look - eye;
    dir.normalise();

    eye += (dir * 6);
    
  }
  else if (!key.add && key.sub)
  {
    Vector3D dir = look - eye;
    dir.normalise();

    eye -= (dir * 6);
  }

  if (look.x < 0.0)
    look.x = 0.0;
  else if (look.x > disp.width)
    look.x = disp.width;

  if (look.y < 0.0)
    look.y = 0.0;
  else if (look.y > disp.height)
    look.y = disp.height;
}

} // namespace BEAST
