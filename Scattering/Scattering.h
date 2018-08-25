#pragma once

// the referred formulae are from the book
// Computational Physics by J M Thijssen
// isbn: 9780521833462, https://doi.org/10.1017/CBO9781139171397
// the theory is in chapter 2

// the parameters and theory and experimental data can be found in
// Molecular beam scattering studies of orbiting resonances and the determination of van der Waals potentials for H–Ne, Ar, Kr, and Xe and for H2–Ar, Kr, and Xe
// by J. Peter Toennies, Wolfgang Welz, and Günther Wolf
// The Journal of Chemical Physics 71, 614 (1979), https://doi.org/10.1063/1.438414


#include <algorithm>

#include "Options.h"
#include "Numerov.h"
#include "SpecialFunctions.h"

#define _USE_MATH_DEFINES
//#include <math.h>

// you can use boost for the same purpose if spherical Bessel functions are not available
#ifdef USE_BETTER_BESSEL
#include <cmath>
#endif

#include <vector>

namespace Scattering
{

	class Scattering
	{
	protected:

		inline static double PhaseShift(double E, unsigned int l, double r1, double r2, double u1, double u2, double constant)
		{
			// 2.9b
			const double K = r1 * u2 / (r2 * u1);
			const double k = sqrt(constant * E); // k = sqrt(2mE/hbar^2) where m is the reduced mass, 2m/hbar^2 is the passed constant - see the Lennard-Jones potential to see how it's calculated

			// 2.9a
			return atan((K * SpecialFunctions::Bessel::j(l, k * r1) - SpecialFunctions::Bessel::j(l, k * r2)) / (K * SpecialFunctions::Bessel::n(l, k * r1) - SpecialFunctions::Bessel::n(l, k * r2)));
		}

		// 2 pi / k, k as above
		inline static double Wavelength(double E, double constant)
		{
			return 2 * M_PI / sqrt(constant * E);
		}


		inline static double PartialCrossSection(double E, double r1, double r2, double u1, double u2, unsigned int l, double constant)
		{
			// 2.8
			const double sdl = sin(PhaseShift(E, l, r1, r2, u1, u2, constant));
			const double k2 = constant * E; // k^2, k computed as above

			return 4. * M_PI / k2 * (2. * l + 1.) * sdl * sdl;
		}

	public:

		static std::vector<std::pair<double, double>> Compute(const Options& options)
		{
			std::vector<std::pair<double, double>> results;
			results.reserve(1000);


			const ScatteringPair& pair = options.scatteringPairs[options.scatteringPair];
			::Scattering::LennardJonesPotential potential(pair.epsilon, pair.rho, pair.m1, pair.m2);

			const double rho = potential.getRho();
			const double rho2 = rho * rho;

			const double maxr = 5. * rho;
			const double startR = 0.7 * rho;
			const double h = (maxr - startR) / 1000; // 100 steps already give 'good' results (for the example in the book), use one order better
			const double h2 = h * h;

			const unsigned int steps = static_cast<unsigned int>(ceil((maxr - startR) / h));

			::Scattering::Numerov numerov(potential);

			const double startVal = potential.SolutionForSmallR(startR);

			const double energyMax = potential.getEpsilon();
			const double energyStart = energyMax / 20.;
			const double energyStep = (energyMax - energyStart) / options.nrPoints;

#ifdef USE_BETTER_BESSEL
			const unsigned int llim = 12;
#else
			const unsigned int llim = 8;
#endif

			for (double E = energyStart; E <= energyMax; E += energyStep)
			{
				double crossSection = 0;

				for (unsigned int l = 0; l <= llim; ++l)
				{
					// this works, but it's not a very good approximation, we can do better
					//const double nextVal = startVal + h * potential.DerivativeForSmallR(startR);

					//see A.54	
					const double deriv = potential.DerivativeForSmallR(startR);
					const double h2fminus = h2 * numerov.getValue(l, E, startR - h);
					const double h2fplus = h2 * numerov.getValue(l, E, startR + h);
					const double h2f = h2 * numerov.getValue(l, E, startR);

					// WARNING: The formula in the book is wrong, you can get the right one from A.52 (substitute w to have it with f and x) and A.53
					const double nextVal = ((2. + 5. * h2f / 6.) * (1. - h2fminus / 6.) * startVal + 2 * h * deriv * (1. - h2fminus / 12.)) /
						((1. - h2fplus / 12.) * (1. - h2fminus / 6.) + (1. - h2fminus / 12.) * (1. - h2fplus / 6.));

					double r1;
					double u1;
					double r2;
					double u2;
					std::tie(r1, u1, r2, u2) = numerov.SolveSchrodinger(startR, startVal, startR + h, nextVal, l, E, steps, Wavelength(E, potential.getConstant()) / 8.); // half of wavelenght does not seem to be sufficiently small, a quarter is already good

					crossSection += PartialCrossSection(E, r1, r2, u1, u2, l, potential.getConstant());
				}

				// convert in units as in the book: meV and rho^2
				results.emplace_back(std::make_pair(E * 27.211385 * 1000, crossSection / rho2));
			}

			return results;
		}
	};

}

