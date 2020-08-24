/**
 * \file bacterium.h The interface for the Bacterium class.
 * \author David Gordon
 */

#ifndef _BACTERIUM_H_
#define _BACTERIUM_H_

#include "animat.h"
#include "distribution.h"
#include "utilities.h"

#include <list>

namespace BEAST {

/**
 * \addtogroup bacteria
 * @{
 */

/**
 * \par
 * A simplified model of a bacteria strain capable of self-organising through
 * chemotaxis.
 * \par
 * In fact, this class represents, in simulation, a large number of bacteria
 * with a global average velocity. The Bacterium takes nutrients from a
 * nutrient Distribution object, and releases chemoattractant and
 * chemorepellent into two other Distribution objects. The precise model of
 * the dynamics depends on a range of coefficients, some of which might be
 * optimised using a GA.
 * \par
 * At the Update stage, the Bacterium checks the level of nutrient, attractant
 * and repellent in its local environment. It adjusts its direction according
 * to the local gradients of these three quantities, their influence determined
 * by [nutrientResponse], [attractantResponse] and [repellentResponse]. This new
 * direction influences the overall velocity according to [gradientInfluence],
 * which ranges between 0 (no influence) and 1 (full influence, original velocity
 * is forgotten).
 * \par
 * The velocity is also influenced by [swarmInfluence] (ranging between 0 and 1
 * again) of the average velocity of bacteria in the local neighbourhood,
 * dictated by [swarmRadius].
 * \par
 * The Bacterium will absorb a quantity of nutrient from the local environment,
 * depending on its [consumptionRate] and the amount present - it will take all
 * it can each timestep, up to [consumptionRate]. Nutrients are converted directly
 * into energy, so the amount of nutrient consumed is simply added to the [energy]
 * level.
 * \par
 * The Bacterium may release attractant or repellent, depending on
 * [attractantThreshold], [repellentThreshold] and the amount of nutrient present.
 * If it does so, the amount released is dictated by the product of local
 * nutrient with [attractantRate] or [repellentRate]. The Bacterium's energy is
 * reduced according to [attractantCost] or [repellentCost].
 * Normal cell processes and the cost of moving around deplete the Bacterium's
 * energy level, this is modelled by simply subtracting [energyRate] each
 * timestep.
 * \par
 * Each timestep each Bacterium loses sporeEnergyRate energy. If it is moving
 * (i.e. not sporulated), it also loses its velocity * energyRate.
 * When sporulated, it will continue to absorb nutrients but will not move or
 * release repellents or attractants.
 * \par
 * If the local nutrient level goes up, the Bacterium will start moving again.
 * If the Bacterium's [energy] level goes above the [reproductionThreshold], the
 * Bacterium will lose [reproductionCost] energy, and then split into two. The
 * new 'offspring' Bacterium will be the same colour and have the same
 * constants and coefficients. The energy of the original Bacterium is split
 * between the two.
 * \par
 * If the Bacterium's energy level drops below [deathThreshold], the Bacterium
 * will die, and is then removed from the simulation.
 * \par
 * For more detailed information on how bacteria actually move, go to this
 * address:
 * \link http://www.mansfield.ohio-state.edu/~sabedon/biol1084.htm#taxis
 */
class Bacterium : public Animat
{
public:
	Bacterium();
	virtual ~Bacterium();

	virtual void	Update();
	/// Override the Animat drawing function and return to a simple blob shape.
	virtual void	Draw() { return Drawable::Draw(); }
	virtual void	Interact(WorldObject*);
	virtual void	UniInteract(WorldObject*);

	void			CheckBoundary();
	void			ReadDistributions();
	void			UpdateDistributions();
	void			ReleaseAttractant();
	void			ReleaseRepellent();
	Vector2D		GetSwarmVelocity();
	Vector2D		GetTumblingVelocity();
	Vector2D		GetNutrientGradient();
	Vector2D		GetAttractantGradient();
	Vector2D		GetRepellentGradient();
	void			UpdateEnergy();
	void			FinishUpdate();

	void			SetNextCheck();
	void			Reproduce();

	void			GetOffspring(std::list<Bacterium*>&)const;
	std::list<Bacterium*> GetOffspring()const;
	void			GetOffspring(std::list<Bacterium const*>&)const;
	void			Reset()		{ offspring.clear(); }
	virtual std::string	ToString()const;
	virtual	void	OnClick();

	void			SetReproductionCost(double r)	{ reproductionCost = r >= 0.0 ? r : 0.0;  }
	void			SetEnergyRate(double e)			{ energyRate = e >= 0.0 ? e : 0.0; }
	void			SetSporeEnergyRate(double s)	{ sporeEnergyRate = s >= 0.0 ? s : 0.0; }
	void			SetAttractantCost(double a)		{ attractantCost = a >= 0.0 ? a : 0.0; }
	void			SetRepellentCost(double r)		{ repellentCost = r >= 0.0 ? r : 0.0; }
	void			SetDeathThreshold(double d)		{ deathThreshold = d; }

	void			SetTumbleTime(double t)			{ tumbleTime = t >= 0.0 ? t : 0.0; }
	  //	void			SetTumbleScale(double t)		{ tumbleScale; }
	void			SetReproductionThreshold(double r)	{ reproductionThreshold = r; }
	void			SetSporulationThreshold(double s){ sporulationThreshold = s; }
	void			SetConsumptionRate(double c)	{ consumptionRate = c >= 0.0 ? c : 0.0; }
	void			SetAttractantRate(double a)		{ attractantRate = a >= 0.0 ? a : 0.0; }
	void			SetRepellentRate(double r)		{ repellentRate = r >= 0.0 ? r : 0.0; }
	void			SetSwarmRadius(double s)		{ swarmRadius = s >= 0.0 ? s : 0.0; }
	void			SetSwarmInfluence(double s)		{ swarmInfluence = limit(0.0, 1.0, s); }
	void			SetGradientInfluence(double g)	{ gradientInfluence = limit(0.0, 1.0, g); }
	void			SetNutrientResponse(double n)	{ nutrientResponse = n >= 0.0 ? n : 0.0; }
	void			SetAttractantResponse(double a)	{ attractantResponse = a >= 0.0 ? a : 0.0; }
	void			SetRepellentResponse(double r)	{ repellentResponse = r >= 0.0 ? r : 0.0; }
	void			SetAttractantThreshold(double a){ attractantThreshold = a; }
	void			SetRepellentThreshold(double r)	{ repellentThreshold = r; }

	void			SetSpeed(double s)				{ SetMinSpeed(s); SetMaxSpeed(s); }

	void			SetEnergy(double e)				{ energy = e; }
	void			SetTotalEnergy(double e)		{ totalEnergy = e; }

	void			SetNutrientDist(Distribution* dist)		{ nutrientDist = dist; }
	void			SetAttractantDist(Distribution* dist)	{ attractantDist = dist; }
	void			SetRepellentDist(Distribution* dist)	{ repellentDist = dist; }
	void			SetTrailDist(Distribution* dist)		{ trailDist = dist; }

	double			GetReproductionCost()const		{ return reproductionCost; }
	double			GetEnergyRate()const			{ return energyRate; }
	double			GetSporeEnergyRate()const		{ return sporeEnergyRate; }
	double			GetAttractantCost()const		{ return attractantCost; }
	double			GetRepellentCost()const			{ return repellentCost; }
	double			GetDeathThreshold()const		{ return deathThreshold; }

	double			GetTumbleTime()const			{ return tumbleTime; }
	double			GetTumbleScale()const			{ return tumbleScale; }
	double			GetReproductionThreshold()const { return reproductionThreshold; }
	double			GetSporulationThreshold()const	{ return sporulationThreshold; }
	double			GetConsumptionRate()const		{ return consumptionRate; }
	double			GetAttractantRate()const		{ return attractantRate; }
	double			GetRepellentRate()const			{ return repellentRate; }
	double			GetSwarmRadius()const			{ return swarmRadius; }
	double			GetSwarmInfluence()const		{ return swarmInfluence; }
	double			GetGradientInfluence()const		{ return gradientInfluence; }
	double			GetNutrientResponse()const		{ return nutrientResponse; }
	double			GetAttractantResponse()const	{ return attractantResponse; }
	double			GetRepellentResponse()const		{ return repellentResponse; }
	double			GetAttractantThreshold()const	{ return attractantThreshold; }
	double			GetRepellentThreshold()const	{ return repellentThreshold; }

	double			GetSpeed()const					{ return GetMaxSpeed(); }

	double			GetEnergy()const				{ return energy; }
	double			GetTotalEnergy()const			{ return totalEnergy; }
	bool			IsSpore()const					{ return isSpore; }

protected:
	// Predefined constants
	double			reproductionCost,		///< Cost of splitting into two.
					energyRate,				///< Rate of energy reduction (subtracted each frame).
					sporeEnergyRate,		///< Rate of energy reduction when sporulated (subtracted each frame).
					attractantCost,			///< Cost to release one unit of attractant.
					repellentCost,			///< Cost to release one unit of repellent.
					deathThreshold;			///< Lowest energy level before Bacterium dies.
	// Individual variables (potentially under genetic control)
	double			tumbleTime,				///< Default time period between tumbles.
					tumbleScale,			///< Scaling factor for tumble frequency.
					reproductionThreshold,	///< Energy level before reproduction occurs.
					sporulationThreshold,	///< Energy level before sporulation.
					consumptionRate,		///< Rate of nutrient consumption.
					attractantRate,			///< Rate of attractant release.
					repellentRate,			///< Rate of repellent release.
					swarmRadius,			///< Neighbourhood radius for local swarm.
					swarmInfluence,			///< Degree of direction influence taken from swarm.
					gradientInfluence,		///< Degree of direction influence taken from gradients.
					nutrientResponse,		///< Degree of response to nutrient gradient.
					attractantResponse,		///< Degree of response to attractant gradient.
					repellentResponse,		///< Degree of response to repellent gradient.
					attractantThreshold,	///< Level of nutrient which must be present for attractant to be released.
					repellentThreshold;		///< Level of nutrient which must be present for repellent to be released.
	// Member data
	Distribution	*nutrientDist,			///< Pointer to the nutrient distribution.
					*attractantDist,		///< Pointer to the attractant distribution.
					*repellentDist,			///< Pointer to the repellent distribution.
					*trailDist;				///< Pointer to the trail distribution.
	std::list<Bacterium*> offspring;		///< List of pointers to offspring bacteria.

	// Process variables
	double			energy,					///< Internal energy level.
					totalEnergy,			///< Global energy total.
					lastNutrient,			///< Last recorded nutrient level.
					lastAttractant,			///< Last recorded attractant level.
					lastRepellent,			///< Last recorded repellent level.
					currentNutrient,		///< Nutrient level at current location.
					currentAttractant,		///< Attractant level at current location.
					currentRepellent;		///< Repellent level at current location.
	int				nextCheck;				///< Frames til next nutrient sampling.
	Vector2D		tumblingVelocity,		///< Tumbling velocity.
					swarmTotalVel;			///< Total neighbourhood velocity.
	int				swarmSize;				///< Number of individuals in neighbourhood.
	bool			isSpore;				///< True if the bacterium has sporulated.

	IMPLEMENT_SERIALISATION("Bacterium", WorldObject)
};

/**
 * @}
 */

} // namespace BEAST

#endif
