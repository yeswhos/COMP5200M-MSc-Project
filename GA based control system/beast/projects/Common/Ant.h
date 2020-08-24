#ifndef _ANT_H
#define _ANT_H


using namespace BEAST;
using namespace std;



class Ant : public Animat3D
{


private:
      
  int mode;
  
  Item* item;
  
  int maxTimeSteps;
  int timeSteps;


  virtual bool DropItem()
  {
    return false;
  }

  virtual bool PickUpItem(Item* l)
  {
    return false;
  }

  

public:
  
  enum {NO_ITEM,
        ITEM};

  Ant()
  {
    item = NULL;
    mode = NO_ITEM;
    maxTimeSteps = 1000000000;
    timeSteps = 0;
  }
  

  Item* GetItem() {return item;}
  int GetMode() {return mode; }


  virtual void DropItemAction()
  {
    item->SetLocation(This.GetLocation());
    //cout << "Dropped item" << endl;
    if (timeSteps > maxTimeSteps)
    {
      
      SetDead(true);
      This.Controls["right"] = 0.0;
      This.Controls["left"] = 0.0;
    }
    
    
  }

  virtual void PickUpItemAction()
  {
    //cout << "Picked up Item" << endl;
  }

  virtual void Control()
  {
    timeSteps++;
    float right = randval<float>(1.0);
    float left = 1.0 - right;
    
    
    This.Controls["right"] = right;
    This.Controls["left"] = left;
    
    if (mode == ITEM)
    {
    
      if (DropItem())
      {
        mode = NO_ITEM;
        item->Drop();
        //item = NULL;
        DropItemAction();
      }
      
    
    }
  }
  
  void SetTimesteps(int steps)
  {
    maxTimeSteps = steps;
  }
  

  virtual void OnCollision(WorldObject* obj)
  {
  
    if (mode == NO_ITEM)
    {
      Item* l;

      if (IsKindOf(obj,l)) {
        if (l->GetMode() == Item::ON_FLOOR && l != item)
        {
          if (PickUpItem(l))
          {
            item = l;
            item->PickUp();
            mode = ITEM;
            PickUpItemAction();
          }
        }

      }
    }
    Animat::OnCollision(obj);
  }
  
};

#endif
