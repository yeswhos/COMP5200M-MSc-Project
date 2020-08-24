
#include "simulation.h"

#if defined _GNUG_
	#define _XOPEN_SOURCE 500
	#include <unistd.h>
#elif defined _WIN32
	#include <windows.h>
#endif

#include <GL/glut.h>

namespace BEAST {

void glut_config(int&, char**);
void glut_setup_callbacks();
void glut_display();
void glut_process_key(unsigned char Key, int MouseX, int MouseY);
void glut_process_special_key(int Key, int MouseX, int MouseY);
void glut_process_mouse_click(int btn, int act, int x, int y);
void glut_process_mouse_movement(int x, int y);
void glut_update();
void glut_reshape_window(GLsizei w, GLsizei h);

Simulation* glutSimObject;
long displaySpeed = 5000;

void glut_start_simulation(int& args, char* argv[], Simulation* pTheSim)
{
	glutSimObject = pTheSim;
	glut_config(args, argv);
	glutMainLoop();
}

void glut_config(int& args, char* argv[])
{
  //Mdk: 2007, changed theWorld for access function (wonder how previous functions could have compiled ?!)
        World& theWorld = glutSimObject->GetWorld();

	glutInit(&args, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(static_cast<int>(theWorld.GetWidth()), static_cast<int>(theWorld.GetHeight()));

	glutInitWindowPosition(100, 100);
	glutCreateWindow("GA Simulation Environment");
	glutIgnoreKeyRepeat(1); // 1 == on

	glut_setup_callbacks();

	theWorld.InitGL();
}

void glut_setup_callbacks()
{
	glutDisplayFunc(glut_display);
	glutKeyboardFunc(glut_process_key);
	glutSpecialFunc(glut_process_special_key);
	glutMouseFunc(glut_process_mouse_click);
	glutMotionFunc(glut_process_mouse_movement);
	glutReshapeFunc(glut_reshape_window);
	glutIdleFunc(glut_update);
	return;
}

void glut_display()
{
  //MdK: 02-01-2007, changed acces function for the world
        static World& theWorld = glutSimObject->GetWorld();

	theWorld.Display();

	glutSwapBuffers();
}

void glut_process_key(unsigned char Key, int MouseX, int MouseY)
{
        static World& theWorld = glutSimObject->GetWorld();

	switch (Key)
	{
		case '+':
			displaySpeed -= 100;
			break;
		case '-':
			displaySpeed += 100;
			break;
		case ' ':
			break;
		case 'q': case 'Q':	case '\033':
			exit(0);
			break;
	}

	glutPostRedisplay();
}

void glut_process_special_key(int Key, int MouseX, int MouseY)
{
//	static Simulation& theSim = *glutSimObject;

	switch (Key) {
	}

	glutPostRedisplay();
}

void glut_process_mouse_click(int btn, int act, int x, int y)
{
	static Simulation& theSim = *glutSimObject;
	//MdK: 02-01-2007, changed acces function for the world

	static World& theWorld = theSim.GetWorld();

	if (btn == GLUT_LEFT_BUTTON) {
		if (act == GLUT_DOWN) theWorld.OnMouseLDown(x, y);
		else if (act == GLUT_UP) theWorld.OnMouseLUp(x, y);
	}
	else if (btn == GLUT_RIGHT_BUTTON) {
		if (act == GLUT_DOWN) theWorld.OnMouseRDown(x, y);
		else if (act == GLUT_UP) theWorld.OnMouseRUp(x, y);
	}
}

void glut_process_mouse_movement(int x, int y)
{
        static World& theWorld = glutSimObject->GetWorld();

	theWorld.OnMouseMove(x, y);

}

void glut_update()
{
	static Simulation& theSim = *glutSimObject;
	static int updateStart, updateFinish;
	updateStart = glutGet(GLUT_ELAPSED_TIME);
	theSim.Update();
	updateFinish = glutGet(GLUT_ELAPSED_TIME);

#if defined _GNUG_
	usleep(displaySpeed - (updateFinish - updateStart));
#elif defined _WIN32
	long wait = static_cast<long>(displaySpeed / 100 - (updateFinish - updateStart));
	if (wait > 0) Sleep(static_cast<DWORD>(wait));
#endif
	glutPostRedisplay();
}


void glut_reshape_window(GLsizei w, GLsizei h)
{
	static Simulation& theSim = *glutSimObject;
	theSim.GetWorld().SetWindow(w, h);
	
	glViewport( 0, 0, w, h );
	
	glutPostRedisplay();
}

} // namespace BEAST
