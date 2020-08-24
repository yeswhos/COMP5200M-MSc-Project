
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace BEAST;


class MultiColourLeaf : public Item
{

public:


  MultiColourLeaf()
  {
    SetRadius(1.5);
    
    //This.SetColour(1.0, 0.0, 0.0);
    
    float R = randval<float>(1.0);
    
    if (R <= 0.5)
      This.SetColour(1.0, 0.0, 0.0);
    else
      This.SetColour(0.0, 1.0, 0.0);
    
    

    //This.SetColour(randval<float>(1.0), randval<float>(1.0), randval<float>(1.0));
    
    This.InitRandom = true;	
  }
  
  virtual float Difference(Item* l)
  {
    float redDiff = fabsf(GetColour(0) - l->GetColour(0));
    float greenDiff = fabsf(GetColour(1) - l->GetColour(1));
    float blueDiff = fabsf(GetColour(2) - l->GetColour(2));
    
    return (redDiff + greenDiff + blueDiff) / 3.0;
  }
  
  
};


class RandomRedLeaf : public Item
{

public:


  RandomRedLeaf()
  {
    SetRadius(1.5);
    
    //This.SetColour(1.0, 0.0, 0.0);
    
    float R = randval<float>(1.0);
    
    This.SetColour(R, 0.0, 0.0);
    
    //This.SetColour(randval<float>(1.0), randval<float>(1.0), randval<float>(1.0));
    
    This.InitRandom = true;	
  }
  
  virtual float Difference(Item* l)
  {
    float redDiff = fabsf(GetColour(0) - l->GetColour(0));
    
    return redDiff;
  }
  
  
};



class SortingAnt : public Ant
{


private:

  float k1;
  float k2;
  float alpha;
  float radius; //radius of detection area
  float area;
  float inverseArea;
  vector<Item*> items;
  
  float FractionOfPerceivedItems(Item* l, Vector2D v)
	{
	  items.clear();
	  This.GetWorld().Get(items);
	  float itemSum = 0.0;
	  int count = 0;
	  for (int i = 0; i < items.size(); i++)
		{
		  if (items[i] != l)
		  {
			  Vector2D diff = items[i]->GetLocation() - v;
			  if (diff.GetLength() < radius)
			  {
          count++;
			    itemSum += 1.0 - (l->Difference(items[i]) / alpha);
			  }
			}
		}
    float f = inverseArea * itemSum;

    /*
    if (count == 3)
    {
      cout << "3" << endl;
      cout << f << endl;
    }
    */
    
    
    //cout << f << endl;
    if (f > 0.0)
      return f;
    else
      return 0.0;
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
  
  float DropProbability(float fraction)
  {
    //float val = fraction / (k2 + fraction);
    if (fraction < k2)
      return 2*fraction;
    else
      return fraction;
    
    //return val * val;
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
  SortingAnt()
  {
    k1 = 0.1;
    k2 = 0.15;
    alpha = 0.5;
    area = 9;
    radius = 5.0;
    inverseArea = 1.0 / area;
    This.InitRandom = true;
    items = vector<Item*>(0);
    This.SetMinSpeed(80.0);
    This.SetMaxSpeed(80.0);
  }
  
  void SetK1(float newk1)
  {
    k1 = newk1;
  }
  
  void SetK2(float newk2)
  {
    k2 = newk2;
  }
  
  void SetAlpha(float newAlpha)
  {
    alpha = newAlpha;
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
	
};

struct dist{
  double eucDiff;
  float diff;
};


class LFSortSimulation : public Simulation
{

private:
	Group<SortingAnt> theAnts;
  Group<RandomRedLeaf> theLeaves;
  int interval;

public:

	LFSortSimulation():
	theAnts(10),
	theLeaves(100)
	{
	
	  interval = 10000;
	  /*
	  for (int j = 0; j < 400; j++)
	  {
	    theLeaves.push_back(new MultiColourLeaf());
	  }
	  
	  Item** items = new Item*[400];
	  
	  for (int i = 0; i < 400; i++)
	  {
	    items[i] = theLeaves[i];
	  }
	  
	  */
	  for (int i = 0; i < 10; i++)
	  {
	    theAnts[i]->SetTimesteps(1000000);
	  }
	  
	  
	  This.Add("Ants", This.theAnts);
		This.Add("Leaves", This.theLeaves);
		This.SetTimeSteps(1200000);
		This.SetGenerations(1);
		
		
		//theLeaves[0]->SetLocation(1.0, 2.0);
		//cout << temp[0]->GetLocation() << endl;
    
	  
		This.SetTimeSteps(-1);
		
		//delete [] items;

		
		//This.GetWorld().SetColour(1.0, 1.0, 1.0);
	  
		
		cout << "Initialised" << endl;

	}
	
	
	virtual bool Update()
  {
    bool update = Simulation::Update();
    int timeSteps = GetTimeStep();
    
    
    if (timeSteps == 1002000)
    {
      vector<Item*> items = vector<Item*>();
      vector<dist> graph = vector<dist>();
      GetWorld().Get(items);
      //double avgDistance = 0;
      int count = 0;
      
      for (int i = 0; i < items.size(); i++)
      {
        for (int j = i; j < items.size(); j++)
        {
          if (i != j)
          {
            count++;
            Vector2D dist = items[i]->GetLocation() - items[j]->GetLocation();
            int len = dist.GetLength();
            struct dist a;
            a.eucDiff = len;
            a.diff = items[i]->Difference(items[j]);
            graph.push_back(a);
            //cout << avgDistance << endl;
          }
        }
      }
      
      ofstream file;
      file.open("/home/joe/Desktop/diffData1.txt");
      
      
      for (int i = 0; i < graph.size(); i++)
      {
        file << graph[i].eucDiff << " ";
      }
      
      file << endl;

      for (int i = 0; i < graph.size(); i++)
      {
        file << graph[i].diff << " ";
      }
      file << endl;
      
      file.close();
    }
    
    return update;
  }
	
};




