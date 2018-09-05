#include "ScatteringApp.h"
#include "ScatteringFrame.h"


wxIMPLEMENT_APP(ScatteringApp);


bool ScatteringApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	frame = new ScatteringFrame("Scattering", wxPoint(50, 50), wxSize(1024, 800));
	frame->Show(true);

	return true;
}