/*
   This file is part of RMAExpress.

    RMAExpress is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    RMAExpress is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RMAExpress; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/**************************************************************************
 **
 ** File: GraphingTester.cpp
 **
 ** Copyright (C) B. M. Bolstad 2006
 **
 ** Testing framework for graphical units
 **
 ** History - Created Sept 28, 2006
 ** Sept 28- 0ct 5 - refined and added additional tests
 **
 **************************************************************************/


#include <wx/wx.h>



#include "boxplot.h"
#include "axes.h"

#define ID_TEST_BOXPLOT 10001
#define ID_TEST_DENSITYCURVE 10002
#define ID_TEST_AXES 10003
#define ID_TEST_AXES_BOX 10004
#define ID_TEST_AXES_BOX_LABELLED 10005
#define ID_TEXT_AXES_BOX_LABELLED_BOXPLOTS 10006

class GraphingTester: public wxApp
{
public:
  virtual bool OnInit();
  
};

class GraphingTesterDrawingWindow;

class GraphingTesterFrame: public wxFrame
{
public:
  GraphingTesterFrame(const wxString &title);
  
  void OnQuit(wxCommandEvent& event);
  void OnDrawBoxplot(wxCommandEvent& event);
  void OnDrawDensityCurve(wxCommandEvent& event);
  void OnDrawAxes(wxCommandEvent& event);
  void OnDrawAxesBox(wxCommandEvent& event);
  void OnDrawAxesBoxLabelled(wxCommandEvent& event);
  void OnDrawAxesBoxLabelledBoxplot(wxCommandEvent& event);
private:
  GraphingTesterDrawingWindow *MyWindow;


  DECLARE_EVENT_TABLE()
};




class GraphingTesterDrawingWindow: public wxWindow
{
public:
	GraphingTesterDrawingWindow(GraphingTesterFrame *parent);
	void OnPaint(wxPaintEvent &WXUNUSED(event));
	void OnClick(wxMouseEvent& event);
	void DrawBoxplot(wxDC& dc);	
	void DrawAxes(wxDC &dc);
	void DrawAxesBox(wxDC &dc);
	void DrawAxesBoxLabelled(wxDC &dc);
	void DrawAxesBoxLabelledBoxplot(wxDC &dc);
	void SetCurrentPlot(int curplot);
	void OnEraseBackground(wxEraseEvent &WXUNUSED(event));
private:
	int currentplot;

	GraphingTesterFrame *myParent;
	
	DECLARE_EVENT_TABLE()
};


DECLARE_APP(GraphingTester)
IMPLEMENT_APP(GraphingTester)


bool GraphingTester::OnInit()
{

  GraphingTesterFrame *frame = new GraphingTesterFrame(wxT("RMAExpress Graphing Tester"));

  frame->Show();
 
  return true;

}    



BEGIN_EVENT_TABLE(GraphingTesterFrame,wxFrame)
  EVT_MENU(wxID_EXIT,GraphingTesterFrame::OnQuit) 
  EVT_MENU(ID_TEST_BOXPLOT,GraphingTesterFrame::OnDrawBoxplot)
  EVT_MENU(ID_TEST_DENSITYCURVE,GraphingTesterFrame::OnDrawDensityCurve)
  EVT_MENU(ID_TEST_AXES,GraphingTesterFrame::OnDrawAxes)
  EVT_MENU(ID_TEST_AXES_BOX,GraphingTesterFrame::OnDrawAxesBox)  
  EVT_MENU(ID_TEST_AXES_BOX_LABELLED,GraphingTesterFrame::OnDrawAxesBoxLabelled)
  EVT_MENU(ID_TEXT_AXES_BOX_LABELLED_BOXPLOTS,GraphingTesterFrame::OnDrawAxesBoxLabelledBoxplot)
 END_EVENT_TABLE()





BEGIN_EVENT_TABLE(GraphingTesterDrawingWindow,wxWindow)
  EVT_PAINT(GraphingTesterDrawingWindow::OnPaint) 
  EVT_ERASE_BACKGROUND(GraphingTesterDrawingWindow::OnEraseBackground) 
  EVT_MOUSE_EVENTS(GraphingTesterDrawingWindow::OnClick)
END_EVENT_TABLE()

void GraphingTesterFrame::OnQuit(wxCommandEvent& event){

  Close();
}




void GraphingTesterDrawingWindow::OnPaint(wxPaintEvent &WXUNUSED(event)){
	
	wxPrintf(_T("Paint Event\n"));  
	
	wxPaintDC dc(this);
	//PrepareDC(dc);
	wxPrintf(_T("%d\n"),currentplot);
	if (currentplot==1){
	
		DrawBoxplot(dc);	
		
	} else if (currentplot ==2){
		DrawAxes(dc);
			
	} else if (currentplot ==3){
		DrawAxesBox(dc);
	} else if (currentplot ==4){
		DrawAxesBoxLabelled(dc);
	} else if (currentplot ==5){
		DrawAxesBoxLabelledBoxplot(dc);
	}
		
}


void GraphingTesterDrawingWindow::OnEraseBackground(wxEraseEvent &WXUNUSED(event)){
	
	wxPrintf(_T("Erase Event\n"));  
	
	
}



void GraphingTesterDrawingWindow::SetCurrentPlot(int curplot){
	
	this->currentplot = curplot;
}

void SetUpDC(wxDC &dc){

  int w,h;
  dc.SetAxisOrientation(true, true);
  dc.GetSize(&w, &h);
  dc.SetDeviceOrigin(0,0);

}


void GraphingTesterDrawingWindow::DrawBoxplot(wxDC &dc){
	double *five_num_summary = new double[5];	
    
	dc.DrawLine(0,150,150,0);
	

	SetUpDC(dc);


	five_num_summary[0] = 1200.0;
	five_num_summary[1] = 1260.0;
	five_num_summary[2] = 1400.0;
	five_num_summary[3] = 1800.0;
	five_num_summary[4] = 1950.0;

	Draw_Single_Boxplot(dc,
						five_num_summary,
						10,
						300,
						1000.0,
						2000.0,
						50,
						10);

	Draw_Single_Boxplot(dc,
						five_num_summary,
						10,
						300,
						1000.0,
						4000.0,
						200,
						30,
						wxRED);

	delete [] five_num_summary;
}






void GraphingTesterFrame::OnDrawBoxplot(wxCommandEvent& event){

	
	MyWindow->SetCurrentPlot(1);		
    
}




void GraphingTesterFrame::OnDrawDensityCurve(wxCommandEvent& event){


}





void GraphingTesterFrame::OnDrawAxes(wxCommandEvent& event){
	
	MyWindow->SetCurrentPlot(2);	
	
}


void GraphingTesterDrawingWindow::DrawAxes(wxDC &dc){
  
  SetUpDC(dc);

  int w,h;
  dc.GetSize(&w, &h);


  plotAxes myAxes(dc,wxPoint(30,30),
	   wxPoint(w-30*3,h-30*3),
	   wxRealPoint(0.0,50.0),
	   wxRealPoint(1000.0,4000.0));

  
  myAxes.Draw(dc);




  /*  Draw_Axes(dc,
	    0.0,
	    50.0,
	    1000.0,
	    4000.0,
	    30,
	    30,
	    w-30*3,
	    h-30*3);
    
  */
    
}



void GraphingTesterFrame::OnDrawAxesBox(wxCommandEvent& event){

	
	MyWindow->SetCurrentPlot(3);	
	
}


void GraphingTesterDrawingWindow::DrawAxesBox(wxDC &dc){
   
  SetUpDC(dc);

  int w,h;
  dc.GetSize(&w, &h);

    
  plotAxes myAxes(dc,wxPoint(30,30),
		  wxPoint(w-30*3,h-30*3),
		  wxRealPoint(0.0,50.0),
		  wxRealPoint(1000.0,4000.0),
		  true);
  
  
  myAxes.Draw(dc);

    
}




void GraphingTesterFrame::OnDrawAxesBoxLabelled(wxCommandEvent& event){
  MyWindow->SetCurrentPlot(4);
}
 
	

void GraphingTesterDrawingWindow::DrawAxesBoxLabelled(wxDC &dc){
  SetUpDC(dc);

  int w,h;
  dc.GetSize(&w, &h);

    
  labeledplotAxes myAxes(dc, wxPoint(80,80),
		  wxPoint(w-30*5,h-30*5),
		  wxRealPoint(0.0,1000.0),
		  wxRealPoint(50.0,4000.0),
		  true);
  
  myAxes.addGrid(10.0, 1000.0);

  wxArrayDouble xtickpoints;
  wxArrayDouble ytickpoints;

  xtickpoints.Add(0.0);
  xtickpoints.Add(10.0);
  xtickpoints.Add(20.0);
  xtickpoints.Add(30.0);
  xtickpoints.Add(40.0);
  xtickpoints.Add(50.0);

  ytickpoints.Add(1000.0);
  ytickpoints.Add(1500.0);
  ytickpoints.Add(2000.0);
  ytickpoints.Add(2500.0);
  ytickpoints.Add(3000.0);
  ytickpoints.Add(3500.0);
  ytickpoints.Add(4000.0);

  myAxes.setTickLocationX(xtickpoints);
  myAxes.setTickLocationY(ytickpoints);

  wxArrayString x_tick_labels;
  wxArrayString y_tick_labels;

  x_tick_labels.Add(wxT("0.0"));
  x_tick_labels.Add(wxT("10.0"));
  x_tick_labels.Add(wxT("20.0"));
  x_tick_labels.Add(wxT("30.0"));
  x_tick_labels.Add(wxT("40.0"));
  x_tick_labels.Add(wxT("50.0"));


  y_tick_labels.Add(wxT("1000.0"));
  y_tick_labels.Add(wxT("1500.0"));
  y_tick_labels.Add(wxT("2000.0"));
  y_tick_labels.Add(wxT("2500.0"));
  y_tick_labels.Add(wxT("3000.0"));
  y_tick_labels.Add(wxT("3500.0"));
  y_tick_labels.Add(wxT("4000.0"));


  myAxes.setTickLabelsX(x_tick_labels);
  myAxes.setTickLabelsY(y_tick_labels);


  wxString x_label = wxT("X Axis Label");
  wxString y_label = wxT("Y Axis Label");

  myAxes.setAxisLabelX(x_label);
  myAxes.setAxisLabelY(y_label);


  myAxes.setTitle(wxT("Demonstration Title"));



  wxPoint xsq[5];
  
  xsq[0] = myAxes.FindPoint(wxRealPoint(10,1000));
  xsq[1] = myAxes.FindPoint(wxRealPoint(20,2000));
  xsq[2] = myAxes.FindPoint(wxRealPoint(30,2500));
  xsq[3] = myAxes.FindPoint(wxRealPoint(40,2000));
  xsq[4] = myAxes.FindPoint(wxRealPoint(50,1200));


  dc.DrawSpline(WXSIZEOF(xsq),xsq);
  myAxes.Draw(dc);

    
}



void GraphingTesterFrame::OnDrawAxesBoxLabelledBoxplot(wxCommandEvent& event){
	MyWindow->SetCurrentPlot(5);
}


void GraphingTesterDrawingWindow::DrawAxesBoxLabelledBoxplot(wxDC &dc){
  
  int w,h;
  dc.GetSize(&w, &h);

  
  labeledplotAxes myAxes(dc, wxPoint(80,80),
		  wxPoint(w-30*5,h-30*5),
			 wxRealPoint(0.5,-0.3),
			 wxRealPoint(4.5,1.5),
		  true);

  myAxes.setupDCforPlot(dc);
//	SetUpDC(dc);
	
	
  wxArrayDouble xtickpoints;
  wxArrayDouble ytickpoints;

  xtickpoints.Add(1.0);
  xtickpoints.Add(2.0);
  xtickpoints.Add(3.0);
  xtickpoints.Add(4.0);
  xtickpoints.Add(5.0);

  ytickpoints.Add(-0.2);
  ytickpoints.Add(0.0);
  ytickpoints.Add(0.2);
  ytickpoints.Add(0.4);
  ytickpoints.Add(0.6);
  ytickpoints.Add(0.8);
  ytickpoints.Add(1.0);
  ytickpoints.Add(1.2);
  ytickpoints.Add(1.4);
  ytickpoints.Add(1.6);

  myAxes.setTickLocationX(xtickpoints);
  myAxes.setTickLocationY(ytickpoints);

  wxArrayString x_tick_labels;
  wxArrayString y_tick_labels;

  x_tick_labels.Add(wxT("1"));
  x_tick_labels.Add(wxT("2"));
  x_tick_labels.Add(wxT("3"));
  x_tick_labels.Add(wxT("4"));
  x_tick_labels.Add(wxT("5"));


  y_tick_labels.Add(wxT("-0.2"));
  y_tick_labels.Add(wxT("0.0"));
  y_tick_labels.Add(wxT("0.2"));
  y_tick_labels.Add(wxT("0.4"));
  y_tick_labels.Add(wxT("0.6"));
  y_tick_labels.Add(wxT("0.8"));
  y_tick_labels.Add(wxT("1.0"));
  y_tick_labels.Add(wxT("1.2"));
  y_tick_labels.Add(wxT("1.4"));
  y_tick_labels.Add(wxT("1.6"));


  myAxes.setTickLabelsX(x_tick_labels);
  myAxes.setTickLabelsY(y_tick_labels);


  myAxes.setTitle(wxT("Boxplot Demonstration"));
 
  double *five_num_summary = new double[5];
  five_num_summary[0] = 0.0;
  five_num_summary[1] = 0.05;
  five_num_summary[2] = 1.0;
  five_num_summary[3] = 1.11;
  five_num_summary[4] = 1.12;

  wxColor tempColor = wxColor(100,100,255);
  
  Draw_Single_Boxplot(dc,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[0])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[1])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[2])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[3])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[4])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,0)).x,
		      myAxes.FindPoint(wxRealPoint(1.4,0)).x - myAxes.FindPoint(wxRealPoint(0.6,0)).x);



  five_num_summary[0] = -0.13;
  five_num_summary[1] = 0.05;
  five_num_summary[2] = 0.3762;
  five_num_summary[3] = 0.71;
  five_num_summary[4] = 1.19;

  tempColor = wxColor(100,255,100);
 
  Draw_Single_Boxplot(dc,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[0])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[1])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[2])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[3])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[4])).y,
		      myAxes.FindPoint(wxRealPoint(2.0,0)).x,
		      myAxes.FindPoint(wxRealPoint(2.4,0)).x - myAxes.FindPoint(wxRealPoint(1.6,0)).x,&tempColor);

  five_num_summary[0] = -0.13;
  five_num_summary[1] = -0.05;
  five_num_summary[2] = 0.3362;
  five_num_summary[3] = 0.4;
  five_num_summary[4] = 2;

  tempColor = wxColor(255,100,100);

  Draw_Single_Boxplot(dc,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[0])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[1])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[2])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[3])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[4])).y,
		      myAxes.FindPoint(wxRealPoint(3.0,0)).x,
		      myAxes.FindPoint(wxRealPoint(3.4,0)).x - myAxes.FindPoint(wxRealPoint(2.6,0)).x,&tempColor);


  five_num_summary[0] = -2.13;
  five_num_summary[1] = -1.05;
  five_num_summary[2] = 0.159923;
  five_num_summary[3] = 0.4;
  five_num_summary[4] = 0.9;

  tempColor = wxColor(100,100,255);

  Draw_Single_Boxplot(dc,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[0])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[1])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[2])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[3])).y,
		      myAxes.FindPoint(wxRealPoint(1.0,five_num_summary[4])).y,
		      myAxes.FindPoint(wxRealPoint(4.0,0)).x,
		      myAxes.FindPoint(wxRealPoint(4.4,0)).x - myAxes.FindPoint(wxRealPoint(3.6,0)).x,&tempColor);

  delete [] five_num_summary;
  
  //  dc.DrawLine(0,0,1000,1000);


  dc.DestroyClippingRegion();
  myAxes.Draw(dc);
  


}




void GraphingTesterDrawingWindow::OnClick(wxMouseEvent& event){
	
	
	myParent->SetFocus();
	
	
}


GraphingTesterDrawingWindow::GraphingTesterDrawingWindow(GraphingTesterFrame *parent):wxWindow(parent,-1,wxDefaultPosition,wxDefaultSize,wxFULL_REPAINT_ON_RESIZE){
	
	myParent = parent;
	
}



GraphingTesterFrame::GraphingTesterFrame(const wxString& title):wxFrame(NULL,wxID_ANY,title)
{

  wxMenu *fileMenu = new wxMenu;
  wxMenu *drawMenu = new wxMenu;
  wxMenuBar *menuBar = new wxMenuBar();
  
  wxBoxSizer *item0 = new wxBoxSizer( wxVERTICAL );

  fileMenu->Append(wxID_EXIT,wxT("E&xit\tAlt-X"),wxT("Quit the program"));
  
  drawMenu->Append(ID_TEST_BOXPLOT,wxT("&Boxplot"),wxT("Draw A Single Boxplot"));
  drawMenu->Append(ID_TEST_AXES,wxT("&Axes"),wxT("Draw A Set of Axes"));
  drawMenu->Append(ID_TEST_AXES_BOX,wxT("&Axes Box"),wxT("Draw A Set of Axes With Box"));
  drawMenu->Append(ID_TEST_AXES_BOX_LABELLED,wxT("&Axes Box Labelled"),wxT("Draw A Set of Labelled Axes With Box"));
  drawMenu->Append(ID_TEXT_AXES_BOX_LABELLED_BOXPLOTS,wxT("&Box plots on labelled axes"),wxT("Draw A Set of Labelled Axes With Boxplots"));


  menuBar->Append(fileMenu,wxT("&File"));
  menuBar->Append(drawMenu,wxT("&Draw"));

  SetMenuBar(menuBar);

  CreateStatusBar(2);
 
	MyWindow = new GraphingTesterDrawingWindow(this);

  
  item0->Add( MyWindow, 1, wxEXPAND | wxGROW | wxALL , 5 );



  this->SetAutoLayout( TRUE );
  this->SetSizer( item0 );

  MyWindow->SetBackgroundColour(*wxWHITE );
  MyWindow->Refresh();

  

}
