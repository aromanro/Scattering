#pragma once

#include <string>
#include <vector>

#define wxNEEDS_DECL_BEFORE_TEMPLATE

#include <wx/fileconf.h>

#include "ScatteringPair.h"

class Options
{
public:
	Options() = default;
	~Options()
	{
		delete m_fileconfig;
	}

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

	int nrPoints = 1000;
	int scatteringPair = 2;
	int nrIntegrationSteps = 1000;

	static const std::vector<Scattering::ScatteringPair> scatteringPairs;

private:
	void Open();
	void Close();

	wxFileConfig *m_fileconfig = nullptr;
};

