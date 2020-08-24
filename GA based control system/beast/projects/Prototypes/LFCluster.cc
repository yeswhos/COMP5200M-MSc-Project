

#include <cstdlib>
#include <vector>

using namespace std;
using namespace BEAST;



// Leaves are small and green
class Leaf : public Item
{

public:

  Leaf()
  {
    This.Radius = 1.5f;
    This.SetColour(ColourPalette[COLOUR_GREEN]);
    This.InitRandom = true;	
  }

};

class ClusterAnt : public Ant
{


private:

  float k1;
  float k2;
  //const static float alpha = 0.5;
  float radius; //radius of detection area
  float area;
  float inverseArea;
  vector<Item*> items;
  
  
  float FractionOfPerceivedItems(Item* l, Vector2D v)
	{
	  items.clear();
	  GetWorld().Get(items);
	  int count = 0;
	  for (int i = 0; i < items.size(); i++)
		{
		  if (items[i] != l)
		  {
			  Vector2D diff = items[i]->GetLocation() - v;
			  if (diff.GetLength() < radius)
			    count++;
			}
		}
		//cout << count << endl;
    float f = inverseArea * count;
    //cout << f << endl;
    if (f > 0)
      return f;
    else
      return 0;
  }
  
  
  float DropProbability(float fraction)
  {
    float val = fraction / (k2 + fraction);
    
    return val * val;
  }
  
  bool RoomToDrop()
  {
    items.clear();
    GetWorld().Get(items);
    for (int i = 0; i < items.size(); i++)
    {
      Vector2D diff = items[i]->GetLocation() - GetLocation();
      if (diff.GetLength() < 2 * items[i]->GetRadius())
        return false;
    }

    return true;
  }
  
  virtual bool DropItem()
  {
    if (RoomToDrop())
    {
      float fraction = FractionOfPerceivedItems(GetItem(), GetLocation());
      float pDrop = DropProbability(fraction);
    
      //cout << pDrop << endl;
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
    float fraction = FractionOfPerceivedItems(l, l->GetLocation());
    float pPickUp = PickUpProbability(fraction);
    
    //cout << pPickUp << endl;
    float R = randval<float>(1.0);
    if (R <= pPickUp)
      return true;
    else
      return false;
  }

public:
  ClusterAnt()
  {
    k1 = 0.1;
    k2 = 0.3;
    radius = 6.0;
    area = 9.0;
    inverseArea = 1.0 / area;
    This.InitRandom = true;
    This.SetMinSpeed(80.0);
    This.SetMaxSpeed(80.0);
    //get items from the world
    items = vector<Item*>();
    
  }
  
  void SetK1(float newk1)
  {
    k1 = newk1;
  }
  
  void SetK2(float newk2)
  {
    k2 = newk2;
  }
  
  void SetRadius(float newRadius)
  {
    radius = newRadius;
  }
  
  void SetArea(float newArea)
  {
    area = newArea;
    inverseArea = 1.0 / area;
  }
  
  virtual void PickUpItemAction()
  {
    GetItem()->SetLocation(-radius - 10, -radius - 10);
    Ant::PickUpItemAction();
  }
  
  /*
  void SetItems(Item** itemPtrs, int num)
  {
    numItems = num;
    delete [] items;
    items = new Item*[numItems];
    
    for (int i = 0; i < numItems; i++)
    {
      items[i] = itemPtrs[i];
    }
  }
  */
	
};

class LFClusterSimulation : public Simulation
{
	Group<ClusterAnt> theAnts;
  Group<Leaf> theLeaves;
  int interval;
  //Group<WorldObject> eval;

public:
	LFClusterSimulation():
	theAnts(10),
	theLeaves(100)
	{
	
	  interval = 10000;
	  /*
	  for (int i = 0; i < 200; i++)
	  {
	    theLeaves.push_back(new Leaf());
	  }
	  
	  Item** items = new Item*[200];
	  
	  for (int i = 0; i < 200; i++)
	  {
	    items[i] = theLeaves[i];
	  }
	
    for (int i = 0; i < 10; i++)
    {
      theAnts[i]->SetItems(items, 200);
    }
    */
    
    //eval.push_back(new AverageDistAnimat(1000));
    //continuous simluation
		This.SetTimeSteps(1000000);
		This.SetGenerations(1);

		This.Add("Ants", This.theAnts);
		This.Add("Leaves", theLeaves);
		//This.Add("EvalAnimat", eval);
		

	}
	
  virtual bool Update()
  {
    int timeSteps = GetTimeStep();
    vector<Item*> items = vector<Item*>();
    
    
    if (timeSteps == 0 || timeSteps % interval == 0)
    {
      GetWorld().Get(items);
      double avgDistance = 0;
      int count = 0;
      
      for (int i = 0; i < items.size(); i++)
      {
        for (int j = i; j < items.size(); j++)
        {
          if (i != j)
          {
            count++;
            Vector2D dist = items[i]->GetLocation() - items[j]->GetLocation();
            //cout << dist << endl;
            avgDistance += dist.GetLength();
            //cout << avgDistance << endl;
          }
        }
      }
      
      //cout << avgDistance << endl;
      cout << avgDistance / static_cast<float>(count) << " ";
    }
    
    return Simulation::Update();
  }
	
	
};

