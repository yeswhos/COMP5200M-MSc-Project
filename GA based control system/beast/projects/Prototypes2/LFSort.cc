
#include <cstdlib>

using namespace std;
using namespace BEAST;

class SortingAnt2 : public Ant
{


protected:

  float k1;
  float k2;
  float alpha;
  float radius; //radius of detection area
  float area;
  float inverseArea;
  vector<Item*> items;
  
private:
  
  
  virtual float FractionOfPerceivedItems(Item* l, Vector2D v)
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
			  if (diff.GetLength() < radius && items[i]->GetMode() == Item::ON_FLOOR)
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
      if (diff.GetLength() <= 2 * items[i]->GetRadius() && items[i]->GetMode() == Item::ON_FLOOR)
        return false;
    }

    return true;
  }
  
  virtual float DropProbability(float fraction)
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
  
  virtual float PickUpProbability(float fraction)
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
  SortingAnt2()
  {
    k1 = 0.1;
    k2 = 0.15;
    alpha = 0.5;
    area = 9;
    radius = 4.5;
    inverseArea = 1.0 / area;
    This.InitRandom = true;
    items = vector<Item*>(0);
    This.SetMinSpeed(80.0);
    This.SetMaxSpeed(80.0);
  }
  
  
  /**
   * Destructor - delete items array.
   * We don't delete the contents of the array as these
   * are the WordObject instances cleaned up by beast.
   *
   */
  ~SortingAnt2()
  {
    
    //delete [] items;
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
  
  virtual void PickUpItemAction()
  {
    GetItem()->SetLocation(-radius - 10, -radius - 10);
    Ant::PickUpItemAction();
  }
	
};


