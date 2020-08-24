// Vim users: for increased viewing pleasure :set ts=4

#include "neuralanimat.h"
#include "sensor.h"
#include "simulation.h"

using namespace std;
using namespace BEAST;

#include <sstream>

const float POINTS_PER_TARGETTIME = 1.0f;
const float POINTS_PER_MARKER = 30.0f;
const float POINTS_PER_COLLISION = -0.5f;

class Marker : public WorldObject
{
public:
	Marker(Vector2D l = Vector2D()):WorldObject(l, 0.0, 10.0){SetColour(ColourPalette[COLOUR_YELLOW]);}
	virtual ~Marker(){}

	virtual void OnCollision(WorldObject* other)
	{
		This.Dead = true;
	}

	IMPLEMENT_SERIALISATION("Marker", WorldObject)
};

class Target : public WorldObject
{
public:
	Target(Vector2D l = Vector2D()):WorldObject(l, 0.0, 10){SetColour(ColourPalette[COLOUR_RED]); }
	virtual ~Target(){}

	IMPLEMENT_SERIALISATION("Target", WorldObject)
};

class Pacman : public EvoDNNAnimat
{
public:
	Pacman()
	{
		This.Add("left", ProximitySensor<Wall>(PI/4, 200.0, -PI/10));
		This.Add("right", ProximitySensor<Wall>(PI/4, 200.0, PI/10));
		This.Add("marker", NearestAngleSensor<Marker>());
		This.Add("target", CollisionSensor<Target>());

		This.Location = Vector2D(400.0, 10.0);

		This.MinSpeed = 0.0;

		This.InitDNN(8);

		This.Radius = 10.0;
	}

	virtual void Init()
	{
		This.timeSpentOnTarget = This.markersFound = 0.0f;
		This.wallCollisions = 1.0f;

		This.EvoDNNAnimat::Init();
	}

	virtual void OnCollision(WorldObject* other)
	{
		Target* tgt;
		Marker* mkr;
		Wall* wll;

		if (IsA(other, tgt)) {
			This.timeSpentOnTarget++;
		}
		else if (IsA(other, mkr)) {
			This.markersFound++;
		}
		else if (IsA(other, wll)) {
			This.wallCollisions++;
		}
	}

	virtual float GetFitness()const
	{
		return This.markersFound * POINTS_PER_MARKER 
			 + This.wallCollisions * POINTS_PER_COLLISION
			 + This.timeSpentOnTarget * POINTS_PER_TARGETTIME;
	}

	virtual string ToString()const
	{
		ostringstream out;
		out << "Current fitness: " << GetFitness();
		return out.str();
	}

private:
	float		timeSpentOnTarget,
				markersFound,
				wallCollisions;
};

class PacmanSimulation : public Simulation
{
	GeneticAlgorithm<Pacman> gaPacman;
	Population<Pacman>	popPacman;
	Group<WorldObject> popWorld;

public:
	PacmanSimulation():
	gaPacman(0.7f, 0.05f),
	popPacman(100, gaPacman)
	{
		This.popWorld.push_back(new Wall(Vector2D(400.0, 00.0), 100.0, 20.0));
		This.popWorld.push_back(new Wall(Vector2D(175.0, 100.0), 400.0, 300.0));
		This.popWorld.push_back(new Wall(Vector2D(625.0, 100.0), 400.0, 200.0));
		This.popWorld.push_back(new Wall(Vector2D(350.0, 300.0), 700.0, 100.0));
		This.popWorld.push_back(new Wall(Vector2D(775.0, 300.0),  50.0, 200.0));
		This.popWorld.push_back(new Wall(Vector2D(450.0, 450.0), 700.0, 100.0));
		This.popWorld.push_back(new Wall(Vector2D( 25.0, 450.0),  50.0, 200.0));
		This.popWorld.push_back(new Wall(Vector2D(400.0, 575.0), 800.0, 50.0));
		This.popWorld.push_back(new Target(Vector2D(80.0, 500.0)));
		This.popWorld.push_back(new Marker(Vector2D(400.0, 20.0)));
		This.popWorld.push_back(new Marker(Vector2D(400.0, 60.0)));
		This.popWorld.push_back(new Marker(Vector2D(400.0, 100.0)));
		This.popWorld.push_back(new Marker(Vector2D(400.0, 140.0)));
		This.popWorld.push_back(new Marker(Vector2D(400.0, 180.0)));
		This.popWorld.push_back(new Marker(Vector2D(400.0, 220.0)));
		This.popWorld.push_back(new Marker(Vector2D(440.0, 220.0)));
		This.popWorld.push_back(new Marker(Vector2D(480.0, 220.0)));
		This.popWorld.push_back(new Marker(Vector2D(520.0, 220.0)));
		This.popWorld.push_back(new Marker(Vector2D(560.0, 220.0)));
		This.popWorld.push_back(new Marker(Vector2D(600.0, 220.0)));
		This.popWorld.push_back(new Marker(Vector2D(640.0, 220.0)));
		This.popWorld.push_back(new Marker(Vector2D(680.0, 220.0)));
		This.popWorld.push_back(new Marker(Vector2D(720.0, 220.0)));

		This.popWorld.push_back(new Marker(Vector2D(720.0, 260.0)));
		This.popWorld.push_back(new Marker(Vector2D(720.0, 300.0)));
		This.popWorld.push_back(new Marker(Vector2D(720.0, 340.0)));
		This.popWorld.push_back(new Marker(Vector2D(720.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(720.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(680.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(640.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(600.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(560.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(520.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(480.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(440.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(400.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(360.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(320.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(280.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(240.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(200.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(160.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D(120.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D( 80.0, 380.0)));
		This.popWorld.push_back(new Marker(Vector2D( 80.0, 420.0)));
		This.popWorld.push_back(new Marker(Vector2D( 80.0, 460.0)));

		This.Add("Pacmans", This.popPacman);
		This.Add("World", This.popWorld);
		This.popPacman.SetTeamSize(1);
		This.gaPacman.SetSelection(GA_TOURNAMENT);
		This.gaPacman.SetParameter(GA_TOURNAMENT_PARAM, 1.0f);

		This.SetAssessments(100);
		This.SetTimeSteps(1000);

		IMPLEMENT_LOADER("Marker", Marker);
		IMPLEMENT_LOADER("Target", Target);
	}
};

