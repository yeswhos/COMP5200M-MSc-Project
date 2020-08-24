

#include <cstdlib>
#include "beast.h"
#include "SituatedSensors.h"

using namespace std;
using namespace BEAST;


class SituatedSortingAnt : public Ant
{


private:

  float k1;
  float k2;
  float alpha;
  float radius; //radius of detection area
  float area;
  float inverseArea;

  
  bool RoomToDrop()
  {
    
    float distance = Vector2D(This.Sensors["XSense"]->GetOutput(), This.Sensors["YSense"]->GetOutput()).GetLength();
    float itemR = GetItem()->GetRadius();
    
    //cout << distance << endl;
    //cout << itemR << endl;
    /*
    if (distance >= itemR)
      cout << "true" << endl;
    else
      cout << "false" << endl;
    */
    
    if (distance >= itemR)
      return true;
    else
      return false;
  }

  virtual void PickUpItemAction()
  {
    GetItem()->SetLocation(-radius - 10, -radius - 10);
    Ant::PickUpItemAction();
  }
  
  float DropProbability(float fraction)
  {
    if (fraction < k2)
      return 2*fraction;
    else
      return fraction;
  }

  float FractionOfPercievedItems(Item* i)
  {
    float red = This.Sensors["RedSense"]->GetOutput();
    float green = This.Sensors["GreenSense"]->GetOutput();
    float blue = This.Sensors["BlueSense"]->GetOutput();
    float count = This.Sensors["Counter"]->GetOutput();
      
    //cout << red << " " << green << " " << blue << endl;
    
    float iRed = i->GetColour(0);
    float iGreen = i->GetColour(1);
    float iBlue = i->GetColour(2);

    //cout << "Detected: " << red << green << blue << endl;
    //cout << "Item:" << iRed << iGreen << iBlue << endl;

    float fraction = inverseArea * (count - (abs(count * (iRed/alpha/3.0) - red) + abs(count * (iBlue/alpha/3.0) - blue) + abs(count * (iGreen/alpha/3.0) - green)));
    
    
    /*
    if (count == 3)
    {
      cout << red << endl;
      if (red == 1)
      {
        cout << "3" << endl;
        cout << red << " " << green << " " << blue << endl;
        cout << iRed << " " << iGreen << " " << iBlue << endl;
        cout << fraction << endl;
      }
    }
    */
    
    //cout << count << endl;
    //cout << fraction << endl;
    
    //cout << count << " " << red << "/" << green << "/" << blue << " " << iRed << "/" << iGreen << "/" << iBlue << " " << fraction << endl;

    if (fraction > 0.0)
      return fraction;
    else
      return 0.0;
  }
  
  virtual bool DropItem()
  {
    if (RoomToDrop())
    {

      float pDrop = DropProbability(FractionOfPercievedItems(GetItem()));
    
      float R = randval<float>(1.0);
      if (R <= pDrop)
        return true;
      else
        return false;
    }
    else
      return false;
  }
  
  float PickUpProbability(float fraction)
  { 
    float val = k1 / (k1 + fraction);
    
    return val * val;
  }
  
  
  virtual bool PickUpItem(Item* l)
  {
    
    float pPickUp = PickUpProbability(FractionOfPercievedItems(l));

    //cout << pPickUp << endl;
    float R = randval<float>(1.0);
    if (R <= pPickUp)
      return true;
    else
      return false;
  }

public:
  SituatedSortingAnt()
  {
    k1 = 0.1;
    k2 = 0.15;
    alpha = 0.5;
    radius = 4.5;
    area = 9.0;
    inverseArea = 1.0 / area;
    This.InitRandom = true;
    This.SetMinSpeed(80.0);
    This.SetMaxSpeed(80.0);

    This.Add("XSense", NearestNonScaleXSensor<Item>());
    This.Add("YSense", NearestNonScaleYSensor<Item>());
    This.Add("RedSense", ColourSensor<Item>(radius, alpha, 0));
    This.Add("GreenSense", ColourSensor<Item>(radius, alpha, 1));
    This.Add("BlueSense", ColourSensor<Item>(radius, alpha, 2));
    This.Add("Counter", CountSensor<Item>(radius));
  }


};


class LFSortSituated : public Simulation
{

private:
  Group<SituatedSortingAnt> theAnts;
  Group<MultiColourLeaf> theLeaves;
  static const int numLeaves = 200;
  static const int numAnts = 10;

public:
  LFSortSituated():
  theLeaves(numLeaves),
  theAnts(numAnts)
  {
    
    This.Add("Ants", This.theAnts);
    This.Add("Leaves", This.theLeaves);

    //turn off the sensor monitor.
    //why? Turn it on and find out.
    GetWorld().Toggle(DISPLAY_MONITOR);
    This.SetTimeSteps(-1);

    cout << "Initialised" << endl;

  }
  
  
  
};


