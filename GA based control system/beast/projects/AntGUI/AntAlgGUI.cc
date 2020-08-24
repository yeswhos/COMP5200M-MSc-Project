#include "AntAlgGUI.h"

BEGIN_EVENT_TABLE(AntAlgGUI, wxFrame)
  EVT_MENU(ID_DEMO_CLUSTER, AntAlgGUI::OnDemoCluster)
	EVT_MENU(ID_DEMO_SORT, AntAlgGUI::OnDemoSort)
	EVT_MENU(ID_DEMO_GRAPH, AntAlgGUI::OnDemoGraph)
END_EVENT_TABLE()

AntAlgGUI::AntAlgGUI(Simulation* sim)
       : wxFrame((wxFrame *)NULL, wxID_ANY, _T("Ant Sorting Simulations"), wxPoint(50,50), wxSize(450,340), wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxRESIZE_BORDER)
{
  simulation = sim;
  menuBar = new wxMenuBar;
  menu = new wxMenu;
  menu->Append(ID_DEMO_CLUSTER, wxT("&General Clustering"));
  menu->Append(ID_DEMO_SORT, wxT("&Colour Sorting"));
  menu->Append(ID_DEMO_GRAPH, wxT("&Graph Partitioning"));
  menuBar->Append(menu, wxT("&Demos"));
  SetMenuBar(menuBar);
  
  wxPanel* centre = new wxPanel(this, -1);
  wxBoxSizer* box = new wxBoxSizer(wxVERTICAL);
  sortPanel = new ColourSortPanel(centre);
  box->Add(sortPanel, 1, wxEXPAND | wxALL, 10);
  centre->SetSizer(box);
  //sortPanel->Show(false);
}

void AntAlgGUI::OnDemoCluster(wxCommandEvent& event)
{
  //sortPanel->Show(true);
  //Group<ClusterAnt>* ants = new Group<ClusterAnt>(20);
  //Group<Leaf>* leaves = new Group<Leaf>(100);
  //static_cast<AntAlgGUISimulation*>(simulation)->StartSim(ants, leaves);
  //simulation->Add("Ants", *ants);
  //simulation->Add("Items", *leaves);
  //simulation->ResetGeneration();
}

void AntAlgGUI::OnDemoSort(wxCommandEvent& event)
{
  //Group<SortingAnt>* ants = new Group<SortingAnt>(20);
  //Group<MultiColourLeaf>* leaves = new Group<MultiColourLeaf>(100);
  //static_cast<AntAlgGUISimulation*>(simulation)->StartSim(ants, leaves);
}

void AntAlgGUI::OnDemoGraph(wxCommandEvent& event)
{
  //Group<ClusterAnt>* ants = Group<ClusterAnts>(20);
  //Group<Leaf>* leaves = Group<Leaf>(100);
}



