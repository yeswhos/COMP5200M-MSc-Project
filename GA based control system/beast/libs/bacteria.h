/**
 * \file bacteria.h Global include for all bacteria-related classes.
 * Include this file to have access to all bacteria-related classes.
 * \author David Gordon
 */

#ifndef _BACTERIA_H_
#define _BACTERIA_H_

#include <sstream>
#include <vector>

#include "distribution.h"
#include "bacterium.h"

namespace BEAST {

/**
 * \addtogroup bacteria Bacteria simulation classes
 * This collection of classes and functors allow the user to implement a
 * variety of simulations involving bacterial chemotaxis, fractal and vortex
 * formations and simple swarming. Note that while some sensors have been
 * included, they are only there for the purpose of letting regular Animats
 * interact with distributions - bacteria have their own streamlined method.
 * @{
 */

/**
 * Plots a two-dimensional Gaussian function in a distribution or distribution
 * kernel.
 */
struct Gaussian2D : public std::binary_function<int, int, double>
{
	Gaussian2D(int centerx, int centery, 
			   double sd = 1.0, double scale = 1.0):
		cx(centerx), cy(centery), sdsq(sd * sd), s(scale){}

	double operator()(int x, int y) {
		return (s / (2 * PI * sdsq)) 
			   * exp(-((x - cx) * (x - cx) + (y - cy) * (y - cy)) / (2 * sdsq));
	}

	int cx, cy;
	double sdsq, s;
};

/**
 * Plots a two dimensional ring.
 */
struct Ring2D : public std::binary_function<int, int, double>
{
	/**
	 * Plots a two dimensional ring.
	 * \param centerx The X coordinate of the center.
	 * \param centery The Y coordinate of the center.
	 * \param OuterRadius The maximum radius of the ring.
	 * \param InnerRadius The minimum radius of the ring (radius of the hole).
	 * \param scale The value to output within the ring area.
	 */
	Ring2D(int centerx, int centery, double OuterRadius = 2.0,
			double InnerRadius = 1.0, double scale = 1.0):
	cx(centerx), cy(centery), rin2(InnerRadius*InnerRadius),
    rout2(OuterRadius*OuterRadius),	s(scale)
	{
		area = PI * (rout2-rin2);
	}

	double operator()(int X, int Y)
	{
		double x = static_cast<double>(X);
		double y = static_cast<double>(Y);

		double n = (x-cx)*(x-cx)+(y-cy)*(y-cy);
		return (s / area) * ((rin2 <=  n && n <= rout2 ) ?
				1.0 : 
				0.0);
		}

	int cx, cy;         ///< The centre of the Gaussian function
	double rin2, rout2; ///< Store the square radii
	double area, s;     ///< Store the area and scale for speed.
};

/**
 * Plots a two dimensional Gaussian ring.
 */
struct GaussianRing2D : public std::binary_function<int, int, double>
{
	/**
	 * Plots a two dimensional ring with a Gaussian function.
	 * \param centerx The X coordinate of the center.
	 * \param centery The Y coordinate of the center.
	 * \param mean The mean for the Gaussian function.
	 * \param standard deviation for the Gaussian function.
	 * \param scale The value to output within the ring area.
	 */
   GaussianRing2D(int centerx, int centery, double mean = 1.0, 
				   double stddev = 1.0, double scale = 1.0):
		cx(centerx), cy(centery), m(mean), sd(stddev),
		sdsq(stddev * stddev), s(scale), k(scale / (stddev * sqrt(TWOPI))){}

    double operator()(int X, int Y)
	{
		double x = static_cast<double>(X);
		double y = static_cast<double>(Y);

		double d = sqrt((x-cx)*(x-cx)+(y-cy)*(y-cy));

		return k * exp(-((d - m) * (d - m)) / (2.0 * sdsq));
	}

    int cx, cy;         ///< The centre of the Gaussian function.
	double m;			///< Store the mean, which is the radius.
	double sd, sdsq;	///< The standard deviation and square thereof.
    double s;			///< Store the scale.
	double k;
};

/**
 * Plots uniform noise in a distribution.
 */
struct UniformNoise : public std::binary_function<int, int, double>
{
	/**
	 * \param minimum The lowest value of noise.
	 * \param maximum The highest value of noise.
	 */
	UniformNoise(double minimum, double maximum):
		range(maximum - minimum), mod(minimum){}
	double operator()(int, int) { return randval(range) + mod; }
	double range, mod;
};

/**
 * Plots normally distributed noise in a distribution.
 */
struct GaussianNoise : public std::binary_function<int, int, double>
{
	/**
	 * \param mean The mean value of the noise.
	 * \param sd The standard deviation of the noise.
	 * \warning Can return negative values.
	 */
	GaussianNoise(double mean, double stddev): m(mean), sd(stddev){}
	double operator()(int, int) { return gaussrand<double>() * sd + m; }
	double m, sd;
};

/**
 * @}
 */

} // namespace BEAST

#endif
