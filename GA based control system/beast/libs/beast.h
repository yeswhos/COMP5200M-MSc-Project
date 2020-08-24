/**
 * \file beast.h
 * \author David Gordon
 * 
 * The main include file for the simulation environment/wx -
 * include this file if you want to run simulations using the
 * wxWindows-based interface.
 */

#ifndef _BEAST_H
#define _BEAST_H

#include <vector>
#include <string>

#include "simulation.h"

/**
 * Denotes the beginning of the simulation table. The simulation table is used
 * to tell the GUI which classes (derived from Simulation) are to be made
 * available in the File menu.

 * \see ADD_SIMULATION
 * \see END_SIMULATION_TABLE
 */
#define BEGIN_SIMULATION_TABLE									\
namespace BEAST {												\
void SetupSimulationTable										\
(std::vector<std::string>& names,								\
 std::vector<GetSimulationBase*>& funcs)						\
{

/**
 * Call this once with the textual name and class name of each simulation you
 * wish to make available in the GUI. ADD_SIMULATION must only be called
 * between BEGIN_SIMULATION_TABLE and END_SIMULATION_TABLE.
 * \see BEGIN_SIMULATION_TABLE
 * \see END_SIMULATION_TABLE
 */
#define ADD_SIMULATION(_name, _type)							\
	names.push_back(_name);										\
	funcs.push_back(new GetSimulation<_type>);

/**
 * Call this to denote the end of your simulation table, the list of
 * simulations to be made available in the GUI.
 * \see BEGIN_SIMULATION_TABLE
 * \see END_SIMULATION_TABLE
 */
#define END_SIMULATION_TABLE									\
}																\
}

namespace BEAST {

struct GetSimulationBase
{
	virtual Simulation* Get() = 0;
        virtual ~GetSimulationBase() = 0;

};

template <class _SimType>
struct GetSimulation : public GetSimulationBase
{
	virtual Simulation* Get()
	{
		return new _SimType;
	}
};

} // namespace BEAST

#if defined _WIN32
	#ifdef DLL_EXPORT
		#define BEAST_DLL __declspec(dllexport)
	#else
		#define BEAST_DLL __declspec(dllimport)
	#endif
#elif defined __GNUG__
	#define BEAST_DLL
	#define HINSTANCE void*
#endif

namespace BEAST
{

static std::map<std::string, HINSTANCE> g_plugins;

bool LoadPlugin(const char* plugin,
					   std::vector<std::string>& names,
					   std::vector<GetSimulationBase*>& funcs);
bool LoadPlugin(const char* plugin, std::map<std::string, GetSimulationBase*>& output);
bool UnloadPlugin(const char* plugin);
int UnloadPlugins();

extern "C" BEAST_DLL void SetupSimulationTable(std::vector<std::string>& names, std::vector<GetSimulationBase*>& funcs);

bool ScreenGrab(World& theWorld, std::string filename);

} // namespace BEAST


#endif
