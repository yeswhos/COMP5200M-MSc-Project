

#include <cstdlib>
#include <vector>
#include <exception>
#include <stdexcept> // out_of_range exception

using namespace std;
using namespace BEAST;


// Graph node
class Node : public Item
{
private:

  vector<Node*> edges;  //edges that point to other nodes
  int numEdges; //number of edges

public:

  Node()
  {
    numEdges = 0;
    This.Radius = 1.5f;
    
    This.SetColour(ColourPalette[COLOUR_GREEN]);
    This.InitRandom = true;
  }
  
  bool HasEdge(Node* n)
  {
    for (int i = 0; i < numEdges; i++)
    {
      if (edges[i] == n)
        return true;
    }
    
    return false;
  }
  
  void AddEdge(Node* n)
  {
    
    if (!HasEdge(n))
    {
      edges.push_back(n);
      numEdges++;
    }
  }
  
  int GetDegree()
  {
    return numEdges;
  }
  
  virtual void Display()
  {
    glLineWidth(1.0);
    Vector2D loc = GetLocation();
    
    const float* colour = GetColour();
    
    glColor3f(colour[0], colour[1], colour[2]);
    
    for (int i = 0; i < numEdges; i++)
    {
      try
      {
        Vector2D loc2 = edges.at(i)->GetLocation();

        glBegin(GL_LINES);
          glVertex2d(loc.GetX(), loc.GetY());
		      glVertex2d(loc2.GetX(), loc2.GetY());
	      glEnd(); // edge line drawn
	    }
	    catch (std::out_of_range& e)
	    {
	      break;
	    }
    }
    
    Item::Display();
  }
  
  virtual float Difference(Node* n)
  {
    int degreeOfN = n->GetDegree();
    
    if (degreeOfN > 0 || numEdges > 0)
    {
    
      int intersection = 0;
    
      for (int i = 0; i < numEdges; i++)
      {
        if (n->HasEdge(edges[i]))
          intersection++;
          //symDifference++;
      }
    
      int total = degreeOfN + numEdges;
      int unionSet = total - (intersection * 2);
    
      //cout << float(unionSet) / float(total) << endl;
      return float(unionSet) / float(total);
      
    }
    
    return 0.0;
  }

};


class GraphAnt : public Ant
{


private:

  float k1;
  float k2;
  float alpha;
  float radius;
  float area;
  float inverseArea;
  
  
  Node** theNodes;
  int numNodes;
  
  float FractionOfPerceivedItems(Node* l, Vector2D v)
	{
	  float itemSum = 0.0;
	  for (int i = 0; i < numNodes; i++)
		{
			Vector2D diff = theNodes[i]->GetLocation() - v;
			if (diff.GetLength() < radius && theNodes[i]->GetMode() == Item::ON_FLOOR)
			{
			  itemSum += 1.0 - (l->Difference(theNodes[i]) / alpha);
			}
		}
		//cout << count << endl;
    float f = inverseArea * itemSum;
    
    //cout << f << endl;
    
    //cout << f << endl;
    if (f > 0.0)
      return f;
    else
      return 0.0;
  }
  
  
  bool RoomToDrop()
  {
    for (int i = 0; i < numNodes; i++)
    {
      Vector2D diff = theNodes[i]->GetLocation() - GetLocation();
      if (diff.GetLength() <= 2*theNodes[i]->GetRadius() && theNodes[i]->GetMode() == Item::ON_FLOOR)
        return false;
    }

    return true;
  }
  
  float DropProbability(float fraction)
  {
    float val = fraction / (k2 + fraction);
    
    return val * val;
  }
  
  virtual bool DropItem()
  {
    if (RoomToDrop())
    {
      float fraction = FractionOfPerceivedItems(static_cast<Node*>(GetItem()), GetLocation());
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
  
  virtual bool PickUpItem(Item* item)
  {
    float fraction = FractionOfPerceivedItems(static_cast<Node*>(item), item->GetLocation());
    float pPickUp = PickUpProbability(fraction);
    
    //cout << pPickUp << endl;
    float R = randval<float>(1.0);
    if (R <= pPickUp)
      return true;
    else
      return false;
  }

public:
  GraphAnt()
  {
    k1 = 0.3;
    k2 = 0.1;
    alpha = 1;
    area = 9;
    inverseArea = 1.0 / area;
    radius = 1.75 * 3.0;
    numNodes = 0;
    theNodes = new Node*[numNodes];
    This.InitRandom = true;
    This.SetMinSpeed(80.0);
    This.SetMaxSpeed(80.0);

  }
  
  void SetItems(Item** itemPtrs, int num)
  {
    numNodes = num;
    delete [] theNodes;
    theNodes = new Node*[numNodes];
    
    for (int i = 0; i < numNodes; i++)
    {
      theNodes[i] = static_cast<Node*>(itemPtrs[i]);
    }
  }

  virtual void Control()
  {

    if (GetMode() == Ant::ITEM)
      GetItem()->SetLocation(GetLocation());
      
    Ant::Control();
  }
  

  virtual void PickUpItemAction()
  {
    GetItem()->SetLocation(GetLocation());
    Ant::PickUpItemAction();
  }
	
	
};

class GraphSimulation : public Simulation
{
	static const int numNodes = 40;

private:

  Group<GraphAnt> theAnts;
	Group<Item> theNodes;
  
  int edgeMatrix[numNodes][numNodes];

  void AddEdges()
  {
    for (int i = 0; i < numNodes; i++)
	  {
	    for (int j = 0; j < numNodes; j++)
	    {
	      
	      if (i != j)
	      {
	        if (edgeMatrix[i][j])
	        {
	          static_cast<Node*>(theNodes[i])->AddEdge(static_cast<Node*>(theNodes[j]));
	          static_cast<Node*>(theNodes[j])->AddEdge(static_cast<Node*>(theNodes[i]));
	        }
	      }
	        
	    }
	  }
  }
  
  
  void initEdgeMatrix()
  {
    for (int i = 0; i < numNodes; i++)
    {
      for (int j = 0; j < numNodes; j++)
      {
        edgeMatrix[i][j] = 0;
      }
    }
  }
  
  void printEdgeMatrix()
  {
    for (int i = 0; i < numNodes; i++)
    {
      for (int j = 0; j < numNodes; j++)
      {
        cout << edgeMatrix[j][i];
      }
      
      cout << endl;
    }
    
  }
  

public:
	GraphSimulation():
	theAnts(10)
	{
	  for (int i = 0; i < numNodes; i++)
	  {
	    theNodes.push_back(new Node());
	  }
	
	  
	  initEdgeMatrix();
	  
	  //set up matrix into two fully interconnected graphs - not necessarily fully connected
	  //first test graph
	  for (int i = 0; i < (numNodes / 2); i++)
	  {
	    for (int j = 0; j < (numNodes / 2); j++)
	    {
	      if (i != j)
	        edgeMatrix[i][j] = 1;
	        
	    }
	  }
	  //second graph
	  for (int i = (numNodes / 2); i < numNodes; i++)
	  {
	    for (int j = (numNodes / 2); j < numNodes; j++)
	    {
	      if (i != j)
	        edgeMatrix[i][j] = 1;
	        
	    }
	  }
	  
	  
	  //add the edges to the graph
	  AddEdges();
	  
	  
	  
	  /*
	  for (int i = 0; i < numNodes; i++)
	  {
	    cout << theNodes[i]->GetDegree() << endl;
	    
	  }
	  */
	  
	  //set up item pointers and pass pointer array to the ants
	  Item** items = new Item*[numNodes];
	  for (int i = 0; i < numNodes; i++)
	  {
	    items[i] = theNodes[i];
    }
	  
	  for (int i = 0; i < theAnts.size(); i++)
	  {
	    theAnts[i]->SetItems(items, numNodes);
    }
	  
	  printEdgeMatrix();
	  
	  
	  //test to see if difference functions have been set up correctly
	  cout << theNodes[0]->Difference(theNodes[1]) << endl;
	  cout << theNodes[0]->Difference(theNodes[30]) << endl;
	  cout << theNodes[30]->Difference(theNodes[22]) << endl;
	
    //continuous simluation
		This.SetTimeSteps(-1);

		This.Add("Ants", This.theAnts);
		This.Add("Leaves", theNodes);

	}
	
	
	
};

