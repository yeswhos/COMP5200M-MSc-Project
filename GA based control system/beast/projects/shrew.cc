// Vim users: for increased viewing pleasure :set ts=4

#include "animat.h"
#include "sensor.h"
#include "sensorbase.h"
#include "beast.h"
#include "simulation.h"
#include "population.h"
#include "neuralanimat.h"
#include "worldobject.h"

using namespace std;
using namespace BEAST;

int counterA = 0;
int counterB = 0;
int counterC = 0;
bool inRangeA = true;
bool inRangeB = true;
bool inRangeC = true;
bool temp = true;
bool tempB = true;
bool tempC = true;
// For more information on this file, take a look at Tutorial 1 in the
// BEAST documentation.
class Circle : public WorldObject
{
public:
	Circle()
	{
		
		This.Radius = 200.0f;							 
		This.SetColour(ColourPalette[COLOUR_BLACK]);
		//This.InitRandom = true;						
		SetLocation(400, 300);
		This.Location = GetLocation();
	}
	virtual ~Circle(){}

};

class circleCenter : public WorldObject
{
public:
	circleCenter()
	{
		
		This.Radius = 3.1f;							 // Cheeses are quite small
		This.SetColour(ColourPalette[COLOUR_WHITE]);// Cheeses are yellow
		//This.InitRandom = true;						 // Cheases are scattered
		SetLocation(400, 300);
		This.Location = GetLocation();
	}
	virtual ~circleCenter(){}
};

class ZumoQueen;
class ZumoLord;
class ZumoKing : public EvoFFNAnimat
{
public:

	ZumoKing():lines(0)
	{
		//IsPolygon();
		This.Radius = 25.0;
		This.Add("lineSensor", ProximitySensor<circleCenter>(2 * PI, 200.0f, -PI));
		//Sensors["lineSensor"] -> SetMatchingFunction(new MatchExact<circleCenter>);
		// This.Add("left", ProximitySensor<ZumoQueen>(PI/4, 200.0, -PI/20));
		// This.Add("right", ProximitySensor<ZumoQueen>(PI/4, 200.0, PI/20));

		This.Add("left", ProximitySensor<ZumoLord>(PI/4, 200.0, -PI/20));
		This.Add("right", ProximitySensor<ZumoLord>(PI/4, 200.0, PI/20));

		This.SetStartOrientation(PI);
		//This.SetInitRandom(true);	// Start in random locations
		This.InitRandom = false;
		This.SetStartLocation(Vector2D(500, 300));
		SetMinSpeed(0.0);
		SetMaxSpeed(100.0);
		This.InitFFN(5);

	}

	// void setEnermy(EvoMouses *enermy) {
	// 	This.enermy = enermy;
	// }

	virtual void Control(){
		This.lines = This.Sensors["lineSensor"] ->GetOutput();
		if(This.lines < 0.015 && This.lines != 0){
			inRangeA = false;
			temp = false;
		}
		if (inRangeA == false){
			counterA++;
			This.SetLocation(Vector2D(500, 300));
			This.SetOrientation(PI);
			//This.SetStartOrientation(PI);
			inRangeA = true;
		}
		// else if(tempB == false){
		// 	This.SetLocation(Vector2D(500, 300));
		// 	This.SetOrientation(PI);
		// 	//This.SetStartOrientation(PI);
		// 	tempB = true;
		// 	inRangeA = true;
		// }
		else if(tempC == false){
			This.SetLocation(Vector2D(500, 300));
			This.SetOrientation(PI);
			//This.SetStartOrientation(PI);
			tempC = true;
			inRangeA = true;
		}
		
		EvoFFNAnimat::Control();
	}

	// void OnCollision(WorldObject* obj){
		
	// 	//setEnermy(enermy);
	// 	FFNAnimat::OnCollision(obj);
	// }
	virtual float GetFitness()const
	{	
		float score = 0.0;

		score = 2 * counterC - counterA + 0.0001 * This.DistanceTravelled;
		if (counterC == 0){
			score = 0.0;
		}
		//score = 5 * counterB - 0.5 * counterA;
		//cout << This.DistanceTravelled << endl;
		if (score < 0){
			score = 0.0;
		} 
		cout << counterC << endl;
		counterC = 0;
		counterA = 0;

		float a = score;
		ofstream out("/home/beast/COMP5400M-Bio-inspired-Computing/CW1/Demo/ZumoKing.txt", ios::app);
		out << a << endl;
		out.close();
		//cout << score << endl;
		return score;
	}

	// Overloading the ToString method allows us to output a small amount of
	// information which is visible in the status bar of the GUI when a
	// mouse is clicked on.
	virtual string ToString()const
	{
		ostringstream out;
		//out << " Power used: " << This.PowerUsed;
		out << "current fitness for defence" << GetFitness();
		return out.str();
	}
private:
	double lines;

};

// class ZumoQueen : public EvoFFNAnimat
// {
// public:
// 	//double lines;
// 	bool inRange = true;
// 	int counter = 0;
	
// 	//EvoMouses *enermy;
// 	ZumoQueen():lines(0)
// 	{
// 		This.Add("lineSensor", ProximitySensor<circleCenter>(2 * PI, 200.0f, -PI));
// 		//Sensors["lineSensor"] -> SetMatchingFunction(new MatchExact<circleCenter>);
// 		This.Add("left", ProximitySensor<ZumoKing>(PI/4, 200.0, -PI/20));
// 		This.Add("right", ProximitySensor<ZumoKing>(PI/4, 200.0, PI/20));

// 		//This.SetInitRandom(true);	// Start in random locations
// 		This.InitRandom = false;
// 		This.SetStartLocation(Vector2D(300, 300));
		
// 		This.Radius = 25.0;
// 		SetMaxSpeed(100.0);
// 		SetMinSpeed(0.0);
// 		This.InitFFN(5);

// 	}

// 	// void setEnermy(EvoMouses *enermy) {
// 	// 	This.enermy = enermy;
// 	// }

// 	virtual void Control(){
// 		This.lines = This.Sensors["lineSensor"] ->GetOutput();
// 		if(This.lines < 0.015 && This.lines != 0){
// 			inRangeB = false;
// 			tempB = false;
// 		}
// 		//cout << This.lines  << endl;
// 		// This.Controls["left"] = 0;
// 		// This.Controls["right"] = 0;
		
// 		EvoFFNAnimat::Control();
// 	}
	
// 	void reLocate(){
// 		This.SetLocation(Vector2D(300, 300));
// 		This.SetOrientation(0.0);
// 	}

// 	void OnCollision(WorldObject* obj){
// 		if (inRangeB == false){
// 			counterB++;
// 			reLocate();
// 			inRangeB = true;
// 		}else if(temp == false){
// 			reLocate();
// 			temp = true;
// 			inRangeB = true;
// 		}
// 		//setEnermy(enermy);
// 		FFNAnimat::OnCollision(obj);
// 	}

// 	virtual float GetFitness()const
// 	{	
// 		float score = 0.0;

// 		score = counterA - 5 * counterB + 0.0001 * This.DistanceTravelled;
// 		//score = counterA - 5 * counterB;
// 		if ((counterA == 0) && (counterB == 0)){
// 			score = 0.0;
// 		}
// 		if (score < 0){
// 			score = 0.0;
// 		}
// 		counterA = 0;
// 		float b = score;
// 		ofstream out("/home/beast/COMP5400M-Bio-inspired-Computing/CW1/Demo/ZumoQueen.txt", ios::app);
// 		out << b << endl;
// 		out.close();
// 		//cout << score << endl;
// 		return score;

// 		// float score = 0.0;
// 		// if(counterA == 0){
// 		// 	return 0.1;
// 		// }
// 		// score = 1 / (10 * counterB + (1 / counterA));
// 		// return score;
// 	}

// 	// Overloading the ToString method allows us to output a small amount of
// 	// information which is visible in the status bar of the GUI when a
// 	// mouse is clicked on.
// 	virtual string ToString()const
// 	{
// 		ostringstream out;
// 		//out << " Power used: " << This.PowerUsed;
// 		out << "current fitness for defence" << GetFitness();
// 		return out.str();
// 	}
// private:
// 	double lines;

// };

class ZumoLord : public Animat
{
public:

	ZumoLord():lines(0)
	{
		This.Add("lineSensor", ProximitySensor<circleCenter>(2 * PI, 200.0f, -PI));
		//Sensors["lineSensor"] -> SetMatchingFunction(new MatchExact<circleCenter>);
		This.Add("left", ProximitySensor<ZumoKing>(PI/4, 200.0, -PI/20));
		This.Add("right", ProximitySensor<ZumoKing>(PI/4, 200.0, PI/20));

		//This.SetInitRandom(true);	// Start in random locations
		This.InitRandom = false;
		This.SetStartLocation(Vector2D(300, 300));
		
		This.Radius = 25.0;
		SetMaxSpeed(100.0);
		SetMinSpeed(0.0);
		//This.InitFFN(5);

	}

	// void setEnermy(EvoMouses *enermy) {
	// 	This.enermy = enermy;
	// }

	virtual void Control(){
		This.lines = This.Sensors["lineSensor"] ->GetOutput();
		if(This.lines < 0.015 && This.lines != 0){
			inRangeC = false;
			tempC = false;
		}
		if (inRangeC == false){
			counterC++;
			reLocate();
			inRangeC = true;
		}else if(temp == false){
			reLocate();
			temp = true;
			inRangeC = true;
		}
		if(This.lines < 0.02 && This.lines != 0){
			This.Controls["left"] = -lines;
			This.Controls["right"] = -lines;
		}
		double ProxRight = This.Sensors["right"] -> GetOutput();
		double ProxLeft = This.Sensors["left"] -> GetOutput();
		if((ProxLeft == 0) && (ProxRight == 0)){
			This.Controls["left"] = -0.8;
			This.Controls["right"] = 0.5;
		}else{
			// This.Controls["left"] =  1.5 * ProxLeft;
			// This.Controls["right"] =  1.5 * ProxRight;
			This.Controls["left"] =  0.7;
			This.Controls["right"] =  0.7;
		}
		
	}
	
	void reLocate(){
		This.SetLocation(Vector2D(300, 300));
		This.SetOrientation(0.0);
	}

	void OnCollision(WorldObject* obj){

		//setEnermy(enermy);
		Animat::OnCollision(obj);
	}

private:
	double lines;

};


class ShrewSimulation : public Simulation
{
	GeneticAlgorithm<ZumoKing> gaKing;
	//GeneticAlgorithm<ZumoQueen> gaQueen;
	Population<ZumoKing> popKing;
	//Population<ZumoQueen> popQueen;
	// Group<EvoMouses> grpShrew;
	Group<Circle> theCircle;
	Group<circleCenter> theCircleCenter;
	Group<ZumoLord> theLord;

public:
	ShrewSimulation():
	gaKing(0.7f, 0.05f),
	//gaQueen(0.7f, 0.05f),
	popKing(5, gaKing),
	theLord(1),
	//popQueen(5, gaQueen),
	theCircle(1),
	theCircleCenter(1)
	{
		This.gaKing.SetSelection(GA_RANK);
		This.gaKing.SetParameter(GA_RANK_SPRESSURE, 2.0);
		// This.gaQueen.SetSelection(GA_TOURNAMENT);
		// This.gaQueen.SetParameter(GA_TOURNAMENT_PARAM, 0.7);
		popKing.SetTeamSize(1);
		//popQueen.SetTeamSize(1);
		SetAssessments(5);
		// if(inRangeA == false || inRangeB)
		This.Add("ZumoKing", This.popKing);
		//This.Add("ZumoQueen", This.popQueen);
		This.Add("ZumoLord", This.theLord);
		This.Add("Circle", This.theCircle);
		This.Add("circleCenter", This.theCircleCenter);
		This.SetTimeSteps(500);
	}
};

