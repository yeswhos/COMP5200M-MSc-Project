#include "bacterium.h"

using namespace std;

namespace BEAST {

/**
 * Sets up the Bacterium with a series of standard values which should be
 * overridden by the constructors of inherited classes.
 */
Bacterium::Bacterium():
reproductionCost(0.4), 
energyRate(0.005), 
sporeEnergyRate(0.01),
attractantCost(0.01), repellentCost(0.01),
deathThreshold(0.0),
tumbleTime(10.0), tumbleScale(10.0),
reproductionThreshold(0.4), 
sporulationThreshold(0.25),
consumptionRate(0.1), attractantRate(0.5), repellentRate(4.0),
swarmRadius(20.0), swarmInfluence(0.5), 
gradientInfluence(0.8),
nutrientResponse(0.8), attractantResponse(0.8), repellentResponse(0.8),
attractantThreshold(0.5), repellentThreshold(0.5),
nutrientDist(NULL),	attractantDist(NULL), 
repellentDist(NULL), trailDist(NULL),
energy(1.0), totalEnergy(0.1), lastNutrient(0.0),
currentNutrient(0.0), currentAttractant(0.0), currentRepellent(0.0),
nextCheck(0), swarmSize(0)
{
	SetInitRandom(true);
	SetSpeed(40.0);
}

/**
 * Deletes all offspring of this Bacterium.
 */
Bacterium::~Bacterium()
{
	for_each(offspring.begin(), offspring.end(), deleter<Bacterium>());
}

/**
 * Updates the location and energy levels of the Bacterium, and causes various
 * changes in the nutrient, attractant and repellent distributions. For a full
 * explanation of what is going on here, see the Bacterium class documentation.
 */
void Bacterium::Update()
{
	CheckBoundary();
	ReadDistributions();
	UpdateDistributions();

	SetVelocity((1.0 - gradientInfluence) * GetVelocity() + 
			 gradientInfluence * 
			 ( nutrientResponse * GetNutrientGradient()
			 + attractantResponse * GetAttractantGradient()
			 - repellentResponse * GetRepellentGradient()));
	SetVelocity((1.0 - swarmInfluence) * GetVelocity()
			 + swarmInfluence * GetSwarmVelocity());

	GetVelocity().SetLength(GetMaxSpeed());

	UpdateEnergy();

	// Apply velocity changes and clean up.
	FinishUpdate();
}

/**
 * Before we do any checks, ensure that bacteria are all within boundaries
 * so we don't end up trying to read off the edges of the distribution.
 */
void Bacterium::CheckBoundary()
{
	if (GetLocation().x <= 0.0) {
		SetVelocityX(-GetVelocity().x);
		SetLocationX(0.0); }
	if (GetLocation().x >= GetWorld().GetWidth()) {
		SetVelocityX(-GetVelocity().x);
		SetLocationX(GetWorld().GetWidth() - 1.0); }
	if (GetLocation().y <= 0.0) {
		SetVelocityY(-GetVelocity().y);
		SetLocationY(0.0); }
	if (GetLocation().y >= GetWorld().GetHeight()) {
		SetVelocityY(-GetVelocity().y);
		SetLocationY(GetWorld().GetHeight() - 1.0); }
}

/// Get local information from whichever distributions are in use.
void Bacterium::ReadDistributions()
{
	if (nutrientDist != NULL) currentNutrient = nutrientDist->GetDensity(GetLocation());
	if (attractantDist != NULL) currentAttractant = attractantDist->GetDensity(GetLocation());
	if (repellentDist != NULL) currentRepellent = repellentDist->GetDensity(GetLocation());
}

/**
 * Consumes nutrient and releases attractant and repellent according to the
 * relevant variables.
 * Spores do not release attractants or relellants.
 */
void Bacterium::UpdateDistributions()
{
	if (nutrientDist != NULL) {

		double amount = consumptionRate <= currentNutrient ? consumptionRate : currentNutrient;
		energy += amount;
		totalEnergy += amount;
		nutrientDist->AddDensity(GetLocation(), static_cast<DistReal>(-amount));

	    if	(!isSpore) {
		    if (currentNutrient >= attractantThreshold) ReleaseAttractant();
		    if (currentNutrient < repellentThreshold) ReleaseRepellent();
	    }
	}

	if (trailDist != NULL) {
		if (trailDist->GetDensity(GetLocation()) <= 0.0) 
			trailDist->SetDensity(GetLocation(), 0.5);
	}
}

/**
 * Releases a quantity of attractant depending on the attractantRate, 
 * attractantThreshold and available energy. The attractant is added to the
 * attractant distribution.
 * \warning Does not check whether the nutrient is above the attractant
 * threshold, this is done in UpdateDistributions.
 */
void Bacterium::ReleaseAttractant()
{
	if (attractantDist == NULL) return;

	double amount = currentNutrient * attractantRate;
	double cost = amount * attractantCost;

	if (energy >= cost) energy -= cost;
	else {
		amount = energy / attractantCost;
		energy = 0.0;
	}

	attractantDist->AddDensity(GetLocation(), static_cast<DistReal>(amount));
}

/**
 * Releases a quantity of repellent depending on the repellentRate, 
 * repellentThreshold and available energy. The repellent is added to the
 * repellent distribution.
 * \warning Does not check whether the nutrient is below the repellent
 * threshold, this is done in UpdateDistributions.
 */
void Bacterium::ReleaseRepellent()
{
	if (repellentDist == NULL) return;

	double amount = (repellentThreshold - (repellentThreshold - currentNutrient)) 
				  / repellentThreshold * repellentRate;
	double cost = amount * repellentCost;

	if (energy >= cost) energy -= cost;
	else {
		// If the amount is too expensive, release as much as possible.
		amount = energy / repellentCost;
		energy = 0.0;
	}

	repellentDist->AddDensity(GetLocation(), static_cast<DistReal>(amount));
}

/**
 * Calculates the average velocity of the individuals within swarmRadius of
 * the current Bacterium.
 */
Vector2D Bacterium::GetSwarmVelocity()
{
	return swarmSize > 0 
		? swarmTotalVel *= 1.0 / static_cast<double>(swarmSize)
		: GetVelocity();
}

/**
 * Returns the current velocity when the tumbling movement system is used.
 */
Vector2D Bacterium::GetTumblingVelocity()
{
	--nextCheck;
	if (nextCheck <= 0) {
		SetNextCheck();
		if (nextCheck <= 0) tumblingVelocity.SetAngle(randval(TWOPI));
	}
	return tumblingVelocity;
}

/**
 * Returns the local gradient of the nutrient distribution.
 * \return A normalised vector representing the direction of the gradient.
 */
Vector2D Bacterium::GetNutrientGradient()
{
	return nutrientDist != NULL
		? nutrientDist->GetGradient(GetLocation()).GetNormalised()
		: Vector2D(0.0, 0.0);
}

/**
 * Returns the local gradient of the attractant distribution.
 * \return A normalised vector representing the direction of the gradient.
 */
Vector2D Bacterium::GetAttractantGradient()
{
	return attractantDist != NULL
		? attractantDist->GetGradient(GetLocation()).GetNormalised()
		: Vector2D(0.0, 0.0);
}

/**
 * Returns the local gradient of the repellent distribution.
 * \return A normalised vector representing the direction of the gradient.
 */
Vector2D Bacterium::GetRepellentGradient()
{
	return repellentDist != NULL
		? repellentDist->GetGradient(GetLocation()).GetNormalised()
		: Vector2D(0.0, 0.0);
}

/**
 * Accounts for the usual energy depletion and decides whether or not to
 * sporulate or unsporulate depending on the 
 */ 
void Bacterium::UpdateEnergy()
{
	if (!isSpore) energy -= energyRate * fabs(GetMaxSpeed());
	energy -= sporeEnergyRate;

	// Sporulate depending on energy.
	isSpore = energy <= sporulationThreshold;

	// Ensure energy never goes below 
	if (energy <= 0.0) energy = 0.0;

	// If the energy level is below the deathThreshold, die next frame.
	if (energy <= deathThreshold) {
		if (trailDist != NULL) trailDist->SetDensity(GetLocation(), 1.0);
		SetDead(true);
	}

	// If there is enough energy to reproduce, do so.
	if (energy >= reproductionThreshold &&
		energy >= reproductionCost) Reproduce();
}

/**
 * This should be called at the end of the Update method to update the
 * location based on the calculated velocity. Also, the swarm process
 * variables are reset.
 */
void Bacterium::FinishUpdate()
{
	// Update the location with the velocity.
	OffsetLocation(GetVelocity() * GetTimeStep());

	swarmSize = 0;
	swarmTotalVel.x = swarmTotalVel.y = 0.0;
}

/**
 * Overloaded to stop bacteria from bothering to check for collisions with
 * distribution objects.
 */
void Bacterium::Interact(WorldObject* obj)
{
	if (dynamic_cast<Distribution*>(obj)) return;
	Animat::Interact(obj);
}

/**
 * Overloaded to ensure that bacteria keep track of the velocities of
 * individuals in their local neighbourhood (determined by swarmRadius).
 */
void Bacterium::UniInteract(WorldObject* obj)
{
	Bacterium* b;
	if (IsKindOf(obj, b) && (b->GetLocation() - GetLocation()).GetLengthSquared()
							< (swarmRadius * swarmRadius)) {
		swarmTotalVel += b->GetVelocity();
		++swarmSize;
	}

	Animat::UniInteract(obj);
}

/**
 * Used with the Tumbling system of movement, this method calculates the time
 * til the next sampling of the environment. This depends on the comparative
 * gradients of the nutrient, repellent and attractant distributions and the
 * tumbleTime and tumbleScale variables.
 */
void Bacterium::SetNextCheck()
{
	double	gradNutrient = currentNutrient - lastNutrient,
			gradAttractant = currentAttractant - lastAttractant,
			gradRepellent = currentRepellent - lastRepellent;

	nextCheck = static_cast<int>(tumbleTime 
								+ gradNutrient * tumbleScale * tumbleTime
								+ gradAttractant * attractantResponse * tumbleTime
								- gradRepellent * repellentResponse * tumbleTime);

	lastNutrient = currentNutrient;
}

/**
 * This will cause a duplicate of the current Bacterium to be added to the
 * World. The Bacterium's energy is reduced by reproductionCost and the
 * remaining energy is divided by two over the two resulting individuals.
 * The offspring's location is immediately behind the parent. A pointer to
 * the new individual is added to the parent's offspring list.
 * \warning This method does not check whether the Bacterium has enough energy
 * to reproduce.
 * \warning This method causes only Bacterium objects to be added to the World,
 * if you require child classes of Bacterium to be produced in reproduction,
 * overload this method.
 */
void Bacterium::Reproduce()
{
	energy -= reproductionCost;
	energy /= 2.0;
	Bacterium* baby = new Bacterium(*this);

	double o = randval(TWOPI);
	SetLocation(Vector2D(GetLocation(), GetRadius(), o));
	baby->SetLocation(Vector2D(GetLocation(), -GetRadius(), o));

	baby->Reset();
	GetWorld().Add(baby);
	offspring.push_back(baby);
}

/**
 * Recursively compiles a list of all the offspring of this Bacterium and its
 * offspring - a whole family tree. Because the list to be filled is passed in
 * by reference, this is the fastest version of the method to use.
 * \param babies The list to be filled with pointers to Bacteriums, passed by
 * reference.
 */
void Bacterium::GetOffspring(std::list<Bacterium*>& babies)const
{
	std::list<Bacterium*>::const_iterator i = offspring.begin();

	for (; i != offspring.end(); ++i) {
		babies.push_back(*i);
        (*i)->GetOffspring(babies);
	}
}

/**
 * Recursively compiles a list of all the offspring of this Bacterium and its
 * offspring - a whole family tree. Because the resulting list is returned,
 * this is a slightly slower version of the method.
 * \return A list of pointers to Bacteriums.
 */
std::list<Bacterium*> Bacterium::GetOffspring()const
{
	std::list<Bacterium*> result;
	GetOffspring(result);
	return result;
}

/**
 * Recursively compiles a list of all the offspring of this Bacterium and its
 * offspring - a whole family tree. Because the list to be filled is passed in
 * by reference, this is the fastest version of the method to use.
 * \param babies The list to be filled with pointers to const Bacteriums, passed
 * by reference.
 */
void Bacterium::GetOffspring(std::list<Bacterium const*>& babies)const
{
	std::list<Bacterium*>::const_iterator i = offspring.begin();

	for (; i != offspring.end(); ++i) {
		babies.push_back(*i);
        (*i)->GetOffspring(babies);
	}
}

/**
 * Outputs essential information about the Bacterium, N - current nutrient
 * concentration, A - current attractant concentration, R - current repellent
 * concentration, E - current energy, T - total food/energy consumed.
 * \return A string containing the specified info.
 * \see WorldObject::OnSelect for an opportunity to pass more information.
 * \see WorldObject::GetLogStream for a means of outputting more info.
 */
string Bacterium::ToString()const
{
	ostringstream out;
	out << "N: " << currentNutrient << " A: " << currentAttractant
		<< " R: " << currentRepellent << " E: " << energy
		<< " T: " << totalEnergy;
	return out.str();
}

/**
 * Outputs detailed information to the log stream (either the log window in
 * the GUI, or standard out in batch mode).
 */
void Bacterium::OnClick()
{
	GetLogStream()
	<< "Reproduction threshold: "	<< reproductionThreshold << '\n'
	<< "Consumption rate: "			<< consumptionRate << '\n'
	<< "Attractant rate: "			<< attractantRate << '\n'
	<< "Repellent rate: "			<< repellentRate << '\n'
	<< "Swarm radius: "				<< swarmRadius << '\n'
	<< "Swarm influence: "			<< swarmInfluence << '\n'
	<< "Gradient influence: "		<< gradientInfluence << '\n'
	<< "Nutrient response: "		<< nutrientResponse << '\n'
	<< "Attractant response: "		<< attractantResponse << '\n'
	<< "Repellent response: "		<< repellentResponse << '\n'
	<< "Attractant threshold: "		<< attractantThreshold << '\n'
	<< "Repellent threshold: "		<< repellentThreshold << '\n'
	<< "Radius:	"					<< GetRadius() << '\n'
	<< "Speed: "					<< GetSpeed() << "\n\n";
}

} // namespace BEAST
