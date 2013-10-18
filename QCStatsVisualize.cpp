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
 ** File: QCStatsVisualize.cpp
 **
 ** Copyright (C) B. M. Bolstad 2007
 **
 ** Visualization of RLE, NUSE.
 **
 ** History - Created Jan 28, 2007
 **
 **
 ** Feb 3, 2007 - add median plots 
 ** Feb 14, 2007 - add ability to save to bitmap format
 ** Mar 4, 2007 - fix NUSE calculations when there are 1 probeset probes
 ** Apr 29, 2007 - fix axes placement for high-res print devices. 
 ** May 12, 2007 - strip .CEL from plot labels
 ** Aug 12, 2007 - fix how "points" are drawn on NUSE/RLE medians plots
 ** Aug 14, 2007 - add "control-chart" type limits     
 ** Aug 16-17, 2007 - add RLE-Nuse Multiplot   
 ** Aug 18-20, 2007 - add a multivariate T2 test statistic plot
 ** 
 **
 **************************************************************************/
#include <wx/wx.h>
#include <wx/dcps.h>
#include <wx/print.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/image.h>
#include <cmath>

#include "QCStatsVisualize.h"
#include "threestep_common.h"
#include "rma_common.h"

#include "Preprocess/matrix_functions.h"

#if RMA_GUI_APP
#include "Graphing/boxplot.h"
#include "Graphing/axes.h"

#include "GlobalPrintSettings.h"
#include "BitmapSettingDialog.h"
#endif

#ifndef _WIN32
#include "RMAExpress.xpm"
#endif

#define ID_POSTSCRIPT_CURRENTPLOT 10000

#define ID_VISUALIZE_RLE 10100
#define ID_VISUALIZE_NUSE 10200
#define ID_VISUALIZE_RLEMEDIANS 10300
#define ID_VISUALIZE_RLEIQRS 10301
#define ID_VISUALIZE_NUSEMEDIANS 10400
#define ID_VISUALIZE_NUSEIQRS 10401
#define ID_VISUALIZE_DRAWSCCLIMITS 10500
#define ID_VISUALIZE_DRAWIQRLIMITS 10600
#define ID_VISUALIZE_RLENUSEMULTIPLOT 10700
#define ID_VISUALIZE_RLENUSEMULTIVARIATECC 10800


#define ID_SAVE_QC_RLE 15100
#define ID_SAVE_QC_NUSE 15200
#define ID_SAVE_QC_RLE_STATS 15300
#define ID_SAVE_QC_NUSE_STATS 15400
#define ID_SAVE_CURRENT 16000
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))

#ifndef _MSC_VER
#define isnan std::isnan
#endif
#if RMA_GUI_APP
wxPrintDialogData g_QCprintDialogData;


// Global print data, to remember settings during the session
wxPrintData *g_QCprintData = (wxPrintData*) NULL ;

// Global page setup data
wxPageSetupData* g_QCpageSetupData = (wxPageSetupData*) NULL;
#endif

///c(x1, x2, y1, y2);
const double pltarea_min_x=0.1225725;
const double pltarea_max_x=0.9372190;
const double pltarea_min_y=0.1524917 ;
const double pltarea_max_y=0.8774086;

#if RMA_GUI_APP

BEGIN_EVENT_TABLE(QCStatsVisualizeFrame,wxFrame)
  EVT_MENU(wxID_EXIT,QCStatsVisualizeFrame::OnQuit) 
  EVT_MENU(ID_POSTSCRIPT_CURRENTPLOT, QCStatsVisualizeFrame::OnPostscriptCurrentPlot)
  EVT_MENU(ID_VISUALIZE_RLE,QCStatsVisualizeFrame::OnVisualizeRLE) 
  EVT_MENU(ID_VISUALIZE_NUSE,QCStatsVisualizeFrame::OnVisualizeNUSE)
  EVT_MENU(ID_VISUALIZE_RLEMEDIANS,QCStatsVisualizeFrame::OnVisualizeRLEMedians) 
  EVT_MENU(ID_VISUALIZE_RLEIQRS,QCStatsVisualizeFrame::OnVisualizeRLEIQRs) 
  EVT_MENU(ID_VISUALIZE_NUSEMEDIANS,QCStatsVisualizeFrame::OnVisualizeNUSEMedians)
  EVT_MENU(ID_VISUALIZE_NUSEIQRS,QCStatsVisualizeFrame::OnVisualizeNUSEIQRs)
  EVT_MENU(ID_VISUALIZE_RLENUSEMULTIPLOT,QCStatsVisualizeFrame::OnVisualizeRLENUSEMultiplot)
  EVT_MENU(ID_VISUALIZE_RLENUSEMULTIVARIATECC,QCStatsVisualizeFrame::OnVisualizeMultivariateControlChart)
  EVT_MENU(ID_VISUALIZE_DRAWSCCLIMITS,QCStatsVisualizeFrame::ToggleSCCLimits )
  EVT_MENU(ID_VISUALIZE_DRAWIQRLIMITS,QCStatsVisualizeFrame::ToggleIQRLimits )
  EVT_MENU(ID_SAVE_QC_RLE, QCStatsVisualizeFrame::OnSaveQCRLE)
  EVT_MENU(ID_SAVE_QC_NUSE, QCStatsVisualizeFrame::OnSaveQCNUSE) 
  EVT_MENU(ID_SAVE_QC_RLE_STATS, QCStatsVisualizeFrame::OnSaveQCRLEStats)
  EVT_MENU(ID_SAVE_QC_NUSE_STATS, QCStatsVisualizeFrame::OnSaveQCNUSEStats)
  EVT_MENU(ID_SAVE_CURRENT, QCStatsVisualizeFrame::OnSaveCurrent)
END_EVENT_TABLE()

#endif
 
// Declare a frame
QCStatsVisualizeFrame   *QCframe = (QCStatsVisualizeFrame *)NULL;



#if RMA_GUI_APP  

void QCStatsVisualizeFrame::OnQuit(wxCommandEvent& event){

  Close();
}

#endif

#if RMA_GUI_APP  
QCStatsVisualizeFrame::QCStatsVisualizeFrame(const wxString& title, expressionGroup *ExpressionVals, wxWindow *parent):wxFrame(parent,wxID_ANY,title){
  fileMenu = new wxMenu;
  drawMenu = new wxMenu;
  menuBar = new wxMenuBar();
  
  wxBoxSizer *item0 = new wxBoxSizer( wxVERTICAL );


  fileMenu->Append(ID_SAVE_CURRENT,wxT("Save"),wxT("Save Current Plot"));
  fileMenu->Append(ID_POSTSCRIPT_CURRENTPLOT,wxT("Print"),wxT("Print Current Plot"));  
  fileMenu->AppendSeparator();
  fileMenu->Append(ID_SAVE_QC_RLE_STATS,wxT("Save RLE Summary Statistics"),wxT("Save RLE Summary Statistics"));
  fileMenu->Append(ID_SAVE_QC_NUSE_STATS,wxT("Save NUSE Summary Statistics"),wxT("Save NUSE Summary Statistics"));
  fileMenu->Append(ID_SAVE_QC_RLE,wxT("Save RLE values"),wxT("Save RLE Values"));
  fileMenu->Append(ID_SAVE_QC_NUSE,wxT("Save NUSE values"),wxT("Save NUSE Values"));  
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT,wxT("E&xit\tAlt-X"),wxT("Quit the program"));




  drawMenu->Append(ID_VISUALIZE_RLE,wxT("&RLE Boxplots"),wxT("Boxplots of RLE"));
  drawMenu->Append(ID_VISUALIZE_NUSE,wxT("&NUSE Boxplots"),wxT("Boxplots of NUSE"));  
  drawMenu->AppendSeparator();
  drawMenu->Append(ID_VISUALIZE_RLEMEDIANS,wxT("RLE Medians plot"),wxT("Plot of RLE Medians"));
  drawMenu->Append(ID_VISUALIZE_NUSEMEDIANS,wxT("NUSE Medians plot"),wxT("Plot of NUSE Medians"));  
  drawMenu->Append(ID_VISUALIZE_RLEIQRS,wxT("RLE IQRs plot"),wxT("Plot of RLE IQRs"));
  drawMenu->Append(ID_VISUALIZE_NUSEIQRS,wxT("NUSE IQRs plot"),wxT("Plot of NUSE IQRs"));
  drawMenu->Append(ID_VISUALIZE_RLENUSEMULTIPLOT, wxT("RLE/NUSE Multiplot"),wxT("Multiplot of RLE/NUSE statistics"));
  drawMenu->AppendCheckItem(ID_VISUALIZE_DRAWSCCLIMITS, wxT("Add Control Limits"),wxT("Add single measurement control chart limits to plots"));
  drawMenu->AppendCheckItem(ID_VISUALIZE_DRAWIQRLIMITS, wxT("Add IQR Limits"),wxT("Use boxplot rules to determine outliers"));
  drawMenu->AppendSeparator();
  drawMenu->Append(ID_VISUALIZE_RLENUSEMULTIVARIATECC,wxT("RLE/NUSE T2 Chart"),wxT("Multivariate Control Chart based on QC summary stats"));

  if (ExpressionVals->count_arrays() < 6){
    drawMenu->Enable(ID_VISUALIZE_DRAWSCCLIMITS,false);
    drawMenu->Enable(ID_VISUALIZE_DRAWIQRLIMITS,false);
    drawMenu->Enable(ID_VISUALIZE_RLENUSEMULTIVARIATECC,false);
  }



  menuBar->Append(fileMenu,wxT("&File"));
  menuBar->Append(drawMenu,wxT("&Draw"));

  SetMenuBar(menuBar);

  CreateStatusBar(2);
 
  MyWindow = new QCStatsVisualizeDrawingWindow(this);
  
  item0->Add( MyWindow, 1, wxEXPAND | wxGROW | wxALL , 5 );



  this->SetAutoLayout( TRUE );
  this->SetSizer( item0 );

  MyWindow->SetBackgroundColour(*wxWHITE );
  MyWindow->Refresh();

  ExpressionValues = new expressionGroup(*ExpressionVals);  

  RLEBoxplotStatistics = NULL;  
  NUSEBoxplotStatistics = NULL;
  T2 = NULL;
  currentplot = 0;

  
  g_QCprintData = new wxPrintData;
  g_QCpageSetupData = new wxPageSetupDialogData;
  g_QCprintData->SetOrientation(wxLANDSCAPE);

  QCframe = this;
#if _WIN32
  SetIcon(wxICON(expressicon));
#else
  SetIcon(wxICON(RMAExpress));
#endif
}


#else

QCStatsVisualizeFrame::QCStatsVisualizeFrame(expressionGroup *ExpressionVals){

  ExpressionValues = new expressionGroup(*ExpressionVals);  

  RLEBoxplotStatistics = NULL;  
  NUSEBoxplotStatistics = NULL;
  T2 = NULL;
  currentplot = 0;

}

#endif


QCStatsVisualizeFrame::~QCStatsVisualizeFrame(){

  if (RLEBoxplotStatistics != NULL)
    delete RLEBoxplotStatistics;

  
  if (NUSEBoxplotStatistics != NULL)
    delete NUSEBoxplotStatistics;

  delete ExpressionValues;
  

};





#if RMA_GUI_APP
void QCStatsVisualizeFrame::OnVisualizeRLE(wxCommandEvent& event){
  //wxPaintDC dc(MyWindow);
  GenerateRLE();
  //DrawRLEBoxplot(dc);
  currentplot = 1; 
  this->Refresh();
}

void QCStatsVisualizeFrame::OnVisualizeRLEMedians(wxCommandEvent& event){
  //wxPaintDC dc(MyWindow);
  GenerateRLE();
  //DrawRLEMedians(dc);
  currentplot = 3; 
  this->Refresh();
}




void QCStatsVisualizeFrame::OnVisualizeNUSE(wxCommandEvent& event){

//  wxPaintDC dc(MyWindow);
  GenerateNUSE();
//  DrawNUSEBoxplot(dc);
  currentplot = 2; 
  this->Refresh();
  
}

void QCStatsVisualizeFrame::OnVisualizeRLEIQRs(wxCommandEvent& event){

 // wxPaintDC dc(MyWindow);
  GenerateRLE();
 // DrawRLEIQRs(dc);
  currentplot = 5; 
  this->Refresh();
  
}




void QCStatsVisualizeFrame::OnVisualizeNUSEMedians(wxCommandEvent& event){

//  wxPaintDC dc(MyWindow);
  GenerateNUSE();
//  DrawNUSEMedians(dc);
  currentplot = 4; 
  this->Refresh();
  
}


void QCStatsVisualizeFrame::OnVisualizeNUSEIQRs(wxCommandEvent& event){

  //wxPaintDC dc(MyWindow);
  GenerateNUSE();
 // DrawNUSEIQRs(dc);
  currentplot = 6; 
  this->Refresh();
  
}


void QCStatsVisualizeFrame::OnVisualizeRLENUSEMultiplot(wxCommandEvent& event){
  //wxPaintDC dc(MyWindow);
  GenerateNUSE();
  GenerateRLE();
  //DrawRLENUSEMultiPlot(dc);
  currentplot = 7; 
  this->Refresh();
}



void QCStatsVisualizeFrame::OnVisualizeMultivariateControlChart(wxCommandEvent& event){
  //wxPaintDC dc(MyWindow);
  GenerateNUSE();
  GenerateRLE();
  ComputeRLENUSE_T2();
 // DrawMultivariateControlChart(dc);
  currentplot = 8; 
  this->Refresh();
}


#endif



void QCStatsVisualizeFrame::GenerateRLE(){


  double *buffer = new double [ExpressionValues->count_arrays()];
  double *buffer2 = new double [ExpressionValues->count_probesets()];
  int i,j;
  double med, UQ, LQ;


  
  if (RLEBoxplotStatistics == NULL){
    RLEBoxplotStatistics = new Matrix(5,ExpressionValues->count_arrays());


    // Make the RLE values
    for (i=0; i < ExpressionValues->count_probesets(); i++){
      for (j=0; j < ExpressionValues->count_arrays(); j++){
	buffer[j] = (*ExpressionValues)[j*ExpressionValues->count_probesets()+ i];
      }
      med = median(buffer, ExpressionValues->count_arrays());
      for (j=0; j < ExpressionValues->count_arrays(); j++){
      	(*ExpressionValues)[j*ExpressionValues->count_probesets()+ i] -=med;
      }
    }
    
    //Now get the summary values.
    for (j=0; j < ExpressionValues->count_arrays(); j++){
      for (i=0; i < ExpressionValues->count_probesets(); i++){
	buffer2[i] = (*ExpressionValues)[j*ExpressionValues->count_probesets()+ i];
      }
      
      
      med = median_nocopy(buffer2,ExpressionValues->count_probesets());
      quartiles(buffer2,ExpressionValues->count_probesets(), &LQ, &UQ);
 
      
      (*RLEBoxplotStatistics)[j*5 + 0] = buffer2[0];
      (*RLEBoxplotStatistics)[j*5 +1] = LQ;
      (*RLEBoxplotStatistics)[j*5 +2] = med;
      (*RLEBoxplotStatistics)[j*5 +3] = UQ;
      (*RLEBoxplotStatistics)[j*5 +4] = buffer2[ExpressionValues->count_probesets()-1];
      
    }


  }

  delete [] buffer;
  delete [] buffer2;

};


void QCStatsVisualizeFrame::GenerateNUSE(){



  double *buffer = new double [ExpressionValues->count_arrays()];
  double *buffer2 = new double [ExpressionValues->count_probesets()];
  int i,j;
  double med, UQ, LQ;

  int num_na=0;

  
  if (NUSEBoxplotStatistics == NULL){
    NUSEBoxplotStatistics = new Matrix(5,ExpressionValues->count_arrays());


    // Make the NUSE values
    for (i=0; i < ExpressionValues->count_probesets(); i++){
      for (j=0; j < ExpressionValues->count_arrays(); j++){
	buffer[j] = ExpressionValues->SE(j*ExpressionValues->count_probesets()+ i);
	//	if (isnan(buffer[j])){
	//  buffer[j] == 1.0;
	//}

      }
      med = median(buffer, ExpressionValues->count_arrays());
      if (med == 0.0){
	med = 1.0;
      }
      if (med != -1.0){
	for (j=0; j < ExpressionValues->count_arrays(); j++){
	  ExpressionValues->SE(j*ExpressionValues->count_probesets()+ i)/=med;
	}
      } else {
	for (j=0; j < ExpressionValues->count_arrays(); j++){
	  ExpressionValues->SE(j*ExpressionValues->count_probesets()+ i)= NAN;
	} 
	num_na++;
      }
	
    }
    
    //Now get the summary values.
    for (j=0; j < ExpressionValues->count_arrays(); j++){
      for (i=0; i < ExpressionValues->count_probesets(); i++){
	if (isnan(ExpressionValues->SE(j*ExpressionValues->count_probesets()+ i))){
	  buffer2[i] = INFINITY;
	} else {
	  buffer2[i] = ExpressionValues->SE(j*ExpressionValues->count_probesets()+ i);
	}
      }
       
   
      med = median_nocopy_hasNA(buffer2,ExpressionValues->count_probesets(),num_na);
      quartiles(buffer2,ExpressionValues->count_probesets()-num_na, &LQ, &UQ);
 
           
      (*NUSEBoxplotStatistics)[j*5 + 0] = buffer2[0];
      (*NUSEBoxplotStatistics)[j*5 +1] = LQ;
      (*NUSEBoxplotStatistics)[j*5 +2] = med;
      (*NUSEBoxplotStatistics)[j*5 +3] = UQ;
      (*NUSEBoxplotStatistics)[j*5 +4] = buffer2[ExpressionValues->count_probesets()-num_na-1];
      // wxPrintf(wxT("%f %f %f %f %f\n"), buffer2[0], buffer2[ExpressionValues->count_probesets()/4],buffer2[ExpressionValues->count_probesets()/2],buffer2[ExpressionValues->count_probesets()*3/4],buffer2[ExpressionValues->count_probesets()-1]);
    }


  }

  delete [] buffer;
  delete [] buffer2;

};



static double computeMin(Matrix &x, int row){

  double min = x[row];
  int j;

  for (j =1; j < x.Cols(); j++){

    if (x[j*x.Rows()  + row] < min){
      
      min = x[j*x.Rows()  + row];
    }
  }
  return min;
}

static double computeMax(Matrix &x, int row){

  double max = x[row];
  int j;

  for (j =1; j < x.Cols(); j++){

    if (x[j*x.Rows()  + row] > max){
      
      max = x[j*x.Rows()  + row];
    }
  }
  return max;

}


static double computeMinCol(Matrix &x, int col){

  double min = x[col*x.Rows() + 0];
  int i;

  for (i =1; i < x.Rows(); i++){

    if (x[col*x.Rows()  + i] < min){
      
      min = x[col*x.Rows()  + i];
    }
  }
  return min;
}

static double computeMaxCol(Matrix &x, int col){

  double max = x[col*x.Rows() + 0];
  int i;

  for (i =1; i < x.Rows(); i++){

    if (x[col*x.Rows()  + i] > max){
      
      max = x[col*x.Rows()  + i];
    }
  }
  return max;
}





static double computeMin2(Matrix &x, int row1, int row2){

  double min = x[row2] - x[row1];
  int j;

  for (j =1; j < x.Cols(); j++){

    if (x[j*x.Rows()  + row2] - x[j*x.Rows()  + row1] < min){
      
      min = x[j*x.Rows()  + row2] - x[j*x.Rows()  + row1];
    }
  }
  return min;
}

static double computeMax2(Matrix &x, int row1, int row2){

  double max = x[row2] - x[row1];
  int j;

  for (j =1; j < x.Cols(); j++){

    if (x[j*x.Rows()  + row2] - x[j*x.Rows()  + row1] > max){
      
      max = x[j*x.Rows()  + row2] - x[j*x.Rows()  + row1];
    }
  }
  return max;

}




static void computeIndividualsControlLimits(Matrix &x, int row1, double *LCL, double *UCL){

  /*   
      UCL = Xbar + 3*(MRbar/1.128)
      Center Line = Xbar  (Not Drawn)
      LCL = Xbar - 3*(MRbar/1.128)
      
      MR = |Xi+1 - Xi|
  */

  double MRBar=0.0;
  double mean=0.0;
  int j;

  mean = x[row1];
    
  for (j=1; j < x.Cols(); j++){
    MRBar+= fabs(x[j*x.Rows()  + row1] - x[(j-1)*x.Rows()  + row1]);
    mean += x[j*x.Rows()  + row1];
  }
  
  mean/=(double)x.Cols();
  MRBar/=(double)(x.Cols()-1);
  
  *UCL = mean + 3.0*MRBar/1.128;
  *LCL = mean - 3.0*MRBar/1.128;

}



static void computeIndividualsControlLimits2(Matrix &x, int row1, int row2, double *LCL, double *UCL){

  /*   
      UCL = Xbar + 3*(MRbar/1.128)
      Center Line = Xbar  (Not Drawn)
      LCL = Xbar - 3*(MRbar/1.128)
      
      MR = |Xi+1 - Xi|
  */

  double MRBar=0.0;
  double mean=0.0;
  int j;

  mean = x[row2] - x[row1];
    
  for (j=1; j < x.Cols(); j++){
    MRBar+= fabs(( x[j*x.Rows()  + row2] - x[j*x.Rows()  + row1]) - (x[(j-1)*x.Rows()  + row2] - x[(j-1)*x.Rows()  + row1]));
    mean += x[j*x.Rows()  + row2] - x[j*x.Rows()  + row1];
  }
  
  mean/=x.Cols();
  MRBar/=(x.Cols()-1);
  
  *UCL = mean + 3.0*MRBar/1.128;
  *LCL = mean - 3.0*MRBar/1.128;

}




static void computeIQRLimits(Matrix &x, int row1, double *LCL, double *UCL){

  double *buffer = new double [x.Cols()];
  double med, LQ, UQ;
  int j;


  for (j=0; j < x.Cols(); j++){
    buffer[j] = x[j*x.Rows()  + row1];
  }

  med = median_nocopy(buffer,x.Cols());
  quartiles(buffer,x.Cols(), &LQ, &UQ);

  *UCL = UQ + 1.5*(UQ-LQ);
  *LCL = LQ - 1.5*(UQ-LQ);
  

  delete [] buffer;
}

static void computeIQRLimits2(Matrix &x, int row1, int row2, double *LCL, double *UCL){

  double *buffer = new double [x.Cols()];
  double med, LQ, UQ;
  int j;


  for (j=0; j < x.Cols(); j++){
    buffer[j] = x[j*x.Rows()  + row2] - x[j*x.Rows()  + row1];
  }

  med = median_nocopy(buffer,x.Cols());
  quartiles(buffer,x.Cols(), &LQ, &UQ);

  *UCL = UQ + 1.5*(UQ-LQ);
  *LCL = LQ - 1.5*(UQ-LQ);
  

  delete [] buffer;
}




#if RMA_GUI_APP  
  
void QCStatsVisualizeFrame::DrawRLEBoxplot(wxDC& dc){

  int i,j;

  int w,h;
  dc.GetSize(&w, &h);
  
  int min_x = min((int)(pltarea_min_x*(double)w),(int)(160*(double)(dc.GetPPI().x)/(double)72));
  int min_y = min((int)(pltarea_min_y*(double)h),(int)(165*(double)(dc.GetPPI().x)/(double)72));
  int max_x = max((int)((pltarea_max_x-pltarea_min_x)*(double)w),(int)(w - 100*(double)(dc.GetPPI().x)/(double)72 - min_x));
  int max_y = max((int)((pltarea_max_y-pltarea_min_y)*(double)h),(int)(h-160*(double)(dc.GetPPI().x)/(double)72 - min_y));
   



  labeledplotAxes myAxes(dc,wxPoint(min_x,min_y),
			 wxPoint(max_x,max_y),
			 wxRealPoint(0.5,-0.5),
			 wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5 ,0.5),
			 true,
			 wxPoint(0,0));
  //dc.SetClippingRegion(80,30*5-80 ,w-30*5,h-30*5);  
  //dc.SetBackground(*wxGREY_BRUSH);
  //dc.Clear();

  //SetUpDC(dc);
  dc.Clear();
  myAxes.setupDCforPlot(dc);


  wxArrayDouble xtickpoints;
  wxArrayDouble ytickpoints;
  wxArrayString x_tick_labels;
  wxArrayString y_tick_labels;

  wxString temp;
  

  for (i =1; i <= ExpressionValues->count_arrays();i++){
    xtickpoints.Add((double)i);
    wxString mynewstring = wxString::Format(_T("%d"), i);
    x_tick_labels.Add(mynewstring); 
  }

  x_tick_labels = ExpressionValues->GetArrayNames();
  
  // Strip the ".CEL" from the labels 
  for (i =0; i < ExpressionValues->count_arrays();i++){
    temp = wxString(x_tick_labels[i]);
    x_tick_labels[i] = temp.Remove(temp.Len() - 4);

  }


  for (i=0; i <= 16; i++){
    ytickpoints.Add((double)(i - 8)/16.0); 
    y_tick_labels.Add(wxString::Format(_T("%.2f"), (double)(i-8)/16.0));
  }
  
  myAxes.setTickLocationX(xtickpoints);
  myAxes.setTickLocationY(ytickpoints);

  myAxes.setTickLabelsX(x_tick_labels);
  myAxes.setTickLabelsY(y_tick_labels);

  myAxes.setTickLabelsPerpendicular(true);


  wxString x_label = wxT("");
  wxString y_label = wxT("RLE");

  myAxes.setAxisLabelX(x_label);
  myAxes.setAxisLabelY(y_label);


  
  myAxes.setTitle(wxT("RLE values by array"));
 




  wxColor tempColor = wxColor(100,100,255);

  for (j =0; j < ExpressionValues->count_arrays();j++){


    Draw_Single_Boxplot(dc,
			myAxes.FindPoint(wxRealPoint(1.0,max((*RLEBoxplotStatistics)[j*5 + 0],-0.5))).y,  // fix clipping problem
			myAxes.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[j*5 + 1])).y,
			myAxes.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[j*5 + 2])).y,
			myAxes.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[j*5 + 3])).y,
			myAxes.FindPoint(wxRealPoint(1.0,min((*RLEBoxplotStatistics)[j*5 + 4],0.5))).y,  // fix clipping problem
			myAxes.FindPoint(wxRealPoint((double)(j+1),0)).x,
			myAxes.FindPoint(wxRealPoint(3.4,0)).x - myAxes.FindPoint(wxRealPoint(2.6,0)).x,&tempColor);
  }

  dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
	      myAxes.FindPoint(wxRealPoint(0.5,0.0)).y,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).y);
  


  dc.DestroyClippingRegion();
  myAxes.Draw(dc);









};







void QCStatsVisualizeFrame::DrawNUSEBoxplot(wxDC& dc){
  int i,j;

  int w,h;
  dc.GetSize(&w, &h);
 
  int min_x = min((int)(pltarea_min_x*(double)w),(int)(160*(double)(dc.GetPPI().x)/(double)72));
  int min_y = min((int)(pltarea_min_y*(double)h),(int)(165*(double)(dc.GetPPI().x)/(double)72));
  int max_x = max((int)((pltarea_max_x-pltarea_min_x)*(double)w),(int)(w - 100*(double)(dc.GetPPI().x)/(double)72 - min_x));
  int max_y = max((int)((pltarea_max_y-pltarea_min_y)*(double)h),(int)(h-160*(double)(dc.GetPPI().x)/(double)72 - min_y));
    
    



  labeledplotAxes myAxes(dc,wxPoint(min_x,min_y),
			 wxPoint(max_x,max_y),
			 wxRealPoint(0.5,0.9),
			 wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5 ,1.2),
			 true,
			 wxPoint(0,0));
  //dc.SetClippingRegion(80,30*5-80 ,w-30*5,h-30*5);  
  //dc.SetBackground(*wxGREY_BRUSH);
  //dc.Clear();

  //SetUpDC(dc);
  dc.Clear();
  myAxes.setupDCforPlot(dc);


  wxArrayDouble xtickpoints;
  wxArrayDouble ytickpoints;
  wxArrayString x_tick_labels;
  wxArrayString y_tick_labels;

  wxString temp;

  for (i =1; i <= ExpressionValues->count_arrays();i++){
    xtickpoints.Add((double)i);
    wxString mynewstring = wxString::Format(_T("%d"), i);
    x_tick_labels.Add(mynewstring); 
  }

  x_tick_labels = ExpressionValues->GetArrayNames();
  
  // Strip the ".CEL" from the labels 
  for (i =0; i < ExpressionValues->count_arrays();i++){
    temp = wxString(x_tick_labels[i]);
    x_tick_labels[i] = temp.Remove(temp.Len() - 4);

  }

  for (i=0; i <= 6; i++){
    ytickpoints.Add((double)(i)/6.0*0.3 + 0.9); 
    y_tick_labels.Add(wxString::Format(_T("%.2f"), (double)(i)/6.0*0.3 + 0.9));
  }
  
  myAxes.setTickLocationX(xtickpoints);
  myAxes.setTickLocationY(ytickpoints);

  myAxes.setTickLabelsX(x_tick_labels);
  myAxes.setTickLabelsY(y_tick_labels);

  myAxes.setTickLabelsPerpendicular(true);


  wxString x_label = wxT("");
  wxString y_label = wxT("NUSE");

  myAxes.setAxisLabelX(x_label);
  myAxes.setAxisLabelY(y_label);


  
  myAxes.setTitle(wxT("NUSE values by array"));
 




  wxColor tempColor = wxColor(100,100,255);

  for (j =0; j < ExpressionValues->count_arrays();j++){


    Draw_Single_Boxplot(dc,
			myAxes.FindPoint(wxRealPoint(1.0,max((*NUSEBoxplotStatistics)[j*5 + 0],0.9))).y,    // avoids some clipping problems
			myAxes.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[j*5 + 1])).y,
			myAxes.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[j*5 + 2])).y,
			myAxes.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[j*5 + 3])).y,
			myAxes.FindPoint(wxRealPoint(1.0,min((*NUSEBoxplotStatistics)[j*5 + 4],1.2))).y,    // avoids some clipping problems
			myAxes.FindPoint(wxRealPoint((double)(j+1),0)).x,
			myAxes.FindPoint(wxRealPoint(3.4,0)).x - myAxes.FindPoint(wxRealPoint(2.6,0)).x,&tempColor);
  }


  dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,1.0)).x,
	      myAxes.FindPoint(wxRealPoint(0.5,1.0)).y,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,1.0)).x,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,1.0)).y);
  



  dc.DestroyClippingRegion();
  myAxes.Draw(dc);




};




void QCStatsVisualizeFrame::DrawRLEMedians(wxDC& dc){

  int i,j;

  int w,h;
  dc.GetSize(&w, &h);
 
  
  int min_x = min((int)(pltarea_min_x*(double)w),(int)(160*(double)(dc.GetPPI().x)/(double)72));
  int min_y = min((int)(pltarea_min_y*(double)h),(int)(165*(double)(dc.GetPPI().x)/(double)72));
  int max_x = max((int)((pltarea_max_x-pltarea_min_x)*(double)w),(int)(w - 100*(double)(dc.GetPPI().x)/(double)72 - min_x));
  int max_y = max((int)((pltarea_max_y-pltarea_min_y)*(double)h),(int)(h-160*(double)(dc.GetPPI().x)/(double)72 - min_y));

  double axes_low = computeMin((*RLEBoxplotStatistics),2);
  double axes_high = computeMax((*RLEBoxplotStatistics),2);

  if (axes_low > -0.1){
    axes_low = -0.1;
  } else {
    axes_low = round((axes_low-0.05)*10.0)/10.0;
  }
  
  
  if (axes_high < 0.1){
    axes_high = 0.1;
  } else {
    axes_high= round((axes_high+.05)*10.0)/10.0;
  }


  labeledplotAxes myAxes(dc,wxPoint(min_x,min_y),
			 wxPoint(max_x,max_y),
			 wxRealPoint(0.5,axes_low),
			 wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5 ,axes_high),
			 true,
			 wxPoint(0,0));
  //dc.SetClippingRegion(80,30*5-80 ,w-30*5,h-30*5);  
  //dc.SetBackground(*wxGREY_BRUSH);
  //dc.Clear();
  myAxes.setBackgroundColor(*wxWHITE_BRUSH);
  //SetUpDC(dc);
  dc.Clear();
  myAxes.setupDCforPlot(dc);


  wxArrayDouble xtickpoints;
  wxArrayDouble ytickpoints;
  wxArrayString x_tick_labels;
  wxArrayString y_tick_labels;

  wxString temp;

  for (i =1; i <= ExpressionValues->count_arrays();i++){
    xtickpoints.Add((double)i);
    wxString mynewstring = wxString::Format(_T("%d"), i);
    x_tick_labels.Add(mynewstring); 
  }

  x_tick_labels = ExpressionValues->GetArrayNames();
  
  // Strip the ".CEL" from the labels 
  for (i =0; i < ExpressionValues->count_arrays();i++){
    temp = wxString(x_tick_labels[i]);
    x_tick_labels[i] = temp.Remove(temp.Len() - 4);

  }


  for (i=0; i <= 8; i++){
    ytickpoints.Add((double)axes_low + (double)i/8 *(axes_high-axes_low)); 
    y_tick_labels.Add(wxString::Format(_T("%.3f"), axes_low + (double)i/8 *(axes_high -axes_low)));
  }
  
  myAxes.setTickLocationX(xtickpoints);
  myAxes.setTickLocationY(ytickpoints);

  myAxes.setTickLabelsX(x_tick_labels);
  myAxes.setTickLabelsY(y_tick_labels);

  myAxes.setTickLabelsPerpendicular(true);


  wxString x_label = wxT("");
  wxString y_label = wxT("RLE");

  myAxes.setAxisLabelX(x_label);
  myAxes.setAxisLabelY(y_label);


  
  myAxes.setTitle(wxT("RLE medians"));
 


  dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
	      myAxes.FindPoint(wxRealPoint(0.5,0.0)).y,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).y);
  


  wxColor tempColor = wxColor(100,100,255);
  double LCL,UCL;
  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWSCCLIMITS)){
    
    
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor,2));
    
    
    computeIndividualsControlLimits((*RLEBoxplotStatistics),2,&LCL,&UCL);
    
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint(0.5,LCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,LCL)).y);
    
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint(0.5,UCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,UCL)).y);
    
    dc.SetPen(oldPen);
  }

  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWIQRLIMITS)){
    wxColor tempColor2 = wxColor(50,255,50);
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor2,2));
    
    
    computeIQRLimits((*RLEBoxplotStatistics),2,&LCL,&UCL);
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint(0.5,LCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,LCL)).y);
    
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint(0.5,UCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,UCL)).y);
    
    dc.SetPen(oldPen);
  }
  


  
  for (j =0; j < ExpressionValues->count_arrays()-1;j++){


    dc.DrawLine(myAxes.FindPoint(wxRealPoint((double)(j+1),0)).x,
		myAxes.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[j*5 + 2])).y,
		myAxes.FindPoint(wxRealPoint((double)(j+2),0)).x,
		myAxes.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[(j+1)*5 + 2])).y);
  }

  for (j =0; j < ExpressionValues->count_arrays();j++){


    dc.DrawCircle(myAxes.FindPoint(wxRealPoint((double)(j+1),0)).x,myAxes.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[j*5 + 2])).y,(int)(3*(double)(dc.GetPPI().x)/(double)72) );
    

  }



  dc.DestroyClippingRegion();
  myAxes.Draw(dc);

}





void QCStatsVisualizeFrame::DrawNUSEMedians(wxDC& dc){

  int i,j;

  int w,h;
  dc.GetSize(&w, &h);

  
  int min_x = min((int)(pltarea_min_x*(double)w),(int)(160*(double)(dc.GetPPI().x)/(double)72));
  int min_y = min((int)(pltarea_min_y*(double)h),(int)(165*(double)(dc.GetPPI().x)/(double)72));
  int max_x = max((int)((pltarea_max_x-pltarea_min_x)*(double)w),(int)(w - 100*(double)(dc.GetPPI().x)/(double)72 - min_x));
  int max_y = max((int)((pltarea_max_y-pltarea_min_y)*(double)h),(int)(h-160*(double)(dc.GetPPI().x)/(double)72 - min_y));
 

  double axes_low = computeMin((*NUSEBoxplotStatistics),2);
  double axes_high = computeMax((*NUSEBoxplotStatistics),2);

  if (axes_low > 0.95){
    axes_low = 0.95;
  } else {
    axes_low = trunc(axes_low*10.0)/10.0;
  }
  
  
  if (axes_high < 1.20){
    axes_high = 1.2;
  } else {
    axes_high = round((axes_high+.05)*10.0)/10.0;
  }
    



  labeledplotAxes myAxes(dc,wxPoint(min_x,min_y),
			 wxPoint(max_x,max_y),
			 wxRealPoint(0.5,axes_low),
			 wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5 ,axes_high),
			 true,
			 wxPoint(0,0));
  //dc.SetClippingRegion(80,30*5-80 ,w-30*5,h-30*5);  
  //dc.SetBackground(*wxGREY_BRUSH);
  //dc.Clear();

  //SetUpDC(dc);
  dc.Clear();
  myAxes.setBackgroundColor(*wxWHITE_BRUSH);
  myAxes.setupDCforPlot(dc);
  

  wxArrayDouble xtickpoints;
  wxArrayDouble ytickpoints;
  wxArrayString x_tick_labels;
  wxArrayString y_tick_labels;

  wxString temp;

  for (i =1; i <= ExpressionValues->count_arrays();i++){
    xtickpoints.Add((double)i);
    wxString mynewstring = wxString::Format(_T("%d"), i);
    x_tick_labels.Add(mynewstring); 
  }

  x_tick_labels = ExpressionValues->GetArrayNames();
  
  // Strip the ".CEL" from the labels 
  for (i =0; i < ExpressionValues->count_arrays();i++){
    temp = wxString(x_tick_labels[i]);
    x_tick_labels[i] = temp.Remove(temp.Len() - 4);

  }

  for (i=0; i <= 10; i++){
    ytickpoints.Add((double)axes_low + (double)i/10*(axes_high-axes_low)); 
    y_tick_labels.Add(wxString::Format(_T("%.3f"), axes_low + (double)i/10 *(axes_high-axes_low)));
  }
  
  myAxes.setTickLocationX(xtickpoints);
  myAxes.setTickLocationY(ytickpoints);

  myAxes.setTickLabelsX(x_tick_labels);
  myAxes.setTickLabelsY(y_tick_labels);

  myAxes.setTickLabelsPerpendicular(true);


  wxString x_label = wxT("");
  wxString y_label = wxT("NUSE");

  myAxes.setAxisLabelX(x_label);
  myAxes.setAxisLabelY(y_label);


  
  myAxes.setTitle(wxT("NUSE medians"));
 


  dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,1.0)).x,
	      myAxes.FindPoint(wxRealPoint(0.5,1.0)).y,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,1.0)).x,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,1.0)).y);
  double LCL,UCL;
  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWSCCLIMITS)){
     
    wxColor tempColor = wxColor(100,100,255);
     
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor,2));
    
    
    computeIndividualsControlLimits((*NUSEBoxplotStatistics),2,&LCL,&UCL);
    
    wxPrintf(wxT("%f %f\n"),LCL,UCL);
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,LCL)).x,
		myAxes.FindPoint(wxRealPoint(0.5,LCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,LCL)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,LCL)).y);
    
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,UCL)).x,
		myAxes.FindPoint(wxRealPoint(0.5,UCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,UCL)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,UCL)).y);
    
    dc.SetPen(oldPen);
  }
  

  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWIQRLIMITS)){
    wxColor tempColor2 = wxColor(50,255,50);
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor2,2));
    
    
    computeIQRLimits((*NUSEBoxplotStatistics),2,&LCL,&UCL);
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,LCL)).x,
		myAxes.FindPoint(wxRealPoint(0.5,LCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,LCL)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,LCL)).y);
    
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,UCL)).x,
		myAxes.FindPoint(wxRealPoint(0.5,UCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,UCL)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,UCL)).y);
    
    dc.SetPen(oldPen);
  }
  







  for (j =0; j < ExpressionValues->count_arrays()-1;j++){


    dc.DrawLine(myAxes.FindPoint(wxRealPoint((double)(j+1),0)).x,
		myAxes.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[j*5 + 2])).y,
		myAxes.FindPoint(wxRealPoint((double)(j+2),0)).x,
		myAxes.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[(j+1)*5 + 2])).y);
  }

  for (j =0; j < ExpressionValues->count_arrays();j++){


    dc.DrawCircle(myAxes.FindPoint(wxRealPoint((double)(j+1),0)).x,myAxes.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[j*5 + 2])).y,(int)(3*(double)(dc.GetPPI().x)/(double)72));
    

  }



  dc.DestroyClippingRegion();
  myAxes.Draw(dc);

}





void QCStatsVisualizeFrame::DrawNUSEIQRs(wxDC& dc){

  int i,j;

  int w,h;
  dc.GetSize(&w, &h);

  
  int min_x = min((int)(pltarea_min_x*(double)w),(int)(160*(double)(dc.GetPPI().x)/(double)72));
  int min_y = min((int)(pltarea_min_y*(double)h),(int)(165*(double)(dc.GetPPI().x)/(double)72));
  int max_x = max((int)((pltarea_max_x-pltarea_min_x)*(double)w),(int)(w - 100*(double)(dc.GetPPI().x)/(double)72 - min_x));
  int max_y = max((int)((pltarea_max_y-pltarea_min_y)*(double)h),(int)(h-160*(double)(dc.GetPPI().x)/(double)72 - min_y));
 

  double axes_low = computeMin2((*NUSEBoxplotStatistics),1,3);
  double axes_high = computeMax2((*NUSEBoxplotStatistics),1,3);


  if (axes_high  < 0.1){
    axes_high = 0.1;
  } else {
    axes_high = round((axes_high + 0.05) * 10.0)/10.0;
  }
  axes_low = 0.0;




  labeledplotAxes myAxes(dc,wxPoint(min_x,min_y),
			 wxPoint(max_x,max_y),
			 wxRealPoint(0.5,axes_low),
			 wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5 ,axes_high),
			 true,
			 wxPoint(0,0));
  //dc.SetClippingRegion(80,30*5-80 ,w-30*5,h-30*5);  
  //dc.SetBackground(*wxGREY_BRUSH);
  //dc.Clear();

  //SetUpDC(dc);
  dc.Clear();
  myAxes.setBackgroundColor(*wxWHITE_BRUSH);
  myAxes.setupDCforPlot(dc);
  

  wxArrayDouble xtickpoints;
  wxArrayDouble ytickpoints;
  wxArrayString x_tick_labels;
  wxArrayString y_tick_labels;

  wxString temp;

  for (i =1; i <= ExpressionValues->count_arrays();i++){
    xtickpoints.Add((double)i);
    wxString mynewstring = wxString::Format(_T("%d"), i);
    x_tick_labels.Add(mynewstring); 
  }

  x_tick_labels = ExpressionValues->GetArrayNames();
  
  // Strip the ".CEL" from the labels 
  for (i =0; i < ExpressionValues->count_arrays();i++){
    temp = wxString(x_tick_labels[i]);
    x_tick_labels[i] = temp.Remove(temp.Len() - 4);

  }

  for (i=0; i <= 10; i++){
    ytickpoints.Add((double)axes_low + (double)i/10*(axes_high-axes_low)); 
    y_tick_labels.Add(wxString::Format(_T("%.3f"), axes_low + (double)i/10*(axes_high-axes_low)));
  }
  
  myAxes.setTickLocationX(xtickpoints);
  myAxes.setTickLocationY(ytickpoints);

  myAxes.setTickLabelsX(x_tick_labels);
  myAxes.setTickLabelsY(y_tick_labels);

  myAxes.setTickLabelsPerpendicular(true);


  wxString x_label = wxT("");
  wxString y_label = wxT("NUSE IQR");

  myAxes.setAxisLabelX(x_label);
  myAxes.setAxisLabelY(y_label);


  
  myAxes.setTitle(wxT("NUSE IQRs"));
 


  /* dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,1.0)).x,
	      myAxes.FindPoint(wxRealPoint(0.5,1.0)).y,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,1.0)).x,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,1.0)).y);
  */
 double LCL,UCL;
  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWSCCLIMITS)){
 
    wxColor tempColor = wxColor(100,100,255);
      
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor,2));
    
    
    computeIndividualsControlLimits2((*NUSEBoxplotStatistics),1,3,&LCL,&UCL);
    
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint(0.5,LCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,LCL)).y);
    
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint(0.5,UCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,UCL)).y);
    
    dc.SetPen(oldPen);
  }
  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWIQRLIMITS)){
    wxColor tempColor2 = wxColor(50,255,50);
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor2,2));
    
    
    computeIQRLimits2((*NUSEBoxplotStatistics),1,3,&LCL,&UCL);
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint(0.5,LCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,LCL)).y);
    
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint(0.5,UCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,UCL)).y);
    
    dc.SetPen(oldPen);
  }


  for (j =0; j < ExpressionValues->count_arrays()-1;j++){


    dc.DrawLine(myAxes.FindPoint(wxRealPoint((double)(j+1),0)).x,
		myAxes.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[j*5 + 3] - (*NUSEBoxplotStatistics)[j*5 + 1])).y,
		myAxes.FindPoint(wxRealPoint((double)(j+2),0)).x,
		myAxes.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[(j+1)*5 + 3] - (*NUSEBoxplotStatistics)[(j+1)*5 + 1])).y);
  }

  for (j =0; j < ExpressionValues->count_arrays();j++){


    dc.DrawCircle(myAxes.FindPoint(wxRealPoint((double)(j+1),0)).x,myAxes.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[j*5 + 3] - (*NUSEBoxplotStatistics)[j*5 + 1])).y,(int)(3*(double)(dc.GetPPI().x)/(double)72));
    

  }



  dc.DestroyClippingRegion();
  myAxes.Draw(dc);

}







void QCStatsVisualizeFrame::DrawRLEIQRs(wxDC& dc){

  int i,j;

  int w,h;
  dc.GetSize(&w, &h);

  
  int min_x = min((int)(pltarea_min_x*(double)w),(int)(160*(double)(dc.GetPPI().x)/(double)72));
  int min_y = min((int)(pltarea_min_y*(double)h),(int)(165*(double)(dc.GetPPI().x)/(double)72));
  int max_x = max((int)((pltarea_max_x-pltarea_min_x)*(double)w),(int)(w - 100*(double)(dc.GetPPI().x)/(double)72 - min_x));
  int max_y = max((int)((pltarea_max_y-pltarea_min_y)*(double)h),(int)(h-160*(double)(dc.GetPPI().x)/(double)72 - min_y));
 
  double axes_low = computeMin2((*RLEBoxplotStatistics),1,3);
  double axes_high = computeMax2((*RLEBoxplotStatistics),1,3);


  if (axes_high  < 0.5){
    axes_high = 0.5;
  } else {
    axes_high = round((axes_high + 0.05) * 10.0)/10.0;
  }
  axes_low = 0.0;



  labeledplotAxes myAxes(dc, wxPoint(min_x,min_y),
			 wxPoint(max_x,max_y),
			 wxRealPoint(0.5,axes_low),
			 wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5 ,axes_high),
			 true,
			 wxPoint(0,0));
  //dc.SetClippingRegion(80,30*5-80 ,w-30*5,h-30*5);  
  //dc.SetBackground(*wxGREY_BRUSH);
  //dc.Clear();

  //SetUpDC(dc);
  dc.Clear();
  myAxes.setBackgroundColor(*wxWHITE_BRUSH);
  myAxes.setupDCforPlot(dc);
  

  wxArrayDouble xtickpoints;
  wxArrayDouble ytickpoints;
  wxArrayString x_tick_labels;
  wxArrayString y_tick_labels;

  wxString temp;

  for (i =1; i <= ExpressionValues->count_arrays();i++){
    xtickpoints.Add((double)i);
    wxString mynewstring = wxString::Format(_T("%d"), i);
    x_tick_labels.Add(mynewstring); 
  }

  x_tick_labels = ExpressionValues->GetArrayNames();
  
  // Strip the ".CEL" from the labels 
  for (i =0; i < ExpressionValues->count_arrays();i++){
    temp = wxString(x_tick_labels[i]);
    x_tick_labels[i] = temp.Remove(temp.Len() - 4);

  }

  for (i=0; i <= 10; i++){
    ytickpoints.Add((double)axes_low + (double)i/10*(axes_high-axes_low)); 
    y_tick_labels.Add(wxString::Format(_T("%.3f"), axes_low + (double)i/10*(axes_high-axes_low)));
  }
  
  myAxes.setTickLocationX(xtickpoints);
  myAxes.setTickLocationY(ytickpoints);

  myAxes.setTickLabelsX(x_tick_labels);
  myAxes.setTickLabelsY(y_tick_labels);

  myAxes.setTickLabelsPerpendicular(true);


  wxString x_label = wxT("");
  wxString y_label = wxT("RLE IQR");

  myAxes.setAxisLabelX(x_label);
  myAxes.setAxisLabelY(y_label);


  
  myAxes.setTitle(wxT("RLE IQRs"));


  


  /* dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,1.0)).x,
     myAxes.FindPoint(wxRealPoint(0.5,1.0)).y,
     myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,1.0)).x,
     myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,1.0)).y);
  */
  double LCL,UCL;
  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWSCCLIMITS)){
    
    wxColor tempColor = wxColor(100,100,255);
    
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor,2));
    
    
    computeIndividualsControlLimits2((*RLEBoxplotStatistics),1,3,&LCL,&UCL);
    
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint(0.5,LCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,LCL)).y);
    
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint(0.5,UCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,UCL)).y);
    
    dc.SetPen(oldPen);
  }
  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWIQRLIMITS)){
    wxColor tempColor2 = wxColor(50,255,50);
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor2,2));
    
    
    computeIQRLimits2((*RLEBoxplotStatistics),1,3,&LCL,&UCL);
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint(0.5,LCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,LCL)).y);
    
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint(0.5,UCL)).y,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,UCL)).y);
    
    dc.SetPen(oldPen);
  }
  

  
  for (j =0; j < ExpressionValues->count_arrays()-1;j++){
    
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint((double)(j+1),0)).x,
		myAxes.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[j*5 + 3] - (*RLEBoxplotStatistics)[j*5 + 1])).y,
		myAxes.FindPoint(wxRealPoint((double)(j+2),0)).x,
		myAxes.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[(j+1)*5 + 3] - (*RLEBoxplotStatistics)[(j+1)*5 + 1])).y);
  }
  
  for (j =0; j < ExpressionValues->count_arrays();j++){
    
    
    dc.DrawCircle(myAxes.FindPoint(wxRealPoint((double)(j+1),0)).x,myAxes.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[j*5 + 3] - (*RLEBoxplotStatistics)[j*5 + 1])).y,(int)(3*(double)(dc.GetPPI().x)/(double)72));
    

  }



  dc.DestroyClippingRegion();
  myAxes.Draw(dc);

}



void QCStatsVisualizeFrame::DrawRLENUSEMultiPlot(wxDC& dc){

  int i,j;

  int w,h;
  dc.GetSize(&w, &h);

  
  int min_x = min((int)(pltarea_min_x*(double)w),(int)(160*(double)(dc.GetPPI().x)/(double)72));
  int min_y = min((int)(pltarea_min_y*(double)h),(int)(165*(double)(dc.GetPPI().x)/(double)72));
  int max_x = max((int)((pltarea_max_x-pltarea_min_x)*(double)w),(int)(w - 100*(double)(dc.GetPPI().x)/(double)72 - min_x));
  int max_y = max((int)((pltarea_max_y-pltarea_min_y)*(double)h),(int)(h-160*(double)(dc.GetPPI().x)/(double)72 - min_y));
 

  labeledplotAxes myAxes(dc, wxPoint(min_x,min_y),
			 wxPoint(max_x,max_y),
			 wxRealPoint(0.5,0.0),
			 wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5 ,1.0),
			 true,
			 wxPoint(0,0));

  dc.Clear();
  myAxes.setBackgroundColor(*wxWHITE_BRUSH);
  myAxes.setupDCforPlot(dc);
  
  
  wxArrayDouble xtickpoints;
  wxArrayDouble ytickpoints;
  wxArrayString x_tick_labels;
  wxArrayString y_tick_labels;

  wxString temp;

  for (i =1; i <= ExpressionValues->count_arrays();i++){
    xtickpoints.Add((double)i);
    wxString mynewstring = wxString::Format(_T("%d"), i);
    x_tick_labels.Add(mynewstring); 
  }

  x_tick_labels = ExpressionValues->GetArrayNames();
  
  // Strip the ".CEL" from the labels 
  for (i =0; i < ExpressionValues->count_arrays();i++){
    temp = wxString(x_tick_labels[i]);
    x_tick_labels[i] = temp.Remove(temp.Len() - 4);

  }
  myAxes.setTickLocationX(xtickpoints);

  myAxes.setTickLabelsX(x_tick_labels);

  myAxes.setTickLabelsPerpendicular(true);

  wxString x_label = wxT("");
  wxString y_label = wxT("");

  myAxes.setAxisLabelX(x_label);
  myAxes.setAxisLabelY(y_label);


  
  myAxes.setTitle(wxT("RLE-NUSE Multiplot"));
    

  double RLE_IQR_LCL,RLE_IQR_UCL;
  double RLE_IQR_LCL2,RLE_IQR_UCL2;
  double RLE_IQR_axes_low = computeMin2((*RLEBoxplotStatistics),1,3);
  double RLE_IQR_axes_high = computeMax2((*RLEBoxplotStatistics),1,3);
  computeIndividualsControlLimits2((*RLEBoxplotStatistics),1,3,&RLE_IQR_LCL,&RLE_IQR_UCL);
  computeIQRLimits2((*RLEBoxplotStatistics),1,3,&RLE_IQR_LCL2,&RLE_IQR_UCL2);
  
  
  if (RLE_IQR_LCL < 0.0){
    RLE_IQR_LCL = 0.0;
  }

  if (RLE_IQR_LCL2 < 0.0){
    RLE_IQR_LCL2 = 0.0;
  }



  if (RLE_IQR_UCL > RLE_IQR_axes_high){
    RLE_IQR_axes_high = RLE_IQR_UCL;
  }
  if (RLE_IQR_UCL2 > RLE_IQR_axes_high){
    RLE_IQR_axes_high = RLE_IQR_UCL2;
  }


  if (RLE_IQR_axes_high  < 0.5){
    RLE_IQR_axes_high = 0.5;
  } else {
    RLE_IQR_axes_high = round((RLE_IQR_axes_high + 0.05) * 10.0)/10.0;
  }
  RLE_IQR_axes_low = 0.0;

  
  double NUSE_IQR_LCL,NUSE_IQR_UCL;
  double NUSE_IQR_LCL2,NUSE_IQR_UCL2;
  double NUSE_IQR_axes_low = computeMin2((*NUSEBoxplotStatistics),1,3);
  double NUSE_IQR_axes_high = computeMax2((*NUSEBoxplotStatistics),1,3);
  computeIndividualsControlLimits2((*NUSEBoxplotStatistics),1,3,&NUSE_IQR_LCL,&NUSE_IQR_UCL);
  computeIQRLimits2((*NUSEBoxplotStatistics),1,3,&NUSE_IQR_LCL2,&NUSE_IQR_UCL2);
  if (NUSE_IQR_LCL < 0.0){
    NUSE_IQR_LCL = 0.0;
  }

  if (NUSE_IQR_UCL > RLE_IQR_axes_high){
    NUSE_IQR_axes_high = RLE_IQR_UCL;
  }
  
  if (NUSE_IQR_axes_high  < 0.1){
    NUSE_IQR_axes_high = 0.1;
  } else {
    NUSE_IQR_axes_high = round((NUSE_IQR_axes_high + 0.05) * 10.0)/10.0;
  }
  NUSE_IQR_axes_low = 0.0;

  double NUSE_MEDIAN_LCL,NUSE_MEDIAN_UCL;
  double NUSE_MEDIAN_LCL2,NUSE_MEDIAN_UCL2;
  double NUSE_MEDIAN_axes_low = computeMin((*NUSEBoxplotStatistics),2);
  double NUSE_MEDIAN_axes_high = computeMax((*NUSEBoxplotStatistics),2);
  computeIndividualsControlLimits((*NUSEBoxplotStatistics),2,&NUSE_MEDIAN_LCL,&NUSE_MEDIAN_UCL);
  computeIQRLimits((*NUSEBoxplotStatistics),2,&NUSE_MEDIAN_LCL2,&NUSE_MEDIAN_UCL2);
  
  if (NUSE_MEDIAN_LCL < NUSE_MEDIAN_axes_low){
    NUSE_MEDIAN_axes_low = NUSE_MEDIAN_LCL;
  }
  if (NUSE_MEDIAN_UCL > NUSE_MEDIAN_axes_high){
    NUSE_MEDIAN_axes_high = NUSE_MEDIAN_UCL;
  }
  
  if (NUSE_MEDIAN_LCL2 < NUSE_MEDIAN_axes_low){
    NUSE_MEDIAN_axes_low = NUSE_MEDIAN_LCL2;
  }
  if (NUSE_MEDIAN_UCL2 > NUSE_MEDIAN_axes_high){
    NUSE_MEDIAN_axes_high = NUSE_MEDIAN_UCL2;
  }

  if (NUSE_MEDIAN_axes_low > 0.95){
    NUSE_MEDIAN_axes_low = 0.95;
  } else {
    NUSE_MEDIAN_axes_low = trunc(NUSE_MEDIAN_axes_low*10.0)/10.0;
  }
  
  
  if (NUSE_MEDIAN_axes_high < 1.20){
    NUSE_MEDIAN_axes_high = 1.2;
  } else {
    NUSE_MEDIAN_axes_high = round((NUSE_MEDIAN_axes_high+.05)*10.0)/10.0;
  }
    
  
  double RLE_MEDIAN_LCL,RLE_MEDIAN_UCL;
  double RLE_MEDIAN_LCL2,RLE_MEDIAN_UCL2;
  double RLE_MEDIAN_axes_low = computeMin((*RLEBoxplotStatistics),2);
  double RLE_MEDIAN_axes_high = computeMax((*RLEBoxplotStatistics),2);
  computeIndividualsControlLimits((*RLEBoxplotStatistics),2,&RLE_MEDIAN_LCL,&RLE_MEDIAN_UCL);
  computeIQRLimits((*RLEBoxplotStatistics),2,&RLE_MEDIAN_LCL2,&RLE_MEDIAN_UCL2);


  if (RLE_MEDIAN_LCL < RLE_MEDIAN_axes_low){
    RLE_MEDIAN_axes_low = RLE_MEDIAN_LCL;
  }
  
  if (RLE_MEDIAN_UCL > RLE_MEDIAN_axes_high){
    RLE_MEDIAN_axes_high = RLE_MEDIAN_UCL;
  }
 
  if (RLE_MEDIAN_LCL2 < RLE_MEDIAN_axes_low){
    RLE_MEDIAN_axes_low = RLE_MEDIAN_LCL2;
  }
  
  if (RLE_MEDIAN_UCL2 > RLE_MEDIAN_axes_high){
    RLE_MEDIAN_axes_high = RLE_MEDIAN_UCL2;
  }


  if (RLE_MEDIAN_axes_low > -0.1){
    RLE_MEDIAN_axes_low = -0.1;
  } else {
    RLE_MEDIAN_axes_low = round((RLE_MEDIAN_axes_low-0.05)*10.0)/10.0;
  }
  
  
  if (RLE_MEDIAN_axes_high < 0.1){
    RLE_MEDIAN_axes_high = 0.1;
  } else {
    RLE_MEDIAN_axes_high= round((RLE_MEDIAN_axes_high+.05)*10.0)/10.0;
  }



  labeledplotAxes myAxes2(dc, wxPoint(min_x,min_y),
			  wxPoint(max_x,(int)(0.25*((double)max_y))),
			  wxRealPoint(0.5,NUSE_IQR_axes_low),
			  wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5 ,NUSE_IQR_axes_high),
			  true,
			  wxPoint(0,0));
 
  labeledplotAxes myAxes3(dc, wxPoint(min_x,int(min_y + 0.25*(max_y))),
			  wxPoint(max_x,(int)(0.25*((double)max_y))),
			  wxRealPoint(0.5,NUSE_MEDIAN_axes_low),
			 wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5 ,NUSE_MEDIAN_axes_high),
			 true,
			 wxPoint(0,0));  
  labeledplotAxes myAxes4(dc, wxPoint(min_x,int(min_y + 0.5*(max_y))),
			  wxPoint(max_x,(int)(0.25*((double)max_y))),
			  wxRealPoint(0.5,RLE_IQR_axes_low),
			  wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5 ,RLE_IQR_axes_high),
			  true,
			  wxPoint(0,0));  
  labeledplotAxes myAxes5(dc, wxPoint(min_x,int(min_y + 0.75*(max_y))),
			  wxPoint(max_x,(int)(0.25*((double)max_y))),
			  wxRealPoint(0.5,RLE_MEDIAN_axes_low),
			  wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5 ,RLE_MEDIAN_axes_high),
			  true,
			  wxPoint(0,0)); 


  myAxes2.setTitle(wxT(""));
  myAxes3.setTitle(wxT(""));
  myAxes4.setTitle(wxT(""));
  myAxes5.setTitle(wxT(""));
  
  y_label = wxT("RLE Median");
  myAxes5.setAxisLabelY(y_label);

  y_label = wxT("RLE IQR");
  myAxes4.setAxisLabelY(y_label);
  
  y_label = wxT("NUSE Median");
  myAxes3.setAxisLabelY(y_label);

  y_label = wxT("NUSE IQR");
  myAxes2.setAxisLabelY(y_label);
  


  /* RLE Medians Plot */
  for (j =0; j < ExpressionValues->count_arrays()-1;j++){
    dc.DrawLine(myAxes5.FindPoint(wxRealPoint((double)(j+1),0)).x,
		myAxes5.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[j*5 + 2])).y,
		myAxes5.FindPoint(wxRealPoint((double)(j+2),0)).x,
		myAxes5.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[(j+1)*5 + 2])).y);
  }
  
  for (j =0; j < ExpressionValues->count_arrays();j++){
    dc.DrawCircle(myAxes5.FindPoint(wxRealPoint((double)(j+1),0)).x,myAxes5.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[j*5 + 2])).y,(int)(3*(double)(dc.GetPPI().x)/(double)72) );
  }
  
  /* NUSE Medians Plot */
  for (j =0; j < ExpressionValues->count_arrays()-1;j++){
    
    
    dc.DrawLine(myAxes3.FindPoint(wxRealPoint((double)(j+1),0)).x,
		myAxes3.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[j*5 + 2])).y,
		myAxes3.FindPoint(wxRealPoint((double)(j+2),0)).x,
		myAxes3.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[(j+1)*5 + 2])).y);
  }
  
  for (j =0; j < ExpressionValues->count_arrays();j++){
    
    
    dc.DrawCircle(myAxes3.FindPoint(wxRealPoint((double)(j+1),0)).x,myAxes3.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[j*5 + 2])).y,(int)(3*(double)(dc.GetPPI().x)/(double)72));
  }


  /* NUSE IQRS Plot */
  for (j =0; j < ExpressionValues->count_arrays()-1;j++){


    dc.DrawLine(myAxes2.FindPoint(wxRealPoint((double)(j+1),0)).x,
		myAxes2.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[j*5 + 3] - (*NUSEBoxplotStatistics)[j*5 + 1])).y,
		myAxes2.FindPoint(wxRealPoint((double)(j+2),0)).x,
		myAxes2.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[(j+1)*5 + 3] - (*NUSEBoxplotStatistics)[(j+1)*5 + 1])).y);
  }

  for (j =0; j < ExpressionValues->count_arrays();j++){
    dc.DrawCircle(myAxes2.FindPoint(wxRealPoint((double)(j+1),0)).x,myAxes2.FindPoint(wxRealPoint(1.0,(*NUSEBoxplotStatistics)[j*5 + 3] - (*NUSEBoxplotStatistics)[j*5 + 1])).y,(int)(3*(double)(dc.GetPPI().x)/(double)72));
  }

 /* RLE IQRS Plot */
  for (j =0; j < ExpressionValues->count_arrays()-1;j++){
    dc.DrawLine(myAxes4.FindPoint(wxRealPoint((double)(j+1),0)).x,
		myAxes4.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[j*5 + 3] - (*RLEBoxplotStatistics)[j*5 + 1])).y,
		myAxes4.FindPoint(wxRealPoint((double)(j+2),0)).x,
		myAxes4.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[(j+1)*5 + 3] - (*RLEBoxplotStatistics)[(j+1)*5 + 1])).y);
  }
  
  for (j =0; j < ExpressionValues->count_arrays();j++){
    dc.DrawCircle(myAxes4.FindPoint(wxRealPoint((double)(j+1),0)).x,myAxes4.FindPoint(wxRealPoint(1.0,(*RLEBoxplotStatistics)[j*5 + 3] - (*RLEBoxplotStatistics)[j*5 + 1])).y,(int)(3*(double)(dc.GetPPI().x)/(double)72));
  }

  wxColor tempColor = wxColor(100,100,255);
  /* Control limits for RLEIQR */


  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWSCCLIMITS)){
    
    wxColor tempColor = wxColor(100,100,255);
    
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor,2));
    
    dc.DrawLine(myAxes4.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes4.FindPoint(wxRealPoint(0.5,RLE_IQR_LCL)).y,
		myAxes4.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes4.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,RLE_IQR_LCL)).y);
    
    
    dc.DrawLine(myAxes4.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes4.FindPoint(wxRealPoint(0.5,RLE_IQR_UCL)).y,
		myAxes4.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes4.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,RLE_IQR_UCL)).y);
    
    dc.SetPen(oldPen);
  }
  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWIQRLIMITS)){
    wxColor tempColor2 = wxColor(50,255,50);
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor2,2));
    dc.DrawLine(myAxes4.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes4.FindPoint(wxRealPoint(0.5,RLE_IQR_LCL2)).y,
		myAxes4.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes4.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,RLE_IQR_LCL2)).y);
    
    
    dc.DrawLine(myAxes4.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes4.FindPoint(wxRealPoint(0.5,RLE_IQR_UCL2)).y,
		myAxes4.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes4.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,RLE_IQR_UCL2)).y);
    
    dc.SetPen(oldPen);
  }
  
    

  /* Control limits for NUSE IQR */
  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWSCCLIMITS)){
 
    wxColor tempColor = wxColor(100,100,255);
      
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor,2));
    
    
    computeIndividualsControlLimits2((*NUSEBoxplotStatistics),1,3,&NUSE_IQR_LCL,& NUSE_IQR_UCL);
    
    
    dc.DrawLine(myAxes2.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes2.FindPoint(wxRealPoint(0.5,NUSE_IQR_LCL)).y,
		myAxes2.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes2.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5, NUSE_IQR_LCL)).y);
    
    
    dc.DrawLine(myAxes2.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes2.FindPoint(wxRealPoint(0.5,NUSE_IQR_UCL)).y,
		myAxes2.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes2.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5, NUSE_IQR_UCL)).y);
    
    dc.SetPen(oldPen);
  }
  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWIQRLIMITS)){
    wxColor tempColor2 = wxColor(50,255,50);
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor2,2));
    dc.DrawLine(myAxes2.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes2.FindPoint(wxRealPoint(0.5,NUSE_IQR_LCL2)).y,
		myAxes2.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes2.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5, NUSE_IQR_LCL2)).y);
    dc.DrawLine(myAxes2.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes2.FindPoint(wxRealPoint(0.5,NUSE_IQR_UCL2)).y,
		myAxes2.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes2.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5, NUSE_IQR_UCL2)).y);
    dc.SetPen(oldPen);
  }


  /* Control limits for NUSE medians */
  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWSCCLIMITS)){
     
    wxColor tempColor = wxColor(100,100,255);
     
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor,2));
    
    dc.DrawLine(myAxes3.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes3.FindPoint(wxRealPoint(0.5,NUSE_MEDIAN_LCL)).y,
		myAxes3.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes3.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,NUSE_MEDIAN_LCL)).y);
    
    
    dc.DrawLine(myAxes3.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes3.FindPoint(wxRealPoint(0.5,NUSE_MEDIAN_UCL)).y,
		myAxes3.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes3.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,NUSE_MEDIAN_UCL)).y);
    
    dc.SetPen(oldPen);
  }
  

  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWIQRLIMITS)){
    wxColor tempColor2 = wxColor(50,255,50);
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor2,2));
    dc.DrawLine(myAxes3.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes3.FindPoint(wxRealPoint(0.5,NUSE_MEDIAN_LCL2)).y,
		myAxes3.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes3.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,NUSE_MEDIAN_LCL2)).y);
    
    
    dc.DrawLine(myAxes3.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes3.FindPoint(wxRealPoint(0.5,NUSE_MEDIAN_UCL2)).y,
		myAxes3.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes3.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,NUSE_MEDIAN_UCL2)).y);
    
    dc.SetPen(oldPen);
  }


  /*RLE Median Limits */

  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWSCCLIMITS)){
    
    
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor,2));
    
    dc.DrawLine(myAxes5.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes5.FindPoint(wxRealPoint(0.5,RLE_MEDIAN_LCL)).y,
		myAxes5.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes5.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,RLE_MEDIAN_LCL)).y);
    
    
    dc.DrawLine(myAxes5.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes5.FindPoint(wxRealPoint(0.5,RLE_MEDIAN_UCL)).y,
		myAxes5.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes5.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,RLE_MEDIAN_UCL)).y);
    
    dc.SetPen(oldPen);
  }

  if(drawMenu->IsChecked(ID_VISUALIZE_DRAWIQRLIMITS)){
    wxColor tempColor2 = wxColor(50,255,50);
    wxPen oldPen = dc.GetPen();
    
    dc.SetPen(wxPen(tempColor2,2));
    
    dc.DrawLine(myAxes5.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes5.FindPoint(wxRealPoint(0.5,RLE_MEDIAN_LCL2)).y,
		myAxes5.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes5.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,RLE_MEDIAN_LCL2)).y);
    
    
    dc.DrawLine(myAxes5.FindPoint(wxRealPoint(0.5,0.0)).x,
		myAxes5.FindPoint(wxRealPoint(0.5,RLE_MEDIAN_UCL2)).y,
		myAxes5.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
		myAxes5.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,RLE_MEDIAN_UCL2)).y);
    
    dc.SetPen(oldPen);
  }

  



  dc.DestroyClippingRegion();
  myAxes.Draw(dc);
  myAxes2.Draw(dc);
  myAxes3.Draw(dc);
  myAxes4.Draw(dc);
  myAxes5.Draw(dc);
}

void QCStatsVisualizeFrame::ComputeRLENUSE_T2(){

  int i,j,k;

  Matrix X(ExpressionValues->count_arrays(),4);
  
  T2 = new Matrix(ExpressionValues->count_arrays(),1);
  
  double S[16];
  double Sinv[16];
  double means[4];
  
  for (j =0; j < ExpressionValues->count_arrays();j++){
    X[0*X.Rows() +j] = (*RLEBoxplotStatistics)[j*5 + 2];
    X[1*X.Rows()  +j] = log((*RLEBoxplotStatistics)[j*5 + 3] - (*RLEBoxplotStatistics)[j*5 + 1]);
    X[2*X.Rows()  +j] = (*NUSEBoxplotStatistics)[j*5 + 2];
    X[3*X.Rows()  +j] = log((*NUSEBoxplotStatistics)[j*5 + 3] - (*NUSEBoxplotStatistics)[j*5 + 1]);
  }
  
  for (j =0; j < 4;j++){
    means[j] = 0.0;
  }
  
  for (j =0; j < ExpressionValues->count_arrays();j++){
    means[0]+=X[0*X.Rows() +j]; 
    means[1]+= X[1*X.Rows()  +j];
    means[2]+= X[2*X.Rows()  +j];
    means[3]+= X[3*X.Rows()  +j];
  }



  // wxPrintf(wxT("Means "));
  for (j =0; j < 4;j++){
    means[j]/= (double)X.Rows();
    //  wxPrintf(wxT("%f "),means[j]);
  }
  // wxPrintf(wxT("\n"));

  
  for (j =0; j < ExpressionValues->count_arrays();j++){
    X[0*X.Rows() +j]-=means[0];
    X[1*X.Rows()  +j]-=means[1]; 
    X[2*X.Rows()  +j]-=means[2]; 
    X[3*X.Rows()  +j]-=means[3]; 
    // wxPrintf(wxT("%f %f %f %f\n"),X[0*X.Rows() +j], X[1*X.Rows()  +j],X[2*X.Rows()  +j],X[3*X.Rows()  +j]);
  }
  

  //wxPrintf(wxT("Covariances\n"));
  for (j = 0; j < 4; j++){
    for (i = 0; i < 4; i++){
      S[j*4 + i] = 0.0;
      for (k =0; k < ExpressionValues->count_arrays(); k++){
	S[j*4 + i]+=X[j*X.Rows() + k]*X[i*X.Rows() + k];
      }
      S[j*4 + i]/=((double)(X.Rows() - 1));
      //   wxPrintf(wxT("%f "),S[j*4 + i]);
    } 
    // wxPrintf(wxT("\n"));
  }
  
  SVD_inverse(S, Sinv, 4);
  
  // wxPrintf(wxT("Inverted Covariances\n"));
  for (j=0; j < 4; j++){
    for (i = 0; i < 4; i++){
      // wxPrintf(wxT("%f "),Sinv[j*4 + i]);
    }
    // wxPrintf(wxT("\n"));
  }


  for (k=0; k < ExpressionValues->count_arrays(); k++){
    (*T2)[k] = 0.0;
    for (j = 0; j < 4; j++){
      S[j] = 0.0;
      for (i = 0; i < 4; i++){
	//	wxPrintf(wxT("Multiplying %d  with %d %d FI %d    "),i,i,j,j*4 + i);
	//	wxPrintf(wxT("%f %f\n"),X[i*X.Rows() + k], Sinv[j*4 + i]);
      	//(*T2)[k]+= (X[i*X.Cols() + k]*Sinv[j*4 + i]);
	S[j]+=(X[i*X.Rows() + k]*Sinv[j*4 + i]);
      }
      //  wxPrintf(wxT("%f "),S[j]);
    }
    //  wxPrintf(wxT("\n"));
    for (j=0; j < 4; j++){
      (*T2)[k]+=S[j]*X[j*X.Rows() + k];
    }
    //   wxPrintf(wxT("%f\n"), (*T2)[k]);
  }
}


/*
  if (n > 100) use these approximations:
  1/beta(.95) = 0.07865422+  0.10539929*x
  1/beta(.99) = 0.19909886+ 0.07531989*x

*/ 

static void ComputeT2_limits(int n, double *UCL1, double *UCL2){

  double beta95[100] ={0,0,0,0,0,
		       0.99888806, 0.97467943, 0.92399046, 0.86464964, 0.80596594, 0.75139537,
                       0.70188977, 0.65740832, 0.61754782, 0.58180341, 0.54967421, 0.52070297,
		       0.49448735, 0.47067941, 0.44898062, 0.42913555, 0.41092558, 0.39416330,
		       0.37868760, 0.36435949, 0.35105869, 0.33868067, 0.32713429, 0.31633976,
		       0.30622701, 0.29673424, 0.28780680, 0.27939619, 0.27145924, 0.26395740,
		       0.25685613, 0.25012445, 0.24373444, 0.23766092, 0.23188107, 0.22637425,
		       0.22112165, 0.21610616, 0.21131217, 0.20672538, 0.20233270, 0.19812213,
		       0.19408261, 0.19020397, 0.18647681, 0.18289247, 0.17944292, 0.17612071,
		       0.17291895, 0.16983122, 0.16685156, 0.16397439, 0.16119455, 0.15850717,
		       0.15590775, 0.15339202, 0.15095604, 0.14859607, 0.14630861, 0.14409039,
		       0.14193831, 0.13984946, 0.13782110, 0.13585065, 0.13393566, 0.13207383,
		       0.13026297, 0.12850104, 0.12678606, 0.12511620, 0.12348970, 0.12190490,
		       0.12036020, 0.11885412, 0.11738522, 0.11595215, 0.11455361, 0.11318836,
		       0.11185524, 0.11055313, 0.10928095, 0.10803770, 0.10682238, 0.10563408,
		       0.10447191, 0.10333500, 0.10222255, 0.10113378, 0.10006794, 0.09902432,
		       0.09800222, 0.09700099, 0.09601999, 0.09505862, 0.09411630};

  double beta99[100] = {0,0,0,0,0,
			 0.9999556, 0.9949874, 0.9745417, 0.9410969, 0.9011227, 0.8591325, 0.8176446,
			 0.7779277, 0.7405468, 0.7056863, 0.6733307, 0.6433646, 0.6156271, 0.5899420,
			 0.5661334, 0.5440337, 0.5234874, 0.5043527, 0.4865008, 0.4698161, 0.4541947,
			 0.4395435, 0.4257791, 0.4128265, 0.4006188, 0.3890955, 0.3782024, 0.3678906,
			 0.3581159, 0.3488383, 0.3400218, 0.3316333, 0.3236431, 0.3160238, 0.3087507,
			 0.3018010, 0.2951539, 0.2887904, 0.2826928, 0.2768450, 0.2712321, 0.2658405,
			 0.2606573, 0.2556709, 0.2508703, 0.2462455, 0.2417870, 0.2374861, 0.2333346,
			 0.2293249, 0.2254499, 0.2217031, 0.2180781, 0.2145691, 0.2111708, 0.2078780,
			 0.2046858, 0.2015898, 0.1985856, 0.1956694, 0.1928372, 0.1900855, 0.1874110,
			 0.1848105, 0.1822809, 0.1798194, 0.1774233, 0.1750900, 0.1728171, 0.1706022,
			 0.1684433, 0.1663382, 0.1642849, 0.1622816, 0.1603264, 0.1584177, 0.1565538,
			 0.1547331, 0.1529542, 0.1512156, 0.1495160, 0.1478542, 0.1462288, 0.1446386,
			 0.1430827, 0.1415597, 0.1400688, 0.1386090, 0.1371791, 0.1357785, 0.1344061,
			 0.1330611, 0.1317427, 0.1304502, 0.1291827};


  
  if (n <= 100){
    *UCL1 = (double)((n -1)*(n-1))/(double)(n)*beta95[n-1];
    *UCL2 = (double)((n -1)*(n-1))/(double)(n)*beta99[n-1];
  } else {
    *UCL1 = (double)((n -1)*(n-1))/(double)(n)*(1.0/(0.07865422+  0.10539929*(double)n));
    *UCL2 = (double)((n -1)*(n-1))/(double)(n)*(1.0/( 0.19909886+ 0.07531989*(double)n));
  }

}





  
void QCStatsVisualizeFrame::DrawMultivariateControlChart(wxDC& dc){

  int i,j;

  int w,h;
  dc.GetSize(&w, &h);
  
  int min_x = min((int)(pltarea_min_x*(double)w),(int)(160*(double)(dc.GetPPI().x)/(double)72));
  int min_y = min((int)(pltarea_min_y*(double)h),(int)(165*(double)(dc.GetPPI().x)/(double)72));
  int max_x = max((int)((pltarea_max_x-pltarea_min_x)*(double)w),(int)(w - 100*(double)(dc.GetPPI().x)/(double)72 - min_x));
  int max_y = max((int)((pltarea_max_y-pltarea_min_y)*(double)h),(int)(h-160*(double)(dc.GetPPI().x)/(double)72 - min_y));
   

  double axes_low = computeMinCol((*T2),0);
  double axes_high = computeMaxCol((*T2),0);

    
  double UCL1, UCL2;

  ComputeT2_limits(ExpressionValues->count_arrays(), &UCL1, &UCL2);
  
  if (UCL2 > axes_high){
    axes_high = UCL2;
  }



  //  wxPrintf(wxT("Axes limits: %f %f\n"),axes_low, axes_high);
  if (axes_high < 5.00){
    axes_high = 5.00;
  } else {
    axes_high = round((axes_high+.05)*10.0)/10.0;
  } 
  axes_low = 0.0;

  labeledplotAxes myAxes(dc, wxPoint(min_x,min_y),
			 wxPoint(max_x,max_y),
			 wxRealPoint(0.5,axes_low),
			 wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5 ,axes_high),
			 true,
			 wxPoint(0,0));
  //dc.SetClippingRegion(80,30*5-80 ,w-30*5,h-30*5);  
  //dc.SetBackground(*wxGREY_BRUSH);
  //dc.Clear();

  //SetUpDC(dc);
  dc.Clear();
  myAxes.setBackgroundColor(*wxWHITE_BRUSH);
  myAxes.setupDCforPlot(dc);
  

  wxArrayDouble xtickpoints;
  wxArrayDouble ytickpoints;
  wxArrayString x_tick_labels;
  wxArrayString y_tick_labels;

  wxString temp;

  for (i =1; i <= ExpressionValues->count_arrays();i++){
    xtickpoints.Add((double)i);
    wxString mynewstring = wxString::Format(_T("%d"), i);
    x_tick_labels.Add(mynewstring); 
  }

  x_tick_labels = ExpressionValues->GetArrayNames();
  
  // Strip the ".CEL" from the labels 
  for (i =0; i < ExpressionValues->count_arrays();i++){
    temp = wxString(x_tick_labels[i]);
    x_tick_labels[i] = temp.Remove(temp.Len() - 4);

  }

  for (i=0; i <= 10; i++){
    ytickpoints.Add((double)axes_low + (double)i/10*(axes_high-axes_low)); 
    y_tick_labels.Add(wxString::Format(_T("%.3f"), axes_low + (double)i/10*(axes_high-axes_low)));
  }
  
  myAxes.setTickLocationX(xtickpoints);
  myAxes.setTickLocationY(ytickpoints);

  myAxes.setTickLabelsX(x_tick_labels);
  myAxes.setTickLabelsY(y_tick_labels);

  myAxes.setTickLabelsPerpendicular(true);


  wxString x_label = wxT("");
  wxString y_label = wxT("T2");

  myAxes.setAxisLabelX(x_label);
  myAxes.setAxisLabelY(y_label);


  
  myAxes.setTitle(wxT("RLE-NUSE T2"));

  wxColor tempColor2 = wxColor(255,25,25);
  wxPen oldPen = dc.GetPen();
  
  dc.SetPen(wxPen(tempColor2,1, wxDOT));
 
  dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
	      myAxes.FindPoint(wxRealPoint(0.5,UCL1)).y,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,UCL1)).y);
 

  dc.SetPen(wxPen(tempColor2,2));
  dc.DrawLine(myAxes.FindPoint(wxRealPoint(0.5,0.0)).x,
	      myAxes.FindPoint(wxRealPoint(0.5,UCL2)).y,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,0.0)).x,
	      myAxes.FindPoint(wxRealPoint((double)ExpressionValues->count_arrays()+ 0.5,UCL2)).y);

  dc.SetPen(oldPen);


  for (j =0; j < ExpressionValues->count_arrays()-1;j++){
    
    
    dc.DrawLine(myAxes.FindPoint(wxRealPoint((double)(j+1),0)).x,
		myAxes.FindPoint(wxRealPoint(1.0,(*T2)[j])).y,
		myAxes.FindPoint(wxRealPoint((double)(j+2),0)).x,
		myAxes.FindPoint(wxRealPoint(1.0,(*T2)[j+1])).y);
  }
  
  for (j =0; j < ExpressionValues->count_arrays();j++){
    dc.DrawCircle(myAxes.FindPoint(wxRealPoint((double)(j+1),0)).x,myAxes.FindPoint(wxRealPoint(1.0,(*T2)[j])).y,(int)(3*(double)(dc.GetPPI().x)/(double)72));
  }



  dc.DestroyClippingRegion();
  myAxes.Draw(dc);


}

void QCStatsVisualizeFrame::ToggleSCCLimits(wxCommandEvent& event){

  MyWindow->Refresh();

}

void QCStatsVisualizeFrame::ToggleIQRLimits(wxCommandEvent& event){

  MyWindow->Refresh();

}







#endif


#if RMA_GUI_APP

void QCStatsVisualizeFrame::OnSaveQCRLE(wxCommandEvent& event){
  
  GenerateRLE();
  
  wxFileDialog
    * ResultsFileDialog =
    new wxFileDialog ( this,
		       wxT("Save the results as text file"),
		       wxT(""),
		       wxT(""),
		       wxT("*.RLE.txt"),
		       wxFD_SAVE,
		       wxDefaultPosition);  
  
  if (ResultsFileDialog->ShowModal() == wxID_OK){
    if (wxFileExists(ResultsFileDialog->GetPath())){
      wxString t=_T("This file already exists. Are you sure that you want to overwrite this file."); 
      wxMessageDialog
	aboutDialog
	(0, t, _T("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	return;
      }
    }
    ExpressionValues->writetofile(ResultsFileDialog->GetFilename(),ResultsFileDialog->GetDirectory(),0);
  } else {
    return;
  }


};



void QCStatsVisualizeFrame::OnSaveQCNUSE(wxCommandEvent& event){

 
  GenerateNUSE();
  
  wxFileDialog
    * ResultsFileDialog =
    new wxFileDialog ( this,
		       wxT("Save the results as text file"),
		       wxT(""),
		       wxT(""),
		       wxT("*.NUSE.txt"),
		       wxFD_SAVE,
		       wxDefaultPosition);  
  
  if (ResultsFileDialog->ShowModal() == wxID_OK){
    if (wxFileExists(ResultsFileDialog->GetPath())){
      wxString t=_T("This file already exists. Are you sure that you want to overwrite this file."); 
      wxMessageDialog
	aboutDialog
	(0, t, _T("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	return;
      }
    }
    ExpressionValues->writeSEtofile(ResultsFileDialog->GetFilename(),ResultsFileDialog->GetDirectory());
  } else {
    return;
  }








};
#endif


void QCStatsVisualizeFrame::WriteRLEStats(wxString path, wxString filename){

  int i, j;
  
  wxFileName my_output_fname(path, filename, wxPATH_NATIVE);
  wxFileOutputStream my_output_file_stream(my_output_fname.GetFullPath());
  wxTextOutputStream my_output_file(my_output_file_stream);
  

  for (j=0; j < ExpressionValues->count_arrays(); j++){
    my_output_file << _T("\t") << ExpressionValues->GetArrayNames()[j];
  }
  my_output_file << _T("\n");
  for (i =0; i < 5; i++){
    
    if (i == 0){
      my_output_file << wxT("Minimum");
    } else  if (i == 1){
      my_output_file << wxT("LQ");
    } else  if (i == 2){
      my_output_file << wxT("Median");
    } else  if (i == 3){
      my_output_file << wxT("UQ");
    } else if (i == 4){
      my_output_file << wxT("Maximum");
    } 
    
    
    for (j =0; j < ExpressionValues->count_arrays();j++){
      my_output_file  << _T("\t");
      my_output_file.WriteDouble((*RLEBoxplotStatistics)[j*5 + i]);
    }
    my_output_file << _T("\n");
  }
}



#if RMA_GUI_APP 
void QCStatsVisualizeFrame::OnSaveQCRLEStats(wxCommandEvent& event){

  GenerateRLE();
  
  wxFileDialog
    * ResultsFileDialog =
    new wxFileDialog ( this,
		       wxT("Save the results as text file"),
		       wxT(""),
		       wxT(""),
		       wxT("*.RLE.txt"),
		       wxFD_SAVE,
		       wxDefaultPosition);  
  
  if (ResultsFileDialog->ShowModal() == wxID_OK){
    if (wxFileExists(ResultsFileDialog->GetPath())){
      wxString t=_T("This file already exists. Are you sure that you want to overwrite this file."); 
      wxMessageDialog
	aboutDialog
	(0, t, _T("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	return;
      }
    }
    WriteRLEStats(ResultsFileDialog->GetDirectory(), ResultsFileDialog->GetFilename());
    /*    wxFileName my_output_fname(ResultsFileDialog->GetDirectory(), ResultsFileDialog->GetFilename(), wxPATH_NATIVE);
	  wxFileOutputStream my_output_file_stream(my_output_fname.GetFullPath());
	  wxTextOutputStream my_output_file(my_output_file_stream);
	  
	  
	  for (j=0; j < ExpressionValues->count_arrays(); j++){
	  my_output_file << _T("\t") << ExpressionValues->GetArrayNames()[j];
	  }
	  my_output_file << _T("\n");
	  for (i =0; i < 5; i++){
	  
	  if (i == 0){
	  my_output_file << wxT("Minimum");
	  } else  if (i == 1){
	  my_output_file << wxT("LQ");
	  } else  if (i == 2){
	  my_output_file << wxT("Median");
	  } else  if (i == 3){
	  my_output_file << wxT("UQ");
	  } else if (i == 4){
	  my_output_file << wxT("Maximum");
	  } 
	  
	  
	  for (j =0; j < ExpressionValues->count_arrays();j++){
	  my_output_file  << _T("\t");
	  my_output_file.WriteDouble((*RLEBoxplotStatistics)[j*5 + i]);
	  }
	  my_output_file << _T("\n");
	  }
    */
  } else {
    return;
  }















};
#endif



void QCStatsVisualizeFrame::WriteNUSEStats(wxString path, wxString filename){
  
  int i,j;
  
  wxFileName my_output_fname(path, filename, wxPATH_NATIVE);
  wxFileOutputStream my_output_file_stream(my_output_fname.GetFullPath());
  wxTextOutputStream my_output_file(my_output_file_stream);


  for (j=0; j < ExpressionValues->count_arrays(); j++){
    my_output_file << _T("\t") << ExpressionValues->GetArrayNames()[j];
  }
  my_output_file << _T("\n");
  for (i =0; i < 5; i++){
    
    if (i == 0){
      my_output_file << wxT("Minimum");
    } else  if (i == 1){
      my_output_file << wxT("LQ");
    } else  if (i == 2){
      my_output_file << wxT("Median");
      } else  if (i == 3){
      my_output_file << wxT("UQ");
    } else if (i == 4){
      my_output_file << wxT("Maximum");
    } 
    

    for (j =0; j < ExpressionValues->count_arrays();j++){
      my_output_file  << _T("\t");
      my_output_file.WriteDouble((*NUSEBoxplotStatistics)[j*5 + i]);
    }
    my_output_file << _T("\n");
  }
}





#if RMA_GUI_APP
void QCStatsVisualizeFrame::OnSaveQCNUSEStats(wxCommandEvent& event){


  GenerateNUSE();
  
  wxFileDialog
    * ResultsFileDialog =
    new wxFileDialog ( this,
		       wxT("Save the results as text file"),
		       wxT(""),
		       wxT(""),
		       wxT("*.NUSE.txt"),
		       wxFD_SAVE,
		       wxDefaultPosition);  
  
  if (ResultsFileDialog->ShowModal() == wxID_OK){
    if (wxFileExists(ResultsFileDialog->GetPath())){
      wxString t=_T("This file already exists. Are you sure that you want to overwrite this file."); 
      wxMessageDialog
	aboutDialog
	(0, t, _T("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	return;
      }
    }
    WriteNUSEStats(ResultsFileDialog->GetDirectory(), ResultsFileDialog->GetFilename());
    /*    wxFileName my_output_fname(ResultsFileDialog->GetDirectory(), ResultsFileDialog->GetFilename(), wxPATH_NATIVE);
	  wxFileOutputStream my_output_file_stream(my_output_fname.GetFullPath());
	  wxTextOutputStream my_output_file(my_output_file_stream);


	  for (j=0; j < ExpressionValues->count_arrays(); j++){
	  my_output_file << _T("\t") << ExpressionValues->GetArrayNames()[j];
	  }
	  my_output_file << _T("\n");
	  for (i =0; i < 5; i++){
	  
	  if (i == 0){
	  my_output_file << wxT("Minimum");
	  } else  if (i == 1){
	  my_output_file << wxT("LQ");
	  } else  if (i == 2){
	  my_output_file << wxT("Median");
	  } else  if (i == 3){
	  my_output_file << wxT("UQ");
	  } else if (i == 4){
	  my_output_file << wxT("Maximum");
	  } 
      
	  
	  for (j =0; j < ExpressionValues->count_arrays();j++){
	  my_output_file  << _T("\t");
	  my_output_file.WriteDouble((*NUSEBoxplotStatistics)[j*5 + i]);
	  }
	  my_output_file << _T("\n");
	  }
    */
  } else {
    return;
  }




};
 





void QCStatsVisualizeFrame::OnPostscriptCurrentPlot(wxCommandEvent& event){





  wxPrintDialogData printDialogData(* g_QCprintData);

  wxPrinter printer(& printDialogData);
  QCStatsVisualizePrintout printout(_T("RMAExpress QC printout"));
  if (!printer.Print(this, &printout, true /*prompt*/))
    {
      if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
	wxMessageBox(_T("There was a problem printing.\nPerhaps your current printer is not set correctly?"), _T("Printing"), wxOK);
      else
	wxMessageBox(_T("You canceled printing"), _T("Printing"), wxOK);
    }
  else
    {
      (*g_QCprintData) = printer.GetPrintDialogData().GetPrintData();
    }
  
};

void QCStatsVisualizeFrame::OnSaveCurrent(wxCommandEvent& event){


  wxString defaultname;
  if (currentplot == 1)
    defaultname = wxT("RLE");
  else if (currentplot == 2)
    defaultname = wxT("NUSE");
  else if (currentplot == 3)
    defaultname = wxT("RLEMedian");
  else if (currentplot == 4)
    defaultname = wxT("NUSEMedian"); 
  else if (currentplot == 5)
    defaultname = wxT("RLEIQR");
  else if (currentplot == 6)
    defaultname = wxT("NUSEIQR");
  else if (currentplot == 7)
    defaultname = wxT("RLENUSEMultiPlot");
  else if (currentplot == 8)
    defaultname = wxT("RLENUSET2");
  else {
    wxString msg=wxT("Sorry there is no current image to save."); 
     wxMessageDialog
       errorDialog
       (0, msg, _T("No current plot to save."), wxOK | wxICON_EXCLAMATION);
     
     errorDialog.ShowModal();

     return;
  }


  BitmapSettingDialog *myBitmapSettingDialog = new BitmapSettingDialog(this, -1,
					    wxT("Bitmap Settings"),
					    wxPoint(100,100),
					    wxDefaultSize,
					    wxDEFAULT_DIALOG_STYLE);
  
  myBitmapSettingDialog->ShowModal();
  defaultname+=wxT(".");
  defaultname+=myBitmapSettingDialog->GetImageType();


  wxString savefilename = wxFileSelector( wxT("Save Image"),
					  wxT(""),
					  defaultname,
					  (const wxChar *)NULL,
					  wxT("PNG files (*.png)|*.png|")
					  wxT("JPEG files (*.jpg)|*.jpg|")
					  wxT("TIFF files (*.tif)|*.tif|")
					  ,
					  wxFD_SAVE);

  if (savefilename.empty() ){
    delete myBitmapSettingDialog;
    return;
  }


  
  /*  if (wxFileExists(savefilename)){      
      wxString t=savefilename + _T(" already exists. Overwrite?"); 
      wxMessageDialog
      aboutDialog
      (0, t, _T("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
      return;
      }
      }
  */

  long horizontalsize,verticalsize;

  horizontalsize = myBitmapSettingDialog->GetWidth();
  verticalsize= myBitmapSettingDialog->GetHeight();

  wxBitmap bitmap( horizontalsize,verticalsize );
  wxMemoryDC memdc;
  memdc.SelectObject( bitmap );
  
  PrepareDC( memdc );
  memdc.Clear();

 
  if (currentplot == 1)
    DrawRLEBoxplot(memdc);
  else if (currentplot == 2)
    DrawNUSEBoxplot(memdc);
  else if (currentplot == 3)
    DrawRLEMedians(memdc); 
  else if (currentplot == 4)
    DrawNUSEMedians(memdc);
  else if (currentplot == 5)
    DrawRLEIQRs(memdc);  
  else if (currentplot == 6)
    DrawNUSEIQRs(memdc);  
  else if (currentplot == 7)
    DrawRLENUSEMultiPlot(memdc);
  else if (currentplot == 8)
    DrawMultivariateControlChart(memdc);
  else {
    delete myBitmapSettingDialog;
    return;
  }



  wxImage image = bitmap.ConvertToImage();
  image.SaveFile(savefilename);

  
  delete myBitmapSettingDialog;



}


BEGIN_EVENT_TABLE(QCStatsVisualizeDrawingWindow,wxWindow)
  EVT_PAINT(QCStatsVisualizeDrawingWindow::OnPaint) 
  EVT_ERASE_BACKGROUND(QCStatsVisualizeDrawingWindow::OnEraseBackground) 	
END_EVENT_TABLE()

void QCStatsVisualizeDrawingWindow::OnEraseBackground(wxEraseEvent &WXUNUSED(event)){

}



QCStatsVisualizeDrawingWindow::QCStatsVisualizeDrawingWindow(QCStatsVisualizeFrame *parent):wxWindow(parent,-1,wxDefaultPosition,wxDefaultSize,wxFULL_REPAINT_ON_RESIZE){

  myParent = parent;
 
}



void QCStatsVisualizeDrawingWindow::OnPaint(wxPaintEvent &WXUNUSED(event)){
 
  // wxPrintf(_T("Paint Event\n"));  

  wxPaintDC dc(this);
  this->SetBackgroundColour(*wxWHITE );
  //this->ClearBackground();
  if (myParent->currentplot == 1){
    myParent->DrawRLEBoxplot(dc);
  } else if (myParent->currentplot == 2){
    myParent->DrawNUSEBoxplot(dc);
  } else if (myParent->currentplot == 3){
    myParent->DrawRLEMedians(dc);
  } else if (myParent->currentplot == 4){
    myParent->DrawNUSEMedians(dc);
  } else if (myParent->currentplot == 5){
    myParent->DrawRLEIQRs(dc);
  } else if (myParent->currentplot == 6){
    myParent->DrawNUSEIQRs(dc);
  } else if (myParent->currentplot == 7){
    myParent->DrawRLENUSEMultiPlot(dc);  
  } else if (myParent->currentplot == 8){
    myParent->DrawMultivariateControlChart(dc);
  } else {
    //this->ClearBackground();  
  }


}








bool QCStatsVisualizePrintout::OnPrintPage(int page)
{
    wxDC *dc = GetDC();
    if (dc)
    {
        if (page == 1)
            DrawPageOne(dc);
	
        return true;
    }
    else
        return false;
}


bool QCStatsVisualizePrintout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument(startPage, endPage))
        return false;

    return true;
}

void QCStatsVisualizePrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool QCStatsVisualizePrintout::HasPage(int pageNum)
{
    return (pageNum == 1);
}

void QCStatsVisualizePrintout::DrawPageOne(wxDC *dc)
{
    // You might use THIS code if you were scaling
    // graphics of known size to fit on the page.

    // We know the graphic is 200x200. If we didn't know this,
    // we'd need to calculate it.
  /* float maxX = 1200;
     float maxY = 915;
     
     // Let's have at least 50 device units margin
     float marginX = 50;
     float marginY = 50;
     
     // Add the margin to the graphic size
     maxX += (2*marginX);
     maxY += (2*marginY);
     
     // Get the size of the DC in pixels
     int w, h;
     dc->GetSize(&w, &h);
     
     // Calculate a suitable scaling factor
     float scaleX=(float)(w/maxX);
     float scaleY=(float)(h/maxY);
     
     // Use x or y scaling factor, whichever fits on the DC
     float actualScale = wxMin(scaleX,scaleY);
     
     // Calculate the position on the DC for centring the graphic
     float posX = (float)((w - (200*actualScale))/2.0);
     float posY = (float)((h - (200*actualScale))/2.0);
     
     // Set the scale and origin
     dc->SetUserScale(actualScale, actualScale);
     dc->SetDeviceOrigin( (long)posX, (long)posY ); */
  
  // Get the logical pixels per inch of screen and printer
  int ppiScreenX, ppiScreenY;
  GetPPIScreen(&ppiScreenX, &ppiScreenY);
  int ppiPrinterX, ppiPrinterY;
  GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);
  
  // This scales the DC so that the printout roughly represents the
  // the screen scaling. The text point size _should_ be the right size
  // but in fact is too small for some reason. This is a detail that will
  // need to be addressed at some point but can be fudged for the
  // moment.
  float scale = 1.0; //(float)((float)ppiPrinterX/(float)ppiScreenX);
  
  // Now we have to check in case our real page size is reduced
  // (e.g. because we're drawing to a print preview memory DC)
  int pageWidth, pageHeight;
  int w, h;
  dc->GetSize(&w, &h);
  GetPageSizePixels(&pageWidth, &pageHeight);
  
  // If printer pageWidth == current DC width, then this doesn't
  // change. But w might be the preview bitmap width, so scale down.
  float overallScale = scale * (float)(w/(float)pageWidth);
  dc->SetUserScale(overallScale, overallScale);
  
  wxPrintf(_T("%d %d\n"),w,h);
  wxPrintf(_T("%d %d\n"),pageWidth, pageHeight);
  wxPrintf(_T("%d %d\n"),ppiScreenX, ppiScreenY);
  wxPrintf(_T("%d %d\n"),ppiPrinterX, ppiPrinterY);
  if (QCframe->currentplot == 1)
    QCframe->DrawRLEBoxplot(*dc);
  else if (QCframe->currentplot == 2)
    QCframe->DrawNUSEBoxplot(*dc);
  else if (QCframe->currentplot == 3)
    QCframe->DrawRLEMedians(*dc); 
  else if (QCframe->currentplot == 4)
    QCframe->DrawNUSEMedians(*dc);
  else if (QCframe->currentplot == 5)
    QCframe->DrawRLEIQRs(*dc); 
  else if (QCframe->currentplot == 6)
    QCframe->DrawNUSEIQRs(*dc);
  else if (QCframe->currentplot == 7)
    QCframe->DrawRLENUSEMultiPlot(*dc);
  else if (QCframe->currentplot == 8)
    QCframe->DrawMultivariateControlChart(*dc);
  


}

#endif
