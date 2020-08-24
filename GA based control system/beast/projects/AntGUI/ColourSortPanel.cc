#include "ColourSortPanel.h"

  
ColourSortPanel::ColourSortPanel(wxPanel* parent)
    : wxPanel(parent, wxID_ANY)
{
  numAnts = new wxTextCtrl(this, -1);
  wxStaticText* st1 = new wxStaticText(this, wxID_ANY, wxT("Num. Ants:"));
  
  numItems = new wxTextCtrl(this, -1);
  wxStaticText* st2 = new wxStaticText(this, wxID_ANY, wxT("Number of Items:"));
  
  
  complete = new wxButton(this, ID_DEMO_COMPLETE, wxT("Run ->"));
  Connect(ID_DEMO_COMPLETE, wxEVT_COMMAND_BUTTON_CLICKED,
    wxCommandEventHandler(ColourSortPanel::OnComplete));
  
  
  wxBoxSizer* fields = new wxBoxSizer(wxVERTICAL);
  
  wxBoxSizer* f1 = new wxBoxSizer(wxHORIZONTAL);
  f1->Add(st1, 0);
  f1->Add(numAnts, 1);
  fields->Add(f1, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
  
  fields->Add(-1, 5);
  
  wxBoxSizer* f2 = new wxBoxSizer(wxHORIZONTAL);
  f2->Add(st2, 0);
  f2->Add(numItems, 1);
  fields->Add(f2, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
  
  fields->Add(-1, 10);
  
  wxBoxSizer* fButton = new wxBoxSizer(wxHORIZONTAL);
  fButton->Add(complete, 1);
  fields->Add(fButton, 0, wxRIGHT, 10);
  
  SetSizer(fields);
  
}

void ColourSortPanel::OnComplete(wxCommandEvent& event)
{
  
}



