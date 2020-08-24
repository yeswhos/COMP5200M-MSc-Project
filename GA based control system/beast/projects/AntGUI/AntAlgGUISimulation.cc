
#include "AntAlgGUISimulation.h"

using namespace std;
  

	AntAlgGUISimulation::AntAlgGUISimulation()
	{
	  
	  //gui = new AntAlgGUI(dynamic_cast<Simulation*>(this));
	  gui = new AntAlgGUI(static_cast<Simulation*>(this));
	  This.SetTimeSteps(-1);
    
    gui->Show(TRUE);
    
	}
	
	
	
	/**
	 * Called on destruction of Simulation.
	 *
	 * This destroys the frame 
	 */
	 
	AntAlgGUISimulation::~AntAlgGUISimulation()
	{
	    gui->Destroy();
	}
