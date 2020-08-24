#include "beast.h"

namespace BEAST {

#if defined _WIN32 && !DLL_EXPORT

typedef void (__cdecl* FUNC)(std::vector<std::string>& names, std::vector<GetSimulationBase*>& funcs);

bool UnloadPlugin(const char* plugin)
{
	return FreeLibrary(g_plugins[plugin]) != 0;
}

bool LoadPlugin(const char* plugin,
					   std::vector<std::string>& names,
					   std::vector<GetSimulationBase*>& funcs)
{
	HINSTANCE hDLL;               // Handle to DLL
	hDLL = LoadLibrary(plugin);
	FUNC pFunc;
		if (hDLL != NULL) {
		pFunc = (FUNC)GetProcAddress(hDLL, "SetupSimulationTable");
		if (pFunc == NULL) return false;
		pFunc(names, funcs);
		if (g_plugins.find(plugin) != g_plugins.end()) {
			UnloadPlugin(plugin);
		}
		g_plugins[plugin] = hDLL;
	}
		return false;
}

int UnloadPlugins()
{
	int unloads = 0;

		for (std::map<std::string, HINSTANCE>::iterator i = g_plugins.begin();
			 i != g_plugins.end(); ++i) {
				if (FreeLibrary(i->second) != 0) ++unloads;
	}

	return unloads;
}

#elif defined __GNUG__

#include <dlfcn.h>

typedef void (* FUNC)(std::vector<std::string>& names, std::vector<GetSimulationBase*>& funcs);

bool UnloadPlugin(const char* plugin)
{
	return dlclose(g_plugins[plugin]) != 0;
}

bool LoadPlugin(const char* plugin,
		std::vector<std::string>& names,
		std::vector<GetSimulationBase*>& funcs)
{
	void* hSO;               // Handle to SO
	hSO = dlopen(plugin, RTLD_LAZY);
	FUNC pFunc;

	if (hSO != NULL) {
		pFunc = (FUNC)dlsym(hSO, "SetupSimulationTable");
		if (pFunc == NULL) {
			std::cerr << "Couldn't find SetupSimulationTable function in " << plugin << std::endl;
			return false;
		}
		pFunc(names, funcs);
		if (g_plugins.find(plugin) != g_plugins.end()) {
			UnloadPlugin(plugin);
		}
		g_plugins[plugin] = hSO;
	}
	else {
		std::cerr << "Failed to load " << plugin << ", " << dlerror() << std::endl;
		return false;
	}

	return true;
}

int UnloadPlugins()
{
	int unloads = 0;

	for (std::map<std::string, void*>::iterator i = g_plugins.begin();
		 i != g_plugins.end(); ++i) {
		if (dlclose(i->second) != 0) ++unloads;
	}

	return unloads;
}


#endif


bool LoadPlugin(const char* plugin, std::map<std::string, GetSimulationBase*>& output)
{
	std::vector<std::string> names;
	std::vector<GetSimulationBase*> funcs;

	if (LoadPlugin(plugin, names, funcs)) {
		std::vector<std::string>::const_iterator i=names.begin();
		std::vector<GetSimulationBase*>::const_iterator j=funcs.begin();
		for (;i != names.end() && j != funcs.end(); ++i, ++j) {
			output[*i] = *j;
		}
		return true;
	}
	return false;
}

} // namespace BEAST
