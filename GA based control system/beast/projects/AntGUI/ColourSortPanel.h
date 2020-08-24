#ifndef _COLOURSORTPANEL_H
#define _COLOURSORTPANEL_H

class ColourSortPanel : public wxPanel
{
  public:
  
    ColourSortPanel(wxPanel* parent);

    void OnComplete(wxCommandEvent& event);

  private:
    
    wxButton* complete;
    wxTextCtrl* numAnts;
    wxTextCtrl* numItems;


};

const int ID_DEMO_COMPLETE = wxID_HIGHEST + 1;

#endif

