#pragma once

// the referred formulae are from the book
// Computational Physics by J M Thijssen
// isbn: 9780521833462, https://doi.org/10.1017/CBO9781139171397
// the theory is in chapter 2

// the parameters and theory and experimental data can be found in
// Molecular beam scattering studies of orbiting resonances and the determination of van der Waals potentials for H–Ne, Ar, Kr, and Xe and for H2–Ar, Kr, and Xe
// by J. Peter Toennies, Wolfgang Welz, and Günther Wolf
// The Journal of Chemical Physics 71, 614 (1979), https://doi.org/10.1063/1.438414


#include <tuple>
#include <limits>


#include "Potential.h"

namespace Scattering
{

	class Function
	{
	public:
		explicit Function(const Potential& pot) : m_pot(pot) {}

		// see 2.10 and 2.11, note that 2 m / hbar^2 is not 1, so this is not F, but 2 m / hbar^2 * F
		// the constant is 2 m / hbar^2, where m is the reduced mass
		inline double operator()(unsigned int l, double E, double position) const
		{
			const double effectivePotential = m_pot.getConstant() * m_pot(position) + l * (l + 1.) / (position * position);

			return  effectivePotential - m_pot.getConstant() * E;
		}

	protected:
		const Potential& m_pot;
	};

	class Numerov
	{
	public:
		explicit Numerov(const Potential& pot) : function(pot) {}

		inline std::tuple<double, double, double, double> SolveSchrodinger(double startPoint, double startValue, double nextPoint, double nextValue, unsigned int l, double E, unsigned int steps, double delta) const
		{
			const double h = nextPoint - startPoint;
			const double h2 = h * h;

			double wprev = startValue;
			double w = nextValue;

			double position = nextPoint;
			double funcVal = function(l, E, position);
			double solution = (1 - h2 / 12. * funcVal) * nextValue;


			for (unsigned int i = 0; i < steps; ++i)
			{
				const double wnext = 2. * w - wprev + h2 * solution * funcVal;
				position += h;
				wprev = w;
				w = wnext;
				funcVal = function(l, E, position);
				solution = getU(w, funcVal, h2);
			}

			const double oldpos = position;
			const double oldsol = solution;

			const double newLimit = oldpos + delta;
			do
			{
				const double wnext = 2. * w - wprev + h2 * solution * funcVal;
				position += h;
				wprev = w;
				w = wnext;
				funcVal = function(l, E, position);
				solution = getU(w, funcVal, h2);
			} while (position < newLimit);

			return std::tuple<double, double, double, double>(oldpos, oldsol, position, solution);
		}

		inline double getValue(unsigned int l, double E, double pos) const
		{
			return function(l, E, pos);
		}

	protected:
		// 2.13
		static inline double getU(double w, double funcVal, double h2)
		{
			return w / (1. - h2 / 12. * funcVal);
		}

		Function function;
	};

}
