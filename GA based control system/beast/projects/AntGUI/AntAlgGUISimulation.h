#ifndef _ANTALGGUISIMULATION_H
#define _ANTALGGUISIMULATION_H

#include "AntAlgGUI.h"

using namespace BEAST;

class AntAlgGUISimulation : public Simulation
{

private:

  AntAlgGUI* gui;
  //bool guiClose;
  //Group<Ant>* ants;
  //Group<Item>* items;

public:

	AntAlgGUISimulation();

	~AntAlgGUISimulation();

	
};

#endif
