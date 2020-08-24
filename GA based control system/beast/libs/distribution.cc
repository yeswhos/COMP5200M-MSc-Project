/**
 * \file distribution.cc Implementation of the Distribution class.
 */

#include "distribution.h"
#include "bacteria.h" // For Gaussian2D

namespace BEAST {

/**
 * Sets up the Distribution with specified width, height and kernel radius.
 * The position is set to 0,0 and the Distribution resizes to the fit the
 * world (maintaining the same resolution specified by c and r). The kernel
 * is initialised as a gaussian distribution and normalised.
 * \param c The number of columns in the Distribution grid.
 * \param r The number of rows in the Distribution grid.
 * \param b The radius of the diffusion kernel, so the width and height are
 * 2 * b + 1.
 */
Distribution::Distribution(int c, int r, int b):
	WorldObject(Vector2D(0,0)),
	rows(r), cols(c),
	tRows(r + b + b), tCols(c + b + b),
	border(b), maxConc(1.0f),
	diffusionSpeed(0), nextDiffusion(1)
{
	distribution = new float[tRows * tCols];
	swapbuffer = new float[tRows * tCols];
	kernel = new Kernel(b + b + 1, b + b + 1);
	kernel->SetDistribution(this);
	kernel->Plot(Gaussian2D(b, b, static_cast<double>(b)));
	kernel->Normalise();
}

/**
 * Deletes the dynamic arrays and kernel used by the Distribution.
 */
Distribution::~Distribution()
{
	delete distribution;
	delete swapbuffer;
	delete kernel;
}

/**
 * Sets the Distribution up so that it's detectable (with the correct edge
 * vector) and has the same width and height as the world.
 */
void Distribution::Init()
{
	using namespace std;

	width = GetWorld().GetWidth();
	height = GetWorld().GetHeight();
	SetEdges(Wall::GetSides(width, height));
	colSize = width / static_cast<double>(cols);
	rowSize = height / static_cast<double>(rows);

	WorldObject::Init();
}

/**
 * Sets the Kernel up so that it can be used on the specified Distribution.
 * \param d		The Distribution this Kernel will be used on.
 */
void Distribution::Kernel::SetDistribution(Distribution* d)
{
	widthjump = d->tCols - width;
	corner = width / 2 + (height / 2) * d->tCols;
}

/**
 * Performs one pass of the Kernel over the specified Distribution.
 * Kernel operations are performed by sliding the Kernel along the cells of
 * the Distribution and replacing each value with the weighted sum of the
 * surrounding pixels, corresponding to the weights of the Kernel. That
 * probably doesn't make sense, so read about neighbourhood operations (e.g.
 * blur) in a good image processing book.
 * \param dist	The Distribution to apply the filter to.
 */
void Distribution::Kernel::Filter(Distribution* dist)const
{
	int		hw = width / 2,				// width and height are odd, so hw and
			hh = height / 2,			// hh are rounded down for border sizes
			yMax = dist->tRows - hh,	// The maximum y value to loop to.
			xMax = dist->tCols - hh;	// The maximum x value to loop to.
	// s and d are pointers which are used to cycle through the values of the
	// distribution array and the corresponding swapbuffer. They are
	// initialised to start just beyond the initial border where neighbourhood
	// operations won't work - eg at row 2 column 2 for a 5 x 5 kernel.
	DistType s = dist->swapbuffer + dist->tCols * hh,
			 d = dist->distribution + dist->tCols * hh - corner;

	// These are used in the convolution portion of the below loop.
	DistReal const* k;	// Used to iterate through the cells of the kernel.
	DistReal* dd;		// Used to iterate through window-sized distribution sections.
	int x, y, i, j;		// Looping variables.

	// Now loop through the cells of the distribution...
	for (y = hh; y < yMax ; ++y) {
		s+=hh; d+=hh;
		for (x = hw; x < xMax; ++x, ++d, ++s) {
			// Convolution happens here:
			k = kernel;	// Start at the first value of the kernel.
			dd = d;		// From the current point in the distribution.
			*s = 0.0;	// Set the current swapbuffer position to 0.0.

			for (i = 0; i < height; ++i) {
				for (j = 0; j < width; ++j, ++k, ++dd) {
					*s += *k * *dd;		// Create weighted sum of kernel x window.
				}
				dd += widthjump;		// Jump ahead to next row of distribution.
			}
		}
		s+=hh; d+=hh;	// Jumps along the distribution and swapbuffer to avoid the
						// border.
	}

	// Swap the freshly filtered buffer data into place.
	std::swap(dist->distribution, dist->swapbuffer);
}

/**
 * Sets the divisor of the kernel by dividing each value by the new divisor.
 */
void Distribution::Kernel::SetDivisor(DistReal d)
{
	for (int i=0; i<(width * height); ++i)
		kernel[i] /= d;
}

/**
 * Ensures that the values in the kernel sum to 1 so that no density is lost.
 */
void Distribution::Kernel::Normalise()
{
	DistReal n = 0.0f;

	for (int i=0; i<(width * height); ++i)
		n += kernel[i];

	for (int i=0; i<(width * height); ++i)
		kernel[i] /= n;
}

/**
 * Returns the density at the specified point.
 */
// TODO: change to linear interpolation
float Distribution::GetDensity(const Vector2D& v)const
{
	return GetDensity(static_cast<int>(v.x / colSize),
					  static_cast<int>(v.y / rowSize));
}

/**
 * Sets the density to the specified value at the given location.
 * \param v		Where in the distribution to update.
 * \param d		The new density value.
 */
void Distribution::SetDensity(const Vector2D& v, float d)
{
	return SetDensity(static_cast<int>(v.x / colSize),
					  static_cast<int>(v.y / rowSize), d);
}

/**
 * Adds the specified value to the distribution at the specified point.
 * \param v		The location to alter, in World coordinates/scale.
 * \param d		The amount to add.
 */
void Distribution::AddDensity(const Vector2D& v, float d)
{
	return AddDensity(static_cast<int>(v.x / colSize),
					  static_cast<int>(v.y / rowSize), d);
}

// Returns the Gradient at the specified point, according to the specified orientation.
DistReal Distribution::GetGradient(const Vector2D& v, double o)const
{
	Vector2D facing(0.0, 0.0, 1.0, o);
	facing.x *= width / static_cast<double>(cols);
	facing.y *= height / static_cast<double>(rows);

	return GetDensity(v + facing) - GetDensity(v - facing);
}

// Returns the gradient at the specified column and row of the distribution.
Vector2D Distribution::GetGradient(int x, int y)const
{
	if (x <= 0 || x >= cols - 1 || y <= 0 || y >= rows - 1)
		return Vector2D(0.0, 0.0);
	x += border;
	y += border;
	return Vector2D(ValueAt(x+1, y) - ValueAt(x-1, y),
					ValueAt(x, y+1) - ValueAt(x, y-1));
}

// Returns the gradient at the specified point in World coordinates.
Vector2D Distribution::GetGradient(const Vector2D& v)const
{
	return GetGradient(static_cast<int>(v.x / colSize),
					   static_cast<int>(v.y / rowSize));
}

/**
 * Displays the distribution with transparency according to density, up to
 * maxConc density (= opaque).
 * \see SetMaxConc
 */
void Distribution::Render()
{
	DistType d = distribution + tCols * border;

//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	int x = 0;
	for (int y = 0; y < rows; ++y) {
		d += border;
		for (x = 0; x < cols; ++x, ++d) {
			if (*d <= 0.0) continue;
			glColor4f(GetColour(0), GetColour(1), GetColour(2), *d / maxConc);
			glRectd(static_cast<double>(x) * colSize,
					static_cast<double>(y) * rowSize,
					static_cast<double>(x + 1) * colSize,
					static_cast<double>(y + 1) * rowSize);
		}
		d += border;
	}
	glDisable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

/// Sets every point on the distribution to the specified value.
void Distribution::Plot(double f)
{
	DistType	d = distribution,
				s = swapbuffer;
	int x = 0;
	for (int y = 0; y < tRows; ++y)
		for (x = 0; x < tCols; ++x, ++d, ++s)
			*d = *s = static_cast<DistReal>(f);
}

Sensor* GradientSensor()
{
	Sensor* s = new Sensor(Vector2D(0.0, 0.0), 0.0);
	s->SetMatchingFunction(new MatchKindOf<Distribution>);
	s->SetEvaluationFunction(new EvalGradient(s));
	s->SetScalingFunction(new ScaleCompose(new ScaleGradient, new ScaleLinear(0.01, -0.01, 0.0, 1.0)));
	
	return s;
}

Sensor* DistributionSensor()
{
	Sensor* s = new Sensor(Vector2D(0.0, 0.0), 0.0);
	s->SetMatchingFunction(new MatchKindOf<Distribution>);
	s->SetEvaluationFunction(new EvalDensity(s));
	s->SetScalingFunction(new ScaleLinear(0.0, 1.0, 0.0, 1.0));
	
	return s;
}

} // namespace BEAST

