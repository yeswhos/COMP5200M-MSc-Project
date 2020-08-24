#ifndef _BEASTFRAME_H
#define _BEASTFRAME_H

#include "wxbeast.h"

#include "wx/textctrl.h"
#include "wx/progdlg.h"
#include "wx/msgdlg.h"

#include "simulation.h"
#include "utilities.h"

#ifdef new
#undef new
#endif

#include <vector>
#include <algorithm>
#include <string>

namespace BEAST {

enum {
	ID_FILE_STARTSIM0 = wxID_HIGHEST + 1,
	ID_FILE_STARTSIM1,
	ID_FILE_STARTSIM2,
	ID_FILE_STARTSIM3,
	ID_FILE_STARTSIM4,
	ID_FILE_STARTSIM5,
	ID_FILE_STARTSIM6,
	ID_FILE_STARTSIM7,
	ID_FILE_STARTSIM8,
	ID_FILE_STARTSIM9,
	ID_FILE_LOAD,
	ID_FILE_LOAD0,
	ID_FILE_LOAD1,
	ID_FILE_LOAD2,
	ID_FILE_LOAD3,
	ID_FILE_LOAD4,
	ID_FILE_LOAD5,
	ID_FILE_LOAD6,
	ID_FILE_LOAD7,
	ID_FILE_LOAD8,
	ID_FILE_LOAD9,
	ID_FILE_SAVE,
	ID_FILE_SAVE0,
	ID_FILE_SAVE1,
	ID_FILE_SAVE2,
	ID_FILE_SAVE3,
	ID_FILE_SAVE4,
	ID_FILE_SAVE5,
	ID_FILE_SAVE6,
	ID_FILE_SAVE7,
	ID_FILE_SAVE8,
	ID_FILE_SAVE9,

	ID_DISP_ANIMATS = wxID_HIGHEST + 51,
	ID_DISP_OBJECTS,
	ID_DISP_TRAILS,
	ID_DISP_COLLISIONS,
	ID_DISP_SENSORS,
	ID_DISP_MONITOR,

	ID_SIM_PAUSE = wxID_HIGHEST + 61,
	ID_SIM_FAST,
	ID_SIM_RESET,

	ID_WORLD_NEXT = wxID_HIGHEST + 71,
	ID_WORLD_PREV,
	ID_WORLD_3D,
	ID_WORLD_2D,

	ID_HELP_ABOUT = wxID_HIGHEST + 81,

	ID_MAIN_TIMER = wxID_HIGHEST + 101
};

struct GetSimulationBase;
class WorldGLCanvas;

/**
 * The main window for the Beast application
 */
class BeastFrame : public wxFrame
{
public:
    BeastFrame(wxFrame *frame,	const wxString& title, const wxPoint& pos,
			   const wxSize& size, const char* plugin = NULL,
			   long style = wxDEFAULT_FRAME_STYLE | wxWANTS_CHARS);
	~BeastFrame();

	void			CreateMenuBar();
	void			CreateFileMenu();
	void			CreateWorldCanvas();
	void			CreateLogWindow();

	void			StartSimulation(int nSim);
	void			CreateSimMenus();

	void			OnTimer(wxTimerEvent& event);

	void			OnStartSim(wxCommandEvent& event);

	void			OnLoad(wxCommandEvent& event);
	void			OnSave(wxCommandEvent& event);
	void			OnExit(wxCommandEvent& event);

	void			OnTglAnimats(wxCommandEvent& event);
	void			OnTglObjects(wxCommandEvent& event);
	void			OnTglTrails(wxCommandEvent& event);
	void			OnTglCollisions(wxCommandEvent& event);
	void			OnTglSensors(wxCommandEvent& event);
	void			OnTglMonitor(wxCommandEvent& event);

	void			OnTglPaused(wxCommandEvent& event);
	void			OnFast(wxCommandEvent& event) { HighSpeed(); }
	void			OnReset(wxCommandEvent& event);

	void			OnNextAnimat(wxCommandEvent& event);
	void			OnPrevAnimat(wxCommandEvent& event);
	
	//World to 2D or 3D by Joseph Shaw
	void      OnWorld3D(wxCommandEvent& event);
	void      OnWorld2D(wxCommandEvent& event);

	void			OnAbout(wxCommandEvent& event);

	void			OnSize(wxSizeEvent& event);

	void			Pause();
	void			Unpause();
	void			HighSpeed();

private:
	std::vector<std::string> m_SimulationNames;
	std::vector<GetSimulationBase*>	m_SimulationFuncs;

	Simulation*		m_pSimulation;
        WorldGLCanvas*	        m_pWorldCanvas;
	wxTimer*		m_pTimer;
	wxStopWatch*	        m_pStopWatch;
	wxMenuBar*		m_pMenuBar;
	wxStatusBar*	        m_pStatusBar;
	wxFrame*		m_pTextFrame;
	wxTextCtrl*		m_pLogOutput;

	std::ostringstream m_LogStream;

	int				m_nCurrentSimId,
					m_nSpeed;

	bool			m_bStarted,
					m_bPaused;

	DECLARE_EVENT_TABLE()
};

} // namespace BEAST

#endif

