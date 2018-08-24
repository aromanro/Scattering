#pragma once

// the referred formulae are from the book
// Computational Physics by J M Thijssen
// isbn: 9780521833462, https://doi.org/10.1017/CBO9781139171397
// the theory is in chapter 2

// the parameters and theory and experimental data can be found in
// Molecular beam scattering studies of orbiting resonances and the determination of van der Waals potentials for H–Ne, Ar, Kr, and Xe and for H2–Ar, Kr, and Xe
// by J. Peter Toennies, Wolfgang Welz, and Günther Wolf
// The Journal of Chemical Physics 71, 614 (1979), https://doi.org/10.1063/1.438414


#include <string>

namespace Scattering
{

	class ScatteringPair
	{
	public:
		ScatteringPair(const std::string& name = "", double eps = 5.9, double r = 3.57, double mass1 = 1., double mass2 = 84.)
			: pairName(name), epsilon(eps), rho(r), m1(mass1), m2(mass2)
		{
		}

		std::string pairName;

		double epsilon;
		double rho;

		double m1;
		double m2;
	};

}

