#pragma once

// the referred formulae are from the book
// Computational Physics by J M Thijssen
// isbn: 9780521833462, https://doi.org/10.1017/CBO9781139171397
// the theory is in chapter 2

// the parameters and theory and experimental data can be found in
// Molecular beam scattering studies of orbiting resonances and the determination of van der Waals potentials for H–Ne, Ar, Kr, and Xe and for H2–Ar, Kr, and Xe
// by J. Peter Toennies, Wolfgang Welz, and Günther Wolf
// The Journal of Chemical Physics 71, 614 (1979), https://doi.org/10.1063/1.438414

namespace Scattering
{

	class Potential
	{
	public:
		virtual double operator()(double position) const = 0;
		virtual double getConstant() const { return 1; }
	};


	// just an example for another potential, it was used for tests while implementing the code
	// see the book for details
	class HarmonicPotential : public Potential
	{
	public:
		virtual double operator()(double position) const override
		{
			return position * position;
		}

	};


	class LennardJonesPotential : public Potential
	{
	public:
		// pass meV and Angstroms, the atomic mass in Daltons
		// default parameters for H-Kr
		LennardJonesPotential(double epsilon = 5.9, double rho = 3.57, double m1 = 1., double m2 = 84.) : m_epsilon(epsilon), m_rho(rho)
		{
			// use atomic units so hbar is 1
			m_epsilon /= 1000. * 27.211385; //meV converted to Hartrees
			m_rho /= 0.52917721092; // Angstroms converted to Bohrs

			// I think the chart in the book uses 1 for this, if a Hydrogen atom and a heavy one are involved it's a good approximation
			const double reducedMass = m1 * m2 / (m1 + m2);

			// this is 1 / (hbar^2 / (2m)) = 2 m / hbar^2
			Constant = 2. * reducedMass * 1822.888486192; // also convert the mass to have the electron mass as unit
		}


		virtual double operator()(double position) const override
		{
			const double rhor = m_rho / position;

			return m_epsilon * (pow(rhor, 12.) - 2. * pow(rhor, 6.));
		}

		inline double SolutionForSmallR(double r) const
		{
			const double C = sqrt(Constant * m_epsilon / 25.);

			return exp(-C * pow(r, -5)); // see eq 2.17
		}

		// it's just the derivative of the above function
		inline double DerivativeForSmallR(double r) const
		{
			const double C = sqrt(Constant * m_epsilon / 25.);

			return 5. * C * pow(r, -6) * SolutionForSmallR(r);
		}

		inline double getEpsilon() const { return m_epsilon; }

		inline double getRho() const { return m_rho; }

		virtual double getConstant() const override { return Constant; }


	protected:
		double Constant;

		double m_epsilon;
		double m_rho;
	};

}

