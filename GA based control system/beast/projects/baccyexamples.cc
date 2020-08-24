// Vim users: for increased viewing pleasure :set ts=4

#include "beast.h"
#include "simulation.h"
#include "bacteria.h"

using namespace std;
using namespace BEAST;

class TestBacterium : public Bacterium
{
public:
	TestBacterium() {
		// These variables should remain constant for the whole simulation.
		// They represent universal constraints within which the bacteria must
		// survive. You are free to alter them, but consider each time how it
		// affects the realism of the simulation. Note that for our purposes,
		// the most realistic values will not necessarily give the best
		// results, and also that the values below are neither realistic nor
		// effective.
		SetReproductionCost(0.4);	// Amount of energy lost in reproduction
		SetEnergyRate(0.005);		// Rate of energy use (* speed)
		SetSporeEnergyRate(0.01);	// Energy used whether moving or not
		SetAttractantCost(0.01);	// Cost to release one unit of attractant
		SetRepellentCost(0.01);		// Cost to release one unit of repellent
		SetDeathThreshold(0.0);		// Minimum energy before bacterium dies

		// These variables might change as a result of a genetic algorithm, or
		// your own experimentation. In most cases, an effect can be switched
		// off by setting the relevant variable to 0.0.
		SetReproductionThreshold(10.0);	// The amount of energy needed before
										// the bacterium reproduces (divides)
		SetSporulationThreshold(0.25);	// Energy level at sporulation
		SetConsumptionRate(0.1);	// Maximum food consumed per timestep
		SetAttractantRate(0.5);		// Amount of attractant released/food unit
		SetRepellentRate(4.0);		// Rate of repellent/inverse of food
		SetSwarmRadius(0.0);		// Radius in which velocities are summed
		SetSwarmInfluence(0.0);		// Degree of influence of swarm velocity
		SetGradientInfluence(1.0);	// Degree of influence of all gradients
		SetNutrientResponse(1.0);	// Relative change in direction wrt
		SetAttractantResponse(0.0);	// nutrient, attractant and repellent
		SetRepellentResponse(0.0);	// respectively
		SetAttractantThreshold(0.5);// Minimum food to trigger attractant
		SetRepellentThreshold(0.5);	// Minimum food to trigger repellent

		// This decides the amount of energy bacteria start with. If you find
		// your bacteria dying as soon as the simulation starts, it might be
		// worth increasing this, but if their consumption rate does not allow
		// them to reach equilibrium with the amount of food available it will
		// usually be worth adjusting the above variables instead.
		SetEnergy(1.0);

		// The speed is always constant, and influences energy consumption.
		SetSpeed(40.0);
	}

	virtual ~TestBacterium() {}
};

// Sets up a few bacteria which demonstrate their gradient-following ability.
class BacteriaGradientSim : public Simulation
{
protected:
	// An object to help us put bacteria into the World.
	Group<TestBacterium>	grpBacteria;
	// An object to help us put distributions into the World.
	Group<WorldObject>		grpWorld;
	// Pointers to Distributions, so we can keep track of them for the
	// duration of the simulation.
	Distribution			*nutrient,
							*trail;
public:
	// Constructor for our Simulation, in which Distributions are set up and
	// everything is put into the World.
	BacteriaGradientSim():
	grpBacteria(30)
	{
		Animat::SetTimeStep(3.0);

		This.GetWorld().Toggle(DISPLAY_COLLISIONS);
		This.GetWorld().Toggle(DISPLAY_TRAILS);
        This.GetWorld().Toggle(DISPLAY_MONITOR);
		This.GetWorld().Toggle(DISPLAY_SENSORS);

		// Set up a new 400 x 300 cell distribution.
		This.nutrient = new Distribution(400, 300, 1);
		This.nutrient->SetColour(1.0f, 1.0f, 1.0f); // Set the colour to white.
		This.nutrient->SetDiffusionSpeed(1); // Make it diffuse once per timestep.
		This.nutrient->Plot(0.5);			// Set each point to 0.5.
		// Add a Gaussian blob to give the bacteria a gradient to climb.
		This.nutrient->Filter(plus<double>(), Gaussian2D(200, 150, 30.0, 5000.0));
		This.nutrient->SetMaxConc(2.0);		// Max display density is 3.
		This.grpWorld.push_back(nutrient);	// Put it into the grpWorld object.

		// Set up a new 800 x 600 distribution.
		This.trail = new Distribution(800, 600, 1);
		This.trail->SetColour(1.0f, 1.0f, 1.0f);	// Set the colour to orange.
		This.trail->Plot(0.0);						// Zero the distribution.
		This.grpWorld.push_back(trail);				// Put it into the grpWorld object.

		This.Add("Bacteria", This.grpBacteria);		// Add the bacteria to the World.
		This.Add("Distributions", This.grpWorld);	// Add the distributions to the World.

		// Ignore the funny syntax; this just gives each Bacterium a pointer
		// to the nutrient and trail distributions.
		grpBacteria.ForEach<Distribution*>(&Bacterium::SetNutrientDist, nutrient);
		grpBacteria.ForEach<Distribution*>(&Bacterium::SetTrailDist, trail);

		// This makes the first 'assessment' go on for ever.
		SetTimeSteps(-1);
	}

};

// Now the bacteria will be adjusted to demonstrate swarming.
class BacteriaSwarmSim : public BacteriaGradientSim
{
public:
	BacteriaSwarmSim()
	{
		grpBacteria.ForEach<double>(&Bacterium::SetSwarmRadius, 20.0);
		grpBacteria.ForEach<double>(&Bacterium::SetSwarmInfluence, 0.5);
	}
};

class BacteriaRepellentSim : public BacteriaSwarmSim
{
protected:
	// A pointer to our repellent object.
	Distribution *repellent;

public:
	BacteriaRepellentSim()
	{
		// Make and add a new, nasty pink distribution for repellent.
		repellent = new Distribution(400, 300, 1);
		repellent->SetColour(1.0f, 0.5f, 0.5f);
		repellent->SetDiffusionSpeed(2);
		repellent->Plot(0.0);
		repellent->SetMaxConc(1.5f);
		nutrient->SetDiffusionSpeed(1); // Make it diffuse like nutrients.
		grpWorld.push_back(repellent);

		// Tell the bacteria about it.
		grpBacteria.ForEach<Distribution*>(&Bacterium::SetRepellentDist, repellent);

		// Make them respond to repellents.
		grpBacteria.ForEach<double>(&Bacterium::SetRepellentResponse, 0.5);

		// Just for a laugh, plot some noise, keep the bacteria guessing...
		nutrient->Filter(plus<double>(), GaussianNoise(0.0, 0.5));
	}
};

// Now we have reproduction, repellent and swarming.
class BacteriaReproSim : public BacteriaRepellentSim
{
public:
	BacteriaReproSim()
	{
		// In principle the bacteria could have reproduced in the previous
		// simulations, but the reproduction threshold was too high.
		grpBacteria.ForEach<double>(&Bacterium::SetReproductionThreshold, 0.4);
		grpBacteria.ForEach<double>(&Bacterium::SetConsumptionRate, 0.2);

	}
};

// Now we have reproduction, repellent and swarming.
class BacteriaAttractantSim : public BacteriaReproSim
{
protected:
	// A pointer to our attractant object.
	Distribution *attractant;

public:
	BacteriaAttractantSim()
	{
		// Make and add a new, slimey green distribution for attractant.
		attractant = new Distribution(400, 300, 1);
		attractant->SetColour(0.5f, 1.0f, 0.5f);
		attractant->SetDiffusionSpeed(2);
		attractant->Plot(0.0);
		attractant->SetMaxConc(1.5f);
		attractant->Update();
		grpWorld.push_back(attractant);

		// Tell the bacteria about it.
		grpBacteria.ForEach<Distribution*>(&Bacterium::SetAttractantDist, attractant);

		// Make them respond to attractants.
		grpBacteria.ForEach<double>(&Bacterium::SetAttractantResponse, 0.5);
	}
};

BEGIN_SIMULATION_TABLE
	ADD_SIMULATION("BacteriaGradient", BacteriaGradientSim)
	ADD_SIMULATION("BacteriaSwarm", BacteriaSwarmSim)
	ADD_SIMULATION("BacteriaRepellent", BacteriaRepellentSim)
	ADD_SIMULATION("BacteriaRepro", BacteriaReproSim)
	ADD_SIMULATION("BacteriaAttractant", BacteriaAttractantSim)
END_SIMULATION_TABLE
