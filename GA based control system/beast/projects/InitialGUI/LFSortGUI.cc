

//undefine This macro as it interferes with wx headers
#ifdef This
#undef This
#endif

#include "wx/wx.h"

//re-define macro
#define This (*this)



using namespace std;
using namespace BEAST;

class Dot2 : public WorldObject
{
public:
	Dot2()
	{
	  SetColour(ColourPalette[COLOUR_YELLOW]);
	  Radius = 5.0;
	  InitRandom = true;
	}
	virtual ~Dot2(){}
	
};


class TestAnimat : public Animat
{
public:
	TestAnimat()
	{
		This.Add("left", ProximitySensor<Dot2>(PI/2, 75.0, -1));
		This.Add("right", ProximitySensor<Dot2>(PI/2, 75.0, 1));

		This.SetInitRandom(true);

		This.SetMinSpeed(0.0);
		This.SetMaxSpeed(95.0);

		This.Radius = 10.0;
	}
	
	virtual void Control()
	{
		This.Controls["left"] = This.Sensors["left"]->GetOutput();
		This.Controls["right"] = This.Sensors["right"]->GetOutput();
	}
};



class LFSortGUI : public Simulation
{

private:

  Group<Dot2> items;
  Group<TestAnimat> animats;
  wxFrame* frame;

public:

  

	LFSortGUI(): items(10), animats(3)
	{

	  frame = new wxFrame((wxFrame *)NULL, -1, _T("Test Window"), wxPoint(50,50), wxSize(450,340), wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxRESIZE_BORDER );
		
		This.Add("Items", items);
		This.Add("Animats", animats);    This.SetTimeSteps(-1);
    frame->Show(TRUE);
	}
	
	
	/**
	 * Called on destruction of Simulation.
	 *
	 * This destroys the frame 
	 */
	~LFSortGUI()
	{
	  frame->Destroy();
	}
	

	
	
};
