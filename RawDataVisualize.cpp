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
 ** File: RawDataVisualize.cpp
 **
 ** Copyright (C) B. M. Bolstad 2007
 **
 ** Visualization of Raw data via boxplots, density plots.
 **
 ** History - Created Jan 5, 2007
 ** Jan 6, 2007 - add boxplot option
 ** Jan 11, 2007 - add stored boxplot statistics
 ** Jan 13-15, 2007 - basic printing infrastructure
 ** Jan 21, 2007 - add Kernel Density style plots
 ** Feb 12, 2007 - add save to bitmap functionality
 ** Apr 29, 2007 - fix axes placement for high-res print devices.
 ** May 12, 2007 - strip .CEL from plot labels
 **
 **************************************************************************/

#include <wx/wx.h>
#include <wx/dcps.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/image.h>

#include "RawDataVisualize.h"

#include "Graphing/boxplot.h"
#include "Graphing/axes.h"

#include "Preprocess/weightedkerneldensity.h"

#ifndef _WIN32
#include "RMAExpress.xpm"
#endif


#include "BitmapSettingDialog.h"

#include "GlobalPrintSettings.h"

#define ID_POSTSCRIPT_CURRENTPLOT 10000
#define ID_PRINT_PREVIEW_CURRENTPLOT 10010

#define ID_VISUALIZE_BOXPLOT 10100
#define ID_VISUALIZE_DENSITY 10200
#define ID_VISUALIZE_DENSITY_INDIVIDUAL 10300

#define ID_SAVE_CURRENT 10400

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))

BEGIN_EVENT_TABLE(RawDataVisualizeFrame,wxFrame)
  EVT_MENU(wxID_EXIT,RawDataVisualizeFrame::OnQuit) 
  EVT_MENU(ID_POSTSCRIPT_CURRENTPLOT, RawDataVisualizeFrame::OnPostscriptCurrentPlot)
  EVT_MENU(ID_PRINT_PREVIEW_CURRENTPLOT,RawDataVisualizeFrame::OnPrintPreview)
  EVT_MENU(ID_VISUALIZE_BOXPLOT,RawDataVisualizeFrame::OnVisualizeRawBoxplot) 
  EVT_MENU(ID_VISUALIZE_DENSITY,RawDataVisualizeFrame::OnVisualizeRawDensityPlot)
  EVT_MENU(ID_VISUALIZE_DENSITY_INDIVIDUAL,RawDataVisualizeFrame::OnVisualizeIndividualRawDensityPlot)
  EVT_MENU(ID_SAVE_CURRENT,RawDataVisualizeFrame::OnSaveCurrent)
//  EVT_KEY_DOWN(RawDataVisualizeFrame::OnKeyDown)
//  EVT_KEY_UP(RawDataVisualizeFrame::OnKeyUp)
//  EVT_CHAR(RawDataVisualizeFrame::OnKeyPress)
END_EVENT_TABLE()


wxPrintDialogData g_printDialogData;


// Global print data, to remember settings during the session
wxPrintData *g_printData = (wxPrintData*) NULL ;

// Global page setup data
wxPageSetupData* g_pageSetupData = (wxPageSetupData*) NULL;


// Declare a frame
RawDataVisualizeFrame   *frame = (RawDataVisualizeFrame*) NULL;


///c(x1, x2, y1, y2);
const double pltarea_min_x=0.1225725;
const double pltarea_max_x=0.9372190;
const double pltarea_min_y=0.1524917 ;
const double pltarea_max_y=0.8774086;



void RawDataVisualizeFrame::OnQuit(wxCommandEvent& event){

  Close();
}




void SetUpDC(wxDC &dc){

  int w,h;
  dc.SetAxisOrientation(true, true);
  dc.GetSize(&w, &h);
  dc.SetDeviceOrigin(0,h);

}











void RawDataVisualizeFrame::DrawRawBoxplot(wxDC &dc){

  int i,j;

  int w,h;
  dc.GetSize(&w, &h);

  int min_x = min((int)(pltarea_min_x*(double)w),(int)(160*(double)(dc.GetPPI().x)/(double)72));
  int min_y = min((int)(pltarea_min_y*(double)h),(int)(165*(double)(dc.GetPPI().x)/(double)72));
  int max_x = max((int)((pltarea_max_x-pltarea_min_x)*(double)w),(int)(w - 100*(double)(dc.GetPPI().x)/(double)72 - min_x));
  int max_y = max((int)((pltarea_max_y-pltarea_min_y)*(double)h),(int)(h-160*(double)(dc.GetPPI().x)/(double)72 - min_y));
   



  labeledplotAxes myAxes(dc,wxPoint(min_x,min_y),
			 wxPoint(max_x,max_y),
			 wxRealPoint(0.5,3.5),
			 wxRealPoint((double)MyRawData->count_arrays()+ 0.5 ,15.5),
			 true,
			 wxPoint(0,0),true,10);
  //dc.SetClippingRegion(80,30*5-80 ,w-30*5,h-30*5);  
  //dc.SetBackground(*wxGREY_BRUSH);
  //dc.Clear();

  //SetUpDC(dc); 




  
  //dc.Clear();
  myAxes.setupDCforPlot(dc);


  wxArrayDouble xtickpoints;
  wxArrayDouble ytickpoints;
  wxArrayString x_tick_labels;
  wxArrayString y_tick_labels;

  wxString temp;

  

  for (i =1; i <= MyRawData->count_arrays();i++){
    xtickpoints.Add((double)i);
    wxString mynewstring = wxString::Format(_T("%d"), i);
    x_tick_labels.Add(mynewstring); 
  }

  x_tick_labels = MyRawData->GetArrayNames();

  // Strip the ".CEL" from the labels 
  for (i =0; i < MyRawData->count_arrays();i++){
    temp = wxString(x_tick_labels[i]);
    x_tick_labels[i] = temp.Remove(temp.Len() - 4);

  }

  for (i=2; i <= 15; i++){
    ytickpoints.Add((double)i); 
    y_tick_labels.Add(wxString::Format(_T("%d"), i));
  }
  
  myAxes.setTickLocationX(xtickpoints);
  myAxes.setTickLocationY(ytickpoints);

  myAxes.setTickLabelsX(x_tick_labels);
  myAxes.setTickLabelsY(y_tick_labels);

  myAxes.setTickLabelsPerpendicular(true);


  wxString x_label = wxT("");
  wxString y_label = wxT("log2 PM");

  myAxes.setAxisLabelX(x_label);
  myAxes.setAxisLabelY(y_label);


  
  myAxes.setTitle(wxT("log2 PM by array for raw data"));
 




  wxColor tempColor = wxColor(100,100,255);

  for (j =0; j < MyRawData->count_arrays();j++){


    Draw_Single_Boxplot(dc,
			myAxes.FindPoint(wxRealPoint(1.0,max(log((*BoxplotStatistics)[j*5 + 0])/log(2.0),3.5))).y,
			myAxes.FindPoint(wxRealPoint(1.0,log((*BoxplotStatistics)[j*5 + 1])/log(2.0))).y,
			myAxes.FindPoint(wxRealPoint(1.0,log((*BoxplotStatistics)[j*5 + 2])/log(2.0))).y,
			myAxes.FindPoint(wxRealPoint(1.0,log((*BoxplotStatistics)[j*5 + 3])/log(2.0))).y,
			myAxes.FindPoint(wxRealPoint(1.0,min(log((*BoxplotStatistics)[j*5 + 4])/log(2.0),15.5))).y,
			myAxes.FindPoint(wxRealPoint((double)(j+1),0)).x,
			myAxes.FindPoint(wxRealPoint(3.4,0)).x - myAxes.FindPoint(wxRealPoint(2.6,0)).x,&tempColor);
  }


  


  dc.DestroyClippingRegion();
  myAxes.Draw(dc);
	
  

}


void RawDataVisualizeFrame::GenerateBoxplotStatistics(){

  double *five_num_summary = new double[5];
  int i,j;

  if (BoxplotStatistics == NULL){

    BoxplotStatistics = new Matrix(5,MyRawData->count_arrays());
    for (j =0; j < MyRawData->count_arrays();j++){
      MyRawData->Compute5Summary(j,five_num_summary);
      for (i=0; i < 5; i++){
	(*BoxplotStatistics)[j*5 + i] = five_num_summary[i];
      }


    }

  }

  delete [] five_num_summary;

}



void RawDataVisualizeFrame::GenerateDensityPlotStatistics(){

  int i;

  double *buffer = new double[MyRawData->count_pm()];
  double *output_x = new double[512];
  double *output =  new double[512];
  int nout = 512;
  int nxxx = MyRawData->count_pm();
  


  if (DensityPlotStatistics_y == NULL){
    DensityPlotStatistics_y = new Matrix(512,MyRawData->count_arrays());
    DensityPlotStatistics_x = new Matrix(512,MyRawData->count_arrays());
    /*    for (i=0; i < 512; i++){
	  output_x[i] = 2.0 + ((double)i/511.0)*(15.5-2.0);
	  }
    */
    for (int j=0; j < MyRawData->count_arrays(); j++){
      
      for (i=0; i < nxxx; i++){
	MyRawData->GetValue(&buffer[i],i,j);
	buffer[i] = log(buffer[i])/log(2.0);
      } 
      KernelDensity_lowmem(buffer, &nxxx,output, output_x,&nout);
      for (i=0; i < 512; i++){
	(*DensityPlotStatistics_y)[j*512 + i] = output[i];
	(*DensityPlotStatistics_x)[j*512 + i] = output_x[i];
      }
    }
  }
  
  

  delete [] output_x;
  delete [] output;
  delete [] buffer;
  
}






void RawDataVisualizeFrame::DrawRawDensityPlot(wxDC& dc){



  //PrepareDC(dc);
  
  //SetUpDC(dc);
 
  int w,h;
  dc.GetSize(&w, &h);

    
  /* labeledplotAxes myAxes(wxPoint(80,80),
     wxPoint(w-30*5,h-30*5),
     wxRealPoint(0.0,1000.0),
     wxRealPoint(50.0,4000.0),
     true);
  */
  
  int min_x = min((int)(pltarea_min_x*(double)w),(int)(160*(double)(dc.GetPPI().x)/(double)72));
  int min_y = min((int)(pltarea_min_y*(double)h),(int)(165*(double)(dc.GetPPI().x)/(double)72));
  int max_x = max((int)((pltarea_max_x-pltarea_min_x)*(double)w),(int)(w - 100*(double)(dc.GetPPI().x)/(double)72 - min_x));
  int max_y = max((int)((pltarea_max_y-pltarea_min_y)*(double)h),(int)(h-160*(double)(dc.GetPPI().x)/(double)72 - min_y));



  labeledplotAxes myAxes(dc, wxPoint(min_x,min_y),
			 wxPoint(max_x,max_y),
			 wxRealPoint(3.0,0.0),
			 wxRealPoint(15.5,1.0),
			 true);
  
  dc.Clear(); 
  myAxes.setBackgroundColor(*wxWHITE_BRUSH);
  myAxes.setupDCforPlot(dc);
  

  // myAxes.addGrid(2, 0.2);

  wxArrayDouble xtickpoints;
  wxArrayDouble ytickpoints;

  int i,j;

  wxArrayString x_tick_labels;
  wxArrayString y_tick_labels;

  for (i=3; i <= 15; i++){
    xtickpoints.Add((double)i); 
    x_tick_labels.Add(wxString::Format(_T("%d"), i));
  }
  
  for (i =0; i <= 10; i++){
    ytickpoints.Add((double)i/10.0); 
    y_tick_labels.Add(wxString::Format(_T("%.2f"), i/10.0));
  }

  myAxes.setTickLocationX(xtickpoints);
  myAxes.setTickLocationY(ytickpoints);

  myAxes.setTickLabelsX(x_tick_labels);
  myAxes.setTickLabelsY(y_tick_labels);


  wxString x_label = wxT("log2 PM");
  wxString y_label = wxT("Density");

  myAxes.setAxisLabelX(x_label);
  myAxes.setAxisLabelY(y_label);


  myAxes.setTitle(wxT("Density plots of log2 PM by array"));



  wxPoint xsq[512];
  
  for (j =0; j < MyRawData->count_arrays();j++){
    for (i =0; i < 512; i++){
      xsq[i] = myAxes.FindPoint(wxRealPoint((*DensityPlotStatistics_x)[j*512 + i], (*DensityPlotStatistics_y)[j*512 + i]));
    }
    dc.DrawSpline(WXSIZEOF(xsq),xsq);
  }
  
  dc.DestroyClippingRegion();
  myAxes.Draw(dc);

  

}

void RawDataVisualizeFrame::DrawIndividualRawDensityPlot(wxDC& dc){

  //PrepareDC(dc);
  
  //SetUpDC(dc);
 
  int w,h;
  dc.GetSize(&w, &h);

    
  /* labeledplotAxes myAxes(wxPoint(80,80),
     wxPoint(w-30*5,h-30*5),
     wxRealPoint(0.0,1000.0),
     wxRealPoint(50.0,4000.0),
     true);
  */
  
  int min_x = min((int)(pltarea_min_x*(double)w),(int)(160*(double)(dc.GetPPI().x)/(double)72));
  int min_y = min((int)(pltarea_min_y*(double)h),(int)(165*(double)(dc.GetPPI().x)/(double)72));
  int max_x = max((int)((pltarea_max_x-pltarea_min_x)*(double)w),(int)(w - 100*(double)(dc.GetPPI().x)/(double)72 - min_x));
  int max_y = max((int)((pltarea_max_y-pltarea_min_y)*(double)h),(int)(h-160*(double)(dc.GetPPI().x)/(double)72 - min_y));


  labeledplotAxes myAxes(dc, wxPoint(min_x,min_y),
			 wxPoint(max_x,max_y),
			 wxRealPoint(3.0,0.0),
			 wxRealPoint(15.5,1.0),
			 true);
  
  dc.Clear(); 
  myAxes.setBackgroundColor(*wxWHITE_BRUSH);
  myAxes.setupDCforPlot(dc);
  

  // myAxes.addGrid(2, 0.2);

  wxArrayDouble xtickpoints;
  wxArrayDouble ytickpoints;

  int i,j;

  wxArrayString x_tick_labels;
  wxArrayString y_tick_labels;

  for (i=3; i <= 15; i++){
    xtickpoints.Add((double)i); 
    x_tick_labels.Add(wxString::Format(_T("%d"), i));
  }
  
  for (i =0; i <= 10; i++){
    ytickpoints.Add((double)i/10.0); 
    y_tick_labels.Add(wxString::Format(_T("%.2f"), i/10.0));
  }

  myAxes.setTickLocationX(xtickpoints);
  myAxes.setTickLocationY(ytickpoints);

  myAxes.setTickLabelsX(x_tick_labels);
  myAxes.setTickLabelsY(y_tick_labels);


  wxString x_label = wxT("log2 PM");
  wxString y_label = wxT("Density");

  myAxes.setAxisLabelX(x_label);
  myAxes.setAxisLabelY(y_label);
  
  myAxes.setTitle(MyRawData->GetArrayNames()[whichdensity]);

  wxPoint xsq[512];
  
  j = whichdensity;
  for (i =0; i < 512; i++){
    xsq[i] = myAxes.FindPoint(wxRealPoint((*DensityPlotStatistics_x)[j*512 + i], (*DensityPlotStatistics_y)[j*512 + i]));
  }
  dc.DrawSpline(WXSIZEOF(xsq),xsq);
  
  

  dc.DestroyClippingRegion();
  myAxes.Draw(dc);

  


}


void RawDataVisualizeFrame::OnKeyPress(wxKeyEvent& event){

  wxString newStr;

  long key = event.GetKeyCode();
  long operation;

  //event.Skip();
  switch(key)
    {
    case WXK_UP:
      newStr += wxT("Up");
      operation = 1;
      break;
    case WXK_DOWN:
      newStr += wxT("Down");
      operation = -1;
      break;
    case WXK_ADD:
      newStr += wxT("+"); 
      operation = 1;
      break;
    case WXK_SUBTRACT:
      newStr += wxT("-");
      operation = -1;
      break;
    case WXK_PAGEUP:
      newStr += wxT("PageUp"); 
      operation = 1;
      break;
    case WXK_PAGEDOWN:
      newStr += wxT("PageDown"); 
      operation = -1;
      break;
    case WXK_NUMPAD_ADD:
      newStr += wxT("NUMPAD_ADD"); 
      operation = 1;
      break;
    case WXK_NUMPAD_SUBTRACT:
      newStr += wxT("NUMPAD_SUBTRACT"); 
      operation = -1;
      break;
    default:
      operation = 1;
      //return wxT(""); // Don't do anything if we don't recognize the key
      break;
    }
  
 // wxPrintf(newStr+wxT("\n"));

  if (operation > 0){
    if (whichdensity < MyRawData->count_arrays()-1){
      whichdensity+=1;
    } else {
      whichdensity = 0;
    }

  } else if (operation < 0){
    if (whichdensity == 0){
      whichdensity = MyRawData->count_arrays()-1;
    } else {
      whichdensity-=1;
    }
  }
  this->Refresh();
};



void RawDataVisualizeFrame::OnKeyDown(wxKeyEvent& event){

  event.Skip();

}


void RawDataVisualizeFrame::OnKeyUp(wxKeyEvent& event){

  event.Skip();

}







void RawDataVisualizeFrame::OnVisualizeRawDensityPlot(wxCommandEvent& event){
	
	GenerateDensityPlotStatistics();
	currentplot = 2;
	this->Refresh();

}

void RawDataVisualizeFrame::OnVisualizeRawBoxplot(wxCommandEvent& event){

	GenerateBoxplotStatistics();
	currentplot = 1;
	this->Refresh();

}

void RawDataVisualizeFrame::OnVisualizeIndividualRawDensityPlot(wxCommandEvent& event){

	GenerateDensityPlotStatistics();
	currentplot = 3;
	this->Refresh();
	this->SetFocus();

}

PMProbeBatch  *RawDataVisualizeFrame::GetRawData(){

  return MyRawData;

}

BEGIN_EVENT_TABLE(RawDataVisualizeDrawingWindow,wxWindow)
  EVT_PAINT(RawDataVisualizeDrawingWindow::OnPaint) 
  EVT_ERASE_BACKGROUND(RawDataVisualizeDrawingWindow::OnEraseBackground) 
  EVT_MOUSE_EVENTS(RawDataVisualizeDrawingWindow::OnClick)
  EVT_KEY_DOWN(RawDataVisualizeDrawingWindow::OnKeyDown)
  EVT_KEY_UP(RawDataVisualizeDrawingWindow::OnKeyUp)
  EVT_CHAR(RawDataVisualizeDrawingWindow::OnKeyPress)
END_EVENT_TABLE()

void RawDataVisualizeDrawingWindow::OnEraseBackground(wxEraseEvent &WXUNUSED(event)){

}



void RawDataVisualizeDrawingWindow::OnClick(wxMouseEvent& event){

  myParent->SetFocus();

}



void RawDataVisualizeDrawingWindow::OnKeyPress(wxKeyEvent& event){
		
	myParent->OnKeyPress(event);

};


void RawDataVisualizeDrawingWindow::OnKeyDown(wxKeyEvent& event){
	
	event.Skip();
	
}


void RawDataVisualizeDrawingWindow::OnKeyUp(wxKeyEvent& event){
	
	event.Skip();
	
}









void RawDataVisualizeDrawingWindow::OnPaint(wxPaintEvent &WXUNUSED(event)){

  wxPaintDC dc(this);
  this->SetBackgroundColour(*wxWHITE );
  
  PrepareDC(dc);
  myParent->PrepareDC(dc);
	
  if (myParent->currentplot == 1){
    dc.Clear();
    myParent->DrawRawBoxplot(dc);
  } else if (myParent->currentplot == 2){
    myParent->DrawRawDensityPlot(dc);
  } else if (myParent->currentplot ==3){
    myParent->DrawIndividualRawDensityPlot(dc);
  } else { 
	dc.Clear();
  }
  
}


void RawDataVisualizeFrame::OnPostscriptCurrentPlot(wxCommandEvent& event){
 
  wxPrintDialogData printDialogData(* g_printData);

  wxPrinter printer(& printDialogData);
  RawDataVisualizePrintout printout(_T("RMAExpress Raw data printout"));
  if (!printer.Print(this, &printout, true /*prompt*/))
    {
      if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
	wxMessageBox(_T("There was a problem printing.\nPerhaps your current printer is not set correctly?"), _T("Printing"), wxOK);
      else
	wxMessageBox(_T("You canceled printing"), _T("Printing"), wxOK);
    }
  else
    {
      (*g_printData) = printer.GetPrintDialogData().GetPrintData();
    }
  

}

void RawDataVisualizeFrame::OnPrintPreview(wxCommandEvent& WXUNUSED(event))
{
    // Pass two printout objects: for preview, and possible printing.
    wxPrintDialogData printDialogData(* g_printData);
    wxPrintPreview *preview = new wxPrintPreview(new RawDataVisualizePrintout, new RawDataVisualizePrintout, & printDialogData);
    if (!preview->Ok())
    {
        delete preview;
        wxMessageBox(_T("There was a problem previewing.\nPerhaps your current printer is not set correctly?"), _T("Previewing"), wxOK);
        return;
    }

    wxPreviewFrame *frame = new wxPreviewFrame(preview, this, _T("Demo Print Preview"), wxPoint(100, 100), wxSize(600, 650));
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show();
}



void RawDataVisualizeFrame::OnSaveCurrent(wxCommandEvent& event){


  wxString defaultname;
  if (currentplot == 1){
    defaultname = wxT("RawDataBoxplot");
  } else if (currentplot == 2){
    defaultname = wxT("RawDataDensityPlot");
  } else if (currentplot == 3){
    defaultname = wxT("RawDataDensityPlotIndividual");
  } else {
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

  if (currentplot == 1){
    DrawRawBoxplot(memdc);
  } else if (currentplot == 2){
    DrawRawDensityPlot(memdc);
  } else {
    delete myBitmapSettingDialog;
    return;
  }

  wxImage image = bitmap.ConvertToImage();
  image.SaveFile(savefilename);

  
  delete myBitmapSettingDialog;



}



RawDataVisualizeDrawingWindow::RawDataVisualizeDrawingWindow(RawDataVisualizeFrame *parent):wxWindow(parent,-1,wxDefaultPosition,wxDefaultSize,wxFULL_REPAINT_ON_RESIZE){

  myParent = parent;
 
}



RawDataVisualizeFrame::RawDataVisualizeFrame(const wxString& title,PMProbeBatch *RawData, wxWindow *parent):wxFrame(parent,wxID_ANY,title,wxDefaultPosition, wxDefaultSize,wxDEFAULT_FRAME_STYLE | wxWANTS_CHARS)
{

  wxMenu *fileMenu = new wxMenu;
  wxMenu *drawMenu = new wxMenu;
  wxMenuBar *menuBar = new wxMenuBar();
  
  wxBoxSizer *item0 = new wxBoxSizer( wxVERTICAL );


  //  fileMenu->Append(ID_PRINT_PREVIEW_CURRENTPLOT,wxT("Print Preview"),wxT("Print Preview Current Plot"));
  fileMenu->Append(ID_SAVE_CURRENT,wxT("Save"),wxT("Save Current Plot"));
  fileMenu->Append(ID_POSTSCRIPT_CURRENTPLOT,wxT("Print"),wxT("Print Current plot")); 
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT,wxT("E&xit\tAlt-X"),wxT("Quit the program"));




  drawMenu->Append(ID_VISUALIZE_BOXPLOT,wxT("&Boxplots"),wxT("Boxplots of Raw PM data"));
  drawMenu->Append(ID_VISUALIZE_DENSITY,wxT("&Density Plots"),wxT("Density plots of Raw PM data"));
  drawMenu->Append(ID_VISUALIZE_DENSITY_INDIVIDUAL,wxT("&Individual Density Plots"),wxT("Density plots of Raw PM data for each array separately"));
  menuBar->Append(fileMenu,wxT("&File"));
  menuBar->Append(drawMenu,wxT("&Draw"));

  SetMenuBar(menuBar);

  CreateStatusBar(2);
 
  MyWindow = new RawDataVisualizeDrawingWindow(this); //wxWindow(this,-1);

  
  item0->Add( MyWindow, 1, wxEXPAND | wxGROW | wxALL , 5 );



  this->SetAutoLayout( TRUE );
  this->SetSizer( item0 );

  MyWindow->SetBackgroundColour(*wxWHITE );
  MyWindow->Refresh();

  MyRawData = RawData;

  BoxplotStatistics = NULL;
  DensityPlotStatistics_y = NULL;
  currentplot = 0;

  whichdensity=0;

  
  g_printData = new wxPrintData;
  g_pageSetupData = new wxPageSetupDialogData;


  frame = this;
#if _WIN32
  SetIcon(wxICON(expressicon));
#else
  SetIcon(wxICON(RMAExpress));
#endif
}

RawDataVisualizeFrame::~RawDataVisualizeFrame(){


  delete MyRawData;



  if (BoxplotStatistics != NULL){
    delete BoxplotStatistics;
  }

  if (DensityPlotStatistics_y != NULL){
    delete DensityPlotStatistics_y;
    delete DensityPlotStatistics_x;
  }




}







bool RawDataVisualizePrintout::OnPrintPage(int page)
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

bool RawDataVisualizePrintout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument(startPage, endPage))
        return false;

    return true;
}

void RawDataVisualizePrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool RawDataVisualizePrintout::HasPage(int pageNum)
{
    return (pageNum == 1);
}

void RawDataVisualizePrintout::DrawPageOne(wxDC *dc)
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
  float overallScale = scale * ((float)w/(float)pageWidth);
  dc->SetUserScale(overallScale, overallScale);

  wxPrintf(_T("%d %d\n"),w,h);
  wxPrintf(_T("%d %d\n"),pageWidth, pageHeight);
  wxPrintf(_T("%d %d\n"),ppiScreenX, ppiScreenY);
  wxPrintf(_T("%d %d\n"),ppiPrinterX, ppiPrinterY);
  if (frame->currentplot == 1)
    frame->DrawRawBoxplot(*dc);
  else if (frame->currentplot == 2)
    frame->DrawRawDensityPlot(*dc);
}

void RawDataVisualizePrintout::DrawPageTwo(wxDC *dc)
{
  
}

// Writes a header on a page. Margin units are in millimetres.
bool WritePageHeader(wxPrintout *printout, wxDC *dc, wxChar *text, float mmToLogical)
{
/*
static wxFont *headerFont = (wxFont *) NULL;
if (!headerFont)
{
headerFont = wxTheFontList->FindOrCreateFont(16, wxSWISS, wxNORMAL, wxBOLD);
}
dc->SetFont(headerFont);
    */

    int pageWidthMM, pageHeightMM;

    printout->GetPageSizeMM(&pageWidthMM, &pageHeightMM);
    wxUnusedVar(pageHeightMM);

    int leftMargin = 10;
    int topMargin = 10;
    int rightMargin = 10;

    float leftMarginLogical = (float)(mmToLogical*leftMargin);
    float topMarginLogical = (float)(mmToLogical*topMargin);
    float rightMarginLogical = (float)(mmToLogical*(pageWidthMM - rightMargin));

    long xExtent, yExtent;
    dc->GetTextExtent(text, &xExtent, &yExtent);
    float xPos = (float)(((((pageWidthMM - leftMargin - rightMargin)/2.0)+leftMargin)*mmToLogical) - (xExtent/2.0));
    dc->DrawText(text, (long)xPos, (long)topMarginLogical);

    dc->SetPen(* wxBLACK_PEN);
    dc->DrawLine( (long)leftMarginLogical, (long)(topMarginLogical+yExtent),
        (long)rightMarginLogical, (long)topMarginLogical+yExtent );

    return true;
}
