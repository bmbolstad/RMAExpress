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

/*****************************************************
 **
 ** file: RMAExpress.cpp
 **
 ** Copyright (C) 2003-2010    B. M. Bolstad
 **
 ** aim: A crossplatform GUI program for generating 
 **      RMA expression estimates.
 **
 ** Created by: B. M. Bolstad  <bmb@bmbolstad.com>
 ** 
 ** Created on: Apr 1, 2003
 **
 ** Description:
 **
 ** RMAExpress is a GUI program for generating RMA
 ** expression estimates. It is implemented in C++
 ** and will use the cross-platform toolkit wxWidgets (Formerly wxWindows)
 **
 ** this file implements the interface part of the program.
 **
 **
 ** History
 ** Apr 01, 2003 - Initial version, which amounts to
 **                opening an empty window.
 ** Apr 16, 2003 - Start adding menu's and an about dialog
 **                Set up empty shell of loading/saving dialogs
 ** Apr 20, 2003 - Introduce a version string
 **                Setup a dialog box for picking analysis options
 ** Apr 21, 2003 - add in call to do normalization, summarization
 ** Apr 23, 2003 - break the reading in, expression calculation 
 **                and outputing of results into three steps.
 ** Apr 24, 2003 - fix up situation with cancel in reading data
 **                files. Allow the user to cancel with message.
 ** Apr 30, 2003 - bump version number.
 ** May 20, 2003 - bump version number to 0.1 beta 3
 ** Jun 03, 2003 - bump version number to 0.1 beta 4
 **                We are looking for memory leaks and other potential
 **                problems with memory usage.
 ** Jun 11, 2003 - Make release official 0.1
 ** Jun 29, 2003 - Change version number to 0.2 alpha 1
 **                Added a number of new menu options
 **                Mostly not yet activated.
 **                A writeLogfile option was implemented
 ** Jul 20, 2003 - Change order of menu items. Keep, reading writing
 **                of processed data in one section
 ** Aug 10, 2003 - Change version number to 0.2 alpha 2
 ** Aug 11, 2003 - ability to add cel files to dataset
 ** Oct 30, 2003 - add in "Show" menu
 ** Oct 31, 2003 - add low memory type option for normalize
 ** Jan 19, 2004 - xmlres for interface, show residual images started (xmlres later removed)
 ** Feb 29, 2004 - added a "\n" to one of the output statements. Updated "copyright" to note 2004
 ** Oct 13, 2004 - make output in natural scale an option
 ** Mar 20, 2005 - Added a "dummy" (ie not yet connected to anything, so changes don't affect anything) preferences dialog box.
 ** Oct 29, 2005 - Store Application Preferences
 ** Nov 3,  2005 - add temp directory, buffer size settings
 **                to stored preferences    
 ** Apr 2, 2006  - the Export Expression values option added         
 ** Sept 16, 2006 - fix compile problems with unicode builds of wxWidgets
 ** Jan 5, 2007 - add a visualize raw data item to the show menu
 ** Jan 27, 2007 - Add an option to ogle between median polish and PLM for the summarization
 ** Jan 28, 2007 - add QCStats option
 ** Mar 1, 2007 - fix logic type errors created by adding celfiles and not disabling/enabling the appropriate menu items at the same
 **               time
 ** May 12, 2007 - Checks for existance of temporary directory on startup
 ** Jan 8, 2008 - Rename Processed File Menu options more clearly 
 ** Feb 5, 2008 - Allow a minimum of 1 array in Buffer (Previous minimum was 5) 
 ** June 26, 2008 - modify about dialog box
 ** 
 *****************************************************/

#include <wx/wx.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>
#include <wx/textctrl.h>
#include <wx/progdlg.h>
#include <wx/file.h>
//#include <wx/xrc/xmlres.h>
#include <wx/image.h>
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>

#include <wx/config.h>

#include <wx/filename.h>
#include <wx/file.h>
#include <wx/utils.h>
#include <wx/aboutdlg.h>

#include "RMAExpress.h"
#include "DataGroup.h"
#include "PMProbeBatch.h"
#include "version_number.h"
#include "residualimages.h"
#include "ResidualsDataGroup.h"
#include "PreferencesDialog.h"
#include "RawDataVisualize.h"
#include "QCStatsVisualize.h"


#ifndef _WIN32
#include "RMAExpress_64.xpm"
#endif

static const wxString norm = _T("Quantile");
static const wxString normMemory = _T("Quantile (Low Memory Overhead)");
static const wxString background = _T("Yes");
static const wxString summary_median_polish = wxT("Median Polish");
static const wxString summary_PLM =wxT("PLM");

static const wxString author = _T("B. M. Bolstad <bmb@bmbolstad.com>");


static const wxString RMAExpressTempFile = _T("RMAXXXXXX");




wxConfig *g_mysettings;



PreprocessDialog::PreprocessDialog(wxWindow* parent)
{
  // wxXmlResource::Get()->LoadDialog(this, parent, wxT("RMAOptions"));
}


/**********************************************************
 **
 ** PreprocessDialog::PreprocessDialog
 **                   ( wxWindow *parent,
 **                     wxWindowID id,
 **                     const wxString &title,
 **                     const wxPoint &position,
 **                     const wxSize& size,
 **                     long style)
 **
 **
 ***********************************************************/

PreprocessDialog::PreprocessDialog
( wxWindow *parent,
   wxWindowID id,
  const wxString &title,
  const wxPoint &position,
  const wxSize& size,
  long style
  ) :
  wxDialog( parent, id, title, position, size, style)
{
  
  wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
  wxString normtitle = _T("Normalization");
  wxString normchoices[] =
    {
        norm,
        wxT("None"),
    };

  NormMethod = new wxRadioBox(this, 
			    -1, 
			    normtitle, 
			    wxDefaultPosition,
			    wxDefaultSize,
			    2,
			    normchoices,
			    1,
			    wxHORIZONTAL,
			    wxDefaultValidator,
			    wxRadioBoxNameStr);
  
  wxString bgtitle = _T("Background Adjust");
  wxString bgchoices[] =
    {
      background,
      wxT("No"),
    };

  BgMethod = new wxRadioBox(this, 
			    -1, 
			    bgtitle, 
			    wxDefaultPosition,
			    wxDefaultSize,
			    2,
			    bgchoices,
			    1,
			    wxHORIZONTAL,
			    wxDefaultValidator,
			    wxRadioBoxNameStr);

  wxString summarytitle = _T("Summarization Method");
  wxString summarychoices[] =
    {
      summary_median_polish,
      summary_PLM,
    };

  SummaryMethod = new wxRadioBox(this, 
			    -1, 
			    summarytitle, 
			    wxDefaultPosition,
			    wxDefaultSize,
			    2,
			    summarychoices,
			    1,
			    wxHORIZONTAL,
			    wxDefaultValidator,
			    wxRadioBoxNameStr);




  topsizer->Add(BgMethod,
                 1,
                 wxEXPAND | wxALL,
                 10
	      );
  
  topsizer->Add(NormMethod,
                 1,
                 wxEXPAND | wxALL,
                 10
		);

  topsizer->Add(SummaryMethod,
		1,
		wxEXPAND | wxALL,
                 10
		);




  StoreResiduals = new wxCheckBox(this, -1, wxT("Store Residuals"));
  
  topsizer->Add(StoreResiduals, 1, wxEXPAND | wxALL, 10 );
    

  wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
  
  button_sizer->Add
    ( new wxButton( this, wxID_OK, _T("OK") ),
     0,
     wxALL,
     10 );

  button_sizer->Add
    (new wxButton( this, wxID_CANCEL, _T("Cancel")),
     0,
     wxALL,
     10 );

  topsizer->Add
    (button_sizer,
     0,
     wxALIGN_CENTER );
  
  SetAutoLayout( TRUE );
  SetSizer( topsizer );
  
  topsizer->Fit( this );
  topsizer->SetSizeHints( this ); 
}


/******************************************************************
 **
 ** void PreprocessDialog::OnOk(wxCommandEvent &event)
 **
 **
 **
 ******************************************************************/

void PreprocessDialog::OnOk(wxCommandEvent &event ){
  
  event.Skip();

};

void PreprocessDialog::OnCancel(wxCommandEvent &event ){
  
  event.Skip();

};



BEGIN_EVENT_TABLE(PreprocessDialog, wxDialog)
  EVT_BUTTON( wxID_OK, PreprocessDialog::OnOk )
  EVT_BUTTON( wxID_CANCEL, PreprocessDialog::OnCancel )
END_EVENT_TABLE()



IMPLEMENT_APP(RMAExpress)

bool RMAExpress::OnInit()
{   

  int xpos,ypos,width,height;
  int buffer_narrays,buffer_nprobes;

  wxString buffer_temppath;

  wxImage::AddHandler( new wxPNGHandler );
  wxImage::AddHandler( new wxJPEGHandler );
  wxImage::AddHandler( new wxTIFFHandler );

  mysettings = new wxConfig(wxT("RMAExpress"),wxT("Bolstad"));
  g_mysettings = mysettings;

  if (mysettings->Exists(wxT("xpos"))){
    mysettings->Read(wxT("xpos"),&xpos);
    mysettings->Read(wxT("ypos"),&ypos);
    mysettings->Read(wxT("width"),&width);
    mysettings->Read(wxT("height"),&height);
	if (width < 200){
		width = 200;
	}
	if (height < 200){
		height = 200;
	}
  } else {
    xpos=50;
    mysettings->Write(wxT("xpos"),xpos);
    ypos=50; 
    mysettings->Write(wxT("ypos"),ypos);
    width = 450;
    mysettings->Write(wxT("width"),width);
    height = 300;
    mysettings->Write(wxT("height"),height);
  }

  

  RMAExpressFrame
    *frame = new RMAExpressFrame(_T("RMAExpress"), xpos, ypos, width, height);

  frame->Show(TRUE);
  SetTopWindow(frame);  
  
 
  
  if (mysettings->Exists(wxT("BufferSize.n.arrays"))){
    mysettings->Read(wxT("BufferSize.n.arrays"),&buffer_narrays);
  } 
   
  if (mysettings->Exists(wxT("BufferSize.n.probes"))){
    mysettings->Read(wxT("BufferSize.n.probes"),&buffer_nprobes);
  } 
  
 
  if (buffer_nprobes > 50000){
    buffer_nprobes = 50000;
  }
  if (buffer_nprobes < 10000){
    buffer_nprobes = 10000;
  }


  if (buffer_narrays > 150){
    buffer_narrays = 150;
  }
  
  if (buffer_narrays < 1){
    buffer_narrays = 1;
  } 
   
 
  if (mysettings->Exists(wxT("temporaryfiles.location"))){
    mysettings->Read(wxT("temporaryfiles.location"),&buffer_temppath);
  }



  
  mysettings->Flush();
 
  frame->SetSettings(mysettings);
  
  frame->SetPreferences( buffer_narrays, buffer_nprobes,buffer_temppath);

  
  // The following code checks to make sure that the temporary directory exists

  


  if (!wxDirExists(buffer_temppath)){
#if _WIN32 
    wchar_t temppath[512];
    if(GetTempPath(512,temppath)!=0){
      buffer_temppath = wxString(temppath);
    } else {
      buffer_temppath = wxString(_T("c:\\"));
    }
#else
    buffer_temppath = wxString(_T("/tmp/"));
#endif
    
    frame->SetPreferences( buffer_narrays, buffer_nprobes,buffer_temppath);
    
    PreferencesDialog myPreferenceDialog(NULL, -1,
					 wxT("RMAExpress Preferences"),
					 wxPoint(100,100),
					 wxDefaultSize,
					 wxDEFAULT_DIALOG_STYLE);
    
    if (frame->myprefs != NULL){
      myPreferenceDialog.SetPreferences(frame->myprefs);
    }
    myPreferenceDialog.ShowModal(); 
    frame->myprefs->SetProbesBufSize(myPreferenceDialog.GetProbesBufSize());
    frame->myprefs->SetArrayBufSize(myPreferenceDialog.GetArraysBufSize());
    frame->myprefs->SetFilePath(myPreferenceDialog.GetTempFileLoc()); 
    mysettings->Write(wxT("temporaryfiles.location"),myPreferenceDialog.GetTempFileLoc());
    mysettings->Flush();
  }
  
  
  return TRUE; 
  
  
}

RMAExpressFrame::RMAExpressFrame
             (const wxChar *title,
                    int xpos, int ypos,
                    int width, int height)
             : wxFrame
                ( (wxFrame *) NULL,
                   -1,
                   title,
                   wxPoint(xpos, ypos),
                   wxSize(width, height)
                )
{
  menuBar  = (wxMenuBar *) NULL;
  fileMenu = (wxMenu *)	NULL;
  aboutMenu = (wxMenu *) NULL;
  showMenu = (wxMenu *) NULL;

  currentexperiment = (DataGroup *) NULL;
  myexprs = (expressionGroup *)NULL;
  myresids = (ResidualsDataGroup *)NULL;

  int buffer_narrays = 30;
  int buffer_nprobes =25000;
  
  


#if _WIN32 
  wchar_t temppath[512];
  if(GetTempPath(512,temppath)!=0){
    myprefs = new Preferences(buffer_nprobes,buffer_narrays,wxString(temppath));
  } else {
    myprefs = new Preferences(buffer_nprobes,buffer_narrays,wxString(_T("c:\\")));
  }
#else
  myprefs = new Preferences(buffer_nprobes,buffer_narrays,wxString(_T("/tmp/")));//(Preferences *)NULL;
#endif
  fileMenu = new wxMenu;
  fileMenu->Append(RMAEXPRESS_READ,  wxT("&Read Unprocessed files"));
  //fileMenu->Append(RMAEXPRESS_BINARYREAD, wxT("Read Processed files"));
  fileMenu->Append(RMAEXPRESS_ADDNEWCEL, wxT("Add new CEL files"));   
  fileMenu->Append(RMAEXPRESS_BINARYWRITE, wxT("Write RME format"));
  fileMenu->AppendSeparator();
  fileMenu->Append(RMAEXPRESS_COMPUTE,  wxT("&Compute RMA measure"));
  fileMenu->Append(RMAEXPRESS_WRITE,  wxT("&Write Results to file (log scale)"));
  fileMenu->Append(RMAEXPRESS_WRITENATURAL,  wxT("&Write Results to file (natural scale)"));
  fileMenu->Append(RMAEXPRESS_EXPORT, wxT("&Export expression values"));
  fileMenu->Append(RMAEXPRESS_LOGFILEWRITE, wxT("Output Log file"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT,  wxT("E&xit"));

  showMenu = new wxMenu;
  showMenu->Append(RMAEXPRESS_SHOW_DATAGROUP,wxT("Data"));
  showMenu->Append(RMAEXPRESS_VISUALIZE_RAWDATA,wxT("Visualize Raw Data"));
  showMenu->Append(RMAEXPRESS_SHOW_RESIDUALS,wxT("Residual Images"));
  showMenu->Append(RMAEXPRESS_VISUALIZE_QC,wxT("Visualize QC Statistics"));
#ifdef BUFFERED
  showMenu->AppendSeparator();
  showMenu->Append(RMAEXPRESS_SHOW_PREFERENCES,wxT("Preferences"));
#endif
  aboutMenu = new wxMenu;
  aboutMenu->Append(wxID_ABOUT, wxT("&About"));

	;

    

  menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, wxT("&File"));
  menuBar->Append(showMenu, wxT("&Show"));
  menuBar->Append(aboutMenu, wxT("&About"));
  SetMenuBar(menuBar);
  CreateStatusBar(3);


  Messages 
   = new wxTextCtrl
  	( this,
  	  -1,
  	  wxString(_T("Welcome to RMAExpress\n")
  	          ),
  	  wxDefaultPosition,
  	  wxDefaultSize,
  	  wxTE_MULTILINE
  	);
  
  *Messages << _T("Written by ")<< author << _T("\n");  
  *Messages << _T("Version: ") << version_number << _T("\n");
  *Messages << _T("http://rmaexpress.bmbolstad.com\n\n");
  
  Messages->SetEditable(false);
  
  fileMenu->Enable(RMAEXPRESS_WRITE,false);
  fileMenu->Enable(RMAEXPRESS_WRITENATURAL,false);  
  fileMenu->Enable(RMAEXPRESS_EXPORT,false);
  fileMenu->Enable(RMAEXPRESS_COMPUTE,false);
  //fileMenu->Enable(RMAEXPRESS_BINARYREAD,true);
  fileMenu->Enable(RMAEXPRESS_ADDNEWCEL,false);
  fileMenu->Enable(RMAEXPRESS_BINARYWRITE,false);

  showMenu->Enable(RMAEXPRESS_SHOW_RESIDUALS,false);
  showMenu->Enable(RMAEXPRESS_VISUALIZE_RAWDATA,false); 
  showMenu->Enable(RMAEXPRESS_VISUALIZE_QC,false);
#if _WIN32
  SetIcon(wxICON(expressicon));
#else
  SetIcon(wxICON(RMAExpress_64));
#endif
}

RMAExpressFrame::~RMAExpressFrame()
{
  int cur_width;
  int cur_height;

  int cur_xpos;
  int cur_ypos;

  this->GetSize(&cur_width,&cur_height);
  this->GetPosition(&cur_xpos,&cur_ypos);
 
  mysettings->Write(wxT("width"),cur_width);
  mysettings->Write(wxT("height"),cur_height);
  mysettings->Write(wxT("xpos"),cur_xpos);
  mysettings->Write(wxT("ypos"),cur_ypos);
  mysettings->Write(wxT("BufferSize.n.arrays"),myprefs->GetArrayBufSize()); 
  mysettings->Write(wxT("BufferSize.n.probes"),myprefs->GetProbesBufSize());
  mysettings->Write(wxT("temporaryfiles.location"),myprefs->GetFilePath());
  mysettings->Flush();
  delete myprefs;
}


BEGIN_EVENT_TABLE (RMAExpressFrame, wxFrame)
  EVT_MENU ( wxID_EXIT,  RMAExpressFrame::OnExit)
  EVT_MENU ( wxID_ABOUT, RMAExpressFrame::OnAbout)
  EVT_MENU ( RMAEXPRESS_READ,  RMAExpressFrame::OnRead)
  EVT_MENU ( RMAEXPRESS_COMPUTE,  RMAExpressFrame::OnCompute)
  EVT_MENU ( RMAEXPRESS_WRITE, RMAExpressFrame::OnWrite)
  EVT_MENU ( RMAEXPRESS_WRITENATURAL, RMAExpressFrame::OnWriteNatural)
  EVT_MENU ( RMAEXPRESS_BINARYWRITE, RMAExpressFrame::OnBinaryWrite)
  EVT_MENU ( RMAEXPRESS_BINARYREAD, RMAExpressFrame::OnBinaryRead)
  EVT_MENU ( RMAEXPRESS_LOGFILEWRITE, RMAExpressFrame::OnLogFileWrite)
  EVT_MENU ( RMAEXPRESS_ADDNEWCEL, RMAExpressFrame::OnAddCEL)
  EVT_MENU ( RMAEXPRESS_SHOW_DATAGROUP, RMAExpressFrame::OnShowDataGroup)
  EVT_MENU ( RMAEXPRESS_SHOW_RESIDUALS, RMAExpressFrame::OnShowResidualImages)
  EVT_MENU ( RMAEXPRESS_SHOW_PREFERENCES, RMAExpressFrame::OnShowPreferencesDialog)
  EVT_MENU ( RMAEXPRESS_EXPORT, RMAExpressFrame::OnExportExpressionValues)
  EVT_MENU ( RMAEXPRESS_VISUALIZE_RAWDATA, RMAExpressFrame::OnVisualizeRawData)
  EVT_MENU ( RMAEXPRESS_VISUALIZE_QC, RMAExpressFrame::OnVisualizeQCStatistics)
END_EVENT_TABLE()


void RMAExpressFrame::SetPreferences(int narrays, int nprobes, wxString tmppath){
  myprefs->SetArrayBufSize(narrays);
  myprefs->SetProbesBufSize(nprobes);
  myprefs->SetFilePath(tmppath);

}

  
void RMAExpressFrame::SetSettings(wxConfig *settings){

  mysettings = settings;

}

void RMAExpressFrame::OnRead (wxCommandEvent & event)
{


 
  try{
    wxArrayString CelFileNames;
    wxArrayString CelFilePaths;
    
    
    if (currentexperiment!= NULL){
      wxString t=_T("You have already loaded some raw data. Are you sure you want to do this?"); 
      wxMessageDialog
	aboutDialog
	(0, t, _T("Delete current data from memory"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	return;
      }
      
      *Messages << _T("Deleting old experiment from memory so we will be ready for new experiment\n");
      delete currentexperiment;  
      currentexperiment = NULL;
      fileMenu->Enable(RMAEXPRESS_COMPUTE,false);  
      showMenu->Enable(RMAEXPRESS_VISUALIZE_RAWDATA,false);
      showMenu->Enable(RMAEXPRESS_VISUALIZE_QC,false); 

      fileMenu->Enable(RMAEXPRESS_WRITE,false);
      fileMenu->Enable(RMAEXPRESS_WRITENATURAL,false);
      fileMenu->Enable(RMAEXPRESS_EXPORT,false);
    }

    if (myexprs  !=NULL){
        delete myexprs;
	myexprs = NULL;


    }
    if (myresids !=NULL){
      delete myresids;
      myresids = NULL;
      showMenu->Enable(RMAEXPRESS_SHOW_RESIDUALS,false);
    }
    
    
    *Messages << _T("............\nStarting Analysis\n");
    *Messages << _T(".............\nSelect CDF file\n");
    
    wxFileDialog
      * GetCDFFileDialog =
      new wxFileDialog ( this,
			 wxT("Please Select your CDF file"),
			 wxT(""),
			 wxT(""),
			 CDFFILETYPES,
			 wxFD_OPEN,
			 wxDefaultPosition);
    
    if (GetCDFFileDialog->ShowModal() == wxID_OK)
      { //SetCurrentFilename(GetCDFFileDialog->GetFilename());
	*Messages << _T("\nCDF file : ") << GetCDFFileDialog->GetFilename() << _T(" in ") << GetCDFFileDialog->GetDirectory()<< _T("\n");
	//SetStatusText(GetCDFFileDialog->GetDirectory(),1);
      } else {
      *Messages << _T("Cancelled\n");
	return;
      }
    
    *Messages << _T(".............\nSelect CEL files\n");
    
    
    wxFileDialog
      * GetCELFileDialog =
      new wxFileDialog ( this,
			 wxT("Please Select your CEL files"),
			 wxT(""),
			 wxT(""),
			 CELFILETYPES,
			 wxFD_MULTIPLE,
			 wxDefaultPosition);
    
    if (GetCELFileDialog->ShowModal() == wxID_OK){
      GetCELFileDialog->GetFilenames(CelFileNames);
      GetCELFileDialog->GetPaths(CelFilePaths);
      
      for (int i =0; i < (int)CelFileNames.GetCount(); i++){
	*Messages << _T("\nCEL files : ") << CelFileNames[i];
      }
      
      *Messages <<_T("\n\n");
    } else {
      *Messages << _T("Cancelled\n");
      return;
    }
    
    *Messages << _T("Reading in data\n");
    *Messages << _T("Opening CDF and CEL files\n");
    
    currentexperiment = new DataGroup(this,GetCDFFileDialog->GetFilename(),GetCDFFileDialog->GetPath(),CelFileNames,CelFilePaths,myprefs);
    
    //fileMenu->Enable(RMAEXPRESS_READ,false);
    fileMenu->Enable(RMAEXPRESS_COMPUTE,true);
    fileMenu->Enable(RMAEXPRESS_BINARYWRITE,true);
    fileMenu->Enable(RMAEXPRESS_ADDNEWCEL,true);

    showMenu->Enable(RMAEXPRESS_VISUALIZE_RAWDATA,true);

    *Messages << _T("Done Reading in datafiles\n\n");
  }
  catch (wxString& Problem){
    *Messages << Problem << _T("\n");
    delete currentexperiment;
    currentexperiment = NULL;
    fileMenu->Enable(RMAEXPRESS_COMPUTE,false); 
  }
}



void RMAExpressFrame::OnCompute (wxCommandEvent & event)
{

 

  if (myexprs !=NULL){
    *Messages << _T("Deleting old expression values from memory so we will be ready for new experiment\n");
    delete myexprs;   
    myexprs = NULL;
  }
  if (myresids != NULL){
    *Messages << _T("Deleting old residuals from memory.\n");
    delete myresids;
    myresids = NULL;
    showMenu->Enable(RMAEXPRESS_SHOW_RESIDUALS,false);
  }




  *Messages << _T("Choose Preprocessing Steps\n");
  
    PreprocessDialog
      myPreprocessDialog ( this,
			   -1,
			   wxT("Select Preprocessing Steps"),
			   wxPoint(100,100),
			   wxSize(200,200)
			   );
  
    
  //PreprocessDialog myPreprocessDialog(this);

    if (myPreprocessDialog.ShowModal() == wxID_CANCEL){
      *Messages << _T("Cancelled\n");
      return;
    }
  this->Enable(false);
  this->Refresh();
  this->Update();
  Messages->Refresh();
  Messages->Update();
  //*Messages << myPreprocessDialog.BgMethod->GetStringSelection();
  //*Messages << myPreprocessDialog.NormMethod->GetStringSelection();


  *Messages << _T("Carrying out Analysis\n");
  
  currentexperiment->ReadOnlyMode(true);

  PMProbeBatch *PMSet = new PMProbeBatch(*currentexperiment,myprefs);

  this->Enable(false);
  Messages->Refresh();
  Messages->Update();

  if (background.Cmp(myPreprocessDialog.BgMethod->GetStringSelection()) == 0){
    *Messages << _T("Background Adjusting.\n");
    PMSet->background_adjust();
  }
  
  this->Refresh();
  this->Update();
  
  if (norm.Cmp(myPreprocessDialog.NormMethod->GetStringSelection()) == 0){
    *Messages << _T("Normalizing Using Quantile Normalization.\n");
    PMSet->normalize(1);   // Low memory Overhead
  } 

  this->Refresh();
  this->Update();


  if (summary_median_polish.Cmp(myPreprocessDialog.SummaryMethod->GetStringSelection()) == 0){
    *Messages << _T("Summarizing Using Median Polish.\n");
    myexprs = PMSet->summarize();
    *Messages << _T("Done Computing RMA Expression Measure.\n");
   } else if (summary_PLM.Cmp(myPreprocessDialog.SummaryMethod->GetStringSelection()) == 0){
    *Messages << _T("Summarizing Using PLM.\n");
    myexprs = PMSet->summarize_PLM();  
    *Messages << _T("Done computing expression measure using PLM.\n");
	this->Enable(true);		
    showMenu->Enable(RMAEXPRESS_VISUALIZE_QC,true);
	this->Enable(false);
  }
   
  if (myPreprocessDialog.StoreResiduals->IsChecked()){
    *Messages << _T("Storing Residuals\n");
    myresids = new ResidualsDataGroup(PMSet,currentexperiment, myprefs);
	  
	this->Enable(true);
    showMenu->Enable(RMAEXPRESS_SHOW_RESIDUALS,true);
	  this->Enable(false);  
  } else {
    myresids = (ResidualsDataGroup *)NULL; 
    showMenu->Enable(RMAEXPRESS_SHOW_RESIDUALS,false);
  }

  this->Enable(true);
  fileMenu->Enable(RMAEXPRESS_WRITE,true);
  fileMenu->Enable(RMAEXPRESS_WRITENATURAL,true);
  fileMenu->Enable(RMAEXPRESS_EXPORT,true);
  //fileMenu->Enable(RMAEXPRESS_COMPUTE,false);

  this->Refresh();
  this->Update();
  this->Enable(true);
  *Messages << _T("\n");
  currentexperiment->ReadOnlyMode(false);

  delete PMSet;

}



void RMAExpressFrame::OnWrite(wxCommandEvent & event){

  
  *Messages << _T("Write Results to file\n");
  
  wxFileDialog
    * ResultsFileDialog =
    new wxFileDialog ( this,
		       wxT("Save the results as text file"),
		       wxT(""),
		       wxT(""),
		       wxT("*.txt"),
		       wxFD_SAVE,
		       wxDefaultPosition);  
  
  if (ResultsFileDialog->ShowModal() == wxID_OK){
    *Messages << _T("Writing Results to file.\n");
    
    


    if (wxFileExists(ResultsFileDialog->GetPath())){
      *Messages << _T("File Exists.\n");
      
      wxString t=_T("This file already exists. Are you sure that you want to overwrite this file."); 
      wxMessageDialog
	aboutDialog
	(0, t, _T("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	*Messages << _T("Not overwritten.\n");
	return;
      }
      

      
    }
    myexprs->writetofile(ResultsFileDialog->GetFilename(),ResultsFileDialog->GetDirectory(),0);
  } else {
    *Messages << _T("Cancelled. Results were not written to file.\n");
    return;
  }
  
  *Messages << _T("Done Writing Results to file.\n");
  
}

void RMAExpressFrame::OnWriteNatural(wxCommandEvent & event){

  
  *Messages << _T("Write Results to file\n");
  
  wxFileDialog
    * ResultsFileDialog =
    new wxFileDialog ( this,
		       wxT("Save the results as text file"),
		       wxT(""),
		       wxT(""),
		       wxT("*.txt"),
		       wxFD_SAVE,
		       wxDefaultPosition);  
  
  if (ResultsFileDialog->ShowModal() == wxID_OK){
    *Messages << _T("Writing Results to file.\n");
    
    


    if (wxFileExists(ResultsFileDialog->GetPath())){
      *Messages << _T("File Exists.\n");
      
      wxString t=_T("This file already exists. Are you sure that you want to overwrite this file."); 
      wxMessageDialog
	aboutDialog
	(0, t, _T("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	*Messages << _T("Not overwritten.\n");
	return;
      }
      

      
    }
    myexprs->writetofile(ResultsFileDialog->GetFilename(),ResultsFileDialog->GetDirectory(),1);
  } else {
    *Messages << _T("Cancelled. Results were not written to file.\n");
    return;
  }
  
  *Messages << _T("Done Writing Results to file.\n");
  
}

void RMAExpressFrame::OnAbout (wxCommandEvent & event)
{ 
  wxString t = _T("RMAExpress");

  wxAboutDialogInfo aboutDialog2;

  
  aboutDialog2.SetVersion(version_number);
  aboutDialog2.SetCopyright(copyright_notice);
  aboutDialog2.SetDescription(t);
  aboutDialog2.SetName(_T("RMAExpress"));
  aboutDialog2.SetWebSite(_T("http://RMAExpress.bmbolstad.com"));
#ifndef _WIN32
  aboutDialog2.SetIcon(wxIcon(RMAExpress_64_xpm));
#endif

  wxAboutBox(aboutDialog2);
}

void RMAExpressFrame::OnExit (wxCommandEvent & event)
{
  if (currentexperiment != 0){
    delete currentexperiment;
  }
  if (myexprs !=0){
    delete myexprs;
  }
  if (myresids !=0){
    delete myresids;
  }

  Close(TRUE);
}



void RMAExpressFrame::OnLogFileWrite(wxCommandEvent & event){
  wxFileDialog
    * LogFileDialog =
    new wxFileDialog ( this,
		       wxT("Save the Log as text file"),
		       wxT(""),
		       wxT(""),
		       wxT("*.txt"),
		       wxFD_SAVE,
		       wxDefaultPosition);  
  
  if (LogFileDialog->ShowModal() == wxID_OK){
    
    if (wxFileExists(LogFileDialog->GetPath())){
      *Messages << _T("File Exists.\n");
      
      wxString t=_T("This file already exists. Are you sure that you want to overwrite this file."); 
      wxMessageDialog
	aboutDialog
	(0, t, wxT("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	*Messages << _T("Not overwritten.\n");
	return;
      }
    }
    Messages->SaveFile(LogFileDialog->GetPath());
  } else {
    *Messages << _T("Cancelled. Log file not written to file.\n");
    return;
  }
  
  *Messages << _T("Done writing log file to file.\n");
 

  

}


void RMAExpressFrame::OnBinaryWrite(wxCommandEvent & event){
   wxDirDialog
    * BinaryDirDialog =
    new wxDirDialog ( this,
		      _T("Choose directory"));  
  
  if (BinaryDirDialog->ShowModal() == wxID_OK){
    // currentexperiment->WriteBinaryCDF(BinaryDirDialog->GetPath());
    currentexperiment->WriteBinaryCEL(BinaryDirDialog->GetPath());
  } else {
    *Messages << _T("Cancelled. No binary output.");
    return;
  }
  
  *Messages << _T("Done writing binary output.\n");
  
}


void RMAExpressFrame::OnBinaryRead(wxCommandEvent & event){
  try {
    wxArrayString RMEFileNames;
    wxArrayString RMEFilePaths;


    if (currentexperiment!= NULL){
      wxString t=_T("You have already loaded some data. Are you sure you want to do this?"); 
      wxMessageDialog
	aboutDialog
	(0, t, wxT("Delete current data from memory"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	return;
      }
      
      *Messages << wxT("Deleting old experiment from memory so we will be ready for new experiment.\n");
      delete currentexperiment;  
      currentexperiment = NULL;
      fileMenu->Enable(RMAEXPRESS_COMPUTE,false);  
    }
    
    wxFileDialog
      * GetRMEFileDialog =
      new wxFileDialog ( this,
			 wxT("Please Select your RME files"),
			 wxT(""),
			 wxT(""),
			 RMEFILETYPES,
			 wxFD_MULTIPLE,
			 wxDefaultPosition);
    
    if (GetRMEFileDialog->ShowModal() == wxID_OK){
      GetRMEFileDialog->GetFilenames(RMEFileNames);
      GetRMEFileDialog->GetPaths(RMEFilePaths);
      
      for (int i =0; i < (int)RMEFileNames.GetCount(); i++){
	*Messages << _T("\nRME files : ") << RMEFileNames[i];
    }
      *Messages << _T("\n\n");
      currentexperiment = new DataGroup(this,RMEFileNames,RMEFilePaths,myprefs);
      
    } else {
      *Messages << _T("Cancelled.\n");
      return;
    }
    fileMenu->Enable(RMAEXPRESS_COMPUTE,true);
    fileMenu->Enable(RMAEXPRESS_ADDNEWCEL,true);  
    showMenu->Enable(RMAEXPRESS_VISUALIZE_RAWDATA,true);
  }
  catch (wxString Problem){
    *Messages << Problem << _T("\n");
    delete currentexperiment;
    currentexperiment = NULL;
    fileMenu->Enable(RMAEXPRESS_COMPUTE,false);    
    showMenu->Enable(RMAEXPRESS_VISUALIZE_RAWDATA,false);
  }

}



void RMAExpressFrame::OnAddCEL (wxCommandEvent & event)
{
 
  try{
    wxArrayString CelFileNames;
    wxArrayString CelFilePaths;
        
    *Messages << _T(".............\nSelect CEL files to add.\n");
    
    
    wxFileDialog
      * GetCELFileDialog =
      new wxFileDialog ( this,
			 wxT("Please Select your CEL files"),
			 wxT(""),
			 wxT(""),
			 CELFILETYPES,
			 wxFD_MULTIPLE,
			 wxDefaultPosition);
    
    if (GetCELFileDialog->ShowModal() == wxID_OK){
      GetCELFileDialog->GetFilenames(CelFileNames);
      GetCELFileDialog->GetPaths(CelFilePaths);
      
      for (int i =0; i < (int)CelFileNames.GetCount(); i++){
	*Messages << _T("\nCEL files : ") << CelFileNames[i];
      }
      
      *Messages << _T("\n\n");
    } else {
      *Messages << _T("Cancelled\n");
      return;
    }

    
      showMenu->Enable(RMAEXPRESS_VISUALIZE_QC,false); 

      fileMenu->Enable(RMAEXPRESS_WRITE,false);
      fileMenu->Enable(RMAEXPRESS_WRITENATURAL,false);
      fileMenu->Enable(RMAEXPRESS_EXPORT,false);
      
      if (myexprs  !=NULL){
        delete myexprs;
	myexprs = NULL;
      }
      if (myresids !=NULL){
	delete myresids;
	myresids = NULL;
	showMenu->Enable(RMAEXPRESS_SHOW_RESIDUALS,false);
      }
    
    
    *Messages << _T("Reading in data\n");
    *Messages << _T("Opening CEL files\n");
    
    currentexperiment->Add(CelFileNames,CelFilePaths);

    fileMenu->Enable(RMAEXPRESS_COMPUTE,true);
    fileMenu->Enable(RMAEXPRESS_BINARYWRITE,true);
    *Messages << _T("Done Reading in datafiles.\n\n");
  }
  catch (wxString Problem){
    *Messages << Problem << _T("\n");
    *Messages << _T("Potential Corruption of data structures, removing DataGroup from memory.\n");
    delete currentexperiment;
    currentexperiment = NULL;
    fileMenu->Enable(RMAEXPRESS_COMPUTE,false); 
  }
}






void  RMAExpressFrame::OnShowDataGroup(wxCommandEvent & event){
  
  long i;
  wxArrayString ArrayNames;


  if (currentexperiment == NULL){
    *Messages << _T("No DataGroup loaded.\n");
    return;
  }

  *Messages << _T("**** Data Summary ****\n");
  
  *Messages << _T("Chip Type : ") << currentexperiment->GetArrayTypeName()[0] << _T("\n");
  *Messages <<  _T("Number of Probesets : ") << currentexperiment->count_probesets() << _T("\n");
  *Messages <<  _T("Number of PM probes : ") << currentexperiment->count_pm() << _T("\n");

  *Messages <<  _T("Number of CEL files : ") << currentexperiment->count_arrays() << _T("\n");
  *Messages <<  _T("\nCEL filenames:\n");
  
  ArrayNames = currentexperiment->GetArrayNames();

  for (i=0; i < (int)ArrayNames.GetCount(); i++){
    *Messages <<  ArrayNames[i] << _T("\n");
  }
  
  *Messages << _T("**********************\n\n");

}





void  RMAExpressFrame::OnShowResidualImages(wxCommandEvent & event){
  
  int i;
  wxArrayString ArrayNames = currentexperiment->GetArrayNames();
  
  ResidualImageDialog myResidualsDialog(this,myresids, -1,
					_T("RMA Residuals"),
					wxPoint(100,100),
					wxDefaultSize,
					 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX |wxCLOSE_BOX); // wxDEFAULT_DIALOG_STYLE
  for (i =0; i < (int)ArrayNames.GetCount(); i++){
    myResidualsDialog.whichchip->Append(ArrayNames[i]);
  }

  
  myResidualsDialog.ShowModal();

}



void RMAExpressFrame::OnShowPreferencesDialog(wxCommandEvent & event){
  
  PreferencesDialog myPreferenceDialog(this, -1,
				       wxT("RMAExpress Preferences"),
				       wxPoint(100,100),
				       wxDefaultSize,
				       wxDEFAULT_DIALOG_STYLE);
  
  if (myprefs != NULL){
    myPreferenceDialog.SetPreferences(myprefs);
  }
  myPreferenceDialog.ShowModal();
  
  
  if (myprefs == NULL){
    myprefs = new Preferences(myPreferenceDialog.GetProbesBufSize(),myPreferenceDialog.GetArraysBufSize(),myPreferenceDialog.GetTempFileLoc());
  } else {
    myprefs->SetProbesBufSize(myPreferenceDialog.GetProbesBufSize());
    myprefs->SetArrayBufSize(myPreferenceDialog.GetArraysBufSize());
    myprefs->SetFilePath(myPreferenceDialog.GetTempFileLoc());
  }


  


#ifdef DEBUG
  wxPrintf(_T("%d %d %s %s\n"), myprefs->GetProbesBufSize(),myprefs->GetArrayBufSize(),myprefs->GetFilePath().c_str(),myprefs->GetFullFilePath().c_str());
#endif
  if (myresids != NULL){
    myresids->ResizeBuffer(myprefs->GetProbesBufSize(),myprefs->GetArrayBufSize());

  }
  
  if (currentexperiment!= NULL){
    currentexperiment->ResizeBuffer(myprefs->GetProbesBufSize(),myprefs->GetArrayBufSize());
  }



}








void RMAExpressFrame::OnExportExpressionValues(wxCommandEvent & event){

  
  *Messages << _T("Exporting expression values to binary format file.\n");
  
  wxFileDialog
    * ResultsFileDialog =
    new wxFileDialog ( this,
		       wxT("Exporting expression values to binary format file"),
		       wxT(""),
		       wxT(""),
		       wxT("*.txt"),
		       wxFD_SAVE,
		       wxDefaultPosition);  
  
  if (ResultsFileDialog->ShowModal() == wxID_OK){
    // *Messages << "Writing Results to file\n";
    
    if (wxFileExists(ResultsFileDialog->GetPath())){
      *Messages << _T("The file ")<< ResultsFileDialog->GetPath() << _T(" already exists.\n");
      
      wxString t= _T("This file already exists. Are you sure that you want to overwrite this file."); 
      wxMessageDialog
	aboutDialog
	(0, t, _T("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	*Messages << _T("File not overwritten\n");
	return;
      }
      

      
    }
    myexprs->writetobinaryfile(ResultsFileDialog->GetFilename(),ResultsFileDialog->GetDirectory(),0);
  } else {
    *Messages << _T("Cancelled. Results were not written to file.\n");
    return;
  }
  
  *Messages << _T("Done Writing Results to file.\n");
  
}




void RMAExpressFrame::OnVisualizeRawData(wxCommandEvent & event){

  *Messages << _T("Visualizing raw data.\n");

  PMProbeBatch *PMSet = new PMProbeBatch(*currentexperiment,myprefs);
  
  RawDataVisualizeFrame *frame = new RawDataVisualizeFrame(wxT("RMAExpress Raw Data Visualizer"),PMSet, this);
  
  frame->Show();
 
  return;



}



void RMAExpressFrame::OnVisualizeQCStatistics(wxCommandEvent & event){

  *Messages << wxT("Visualizing QC statistics\n");

  QCStatsVisualizeFrame *frame = new QCStatsVisualizeFrame(wxT("RMAExpress QC Statistics Visualizer"),myexprs, this);
 
  frame->Show();
 
  return;

}
