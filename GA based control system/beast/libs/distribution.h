/**
 * \file distribution.h Implements a two-dimensional density distribution.
 * \author David Gordon
 */

#ifndef _DISTRIBUTION_H_
#define _DISTRIBUTION_H_

#include "sensor.h"
#include "world.h"

namespace BEAST {

/**
 * \addtogroup bacteria
 * @{
 */

/** 
 * For speed, Distributions use floats but this typedef makes it
 * possible to switch to doubles if higher accuracy is required.
 */
typedef float DistReal;

/**
 * Implements a grid which stores spatial density information to a specified
 * resolution, e.g. nutrients in an agar dish. Diffusion is made possible
 * using a fast convolution function. The distribution can be accessed directly
 * or through a number of sensors.
 */
class Distribution : public WorldObject
{
	typedef DistReal* DistType;

public:

	/**
	 * Implements diffusion and other neighbourhood operations.
	 */
	struct Kernel {
		Kernel(int w, int h):
			width(w / 2 * 2 + 1), height(h / 2 * 2 + 1),
			kernel(new float[w * h])
		{
		}
		~Kernel()
		{
			delete kernel;
		}
		void			SetDistribution(Distribution*);
		void 			Set(int x, int y, DistReal v) { kernel[y * width + x] = v; }
		void 			SetDivisor(DistReal d);
		void			Normalise();
		/// Returns the value at coordinate x, y of the Kernel.
		float			Get(int x, int y) { return kernel[y * width + x]; }
		void			Filter(Distribution*)const;

		/**
		 * Plots the specified function (e.g. Gaussian2D) in the kernel.
		 * \param func	Function with (int, int) input and float output.
		 */
		template <class _Func>
		void Plot(_Func func)
		{
			DistReal* d = kernel;
			int x = 0;
			for (int y = 0; y < width; ++y)
				for (x = 0; x < height; ++x, ++d)
					*d = static_cast<DistReal>(func(x, y));
		}

		int width, height, widthjump, corner;
		DistReal* kernel;
		mutable DistReal const* k;
		mutable DistReal* d;
		mutable int i, j;
	};

	friend struct Kernel;

	Distribution(int c, int r, int b = 1);
	virtual ~Distribution();
	virtual void	Init();
	/**
	 * Filters the distribution every n frames, where n is specified by the
	 * diffusion speed.
	 * \see SetDiffusionSpeed
	 */
	virtual void	Update()
	{
		if (diffusionSpeed > 0 && --nextDiffusion <= 0) {
			nextDiffusion = diffusionSpeed;
			kernel->Filter(this);
		}
	}

	/// Provides access to the kernel so new convolutions can be plotted.
	Kernel&			GetKernel()const	{ return *kernel; }

	/// Provides direct access to distribution data by column and row.
	inline DistReal&ValueAt(int x, int y)const 
					{ return distribution[y * tCols + x]; }

	/// Returns the density at the specified column and row.
	DistReal		GetDensity(int x, int y)const
	{	
		x += border;
		y += border;
		return ValueAt(x, y);
	}
	DistReal		GetDensity(const Vector2D& v)const;
	DistReal		GetGradient(const Vector2D& v, double o)const;
	Vector2D		GetGradient(int x, int y)const;
	Vector2D		GetGradient(const Vector2D& v)const;

	void			SetDensity(int x, int y, float d) { ValueAt(x+border, y+border) = d; }
	void			SetDensity(const Vector2D& v, float d);

	void			AddDensity(int x, int y, float d) { ValueAt(x+border, y+border) += d; }
	void			AddDensity(const Vector2D& v, float d);

	/**
	 * Sets the interval for calling diffusion, e.g. every 2 timesteps.
	 * 0 disables diffusion altogether.
	 */
	void			SetDiffusionSpeed(int s)	{ diffusionSpeed = s; nextDiffusion = s; }
	/**
	 * Specifies the rate at which the distribution decays: 1.0 for no decay,
	 * 0.5 to reduce by half on every diffusion.
	 */
	void			SetDecayRate(DistReal r)	{ kernel->SetDivisor(r); }

	/**
	 * Specifies the maximum expected concentration for purposes of display.
	 */
	void			SetMaxConc(float f)			{ maxConc = f; }

	virtual void	Render();

	void			Plot(double val);

	/**
	 * Plots the specified function so the value at each coordinate x, y becomes
	 * func(x, y).
	 * \param func	Any function with input (int, int) output float.
	 */
	template <class _Func>
	void Plot(_Func func)
	{
		DistType	d = distribution + border * tCols,
					s = swapbuffer + border * tCols;
		int x = 0;
		for (int y = 0; y < rows; ++y) {
			d += border;
			s += border;
			for (x = 0; x < cols; ++x, ++d, ++s)
				*d = *s = static_cast<DistReal>(func(x, y));
			d += border;
			s += border;
		}
	}

	/**
	 * Replaces the value at each coordinate with op(oldval, func(x, y). This
	 * allows functions to be added and subtracted from the distribution, e.g.
	 * \code dist.Filter(plus<double>(), GaussianNoise(0.0, 1.5)) \endcode
	 * \param func	Any function with input (int, int) output float.
	 */
	template <class _Op, class _Func>
	void Filter(_Op op, _Func func)
	{
		DistType	d = distribution + border * tCols,
					s = swapbuffer + border * tCols;
		int x = 0;
		for (int y = 0; y < rows; ++y) {
			d += border;
			s += border;
			for (x = 0; x < cols; ++x, ++d, ++s)
				*d = *s = static_cast<DistReal>(op(*d, func(x, y)));
			d += border;
			s += border;
		}
	}

	/**
	 * Replaces each value of the distribution with op(oldval).
	 * \param op	A unary operator with input and output float.
	 */
	template <class _Op>
	void Filter(_Op op)
	{
		DistType	d = distribution + border * tCols,
					s = swapbuffer + border * tCols;
		int x = 0;
		for (int y = 0; y < rows; ++y) {
			d += border;
			s += border;
			for (x = 0; x < cols; ++x, ++d, ++s)
				*d = *s = static_cast<DistReal>(op(*d));
			d += border;
			s += border;
		}
	}

protected:
	double		width,			///< The real-valued width (same as World).
				height,			///< The real-valued height (same as World).
				colSize,		///< The real-valued width of each column.
				rowSize;		///< The real-valued height of each row.
	int			rows,			///< The number of accessible rows.
				cols,			///< The number of accessible columns.
				tRows,			///< The total rows including convolution border.
				tCols,			///< The total columns including convolution border.
				border;			///< The size of the convolution border (used to make neighbourhood operations work).
	float		maxConc;		///< The maximum concentration, for display purposes.

	int			diffusionSpeed;	///< How often (in timesteps) the diffusion kernel is used.

	int			nextDiffusion;	///< Timesteps left til the next diffusion.

	DistType	distribution, swapbuffer;	///< Dynamic arrays containing distribution data.
	Kernel*		kernel;			///< Used for diffusion.

};

struct EvalDensity : public SensorEvalFunction {
	EvalDensity(WorldObject* o): owner(o){}
	virtual void operator()(WorldObject* o, const Vector2D&) {
		density = dynamic_cast<Distribution*>(o)->GetDensity(owner->GetLocation());
	}
	virtual double GetOutput()const {
		return static_cast<double>(density);
	}
	DistReal density;
	WorldObject* owner;
};

struct ScaleGradient : public SensorScaleFunction {
	virtual double operator()(double n) {
		return 2.0 * atan(n) / PI;
	}
};

struct EvalGradient : public SensorEvalFunction {
	EvalGradient(WorldObject* o): owner(o){}
	virtual void operator()(WorldObject* o, const Vector2D&) {
		gradient = dynamic_cast<Distribution*>(o)->GetGradient(owner->GetLocation(), owner->GetOrientation());
	}
	virtual double GetOutput()const { return static_cast<double>(gradient); }
	DistReal gradient;
	WorldObject* owner;
};

Sensor* GradientSensor();
Sensor* DistributionSensor();

struct ZeroDistribution : public std::binary_function<int, int, double>
{	double operator()(int x, int y) { return 0.0; }	};

struct LimitDistribution : public std::unary_function<double, double>
{	
	LimitDistribution(double L, double U): l(L), u(U){}
	double operator()(double d) { return limit(l, u, d); }
	double l, u;
};

/**
 * @}
 */

} // namespace BEAST

#endif

