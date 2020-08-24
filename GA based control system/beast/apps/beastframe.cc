#include "beast.h"
#include "beastframe.h"
#include "worldglcanvas.h"

#include <iostream>

using namespace std;


namespace BEAST {

BEGIN_EVENT_TABLE(BeastFrame, wxFrame)
	EVT_TIMER(ID_MAIN_TIMER,	BeastFrame::OnTimer)
	EVT_MENU(wxID_EXIT,			BeastFrame::OnExit)

	EVT_MENU(ID_FILE_STARTSIM0,	BeastFrame::OnStartSim)
	EVT_MENU(ID_FILE_STARTSIM1,	BeastFrame::OnStartSim)
	EVT_MENU(ID_FILE_STARTSIM2,	BeastFrame::OnStartSim)
	EVT_MENU(ID_FILE_STARTSIM3,	BeastFrame::OnStartSim)
	EVT_MENU(ID_FILE_STARTSIM4,	BeastFrame::OnStartSim)
	EVT_MENU(ID_FILE_STARTSIM5,	BeastFrame::OnStartSim)
	EVT_MENU(ID_FILE_STARTSIM6,	BeastFrame::OnStartSim)
	EVT_MENU(ID_FILE_STARTSIM7,	BeastFrame::OnStartSim)
	EVT_MENU(ID_FILE_STARTSIM8,	BeastFrame::OnStartSim)
	EVT_MENU(ID_FILE_STARTSIM9,	BeastFrame::OnStartSim)

	EVT_MENU(ID_FILE_LOAD0,		BeastFrame::OnLoad)
	EVT_MENU(ID_FILE_LOAD1,		BeastFrame::OnLoad)
	EVT_MENU(ID_FILE_LOAD2,		BeastFrame::OnLoad)
	EVT_MENU(ID_FILE_LOAD3,		BeastFrame::OnLoad)
	EVT_MENU(ID_FILE_LOAD4,		BeastFrame::OnLoad)
	EVT_MENU(ID_FILE_LOAD5,		BeastFrame::OnLoad)
	EVT_MENU(ID_FILE_LOAD6,		BeastFrame::OnLoad)
	EVT_MENU(ID_FILE_LOAD7,		BeastFrame::OnLoad)
	EVT_MENU(ID_FILE_LOAD8,		BeastFrame::OnLoad)
	EVT_MENU(ID_FILE_LOAD9,		BeastFrame::OnLoad)

	EVT_MENU(ID_FILE_SAVE0,		BeastFrame::OnSave)
	EVT_MENU(ID_FILE_SAVE1,		BeastFrame::OnSave)
	EVT_MENU(ID_FILE_SAVE2,		BeastFrame::OnSave)
	EVT_MENU(ID_FILE_SAVE3,		BeastFrame::OnSave)
	EVT_MENU(ID_FILE_SAVE4,		BeastFrame::OnSave)
	EVT_MENU(ID_FILE_SAVE5,		BeastFrame::OnSave)
	EVT_MENU(ID_FILE_SAVE6,		BeastFrame::OnSave)
	EVT_MENU(ID_FILE_SAVE7,		BeastFrame::OnSave)
	EVT_MENU(ID_FILE_SAVE8,		BeastFrame::OnSave)
	EVT_MENU(ID_FILE_SAVE9,		BeastFrame::OnSave)

	EVT_MENU(ID_DISP_ANIMATS,	BeastFrame::OnTglAnimats)
	EVT_MENU(ID_DISP_OBJECTS,	BeastFrame::OnTglObjects)
	EVT_MENU(ID_DISP_TRAILS,	BeastFrame::OnTglTrails)
	EVT_MENU(ID_DISP_COLLISIONS,BeastFrame::OnTglCollisions)
	EVT_MENU(ID_DISP_SENSORS,	BeastFrame::OnTglSensors)
	EVT_MENU(ID_DISP_MONITOR,	BeastFrame::OnTglMonitor)

	EVT_MENU(ID_SIM_PAUSE,		BeastFrame::OnTglPaused)
	EVT_MENU(ID_SIM_FAST,		BeastFrame::OnFast)
	EVT_MENU(ID_SIM_RESET,		BeastFrame::OnReset)

	EVT_MENU(ID_WORLD_NEXT,		BeastFrame::OnNextAnimat)
	EVT_MENU(ID_WORLD_PREV,		BeastFrame::OnPrevAnimat)
	EVT_MENU(ID_WORLD_3D,     BeastFrame::OnWorld3D)
	EVT_MENU(ID_WORLD_2D,     BeastFrame::OnWorld2D)

	EVT_MENU(ID_HELP_ABOUT,		BeastFrame::OnAbout)

	EVT_SIZE(					BeastFrame::OnSize)
END_EVENT_TABLE()

BeastFrame::BeastFrame(wxFrame *frame, const wxString& title,
					   const wxPoint& pos, const wxSize& size,
					   const char* plugin, long style):
wxFrame(frame, -1, title, pos, size, style),
m_pSimulation(NULL), m_pWorldCanvas(NULL), m_pTimer(NULL),
m_nCurrentSimId(0), m_nSpeed(33), m_bPaused(false)
{
#if defined(__WXMSW__)
	SetIcon(wxIcon("beast"));
#endif

	wxAcceleratorEntry entries[3];
	entries[0].Set(wxACCEL_NORMAL, WXK_SPACE, ID_SIM_FAST);
	entries[1].Set(wxACCEL_NORMAL, WXK_TAB,   ID_WORLD_NEXT);
	entries[2].Set(wxACCEL_SHIFT,  WXK_TAB,	  ID_WORLD_PREV);
	wxAcceleratorTable accel(3, entries);
	SetAcceleratorTable(accel);

	if (plugin != NULL) {
		LoadPlugin(plugin, m_SimulationNames, m_SimulationFuncs);
	}

	CreateMenuBar();
	m_pStatusBar = CreateStatusBar(2);
	SetStatusText(_T("Ready"), 0);
	CreateLogWindow();

	m_pStopWatch = new wxStopWatch();
}

BeastFrame::~BeastFrame()
{
	delete m_pTimer;
	delete m_pStopWatch;
	delete m_pSimulation;
	UnloadPlugins();
}

void BeastFrame::CreateMenuBar()
{
  int i = WXK_LEFT;
	// Make a menubar
	m_pMenuBar = new wxMenuBar;

	wxMenu* winMenu = new wxMenu;

	using namespace std;

	m_pMenuBar->Append(winMenu,			_T("&File"));

	winMenu = new wxMenu;
	winMenu->Append(ID_DISP_ANIMATS,	_T("Animats"),	_T("Toggle display of animats"),		true);
	winMenu->Append(ID_DISP_OBJECTS,	_T("WorldObjects"),	_T("Toggle display of worldobjects"),	true);
	winMenu->Append(ID_DISP_TRAILS,		_T("Trails"),		_T("Toggle display of animat trails"),	true);
	winMenu->Append(ID_DISP_COLLISIONS,     _T("Collisions"),	_T("Toggle display of collisions"),		true);
	winMenu->Append(ID_DISP_SENSORS,	_T("Sensors"),	_T("Toggle display of sensor ranges"),	true);
	winMenu->Append(ID_DISP_MONITOR,	_T("Monitor"),	_T("Toggle display of monitor output"), true);
	m_pMenuBar->Append(winMenu,			_T("&View"));

	winMenu = new wxMenu;
	winMenu->Append(ID_SIM_PAUSE,		_T("&Pause"),		_T("Pause the simulation"));
	winMenu->Append(ID_SIM_FAST,		_T("High speed"),	_T("Turns off the display and runs at top speed"));
	winMenu->Append(ID_SIM_RESET,		_T("&Reset"),		_T("Clear all simulation data and restart"));
	m_pMenuBar->Append(winMenu,		_T("&Simulation"));

	winMenu = new wxMenu;
	winMenu->Append(ID_WORLD_NEXT,		_T("&Next animat"),	_T("Selects the next animat"));
	winMenu->Append(ID_WORLD_PREV,		_T("&Previous animat"),	_T("Selects the previous animat"));
	winMenu->Append(ID_WORLD_2D,      _T("&2D Simulation"), _T("Changes World to 2D"));
	winMenu->Append(ID_WORLD_3D,      _T("&3D Simulation"), _T("Changes World to 3D"));
	m_pMenuBar->Append(winMenu,			_T("&World"));

	winMenu = new wxMenu;
	winMenu->Append(ID_HELP_ABOUT,		_T("About"));
	m_pMenuBar->Append(winMenu,			_T("&Help"));

	m_pMenuBar->Check(ID_DISP_ANIMATS,	true);
	m_pMenuBar->Check(ID_DISP_OBJECTS,	true);
	m_pMenuBar->Check(ID_DISP_TRAILS,	true);
	m_pMenuBar->Check(ID_DISP_COLLISIONS, true);
	m_pMenuBar->Check(ID_DISP_SENSORS,	true);
	m_pMenuBar->Check(ID_DISP_MONITOR,	true);

	CreateSimMenus();

	SetMenuBar(m_pMenuBar);
}

void BeastFrame::CreateWorldCanvas()
{
	int width, height;
	GetClientSize(&width, &height);
	m_pWorldCanvas = new WorldGLCanvas(m_pSimulation->GetWorld(), this);
	wxSizeEvent e;
	OnSize(e);
}

void BeastFrame::CreateLogWindow()
{
  m_pTextFrame = new wxFrame(this, -1, _T("Log Window"), wxPoint(GetPosition().x,GetSize().GetHeight() + 60),
							   wxSize(GetSize().GetWidth(), 120),
							   wxDEFAULT_FRAME_STYLE
#if defined(__WXMSW__)
							   | wxFRAME_TOOL_WINDOW
#endif
							  );
	int width, height;
	m_pTextFrame->GetClientSize(&width, &height);
	m_pLogOutput = new wxTextCtrl(m_pTextFrame, -1, _T(""), wxPoint(0,0), wxSize(width, height),
								  wxTE_READONLY | wxTE_MULTILINE);

	m_pTextFrame->Show(true);
}

void BeastFrame::StartSimulation(int nSim)
{
	if (m_pWorldCanvas != NULL) m_pWorldCanvas->Destroy();
	if (m_pSimulation != NULL) delete m_pSimulation;
	if (m_pTimer != NULL) delete m_pTimer;

	m_nCurrentSimId = nSim;
	m_pSimulation = m_SimulationFuncs[m_nCurrentSimId]->Get();

	CreateSimMenus();

	m_pTimer = new wxTimer(this, ID_MAIN_TIMER);
	m_pTimer->Start(m_nSpeed, true);

	CreateWorldCanvas();

	m_pSimulation->SetLogStream(m_LogStream);

	m_pSimulation->Init();
}

void BeastFrame::CreateSimMenus()
{
	wxMenu* pMenu = new wxMenu;
	wxMenu* pSubMenuLoad = new wxMenu;
	wxMenu* pSubMenuSave = new wxMenu;
	const Simulation& simulation = *m_pSimulation;

	vector<string>::const_iterator i = m_SimulationNames.begin();
	vector<GetSimulationBase*>::const_iterator j = m_SimulationFuncs.begin();
	for (int n=0; i != m_SimulationNames.end() && n < 10; ++i, ++j, ++n) {
	  pMenu->Append(ID_FILE_STARTSIM0 + n, wxString("Start simulation: ",*wxConvCurrent) + wxString(i->c_str(),*wxConvCurrent));
	}

	if (m_pSimulation != NULL) {
		wxString strName;

		map<string, SimObject*>::const_iterator i = simulation.GetContents().begin();
		for (int n = 0; i != simulation.GetContents().end() && n < 10; ++i, ++n) {
		  strName = wxString(i->first.c_str(),*wxConvCurrent);
			pSubMenuLoad->Append(ID_FILE_LOAD0 + n, strName, wxString("Load saved ",*wxConvCurrent) + strName);
			pSubMenuSave->Append(ID_FILE_SAVE0 + n, strName, wxString("Save current ",*wxConvCurrent) + strName);
		}

		pMenu->Append(ID_FILE_LOAD,		_T("&Load"),		pSubMenuLoad,	_T("Load saved..."));
		pMenu->Append(ID_FILE_SAVE,		_T("&Save"),		pSubMenuSave,	_T("Save current..."));
	}
	else {
	        pMenu->Append(ID_FILE_LOAD,		_T("&Load"),		_T("Load saved simulation data"));
	}

	pMenu->Append(wxID_EXIT,		_T("E&xit"));


	delete m_pMenuBar->Remove(0);
	m_pMenuBar->Insert(0, pMenu, _T("&File"));
}

void BeastFrame::OnTimer(wxTimerEvent& event)
{
	m_pStopWatch->Start();
	m_pTimer->Stop(); // Stop the timer for the moment

	// Perform the update loop and update display if the simulation
	// hasn't finished
	if (m_pSimulation->Update()) {
	  SetStatusText(wxString(m_pSimulation->ToString().c_str(),*wxConvCurrent), 0);

	        wxString strLogOutput = wxString(m_LogStream.str().c_str(),*wxConvCurrent);
		if (!strLogOutput.empty()) {
			m_pLogOutput->AppendText(strLogOutput);
			m_LogStream.str("");
		}

		m_pWorldCanvas->Refresh();

		m_pStopWatch->Pause();
		// Work out the interval til the next timer
		int nStartTime = m_nSpeed - m_pStopWatch->Time();
		// Set a one-shot timer if the simulation is going too slowly
		m_pTimer->Start(nStartTime > m_pStopWatch->Time() ? nStartTime : m_pStopWatch->Time(), true);
	}

	m_pStopWatch->Pause();
}

void BeastFrame::OnStartSim(wxCommandEvent& event)
{
	StartSimulation(event.GetId() - ID_FILE_STARTSIM0);
}

void BeastFrame::OnLoad(wxCommandEvent& event)
{
	const Simulation& simulation = *m_pSimulation;

	bool bWasPaused = m_bPaused;

	if (!bWasPaused) Pause();

	wxString strName, strFileName;

	map<string, SimObject*>::const_iterator i = simulation.GetContents().begin();
	for (int n = 0; i != simulation.GetContents().end() && n < 10; ++i, ++n) {
		if (event.GetId() - ID_FILE_LOAD0 == n) {
		  strName = wxString(i->first.c_str(),*wxConvCurrent) + wxString(".pop",*wxConvCurrent);
		  strFileName = wxFileSelector(_T("Load Population"), _T(""), strName,
						     _T("*.pop"), _T("Population files (*.pop) | *.pop"),
										 wxFC_OPEN | wxFD_FILE_MUST_EXIST);

			if (!strFileName.empty()) {
				try {
					i->second->Load(strFileName.mb_str());
					m_pSimulation->ResetGeneration();
				}
				catch (SerialException& s) {
				  wxMessageDialog(this, wxString(s.ToString().c_str(),*wxConvCurrent),
						  _T("Error"), wxOK | wxCENTRE).ShowModal();
				}
			}

			break;
		}
	}

	if (!bWasPaused) Unpause();
}

void BeastFrame::OnSave(wxCommandEvent& event)
{
	const Simulation& simulation = *m_pSimulation;

	bool bWasPaused = m_bPaused;

	if (!bWasPaused) Pause();

	wxString strName, strFileName;

	map<string, SimObject*>::const_iterator i = simulation.GetContents().begin();
	for (int n = 0; i != simulation.GetContents().end() && n < 10; ++i, ++n) {
		if (event.GetId() - ID_FILE_SAVE0 == n) {
		        strName = wxString(i->first.c_str(),*wxConvCurrent) + wxString(".pop",*wxConvCurrent);
			strFileName = wxFileSelector(_T("Save Population As"), _T(""), strName,
						     _T("*.pop"), _T("Population files (*.pop) | *.pop"),
										 wxFC_SAVE | wxFD_OVERWRITE_PROMPT);

			if (!strFileName.empty()) {
				try {
				    i->second->Save(strFileName.mb_str());
				}
				catch (SerialException& s) {
				  wxMessageDialog(this, wxString(s.ToString().c_str(),*wxConvCurrent),
						  _T("Error"), wxOK | wxCENTRE).ShowModal();
				}
			}

			break;
		}
	}

	if (!bWasPaused) Unpause();
}

void BeastFrame::OnExit(wxCommandEvent& event)
{
	if (m_pTimer != NULL) m_pTimer->Stop();
	Destroy();
}

void BeastFrame::OnTglAnimats(wxCommandEvent& event)
{
	m_pWorldCanvas->Toggle(DISPLAY_ANIMATS);
	m_pMenuBar->Check(ID_DISP_ANIMATS, event.IsChecked());
}

void BeastFrame::OnTglObjects(wxCommandEvent& event)
{
	m_pWorldCanvas->Toggle(DISPLAY_WORLDOBJECTS);
	m_pMenuBar->Check(ID_DISP_OBJECTS, event.IsChecked());
}

void BeastFrame::OnTglTrails(wxCommandEvent& event)
{
	m_pWorldCanvas->Toggle(DISPLAY_TRAILS);
	m_pMenuBar->Check(ID_DISP_TRAILS, event.IsChecked());
}

void BeastFrame::OnTglCollisions(wxCommandEvent& event)
{
	m_pWorldCanvas->Toggle(DISPLAY_COLLISIONS);
	m_pMenuBar->Check(ID_DISP_COLLISIONS, event.IsChecked());
}

void BeastFrame::OnTglSensors(wxCommandEvent& event)
{
	m_pWorldCanvas->Toggle(DISPLAY_SENSORS);
	m_pMenuBar->Check(ID_DISP_SENSORS, event.IsChecked());
}

void BeastFrame::OnTglMonitor(wxCommandEvent& event)
{
	m_pWorldCanvas->Toggle(DISPLAY_MONITOR);
	m_pMenuBar->Check(ID_DISP_MONITOR, event.IsChecked());
}

void BeastFrame::OnTglPaused(wxCommandEvent& event)
{
	if (m_bPaused) {
		Unpause();
	}
	else {
		Pause();
	}
}

void BeastFrame::OnReset(wxCommandEvent& event)
{
	StartSimulation(m_nCurrentSimId);
}

void BeastFrame::OnNextAnimat(wxCommandEvent& event)
{
	if (m_pSimulation == NULL) return;

	m_pSimulation->GetWorld().OnSelectNext();
}

void BeastFrame::OnPrevAnimat(wxCommandEvent& event)
{
	if (m_pSimulation == NULL) return;

	m_pSimulation->GetWorld().OnSelectPrevious();
}


//Functions created for 2D and 3D worlds by Joseph Shaw

void BeastFrame::OnWorld3D(wxCommandEvent& event)
{
  if (m_pSimulation == NULL) return;
  
  m_pSimulation->GetWorld().World3D();
}

void BeastFrame::OnWorld2D(wxCommandEvent& event)
{
  if (m_pSimulation == NULL) return;
  
  m_pSimulation->GetWorld().World2D();
}


void BeastFrame::OnAbout(wxCommandEvent& event)
{
  wxMessageBox(_T("BEAST"),
	       _T("Bioinspired Evolutionary Agent Simulation Toolkit\nVersion 0.00001"),
				 wxICON_INFORMATION);
}

void BeastFrame::OnSize(wxSizeEvent& event)
{
	wxFrame::OnSize(event);
	Refresh();
}

/////////////////////////////// NON-EVENT METHODS ////////////////////////////

void BeastFrame::Pause()
{
	if (m_pSimulation == NULL) return;

	if (m_bPaused) return;
	else {
		m_pTimer->Stop();
		m_pStopWatch->Pause();
		m_pMenuBar->SetLabel(ID_SIM_PAUSE, _T("Un&pause"));
		m_pMenuBar->SetHelpString(ID_SIM_PAUSE, _T("Unpause the simulation"));
		m_bPaused = true;
	}
}

void BeastFrame::Unpause()
{
	if (m_pSimulation == NULL) return;

	if (!m_bPaused) return;
	else {
		m_pTimer->Start(m_nSpeed, true);
		m_pMenuBar->SetLabel(ID_SIM_PAUSE, _T("&Pause"));
		m_pMenuBar->SetHelpString(ID_SIM_PAUSE, _T("Pause the simulation"));
		m_bPaused = false;
	}
}

void BeastFrame::HighSpeed()
{
	if (m_pSimulation == NULL) return;

	m_pTimer->Stop();

	wxProgressDialog dlgFast(_T("High Speed Mode"),
				 _T(""),
							 m_pSimulation->GetTotalTimeSteps(),
							 this, wxPD_CAN_ABORT | wxPD_APP_MODAL |
							 wxPD_ELAPSED_TIME);

	dlgFast.SetFocus();

	while(dlgFast.Update(m_pSimulation->GetTimeStep(), wxString(m_pSimulation->ToString().c_str(),*wxConvCurrent))) {
		for (int i=0; i<10; ++i) m_pSimulation->Update();

		// Outputs the current log stream contents to the log window and
		*m_pLogOutput << wxString(m_LogStream.str().c_str(),*wxConvCurrent);
		// clears the log stream
		m_LogStream.str("");
	}

	m_pTimer->Start(m_nSpeed, true);
}

} // namespace BEAST

