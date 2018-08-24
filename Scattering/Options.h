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
		
	void Load();
	void Save();

	void Open();
	void Close();

	int nrPoints;
	int scatteringPair;

	static const std::vector<Scattering::ScatteringPair> scatteringPairs;

protected:
	wxFileConfig *m_fileconfig;
};

