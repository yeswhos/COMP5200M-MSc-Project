
using namespace std;
using namespace BEAST;


class TestSphere : public WorldObject3D
{



public:


  TestSphere()
  {
    This.Radius = 3.0f;
    This.SetColour(ColourPalette[COLOUR_GREEN]);
    This.InitRandom = true;	
    This.SetSolid(true);
  }

};


class TestAnimat3D : public Animat3D
{
  
public:
  TestAnimat3D()
  {

    This.InitRandom = true;
    This.SetMinSpeed(80.0);
    This.SetMaxSpeed(80.0);

  }

  virtual void Control()
  {
    float right = randval<float>(1.0);
    float left = 1.0 - right;
    
	  
    This.Controls["right"] = right;
    This.Controls["left"] = left;
      }
	
};

class Test3DSim : public Simulation
{
	Group<TestAnimat3D> theAnimats;
  Group<TestSphere> theSpheres;

public:
	Test3DSim():
	theAnimats(10),
	theSpheres(80)
	{

		This.SetTimeSteps(-1);

		This.Add("Animats", This.theAnimats);
		This.Add("Spheres", This.theSpheres);
	}
	
	
	
};

