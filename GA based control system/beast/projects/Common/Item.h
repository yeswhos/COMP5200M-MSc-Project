#ifndef _ITEM_H
#define _ITEM_H

using namespace BEAST;

class Item : public WorldObject3D
{

private:
      
  int mode;

public:

  enum {ON_FLOOR,
        PICKED_UP};
  
  Item()
  {
    
    mode = ON_FLOOR;
  }
  
  virtual void Display()
  {
    WorldObject::Display();
  }
  
  int GetMode()
  {
    return mode;
  }
  
  void PickUp()
  {
    if (mode == ON_FLOOR)
      mode = PICKED_UP;
  }
  
  void Drop()
  {
    if (mode == PICKED_UP)
      mode = ON_FLOOR;
  }
  
  
  virtual float Difference(Item* i)
  {
    return 0.0f;
  }

};


#endif
