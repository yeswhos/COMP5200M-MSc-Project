/**
 * \file glutsimenv.h
 * 
 * The main include file for the simulation environment/GLUT -
 * include this file if you want to run simulations using the simple
 * GLUT-based interface.
 */

#ifndef _SIMENV_H
#define _SIMENV_H

#include <iostream>

#include "simulation.h"

#define START_SIMULATION(_SimClass) \
int main(int argc, char* argv[])\
{\
	_SimClass theSimulation;\
	glut_start_simulation(argc, argv, &theSimulation);\
	return 0;\
}

namespace BEAST { 

void glut_start_simulation(int& args, char* argv[], Simulation* pTheSim);

} //namespace BEAST

#endif
