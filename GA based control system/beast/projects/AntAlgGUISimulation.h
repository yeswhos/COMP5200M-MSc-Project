#ifndef _ANTALGGUISIMULATION_H
#define _ANTALGGUISIMULATION_H

#include "AntAlgGUI.h"

using namespace BEAST;

class AntAlgGUISimulation : public Simulation
{

private:

  AntAlgGUI* gui;
  bool guiClose = false;

public:

	AntAlgGUISimulation();
	
	void StartSim(SimObject& ants, SimObject& items);

	~AntAlgGUISimulation();

	
};

#endif
