// Vim users: for increased viewing pleasure :set ts=4

#include "neuralanimat.h"
#include "sensor.h"
#include "simulation.h"
#include "signaller.h"

using namespace std;
using namespace BEAST;

const double SEX_SENSOR_RANGE = 100.0;

enum MaleStateType {
	MALE_LOW,
	MALE_HIGH
};

enum MaleColourType {
	MALE_BLUE,
	MALE_RED
};

class Female;

class Male : 
	public DNNAnimat, 
	public Evolver<GAVariant>, 
	public Signaller<MaleStateType, MaleColourType>
{
public:
	Male()
	{
		Add("male sensor", NearestAngleSensor<Male>());
		Add("female sensor", NearestAngleSensor<Female>());

		Randomise(2, 2);

		matings = 0.0f;

		This.Radius = 7.0;
		This.MinSpeed = 0.0;
		This.MaxSpeed = 30.0;

		Drawable::SetColour(ColourPalette[COLOUR_BLUE]);

		InitDNN(6);
		SetInitRandom(true);
	}

	virtual void Update()
	{
		timeTaken++; DNNAnimat::Update();
	}

	virtual void Init()
	{
		if (GetState() == MALE_HIGH) {
			SetColour(ANIMAT_CENTRE, ColourPalette[COLOUR_WHITE]);
		}
		else {
			SetColour(ANIMAT_CENTRE, ColourPalette[COLOUR_BLACK]);
		}

		if (GetSignal() == MALE_RED) {
			SetColour(ANIMAT_BODY, ColourPalette[COLOUR_RED]);
		}
		else {
			SetColour(ANIMAT_BODY, ColourPalette[COLOUR_BLUE]);
		}

		matings = 0.0f;
		timeTaken = 0.0f;

		DNNAnimat::Init();
	}

	virtual genotype_type GetGenotype()const
	{
		genotype_type myGenotype;
		vector<float> annConfig = GetBrain().GetConfiguration();

		myGenotype.push_back(GetSignal(MALE_LOW) == MALE_RED ? true : false);
		myGenotype.push_back(GetSignal(MALE_HIGH) == MALE_RED ? true : false);

		copy(annConfig.begin(), annConfig.end(), back_inserter(myGenotype));

		return myGenotype;
	}

	virtual void SetGenotype(const genotype_type& myGenotype)
	{
		genotype_type::const_iterator i = myGenotype.begin();

		SetSignal(MALE_LOW,  *i ? MALE_RED : MALE_BLUE); ++i;
		SetSignal(MALE_HIGH, *i ? MALE_RED : MALE_BLUE); ++i;

		vector<float> annConfig;

		copy(i, myGenotype.end(), back_inserter(annConfig));

		GetBrain().SetConfiguration(annConfig);
	}

	void Mate()
	{
		matings++;

		if (matings >= maxMatings) {
			This.Dead = true;
		}
	}

	float GetFitness()const
	{
		return (GetCost() + matings) / timeTaken;
	}

private:
	float			matings;
	float			timeTaken;

	static int		maxMatings;
};

class Female : public DNNAnimat, public Evolver<GAVariant>
{
public:
	Female()
	{
		Add("male sensor", NearestAngleSensor<Male>());
		Add("male signal sensor", NearestSignalSensor<Male>(1));
		Add("female sensor", NearestAngleSensor<Female>());

		mateWithRed = irand(2) == 0;
		mateWithBlue = irand(2) == 0;

		Drawable::SetColour(ColourPalette[COLOUR_PINK]);
		SetColour(ANIMAT_BODY, ColourPalette[COLOUR_PINK]);

		This.MinSpeed = -5.0;
		InitDNN(6);
		SetInitRandom(true);
	}

	virtual void Init()
	{/*
		if (mateWithRed) {
			SetColour(ANIMAT_BODY, ColourPalette[COLOUR_RED]);
		}
		else {
			SetColour(ANIMAT_BODY, ColourPalette[COLOUR_WHITE]);
		}

		if (mateWithBlue) {
			SetColour(ANIMAT_CENTRE, ColourPalette[COLOUR_BLUE]);
		}
		else {
			SetColour(ANIMAT_CENTRE, ColourPalette[COLOUR_WHITE]);
		}
	*/
		matingSuccess = 0;
		matings = 0;
		DNNAnimat::Init();
	}

	virtual genotype_type GetGenotype()const
	{
		genotype_type myGenotype;
		vector<float> annConfig = GetBrain().GetConfiguration();

	//	myGenotype.push_back(mateWithBlue);
	//	myGenotype.push_back(mateWithRed);

		copy(annConfig.begin(), annConfig.end(), back_inserter(myGenotype));

		return myGenotype;
	}

	virtual void SetGenotype(const genotype_type& myGenotype)
	{
		genotype_type::const_iterator i = myGenotype.begin();

	//	mateWithBlue = *i;		++i;
	//	mateWithRed = *i;		++i;

		vector<float> annConfig(i, myGenotype.end());

		static_cast<DynamicalNet>(This.Brain).SetConfiguration(annConfig);
	}

	void OnCollision(WorldObject* obj)
	{
		Male* ptr;

		// Check if it's a male
		if (IsKindOf(obj,ptr)) {
			// Female decides if she's having it
		//	if ((mateWithRed && ptr->GetSignal() == MALE_RED)
		//	|| (mateWithBlue && ptr->GetSignal() == MALE_BLUE)) {
				// If so, she mates and male gets a point
				ptr->Mate();

				matings++;

				// And now the female dies with a +ve or -ve fitness modification
				if (ptr->GetState() == MALE_HIGH) {
					matingSuccess++;
					if (matings >= maxMatings) {
						This.Dead = true;
					}
				}
				else {
					matingSuccess--;
					if (matings >= maxMatings) {
						This.Dead = true;
					}
				}
		//	}
		}
		DNNAnimat::OnCollision(obj);
	}

	float GetFitness()const
	{ 
		return matingSuccess / This.PowerUsed.as<float>();
	}

private:
	bool			mateWithRed,
					mateWithBlue;

	int				matingSuccess;
	int				matings;

	static int		maxMatings;
};

int Male::maxMatings = 1;
int Female::maxMatings = 1;

class SignalSimulation : public Simulation
{
	GeneticAlgorithm<Male> gaMale;
	GeneticAlgorithm<Female> gaFemale;
	Population<Male> popMale;
	Population<Female> popFemale;

public:
	SignalSimulation():
	gaMale(0.7f, 0.07f), gaFemale(0.7f, 0.07f), 
	popMale(30,gaMale), popFemale(30,gaFemale)
	{
		gaMale.SetSelection(GA_TOURNAMENT);
		gaMale.SetParameter(GA_TOURNAMENT_SIZE, 2);
		gaMale.SetParameter(GA_TOURNAMENT_PARAM, 1.0f);

		gaFemale.SetSelection(GA_TOURNAMENT);
		gaFemale.SetParameter(GA_TOURNAMENT_SIZE, 2);
		gaFemale.SetParameter(GA_TOURNAMENT_PARAM, 1.0f);
		gaFemale.SetFitnessFix(GA_FIX);

		Add("Males", popMale);
		Add("Females", popFemale);

		Male::SetCost(MALE_LOW, MALE_RED, 1.5f);
		Male::SetCost(MALE_LOW, MALE_BLUE, 0.0f);
		Male::SetCost(MALE_HIGH, MALE_RED, 0.5f);
		Male::SetCost(MALE_HIGH, MALE_BLUE, 0.0f);

		SetAssessments(20);
		SetTimeSteps(1000);
		Animat::SetTimeStep(0.1);
	}
};

