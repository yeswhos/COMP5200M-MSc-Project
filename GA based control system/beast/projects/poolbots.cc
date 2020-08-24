// Vim users: for increased viewing pleasure :set ts=4

#include "neuralanimat.h"
#include "sensor.h"
#include "simulation.h"

using namespace std;
using namespace BEAST;

#include <sstream>

class RedBall : public WorldObject
{
public:
	RedBall():
	  WorldObject(Vector2D(), 0.0, 10.0){SetColour(ColourPalette[COLOUR_RED]);SetInitRandom(true);}
	virtual ~RedBall(){}

	virtual void OnCollision(WorldObject* other)
	{
		dead = true;
	}

	IMPLEMENT_SERIALISATION("RedBall", WorldObject)
};

class YellowBall : public WorldObject
{
public:
	YellowBall(Vector2D l = Vector2D()):
	  WorldObject(l, 0.0, 10.0){SetColour(ColourPalette[COLOUR_YELLOW]);SetInitRandom(true);}
	virtual ~YellowBall(){}

	virtual void OnCollision(WorldObject* other)
	{
		dead = true;
	}

	IMPLEMENT_SERIALISATION("YellowBall", WorldObject)
};

class RedPad : public WorldObject
{
public:
	RedPad(): WorldObject(Vector2D(400.0, 300.0), 0.0, 20.0), life(20)
	{ SetColour(ColourPalette[COLOUR_RED]); }
	virtual void Update()
	{ --life; if (life == 0) dead = true; }
private:
	int life;
};

class YellowPad : public WorldObject
{
public:
	YellowPad(): WorldObject(Vector2D(400.0, 300.0), 0.0, 20.0), life(20)
	{ SetColour(ColourPalette[COLOUR_YELLOW]); }
	virtual void Update()
	{ --life; if (life == 0) dead = true; }
private:
	int life;
};

class PoolBot : public EvoDNNAnimat
{
public:
	PoolBot()
	{
		Add("nearestred", NearestAngleSensor<RedBall>());
		Add("nearestyellow", NearestAngleSensor<YellowBall>());
		Add("touchingred", CollisionSensor<RedPad>());
		Add("touchingyellow", CollisionSensor<YellowPad>());

		SetLocation(Vector2D(400.0, 300.0));
		SetMinSpeed(0.0);
		InitDNN(8);

		SetRadius(10.0f);
	}

	virtual void Init()
	{
		redsFound = 0;
		yellowsFound = 0;
		collectReds = false;

		EvoDNNAnimat::Init();
	}

	virtual void OnCollision(WorldObject* other)
	{
		RedBall* r;
		YellowBall* y;
		RedPad* rp;
		YellowPad* ry;

		if (IsA(other, r)) {
			++redsFound;
		}
		else if (IsA(other, y)) {
			++yellowsFound;
		}
		else if (IsA(other, rp)) {
			collectReds = true;
			SetColour(ANIMAT_BODY, ColourPalette[COLOUR_RED]);
			Drawable::Init();
		}
		else if (IsA(other, ry)) {
			collectReds = false;
			SetColour(ANIMAT_BODY, ColourPalette[COLOUR_YELLOW]);
			Drawable::Init();
		}

		EvoDNNAnimat::OnCollision(other);
	}

	virtual float GetFitness()const
	{
		return static_cast<float>(collectReds ?
								  redsFound - yellowsFound :
								  yellowsFound - redsFound);
	}

	virtual string ToString()const
	{
		ostringstream out;
		out << "Collecting " << ((redsFound + yellowsFound > 0) ?
			( collectReds ? "reds" : "yellows") : "unknown" )
			<< ", Reds: " << redsFound << ", Yellows: " << yellowsFound
			<< ", Fitness: " << GetFitness();
		return out.str();
	}

private:
	int		redsFound,
			yellowsFound;
	bool	collectReds;
};

class PoolBotSimulation : public Simulation
{
	GeneticAlgorithm<PoolBot> gaPoolBot;
	Population<PoolBot>	popPoolBot;
	Group<YellowBall> popYellows;
	Group<RedBall> popReds;

public:
	PoolBotSimulation():
	gaPoolBot(0.7f, 0.05f),
	popPoolBot(25, gaPoolBot),
	popYellows(20),
	popReds(20)
	{
		Add("PoolBots", popPoolBot);
		Add("Reds", popReds);
		Add("Yellows", popYellows);
		popPoolBot.SetTeamSize(1);
		gaPoolBot.SetSelection(GA_TOURNAMENT);
		gaPoolBot.SetParameter(GA_TOURNAMENT_PARAM, 1.0f);

		SetAssessments(100);
		SetTimeSteps(1000);

		IMPLEMENT_LOADER("RedBall", RedBall);
		IMPLEMENT_LOADER("YellowBall", YellowBall);
	}

	void BeginAssessment()
	{
		if (irand(2) == 0) {
			theWorld.Add(new RedPad);
		}
		else {
			theWorld.Add(new YellowPad);
		}

		return Simulation::BeginAssessment();
	}
};

