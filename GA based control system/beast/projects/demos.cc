#include "beast.h"

#include "3DStuff/WorldObject3D.cc"
#include "3DStuff/Animat3D.cc"
#include "Common/Item.h"
#include "Common/Ant.h"
#include "Evaluation/AverageEvaluation.cc"
#include "braitenberg.cc"
#include "shrew.cc"
#include "chase.cc"
#include "mouse.cc"
#include "signal.cc"
#include "pacman.cc"
#include "Prototypes/LFCluster.cc"
#include "Prototypes/LFSort.cc"
#include "Prototypes2/LFSort.cc"
#include "Apps/KLSGraphPart.cc"
#include "InitialGUI/LFSortGUI.cc"
#include "3DStuff/Test3D.cc"
#include "Situatedness/LFSortSituated.cc"
#include "AntGUI/AntAlgGUISimulation.cc"
#include "AntGUI/AntAlgGUI.cpp"
//#include "AntGUI/ColourSortPanel.cc"

BEGIN_SIMULATION_TABLE
	// ADD_SIMULATION("Braitenberg", BraitenbergSimulation)
	ADD_SIMULATION("Shrews", ShrewSimulation)
// 	ADD_SIMULATION("Mice", MouseSimulation)
// 	ADD_SIMULATION("Chase", ChaseSimulation)
// 	ADD_SIMULATION("Signal", SignalSimulation)
// 	ADD_SIMULATION("Pacman", PacmanSimulation)
// 	ADD_SIMULATION("LFCluster - prototype", LFClusterSimulation);
// 	ADD_SIMULATION("LFSort - prototype", LFSortSimulation);
// 	ADD_SIMULATION("KLSGraphPartition - prototype", GraphSimulation);
// 	//ADD_SIMULATION("Initial GUI with LFSort", LFSortGUI);
// 	ADD_SIMULATION("Ant Algorithm GUI", AntAlgGUISimulation);
// 	ADD_SIMULATION("A Test of 3Dness", Test3DSim);
//   ADD_SIMULATION("Situated Sort", LFSortSituated);
	
END_SIMULATION_TABLE
