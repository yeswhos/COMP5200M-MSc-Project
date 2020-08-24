// Vim users: for increased viewing pleasure :set ts=4

#include "beast.h"
#include "simulation.h"
#include "bacteria.h"

using namespace std;
using namespace BEAST;

/**
 * This class is derived from Bacterium and Evolver, so that Bacteriums can
 * be Evolved using GeneticAlgorithm. All the relevant variables in Bacterium
 * are doubles, so Evolver is given the double type, meaning that the
 * genotype will be a vector of doubles.
 */
class EvoBacterium : public Bacterium, public Evolver<double>
{
public:
	EvoBacterium()
	{
		// The following variables are constant for the simulation and are not
		// touched by the genetic algorithm, though you're free to change them
		// as you wish, or introduce them into the genotype if you think it'll
		// help...
		SetReproductionCost(0.0);	// default: 5.0
		SetEnergyRate(0.01);		// default: 0.005
		SetSporeEnergyRate(0.01);	// default: 0.01
		SetAttractantCost(0.01);	// default: 0.01
		SetRepellentCost(0.01);		// default: 0.01
		SetDeathThreshold(0.1);		// default: 0.0
		SetEnergy(0.1);			// default: 1.0

		// The following 15 variables make an appearance in the genotype (see
		// GetGenotype and SetGenotype). Some have been set to constant values
		// and others are commented out, in which case they default to the
		// constants given to the right. The randval calls initialise the 
		// variables within suggested ranges, which can then form the basis of
		// an evolutionary simulation.
		// Each time you 'switch on' a gene here, be sure to do the same in
		// SetGenotype, otherwise the gene won't be expressed in the simulated
		// bacteria (they'll just have the constant or random value set here
		// instead.)
		// To turn off a feature altogether, set the corresponding parameter 
		// value to an extreme (either 0.0 or a very large number).
		SetReproductionThreshold(randval(10.0));	// default: 0.4
		SetConsumptionRate(randval(0.2));		// default: 0.1
		SetSporulationThreshold(0.0);			// default: 0.25
//		SetAttractantRate(randval(2.0));		// default: 0.5
//		SetRepellentRate(randval(2.0));			// default: 4.0
//		SetSwarmRadius(randval(2.0));			// default: 20.0
//		SetSwarmInfluence(randval(2.0));		// default: 0.5
//		SetGradientInfluence(randval(2.0));		// default: 0.8
//		SetAttractantThreshold(randval(2.0));		// default: 0.5
//		SetRepellentThreshold(randval(2.0));		// default: 0.5
//		SetNutrientResponse(randval(2.0));		// default: 0.8
//		SetAttractantResponse(randval(2.0));		// default: 0.8
//		SetRepellentResponse(randval(2.0));		// default: 0.8

		// We set the speed to 0.0 here so that bacteria are non-motile.
		SetSpeed(0.0);					// default: 40.0

		// Things tend to work better if the bacteria don't take up too much
		// space - feel free to experiment with this but in general for the
		// purpose of making fractals, the size of the bacteria is best left
		// small.
		SetRadius(1.5);					// default: 5.0

		// Instead of initialising in random places, we want them to appear in
		// a blob in the middle (see Init())
		SetInitRandom(false);

	}

	/// Virtual destructor required for polymorphism.
	virtual ~EvoBacterium(){}

	// This function ensures that when the new bacteria are added to the
	// World, they appear in a slightly random blob, in the centre.
	virtual void Init()
	{
		Vector2D centre(GetWorld().GetWidth() / 2.0,
						GetWorld().GetHeight() / 2.0);
		SetStartLocation(centre + Vector2D(randval(30.0) - 15.0,
						 randval(30.0) - 15.0));
		return Bacterium::Init();
	}

	// Defines the genotype of the EvoBacterium - the order and number of
	// values stored here should match those in SetGenotype. Don't change
	// this method unless you want to introduce new variables into the
	// evolutionary simulation, which shouldn't be necessary for the
	// coursework.
	virtual genotype_type GetGenotype()const
	{
		genotype_type g;

		g.push_back(GetReproductionThreshold());
		g.push_back(GetSporulationThreshold());
		g.push_back(GetConsumptionRate());
		g.push_back(GetAttractantRate());
		g.push_back(GetRepellentRate());
		g.push_back(GetSwarmRadius());
		g.push_back(GetSwarmInfluence());
		g.push_back(GetGradientInfluence());
		g.push_back(GetNutrientResponse());
		g.push_back(GetAttractantResponse());
		g.push_back(GetRepellentResponse());
		g.push_back(GetAttractantThreshold());
		g.push_back(GetRepellentThreshold());
		g.push_back(GetSpeed());
		g.push_back(GetRadius());
		
		return g;
	}

	// Copies the values from the genotype into the right place in the
	// EvoBacterium. Commented lines mean that instead of using evolved
	// parameters, the EvoBacterium will stick to its default settings from
	// the constructor.
	virtual void SetGenotype(const genotype_type& g)
	{
		SetReproductionThreshold(	g[0]);
//		SetSporulationThreshold(	g[1]);
		SetConsumptionRate(		g[2]);		
//		SetAttractantRate(		g[3]);			
//		SetRepellentRate(		g[4]);			
//		SetSwarmRadius(			g[5]);			
//		SetSwarmInfluence(		g[6]);			
//		SetGradientInfluence(		g[7]);		
//		SetNutrientResponse(		g[8]);		
//		SetAttractantResponse(		g[9]);		
//		SetRepellentResponse(		g[10]);		
//		SetAttractantThreshold(		g[11]);	
//		SetRepellentThreshold(		g[12]);		
//		SetSpeed(			g[13]);
//		SetRadius(			g[14]);
	}

	// Returns a float which indicates the relative fitness of this
	// EvoBacterium. A few things to note:
	// The EvoBacteriums which go into the World at the beginning of the
	// assessment are the *only* ones which get assessed. All their offspring
	// are regular Bacterium objects which don't go into the GA at the end.
	// This means that to assess whole strains of bacteria (i.e. every one of
	// a particular colour), you need to make use of the information the
	// original EvoBacterium has about its progeny. A full list of offspring
	// can be obtained with the GetOffspring method, and this list can be
	// worked on using various STL algorithms, examples are given.
	virtual float GetFitness()const
	{
		list<Bacterium const*> babies;	// Make a list to contain offspring.
		GetOffspring(babies);			// Copy the babies into the list.
		babies.push_back(this);			// Add the current individual.

		// Now for some possible methods of scoring fitness. Floats are used
		// instead of ints to make the final arithmetic clearer and warning-
		// free when compiled.

		// Total bacteria (dead and alive) is simply the size of the list.
		float numTotal = static_cast<float>(babies.size());
		// We can work out the number of dead ones using count_if and the
		// IsDead member function (see a good STL ref if the following is
		// unclear, and you actually care how it works...)
		float numDead = count_if(babies.begin(), babies.end(),
					 mem_fun(&Bacterium::IsDead));
		float numAlive = numTotal - numDead;

		/*
		// We can find out the total energy consumed for the lifetime of 
		// every bacterium (dead or alive) like this:
		float totalEnergy = accumulate_fun(babies.begin(), babies.end(),
					   0.0f, mem_fun(&Bacterium::GetTotalEnergy));
		*/

		return numAlive;
		// return numTotal;
		// return numTotal / totalEnergy;
		// return numAlive / totalEnergy;
	}
};

/**
 * Sets up a simulation in which a population of EvoBacteria are all assessed
 * together.
 */
class BacteriaFractalSim : public Simulation
{
protected:
	// Population container for our evolvable bacteria.
	Population<EvoBacterium>	grpBacteria;
	// Genetic Algorithm to use on the bacteria.
	GeneticAlgorithm<EvoBacterium>	gaBacteria;
	// Container for the distributions.
	Group<WorldObject>			grpWorld;	// Contains the Distribution
	// Pointers to keep track of distributions across assessments.
	Distribution				*nutrient,
								*attractant,
								*repellent,
								*trail;
public:
	/// Constructor, sets up all the objects in the simulation.
	BacteriaFractalSim():
	// Evolve a population of 30 bacteria and link our GA to that population.
	grpBacteria(30, gaBacteria)
	{
		// Bacteriums work quite differently to Animats, with a different
		// system of trails and simplified sensors, so these are toggled off.
		This.GetWorld().Toggle(DISPLAY_COLLISIONS);
		This.GetWorld().Toggle(DISPLAY_TRAILS);
       	This.GetWorld().Toggle(DISPLAY_MONITOR);
		This.GetWorld().Toggle(DISPLAY_SENSORS);

		// Set the width and height of the World. If you find your bacteria
		// grow too quickly and you need more space, change these settings.
		// Remember that the resolution of the distributions is the same
		// regardless of the size of the World, and you might get better
		// results by reducing the radius of the bacteria.
		This.GetWorld().SetWidth(800.0);	// default: 800.0
		This.GetWorld().SetHeight(600.0);	// default: 600.0

		// Use rank selection in the GA. Other options are GA_ROULETTE and
		// GA_TOURNAMENT. Check the GeneticAlgorithm documentation for more
		// details.
		This.gaBacteria.SetSelection(GA_RANK);

		// Set up the nutrient distribution with a resolution of 400 x 300 and
		// a diffusion kernel of radius 2. This means that the Distribution
		// will be convolved with a 5x5 kernel (radius 1 gives a 3x3 kernel).
		// The larger the kernel, the faster the apparent diffusion, but the
		// slower the simulation goes.
		This.nutrient = new Distribution(400, 300, 2);
		This.nutrient->SetColour(1.0f, 1.0f, 1.0f);
		// The diffusion kernel works on the distribution every frame. Set
		// diffusion speed to 2 for every other frame, 0 for never.
		This.nutrient->SetDiffusionSpeed(1);
		This.grpWorld.push_back(nutrient);	// Add the distribution to grpWorld.

/*		This commented section adds an attractant distribution, diffusing
//		every other frame.
		This.attractant = new Distribution(400, 300, 1);
		This.attractant->SetDiffusionSpeed(2);
		This.attractant->SetColour(0.3f, 1.0f, 0.3f);
		This.grpWorld.push_back(This.attractant);
//*/

/*		This commented section adds a repellent distribution, diffusing
//		every frame.
		This.repellent = new Distribution(400, 300, 1);
		This.repellent->SetDiffusionSpeed(1);
		This.repellent->SetColour(1.0f, 0.5f, 0.5f);
		This.grpWorld.push_back(This.repellent);
//*/

		// Here we set up a trail distribution with resolution to match the
		// size of the window. The trail is not involved in the simulation and
		// just tracks the movement of the bacteria, thus it does not diffuse
		// and so doesn't drain CPU time too much.
		This.trail = new Distribution(800, 600, 1);
		This.trail->SetColour(1.0f, 0.5f, 0.0f);
		This.grpWorld.push_back(This.trail);

		This.Add("Bacteria", This.grpBacteria);
		This.Add("Distributions", This.grpWorld);

		// Here we set some parameters for the whole simulation. A simulation
		// consists of a number of runs which contain generations which
		// contain assessments which contain timesteps. One timestep is one
		// frame of the animation. An assessment runs for a certain number of
		// timesteps and at the end a fitness score is stored. After a certain
		// number of timesteps, the generation ends and any populations are
		// put through their respective GAs. After a certain number of
		// generations, the simulation might be run again from the start.
		This.SetTimeSteps(500);	// default: 1000
//		This.SetAssessments(30);	// default: 1 - don't uncomment this here,
					// use the other simulation object instead
		This.SetGenerations(500);	// default: unlimited
//		This.SetRuns(1);		// default: 1

	}

	/**
	 * This function is called at the beginning of each generation. There are
	 * other functions like this: EndGeneration, BeginAssessment(below),
	 * EndAssessment, BeginRun, EndRun. Any of these can be used to perform
	 * additional setup at any stage of the simulation. Always remember to
	 * call the original Simulation:: version of the function somewhere in
	 * your overridden version.
	 */
	virtual void BeginGeneration()
	{
		// This slightly clumsy code calls functions on each member of a group
		// with the specified values, in this case to set the distributions on
		// each Bacterium.
		This.grpBacteria.ForEach<Distribution*>(&Bacterium::SetNutrientDist, This.nutrient);
//		This.grpBacteria.ForEach<Distribution*>(&Bacterium::SetAttractantDist, This.attractant);
//		This.grpBacteria.ForEach<Distribution*>(&Bacterium::SetRepellentDist, This.repellent);
		This.grpBacteria.ForEach<Distribution*>(&Bacterium::SetTrailDist, This.trail);

		Simulation::BeginGeneration();
	}

	/**
	 * This function is called at the beginning of each assessment.
	 */
	virtual void BeginAssessment()
	{
		// The nutrient distribution can be set up in various ways, in this
		// case it's a uniform 0.5.
		This.nutrient->Plot(0.5);
//		This.nutrient->Plot(Gaussian2D(200, 150, 30.0, 6000.0));
//		This.nutrient->Filter(plus<double>(), GaussianRing2D(200, 150, 100.0, 5.0, 10.0));
//		This.nutrient->Filter(plus<double>(), GaussianNoise(0.0, 0.5));
//		This.nutrient->Filter(bind2nd(divides<double>(), 1.5));
		// This can be used to clip the distribution within certain values.
//		This.nutrient->Filter(LimitDistribution(0.0, 3.0));

		// The display varies the brightness of cells according to density,
		// to do this it needs to know the highest expected concentration. If
		// you find your distributions just look white, try raising this.
		This.nutrient->SetMaxConc(1.0);

		This.trail->Plot(0.0);		// Reset the trail distribution.
//		This.attractant->Plot(0.0);	// Reset the attractant distribution.
//		This.repellent->Plot(0.0);	// Reset the repellent distribution.

		Simulation::BeginAssessment();
	}

};

/**
 * This is just like ChemotaxisSimulation except now the bacteria are being
 * assessed separately, so that different strains don't squeeze each other
 * out of play.
 */
class BacteriaFractalSim2 : public BacteriaFractalSim
{
public:
	BacteriaFractalSim2()
	{
		This.grpBacteria.SetTeamSize(1);	// One individual per assessment.
		This.SetAssessments(30);			// One assessment per grpBacteria member.
	}
};

BEGIN_SIMULATION_TABLE
	ADD_SIMULATION("BacteriaFractal", BacteriaFractalSim)
	ADD_SIMULATION("BacteriaFractalSim", BacteriaFractalSim2)
END_SIMULATION_TABLE

