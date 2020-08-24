// Vim users: for increased viewing pleasure :set ts=4

#include "animat.h"
#include "sensor.h"

// For more information on this file, take a look at Tutorial 1 in the
// BEAST documentation.

using namespace BEAST;

class test : public Animat // Shrew is derived from Animat
{
public:
	test()
	{
		This.Add("left", ProximitySensor<test>(PI/5, 200.0, -PI/20));
		This.Add("right", ProximitySensor<test>(PI/5, 200.0, PI/20));

		This.SetInitRandom(true);	// Start in random locations

		This.Radius = 20.0;			// Shrews are a little bigger than usual
	}
	virtual ~test(){}

	virtual void Control()
	{
		This.Controls["left"] = This.Sensors["right"]->GetOutput();
		This.Controls["right"] = This.Sensors["left"]->GetOutput();
	}
};

class testSimulation : public Simulation
{
	Group<test> grpTest;

public:
	testSimulation():
	grpTest(2)
	{
		This.Add("test", This.grpTest);
	}
};

