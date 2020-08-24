// Vim users: for increased viewing pleasure :set ts=4
#include "beast.h"
#include "neuralanimat.h"
#include "sensor.h"
#include "population.h"
#include <time.h>
#include <sys/time.h>

using namespace std;
using namespace BEAST;

namespace BEAST{

class InternalState 
{
private:
	int state;
public:
	InternalState(){state=0;}
	bool isAvailable(){return state==0;}
	bool isCarrying(){return state==1;}
	bool isDropped(){return state==2;}
	void setAvailable(){state=0;}
	void setCarrying(){state=1;}
	void setDropped(){state=2;}	
};


// A Cheese is a small, yellow WorldObject which changes position when
// the Eaten method is called on it (i.e. a Mouse collides with it)
class Cheese : public WorldObject
{
public:
	Cheese()
	{
		This.Radius = 2.5f;							 // Cheeses are quite small
		This.SetColour(ColourPalette[COLOUR_YELLOW]);// Cheeses are yellow
		This.InitRandom = true;						 // Cheases are scattered
	
	}
	virtual ~Cheese(){}

	void pickup()
	{
		This.SetLocation(-10,-10);
	}

	void drop(double x, double y)
	{
		This.SetLocation(x,y);
	}

	// When a Cheese is Eaten, it reappears in a random location.
	void Eaten()
	{
		This.Location = This.MyWorld->RandomLocation();
	}
};

// In an ideal world, EvoMouse would inherit from Mouse, thereby getting
// the same OnCollision function and initialisation code as Mouse, but
// it's more convenient to inherit from EvoFFNAnimat which gives us all
// the GA and FFN code. Multiple inheritance would be another option, but
// introduces a host of other unwanted complications.
class EvoAnt : public EvoFFNAnimat
{
public:
	EvoAnt(): cheesesFound(0)
	{
		This.Add("angle", NearestAngleSensor<Cheese>());
		// Here we define the density sensor. Has to count Cheese objects, within 100 pixels radious circinference
		This.Add("density", DensitySensor<Cheese>(1*PI, 8.0, 0.0));

		This.Add("proximity", ProximitySensor<EvoAnt>(2*PI/3, 75.0, 0.0));
		This.InitRandom = true;
		This.state.setAvailable();
		This.InitFFN(4);
	}

	// This is identical to the OnCollision method for Mouse, except here we
	// are also recording the number of cheeses eaten.
	virtual void OnCollision(WorldObject* obj)
	{
		Cheese* cheese;
		
		EvoAnt* ant;
	
		struct timeval tv;

		gettimeofday(&tv,0);

		long total = tv.tv_sec ; 
			
		if (This.state.isDropped())
		{
			if (((total<0) and (miliseconds > total)) or ((total > 0) and (miliseconds < total))){
				This.state.setAvailable();
			}
		}

 		if (IsKindOf(obj,cheese)) {
			if (This.state.isAvailable()){
				// this is where I tried the output of the sensor.
				// We want the ants to pick up cheeses that are in places
				// with low density of cheeses, and not the ones that are in 
				// high density areas.
				// If the density is low, the ant will be rewarded and
				// if it is high, it will be punished

				//double o = This.Sensors["density"]->GetOutput();
				//cout <<"Output of the density sensor: " <<o <<"\n";
				cheesesFound++;
				picked_cheese =cheese;
				picked_cheese->pickup();
				This.state.setCarrying();
			}else if (This.state.isCarrying()){
				// Here is the other place where I check the density
				// Here is the opposing situation
				// we want to reward the ant if it drops the cheese 
				// in a high density location, and punish it if it
				// drops it in a low density location
				cheesesFound++;
				Vector2D loc=This.GetLocation();
				double x = loc.GetX();
				double y = loc.GetY();
				double orientation = This.GetOrientation();
				if (orientation < (PI/2)){
					x-=2.0;
					y-=2.0;
				}else if ((orientation >=(PI/2)) and (orientation < PI)){
					x+=2.0;
					y-=2.0;
				}else if ((orientation >=PI) and (orientation < (3*PI/2))){
					x+=2.0;
					y+=2.0;
				}else{
					x-=2.0;
					y+=2.0;
				}
				picked_cheese->drop(x, y);				
				This.state.setDropped();
				miliseconds = total + 2;
				picked_cheese=NULL;
			}
		}else if (IsKindOf(obj,ant)){
			collisions++;
		}

		EvoFFNAnimat::OnCollision(obj);
	}

	// The EvoMouse's fitness is the amount of cheese collected, divided by
	// the power usage, so a mouse is penalised for simply charging around
	// as fast as possible and randomly collecting cheese - it needs to find
	// its target carefully.
	virtual float GetFitness()const
	{
		float peso;
		if (collisions<10){
			peso = 1.5;
		}else if (collisions<20){
			peso = 1.0;
		}else{
			peso = 0.7;
		}
		return This.cheesesFound > 0 ? (static_cast<float>(This.cheesesFound) * peso) *( This.DistanceTravelled.as<float>()/100) : 0;
	}

	// Overloading the ToString method allows us to output a small amount of
	// information which is visible in the status bar of the GUI when a
	// mouse is clicked on.
	virtual string ToString()const
	{
		ostringstream out;
		out << " Power used: " << This.PowerUsed;
		return out.str();
	}

private:
	int cheesesFound;	// The number of cheeses collected for this run.
	int collisions;
	InternalState state;
	Cheese* picked_cheese;
	long miliseconds; 
};

class Final : public Simulation
{
	GeneticAlgorithm<EvoAnt> theGA;
	Population<EvoAnt> theAnts;
	Group<Cheese> theCheeses;

public:
	Final():
	theGA(0.7f, 0.05f),	// Crossover probability of 0.7, mutation probability of 0.05
	theAnts(30, theGA), // 30 mice are in the population.
	theCheeses(300)		// 30 cheeses are around at one time.
	{
		// We're using a rank selection method. Consult the BEAST
		// documentation for GeneticAlgorithm, the ar23 course slides or
		// a good book on GAs for more details.
		This.theGA.SetSelection(GA_RANK);
		// The ranking selection pressure is set to 2.
		This.theGA.SetParameter(GA_RANK_SPRESSURE, 2.0);

		This.SetTimeSteps(1000);
		This.SetGenerations(3000);
		This.Add("Ants", This.theAnts);
		This.Add("Cheeses", This.theCheeses);
	}

};

}

BEGIN_SIMULATION_TABLE
	ADD_SIMULATION("Swarm clustering", Final)
END_SIMULATION_TABLE


