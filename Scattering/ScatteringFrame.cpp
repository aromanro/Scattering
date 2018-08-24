#include "ScatteringFrame.h"

#include "Scattering.h"

#include "OptionsFrame.h"


#include <vtkAutoInit.h>

#include <thread>


VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingContextOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

#define MY_VTK_WINDOW 102

#define ID_CALCULATE 105

wxBEGIN_EVENT_TABLE(ScatteringFrame, wxFrame)
EVT_MENU(ID_CALCULATE, ScatteringFrame::OnCalculate)
EVT_UPDATE_UI(ID_CALCULATE, ScatteringFrame::OnUpdateCalculate)
EVT_MENU(wxID_EXIT, ScatteringFrame::OnExit)
EVT_MENU(wxID_PREFERENCES, ScatteringFrame::OnOptions)
EVT_MENU(wxID_ABOUT, ScatteringFrame::OnAbout)
EVT_TIMER(101, ScatteringFrame::OnTimer)
EVT_ERASE_BACKGROUND(ScatteringFrame::OnEraseBackground)
wxEND_EVENT_TABLE()


ScatteringFrame::ScatteringFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame(NULL, wxID_ANY, title, pos, size),
	timer(this, 101), runningThreads(0)
{
	wxMenu *menuFile = new wxMenu;

	menuFile->Append(ID_CALCULATE, "C&alculate\tCtrl+a", "Starts computing");
	menuFile->Append(wxID_SEPARATOR);
	menuFile->Append(wxID_EXIT);

	wxMenu *menuView = new wxMenu;
	menuView->Append(wxID_PREFERENCES);

	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuView, "&View");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Welcome to Scattering!");

	m_pVTKWindow = new wxVTKRenderWindowInteractor(this, MY_VTK_WINDOW);
	m_pVTKWindow->UseCaptureMouseOn();
	//m_pVTKWindow->DebugOn();
	m_pVTKWindow->DebugOff();

	ConstructVTK();

	std::vector<std::pair<double, double>> empty_results;
	ConfigureVTK("", empty_results);
	
	currentOptions.Open();
	currentOptions.Load();
}


ScatteringFrame::~ScatteringFrame()
{
	DestroyVTK();
	if (m_pVTKWindow) m_pVTKWindow->Delete();
}

void ScatteringFrame::ConstructVTK()
{
	pRenderer = vtkRenderer::New();
	pContextView = vtkContextView::New();

	vtkRenderWindow *pRenderWindow = m_pVTKWindow->GetRenderWindow();
	pRenderWindow->AddRenderer(pRenderer);
	pContextView->SetInteractor(pRenderWindow->GetInteractor());
	//pContextView->GetInteractor()->Initialize();

	pChart = vtkChartXY::New();
	pChart->SetRenderEmpty(true);		
	pContextView->GetScene()->AddItem(pChart);
}

void ScatteringFrame::DestroyVTK()
{
	if (pChart) pChart->Delete();
	if (pRenderer) pRenderer->Delete();
	if (pContextView) pContextView->Delete();
}


void ScatteringFrame::OnCalculate(wxCommandEvent& /*event*/)
{
	Compute();
}

void ScatteringFrame::OnUpdateCalculate(wxUpdateUIEvent& event)
{
	event.Enable(isFinished());
}


void ScatteringFrame::ConfigureVTK(const std::string& name, const std::vector<std::pair<double, double>>& results)
{
	pChart->ClearPlots();

	if (name.empty()) pChart->SetTitle("Scattering Cross Section");
	else
	{
		std::string Name = name;
		Name += " Scattering Cross Section";
		pChart->SetTitle(Name.c_str());
	}


	pChart->SetAutoAxes(false);

	pChart->GetAxis(vtkAxis::BOTTOM)->SetTitle("Energy (meV)");
	pChart->GetAxis(vtkAxis::LEFT)->SetTitle("Cross Section (rho^2)");

	if (results.empty()) return;

	int numPoints = results.size();


	// set up the data table

	vtkNew<vtkTable> table;

	vtkNew<vtkFloatArray> arrX;
	arrX->SetName("X");
	table->AddColumn(arrX.GetPointer());

	vtkNew<vtkFloatArray> arrC;
	arrC->SetName("Y");
	table->AddColumn(arrC.GetPointer());

	table->SetNumberOfRows(numPoints);


	// set values for X axis column
	for (int i = 0; i < numPoints; ++i)
	{
		table->SetValue(i, 0, results[i].first);
		table->SetValue(i, 1, results[i].second);
	}


	// add the line to the chart

	vtkPlot *line = pChart->AddPlot(vtkChart::LINE);
	// Use columns 0 and 1 for x and y
	line->SetInputData(table.GetPointer(), 0, 1);
	  
	// Make the plot red, with a width of 2.0 pixels
	line->SetColor(255, 0, 0, 255);
	line->SetWidth(2.0);		
}

bool ScatteringFrame::isFinished() const
{
	return 0 == runningThreads;
}


void ScatteringFrame::OnOptions(wxCommandEvent& /*event*/)
{	
	OptionsFrame *optionsFrame = new OptionsFrame("Options", this);
	optionsFrame->options = currentOptions;
	if (wxID_OK == optionsFrame->ShowModal())
	{
		currentOptions = optionsFrame->options;
		currentOptions.Save();
	}

	delete optionsFrame;	
}


void ScatteringFrame::Compute()
{
	if (!isFinished()) return;

	wxBeginBusyCursor();

	
	computeOptions = currentOptions;

	SetTitle("Computing - Scattering");

	runningThreads = 1;

	std::thread([this]()
	{
		results = Scattering::Scattering::Compute(computeOptions);

		runningThreads = 0;
	}).detach();

	timer.Start(100);	
}


void ScatteringFrame::OnTimer(wxTimerEvent& WXUNUSED(event))
{	
	if (isFinished())
	{
		timer.Stop();
		StopThreads();

		SetTitle("Finished - Scattering");

		Refresh();
	}
}

void ScatteringFrame::OnEraseBackground(wxEraseEvent &event)
{
  event.Skip(false);
}



void ScatteringFrame::OnExit(wxCommandEvent& /*event*/)
{
	Close(true);
}

void ScatteringFrame::OnAbout(wxCommandEvent& /*event*/)
{
	wxMessageBox("Scattering ver 1.0", "About Scattering", wxOK | wxICON_INFORMATION);
}


void ScatteringFrame::StopThreads(bool cancel)
{
	SetTitle("Finished - Scattering");


	if (!cancel)
		ConfigureVTK(computeOptions.scatteringPairs[computeOptions.scatteringPair].pairName, results);


	if (wxIsBusy()) wxEndBusyCursor();
}