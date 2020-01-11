#pragma once

#include <string>
#include <vector>

#define wxNEEDS_DECL_BEFORE_TEMPLATE

#include <wx/fileconf.h>

#include "ScatteringPair.h"

class Options
{
public:
	Options();
		
	// avoid double deletion of m_fileconfig at destruction if copied
	Options(const Options& other)
		:
		nrPoints(other.nrPoints),
		scatteringPair(other.scatteringPair),
		nrIntegrationSteps(other.nrIntegrationSteps),
		m_fileconfig(nullptr)
	{
	}

	Options& operator=(const Options& other)
	{
		nrPoints = other.nrPoints;
		scatteringPair = other.scatteringPair;
		nrIntegrationSteps = other.nrIntegrationSteps;
		m_fileconfig = nullptr;

		return *this;
	}


	void Load();
	void Save();

	void Open();
	void Close();

	int nrPoints;
	int scatteringPair;
	int nrIntegrationSteps;

	static const std::vector<Scattering::ScatteringPair> scatteringPairs;

protected:
	wxFileConfig *m_fileconfig;
};

