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
 ** file: RMADataConv.cpp
 **
 ** Copyright (C) 2003-2008    B. M. Bolstad
 **
 ** aim: A simple dialog based application for converting 
 **      CEL and CDF files to RMA express application
 **
 ** Created by: B. M. Bolstad  <bolstad@stat.berkeley.edu>
 ** 
 ** Created on: Sept 9, 2003
 **
 ** Description:
 ** This is a simple program for converting data types.
 **
 ** this file implements the interface part of the program.
 **
 **
 ** History
 ** Sept 9, 2003  - Initial version
 ** Sept 10, 2003 - Start implementing functionality of browse buttons
 **                 Initial implementation of covert function (error 
 **                 checks for all the inputs)
 ** Sep 11, 2003  - Continue implementation of convert function 
 ** Sep 12, 2003  - more on convert. Now handles the forceName
 **                 Restrictlist.
 ** Mar 22, 2005  - Now handles the buffered matrix case.
 **                 Dialog box now has a frame
 ** Mar 24, 2005  - Preferences Dialog box/Button
 ** Sept 16, 2006 - fix potential compile problems on unicode builds of wxWidgets
 ** Jan 5, 2008   - Add PGF/CLF conversion functionality
 ** Feb 8, 2008   - Add PS ability to PGF/CLF functionality
 ** Mar 17, 2008  - Add MPS ability to PGF/CLF functionality
 ** Jun 26, 2008  - Add About Dialog box
 **
 *****************************************************/

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/aboutdlg.h>

#include "RMADataConv.h"

#include "version_number.h"
#include "DataGroup.h"
#include "PreferencesDialog.h"
#include "PGF_CLF_to_RME.h"

#ifndef _WIN32
#include "RMADataConv_64.xpm"
#endif



RMADataConvDlg::RMADataConvDlg(const wxString& title, const wxPoint& pos, const wxSize& size): wxDialog((wxDialog *)NULL, -1, title, pos, size,wxDEFAULT_DIALOG_STYLE |wxRESIZE_BORDER)
{

  wxBoxSizer *item0 = new wxBoxSizer( wxVERTICAL); 
  
  wxBoxSizer *FileSelectors = new wxBoxSizer( wxHORIZONTAL); 

  wxBoxSizer *cdfControls = new wxBoxSizer( wxVERTICAL );
  wxBoxSizer *pgfclfControls = new wxBoxSizer( wxVERTICAL );
 
  item0->Add(FileSelectors , 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

  wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(this, wxID_ANY, 
                                                            _("CEL/CDF conversion"));
  wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);

  itemStaticBoxSizer3->Add(cdfControls, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

  wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, 
                                                            _("PGF/CLF conversion"));
  wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);

  itemStaticBoxSizer4->Add(pgfclfControls, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  FileSelectors->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, 5);
  FileSelectors->Add(itemStaticBoxSizer4, 1, wxGROW|wxALL, 5);


  /* Cel Directory */
  wxBoxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );
  wxStaticText *item2 = new wxStaticText( this, ID_TEXT, wxT("CEL File Directory"), wxDefaultPosition, wxSize(125,-1), 0 );
  item1->Add( item2, 0, wxALIGN_CENTER|wxALL, 5 );
  wxTextCtrl *item3 = new wxTextCtrl( this, ID_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(225,-1), 0 );
  item1->Add( item3, 0, wxALIGN_CENTER|wxALL, 5 );
  CelDirectory = item3;
  wxButton *item4 = new wxButton( this, CELBROWSE_BUTTON, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  item1->Add( item4, 0, wxALIGN_CENTER|wxALL, 5 );  
  cdfControls->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  

  /* CDF file */
  wxBoxSizer *item5 = new wxBoxSizer( wxHORIZONTAL );
  wxStaticText *item6 = new wxStaticText( this, ID_TEXT, wxT("CDF file"), wxDefaultPosition, wxSize(125,-1), wxALIGN_LEFT );
  item5->Add( item6, 0, wxALIGN_CENTER|wxALL, 5 );
  wxTextCtrl *item7 = new wxTextCtrl( this, ID_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(225,-1), 0 );
  item5->Add( item7, 0, wxALIGN_CENTER|wxALL, 5 );
  CdfFile = item7;
  wxButton *item8 = new wxButton( this, CDFBROWSE_BUTTON, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  item5->Add( item8, 0, wxALIGN_CENTER|wxALL, 5 );
  cdfControls->Add( item5, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  

  /* Restrict file */
  wxBoxSizer *item9 = new wxBoxSizer( wxHORIZONTAL );
  wxStaticText *item10 = new wxStaticText( this, ID_TEXT, wxT("Restrict File"), wxDefaultPosition, wxSize(125,-1), 0 );
  item9->Add( item10, 0, wxALIGN_CENTER|wxALL, 5 );
  wxTextCtrl *item11 = new wxTextCtrl( this, ID_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(225,-1), 0 );
  item9->Add( item11, 0, wxALIGN_CENTER|wxALL, 5 );
  RestrictFile = item11;
  wxButton *item12 = new wxButton( this, RESTRICTBROWSE_BUTTON, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  item9->Add( item12, 0, wxALIGN_CENTER|wxALL, 5 );
  cdfControls->Add( item9, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  

  /* Force CDF name field */
  wxBoxSizer *item13 = new wxBoxSizer( wxHORIZONTAL );
  wxStaticText *item14 = new wxStaticText( this, ID_TEXT, wxT("Force CDF Name"), wxDefaultPosition, wxSize(125,-1), 0 );
  item13->Add( item14, 0, wxALIGN_CENTER|wxALL, 5 );  
  wxTextCtrl *item15 = new wxTextCtrl( this, ID_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(225,-1), 0 );
  item13->Add( item15, 0, wxALIGN_CENTER|wxALL, 5 );
  ForceBox = item15;
  cdfControls->Add( item13, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  

  /* PGF File */
  wxBoxSizer *item25 = new wxBoxSizer( wxHORIZONTAL );
  wxStaticText *item26 = new wxStaticText( this, ID_TEXT, wxT("PGF File"), wxDefaultPosition, wxSize(125,-1), 0 );
  item25->Add( item26, 0, wxALIGN_CENTER|wxALL, 5 );
  wxTextCtrl *item27 = new wxTextCtrl( this, ID_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(225,-1), 0 );
  item25->Add( item27, 0, wxALIGN_CENTER|wxALL, 5 );
  PGFFile = item27;  
  wxButton *item28 = new wxButton( this, PGFBROWSE_BUTTON, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  item25->Add( item28, 0, wxALIGN_CENTER|wxALL, 5 );
  pgfclfControls->Add( item25, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  
  /* CLF File */
  wxBoxSizer *item29 = new wxBoxSizer( wxHORIZONTAL );
  wxStaticText *item30 = new wxStaticText( this, ID_TEXT, wxT("CLF File"), wxDefaultPosition, wxSize(125,-1), 0 );
  item29->Add( item30, 0, wxALIGN_CENTER|wxALL, 5 );
  wxTextCtrl *item31 = new wxTextCtrl( this, ID_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(225,-1), 0 );
  item29->Add( item31, 0, wxALIGN_CENTER|wxALL, 5 );
  CLFFile = item31;  
  wxButton *item32 = new wxButton( this, CLFBROWSE_BUTTON, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  item29->Add( item32, 0, wxALIGN_CENTER|wxALL, 5 );
  pgfclfControls->Add( item29, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  
  /* ps File */
  wxBoxSizer *item33 = new wxBoxSizer( wxHORIZONTAL );
  wxStaticText *item34 = new wxStaticText( this, ID_TEXT, wxT("PS file"), wxDefaultPosition, wxSize(125,-1), 0 );
  item33->Add( item34, 0, wxALIGN_CENTER|wxALL, 5 );
  wxTextCtrl *item35 = new wxTextCtrl( this, ID_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(225,-1), 0 );
  item33->Add( item35, 0, wxALIGN_CENTER|wxALL, 5 );
  PSFile = item35;  
  wxButton *item36 = new wxButton( this, PSBROWSE_BUTTON, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  item33->Add( item36, 0, wxALIGN_CENTER|wxALL, 5 );
  pgfclfControls->Add( item33, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

  // item35->Enable(false);
  //item36->Enable(false);



  /* MPS File */
  wxBoxSizer *item37 = new wxBoxSizer( wxHORIZONTAL );
  wxStaticText *item38 = new wxStaticText( this, ID_TEXT, wxT("MPS file"), wxDefaultPosition, wxSize(125,-1), 0 );
  item37->Add( item38, 0, wxALIGN_CENTER|wxALL, 5 );
  wxTextCtrl *item39 = new wxTextCtrl( this, ID_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(225,-1), 0 );
  item37->Add( item39, 0, wxALIGN_CENTER|wxALL, 5 );
  MPSFile = item39;  
  wxButton *item40 = new wxButton( this, MPSBROWSE_BUTTON, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  item37->Add( item40, 0, wxALIGN_CENTER|wxALL, 5 );
  pgfclfControls->Add( item37, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  
  item39->Enable(true);
  item40->Enable(true);







  /* Output Directory */
  wxBoxSizer *item21 = new wxBoxSizer( wxHORIZONTAL );
  wxStaticText *item22 = new wxStaticText( this, ID_TEXT, wxT("Output directory"), wxDefaultPosition, wxSize(125,-1), 0 );
  item21->Add( item22, 0, wxALIGN_CENTER|wxALL, 5 );
  wxTextCtrl *item23 = new wxTextCtrl( this, ID_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(750,-1), 0 );
  item21->Add( item23, 0, wxALIGN_CENTER|wxALL, 5 );
  OutputDirectory = item23;
  wxButton *item24 = new wxButton( this, OUTPUTBROWSE_BUTTON, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
  item21->Add( item24, 0, wxALIGN_CENTER|wxALL, 5 );
  item0->Add( item21, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
  
  
  /* Buttons for doing stuff */
  wxBoxSizer *item17 = new wxBoxSizer( wxHORIZONTAL );
  
  wxButton *item18 = new wxButton( this, ABOUT_BUTTON, wxT("About"), wxDefaultPosition, wxDefaultSize, 0 );
  item17->Add( item18, 0, wxALIGN_CENTER|wxALL, 5 );
#ifdef BUFFERED
  wxButton *item18b = new wxButton( this, PREF_BUTTON, wxT("Preferences"), wxDefaultPosition, wxDefaultSize, 0 );
  item17->Add( item18b, 0, wxALIGN_CENTER|wxALL, 5 );
#endif

  
  wxButton *item19 = new wxButton( this, CONVERT_BUTTON, wxT("Convert"), wxDefaultPosition, wxDefaultSize, 0 );
  item17->Add( item19, 0, wxALIGN_CENTER|wxALL, 5 );
  
  wxButton *item20 = new wxButton( this, QUIT_BUTTON, wxT("Quit"), wxDefaultPosition, wxDefaultSize, 0 );
  item17->Add( item20, 0, wxALIGN_CENTER|wxALL, 5 );

  item0->Add( item17, 0, wxALIGN_CENTER|wxALL, 5 );
  
  this->SetAutoLayout( TRUE );
  this->SetSizer( item0 );
  
  item0->Fit( this );

#if defined(_WIN32) && defined(BUFFERED)
  wchar_t temppath[512];
  if(GetTempPath(512,temppath)!=0){
    myprefs = new Preferences(25000,30,wxString(temppath));
  } else {
    myprefs = new Preferences(25000,30,wxString(_T("c:\\")));
  }
#elif defined(BUFFERED)
  myprefs = new Preferences(25000,30,wxString(_T("/tmp/"))); 
#endif


#if _WIN32
  SetIcon(wxICON(converticon));
#else
  SetIcon(wxICON(RMADataConv_64));
#endif

}
 

RMADataConvDlg::~RMADataConvDlg(){
  delete myprefs;
}





void RMADataConvDlg::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  wxString t = _T("RMAExpress Data Convertor");

  wxAboutDialogInfo aboutDialog2;

  
  aboutDialog2.SetVersion(version_number);
  aboutDialog2.SetCopyright(copyright_notice);
  aboutDialog2.SetDescription(t);
  aboutDialog2.SetName(_T("RMADataConv"));
  aboutDialog2.SetWebSite(_T("http://RMAExpress.bmbolstad.com"));
#ifndef _WIN32
  aboutDialog2.SetIcon(wxIcon(RMADataConv_64_xpm));
#endif  

  wxAboutBox(aboutDialog2);
}  




void RMADataConvDlg::OnQuit(wxCommandEvent& WXUNUSED(event))
{
   // --> Don't use Close with a wxDialog,
   // use Destroy instead.
   Destroy();
}

void RMADataConvDlg::OnQuit2(wxCloseEvent&WXUNUSED(event)) //  wxCommandEvent& WXUNUSED(event))
{
   // --> Don't use Close with a wxDialog,
   // use Destroy instead.
   Destroy();
}




void RMADataConvDlg::OnCelBrowse(wxCommandEvent& WXUNUSED(event)){
  wxDirDialog
    * CELDirDialog =
    new wxDirDialog ( this,
		      wxT("Choose directory"));  
  
  if (CELDirDialog->ShowModal() == wxID_OK){
    CelDirectory->SetValue(CELDirDialog->GetPath());
  } 
}

void RMADataConvDlg::OnCdfBrowse(wxCommandEvent& WXUNUSED(event)){
  

  wxFileDialog
    * CdfFileDialog =
    new wxFileDialog ( this,
		       wxT("Select the CDF file"),
		       wxT(""),
		       wxT(""),
		       CDFFILETYPES,
		       wxFD_OPEN,
		       wxDefaultPosition);  
  
  if (CdfFileDialog->ShowModal() == wxID_OK){
    CdfFile->SetValue(CdfFileDialog->GetPath());
  } 

}


void RMADataConvDlg::OnPGFBrowse(wxCommandEvent& WXUNUSED(event)){
  

  wxFileDialog
    * PGFFileDialog =
    new wxFileDialog ( this,
		       wxT("Select the PGF file"),
		       wxT(""),
		       wxT(""),
		       PGFFILETYPES,
		       wxFD_OPEN,
		       wxDefaultPosition);  
  
  if (PGFFileDialog->ShowModal() == wxID_OK){
    PGFFile->SetValue(PGFFileDialog->GetPath());
  } 

}

void RMADataConvDlg::OnCLFBrowse(wxCommandEvent& WXUNUSED(event)){
  

  wxFileDialog
    * CLFFileDialog =
    new wxFileDialog ( this,
		       wxT("Select the CLF file"),
		       wxT(""),
		       wxT(""),
		       CLFFILETYPES,
		       wxFD_OPEN,
		       wxDefaultPosition);  
  
  if (CLFFileDialog->ShowModal() == wxID_OK){
    CLFFile->SetValue(CLFFileDialog->GetPath());
  } 

}


void RMADataConvDlg::OnPSBrowse(wxCommandEvent& WXUNUSED(event)){
  

  wxFileDialog
    * PSFileDialog =
    new wxFileDialog ( this,
		       wxT("Select the PS file"),
		       wxT(""),
		       wxT(""),
		       PSFILETYPES,
		       wxFD_OPEN,
		       wxDefaultPosition);  
  
  if (PSFileDialog->ShowModal() == wxID_OK){
    PSFile->SetValue(PSFileDialog->GetPath());
  } 

}

void RMADataConvDlg::OnMPSBrowse(wxCommandEvent& WXUNUSED(event)){
  

  wxFileDialog
    * MPSFileDialog =
    new wxFileDialog ( this,
		       wxT("Select the MPS file"),
		       wxT(""),
		       wxT(""),
		       MPSFILETYPES,
		       wxFD_OPEN,
		       wxDefaultPosition);  
  
  if (MPSFileDialog->ShowModal() == wxID_OK){
    MPSFile->SetValue(MPSFileDialog->GetPath());
  } 

}




void RMADataConvDlg::OnRestrictBrowse(wxCommandEvent& WXUNUSED(event)){

  wxFileDialog
    * RestrictFileDialog =
    new wxFileDialog ( this,
		       wxT("Select the Restrict file"),
		       wxT(""),
		       wxT(""),
		       wxT(""),
		       wxFD_OPEN,
		       wxDefaultPosition);  
  
  if (RestrictFileDialog->ShowModal() == wxID_OK){
    RestrictFile->SetValue(RestrictFileDialog->GetPath());
  } 

}


void RMADataConvDlg::OnOutputBrowse(wxCommandEvent& WXUNUSED(event)){
  wxDirDialog
    * OutputDirDialog =
    new wxDirDialog ( this,
		      wxT("Choose directory"));  
  
  if (OutputDirDialog->ShowModal() == wxID_OK){
    OutputDirectory->SetValue(OutputDirDialog->GetPath());
  } 
}




/**********************************************************************************
 **
 **
 **
 ** This function actually does the conversion. To Convert we need at the minimum
 ** either a CDF file or a Directory containing CEL files and an outputdirectory
 **
 *********************************************************************************/

void RMADataConvDlg::OnConvert(wxCommandEvent& WXUNUSED(event)){

  wxString ErrorMessage;
  
  wxString outputlocation, CelLocation, CdfLocation;
  wxString PGFLocation, CLFLocation;
  wxString PSLocation, MPSLocation;

  wxArrayString my_celfiles;
  
  wxArrayString restrict_names;
  
    
    // check that we have an output directory and it is valid
    
    outputlocation = OutputDirectory->GetValue();
    
    if (outputlocation.IsEmpty()){
      wxString ErrorMessage = _T("No output location specified");
      wxMessageDialog aboutDialog( this, ErrorMessage, wxT("Error Message"), wxOK | wxICON_HAND);
      aboutDialog.ShowModal();

      return;
    }
    
    if (!wxDirExists(outputlocation)){ 
      wxString ErrorMessage = _T("Specified output location does not exist");
      wxMessageDialog aboutDialog( this, ErrorMessage, wxT("Error Message"), wxOK |wxICON_HAND);
      aboutDialog.ShowModal();
      return;
    }
  
  

    // Check that we have either CEL and/or CDF or alternatively a PGF/CLF pair
  
    CelLocation = CelDirectory->GetValue();
    CdfLocation = CdfFile->GetValue();
    PGFLocation = PGFFile->GetValue();
    CLFLocation = CLFFile->GetValue();

    if ((CelLocation.IsEmpty() && CdfLocation.IsEmpty()) && (PGFLocation.IsEmpty() || CLFLocation.IsEmpty())){
      
      wxString ErrorMessage = _T("Need either CEL files, CDF file or PGF/CLF pair to convert.");
      wxMessageDialog aboutDialog( this, ErrorMessage, wxT("Error Message"), wxOK |wxICON_HAND);
      aboutDialog.ShowModal();
      return;
    }
    
    bool haveCELorCDF = !CelLocation.IsEmpty() || !CdfLocation.IsEmpty();
    bool havePGFCLFpair = !PGFLocation.IsEmpty() && !CLFLocation.IsEmpty();
    bool havePGForCLFpair = !PGFLocation.IsEmpty() || !CLFLocation.IsEmpty();

    PSLocation = PSFile->GetValue();
    MPSLocation = MPSFile->GetValue();

    //    bool haveOtherCELCDFFields =;
    bool haveOtherPGFCLFFields = !PSLocation.IsEmpty() || !MPSLocation.IsEmpty();
    bool haveOtherPGFCLFFieldsPS = !PSLocation.IsEmpty();
    bool haveOtherPGFCLFFieldsMPS = !MPSLocation.IsEmpty();

    if (haveCELorCDF && havePGFCLFpair){
      wxString ErrorMessage = _T("CEL/CDF file fields and PGF/CLF fields can not both be defined.");
      wxMessageDialog aboutDialog( this, ErrorMessage, wxT("Error Message"), wxOK |wxICON_HAND);
      aboutDialog.ShowModal();
      return;
    }
    
    if (haveCELorCDF && havePGForCLFpair){
      wxString ErrorMessage = _T("CEL/CDF file fields and PGF/CLF fields can not both be defined.");
      wxMessageDialog aboutDialog( this, ErrorMessage, wxT("Error Message"), wxOK |wxICON_HAND);
      aboutDialog.ShowModal();
      return;
    }
    
    
    
    if (haveCELorCDF && haveOtherPGFCLFFields){
      wxString ErrorMessage = _T("CEL/CDF file fields and PS or MPS file fields can not both be defined.");
      wxMessageDialog aboutDialog( this, ErrorMessage, wxT("Error Message"), wxOK |wxICON_HAND);
      aboutDialog.ShowModal();
      return;
    }

    if (haveOtherPGFCLFFieldsPS && haveOtherPGFCLFFieldsMPS){
      wxString ErrorMessage = _T("PS and MPS file fields can not both be defined. Only one may be used.");
       wxMessageDialog aboutDialog( this, ErrorMessage, wxT("Error Message"), wxOK |wxICON_HAND);
       aboutDialog.ShowModal();
       return;
    }

    /* CEL file/CDF file conversion */
    if (haveCELorCDF){
      // Check if CelLocation is non empty then if we actually have Cel files
      
      if (!CelLocation.IsEmpty()){
	wxDir CelDir(CelLocation);
	if (!((CelDir.HasFiles(_T("*.cel"))) || (CelDir.HasFiles(_T("*.CEL"))))){
	  wxString ErrorMessage = _T("No CEL files in specified location.");
	  wxMessageDialog aboutDialog( this, ErrorMessage, wxT("Error Message"), wxOK |wxICON_HAND);
	  aboutDialog.ShowModal();
	  return;
	} else {
	  CelDir.GetAllFiles(CelLocation,&my_celfiles,_T("*.cel"),wxDIR_FILES);
	  CelDir.GetAllFiles(CelLocation,&my_celfiles,_T("*.CEL"),wxDIR_FILES);
	  
	}
      }
      
      // Check if CdfLocation is non empty then whether the CDF file exists
      
      if (!CdfLocation.IsEmpty()){
	if (!wxFileExists(CdfLocation)){
	  wxString ErrorMessage = _T("This CDF file does not seem to exist :");
	  ErrorMessage.Append(CdfLocation);
	  wxMessageDialog aboutDialog( this, ErrorMessage, wxT("Error Message"), wxOK |wxICON_HAND);
	  aboutDialog.ShowModal();
	  return;
	}
      }
      
      
      // Now see if restrict and Force exist at all (NOTE these are optional)
      wxString restrictFile = RestrictFile->GetValue();
      wxString forceName = ForceBox->GetValue();
      
      
      if (!restrictFile.IsEmpty()){
	if (!wxFileExists(restrictFile)){
	  wxString ErrorMessage = _T("This restrict file does not seem to exist :");
	  ErrorMessage.Append(restrictFile);
	  wxMessageDialog aboutDialog( this, ErrorMessage, wxT("Error Message"), wxOK |wxICON_HAND);
	  aboutDialog.ShowModal();
	  return;
	} else {
	  wxFileInputStream my_restrict_file_stream(restrictFile);
	  wxTextInputStream my_restrict_file(my_restrict_file_stream);
	  wxString LineBuffer;
	  
	  while (!my_restrict_file_stream.Eof()){
	    LineBuffer = my_restrict_file.ReadLine();
	    LineBuffer.Trim();
	    if (!LineBuffer.IsEmpty())
	      restrict_names.Add(LineBuffer);
	  }
	}
      }
      
      if (!forceName.IsEmpty()){
	forceName = forceName.Trim(TRUE);
	forceName = forceName.Trim(FALSE);
	if (forceName.IsEmpty()){
	  wxString ErrorMessage = _T("Error in Force Name.");
	  wxMessageDialog aboutDialog( this, ErrorMessage, wxT("Error Message"), wxOK |wxICON_HAND);
	  aboutDialog.ShowModal();
	  return;
	}
      }
      
      // Now actually do the converting. To do this we read in an DataGroup
      
      try{
	
	if (!CdfLocation.IsEmpty() & CelLocation.IsEmpty()){
	  // Only CDF information
	  
	  
	  DataGroup mydata((wxWindow *)this,CdfLocation);
	  
	  if (!forceName.IsEmpty()){
	    mydata.SetArrayTypeName(forceName);
	  }
	  
	  if (restrictFile.IsEmpty()){
	    mydata.WriteBinaryCDF(outputlocation);
	  } else {
	    mydata.WriteBinaryCDF(outputlocation, restrictFile, restrict_names);
	  }
	} else if (CdfLocation.IsEmpty() & !CelLocation.IsEmpty()){
	  // Only CEL information
	  
	  my_celfiles.Sort();
	  
	  DataGroup mydata((wxWindow *)this,my_celfiles,myprefs);
	  
	  if (!forceName.IsEmpty()){
	    mydata.SetArrayTypeName(forceName);
	  }
	  mydata.WriteBinaryCEL(outputlocation);
	} else {
	  // Both CDF and CEL information
	  
	  wxFileName my_cdf(CdfLocation);  
	  
	  wxArrayString CelFullName;
	  wxArrayString CelPath;
	  
	  my_celfiles.Sort();
	  
	  for (int i = 0; i < my_celfiles.Count(); i++){
	    wxFileName my_cel(my_celfiles[i]);
	    CelFullName.Add(my_cel.GetFullName());
	    CelPath.Add(my_celfiles[i]);
	    //CelPath.Add(my_cel.GetPath());
	  }
#ifndef BUFFERED
	  Preferences myprefs(25000,30,wxString(outputlocation));
	  DataGroup mydata((wxWindow *)this,my_cdf.GetFullName(),CdfLocation,CelFullName,CelPath, &myprefs);
#else
	  DataGroup mydata((wxWindow *)this,my_cdf.GetFullName(),CdfLocation,CelFullName,CelPath, myprefs);
#endif
	  if (!forceName.IsEmpty()){
	    mydata.SetArrayTypeName(forceName);
	  }
	  
	  if (restrictFile.IsEmpty()){
	    mydata.WriteBinaryCDF(outputlocation);
	  } else {
	    mydata.WriteBinaryCDF(outputlocation,restrictFile, restrict_names);
	  }
	  mydata.WriteBinaryCEL(outputlocation);

	}
	return;
	
      }
      catch(wxString ErrorMessage){
	wxMessageDialog aboutDialog( this, ErrorMessage, wxT("Error Message"), wxOK | wxICON_HAND);
	aboutDialog.ShowModal();
      }
    }

    if (havePGFCLFpair){
      try{
	if (haveOtherPGFCLFFieldsPS){
	  Convert_PGF_CLF_to_RME_with_PS(PGFLocation, CLFLocation,PSLocation,outputlocation);
	} else if (haveOtherPGFCLFFieldsMPS) { 
	  Convert_PGF_CLF_to_RME_with_MPS(PGFLocation, CLFLocation,MPSLocation,outputlocation);
	} else {
	  Convert_PGF_CLF_to_RME(PGFLocation, CLFLocation,outputlocation);
	}
      }
      catch (wxString ErrorMessage){
	wxMessageDialog aboutDialog( this, ErrorMessage, wxT("Error Message"), wxOK | wxICON_HAND);
	aboutDialog.ShowModal();
      }
    }
}

void RMADataConvDlg::OnPreferences(wxCommandEvent& WXUNUSED(event)){
  PreferencesDialog myPreferenceDialog(this, -1,
				       wxT("RMAExpress Data Convertor Preferences"),
				       wxPoint(100,100),
				       wxDefaultSize,
				       wxDEFAULT_DIALOG_STYLE);
#ifdef BUFFERED
  if (myprefs != NULL){
    myPreferenceDialog.SetPreferences(myprefs);
  }
#endif

  myPreferenceDialog.ShowModal();

#ifdef BUFFERED
  if (myprefs == NULL){
    myprefs = new Preferences(myPreferenceDialog.GetProbesBufSize(),myPreferenceDialog.GetArraysBufSize(),myPreferenceDialog.GetTempFileLoc());
  } else {
    myprefs->SetProbesBufSize(myPreferenceDialog.GetProbesBufSize());
    myprefs->SetArrayBufSize(myPreferenceDialog.GetArraysBufSize());
    myprefs->SetFilePath(myPreferenceDialog.GetTempFileLoc());
  }
#endif


}



BEGIN_EVENT_TABLE(RMADataConvDlg, wxDialog)
  EVT_BUTTON(CELBROWSE_BUTTON,    RMADataConvDlg::OnCelBrowse)
  EVT_BUTTON(CDFBROWSE_BUTTON,    RMADataConvDlg::OnCdfBrowse)
  EVT_BUTTON(RESTRICTBROWSE_BUTTON,    RMADataConvDlg::OnRestrictBrowse)
  EVT_BUTTON(PGFBROWSE_BUTTON,    RMADataConvDlg::OnPGFBrowse)
  EVT_BUTTON(CLFBROWSE_BUTTON,    RMADataConvDlg::OnCLFBrowse)
  EVT_BUTTON(PSBROWSE_BUTTON,    RMADataConvDlg::OnPSBrowse)  
  EVT_BUTTON(MPSBROWSE_BUTTON,    RMADataConvDlg::OnMPSBrowse)
  EVT_BUTTON(OUTPUTBROWSE_BUTTON,    RMADataConvDlg::OnOutputBrowse)
  EVT_BUTTON(PREF_BUTTON,    RMADataConvDlg::OnPreferences)
  EVT_BUTTON(ABOUT_BUTTON,    RMADataConvDlg::OnAbout)
  EVT_BUTTON(CONVERT_BUTTON,    RMADataConvDlg::OnConvert)
  EVT_BUTTON(QUIT_BUTTON,    RMADataConvDlg::OnQuit)
  EVT_CLOSE(RMADataConvDlg::OnQuit2)
END_EVENT_TABLE()
  



IMPLEMENT_APP(RMADataConv)

bool RMADataConv::OnInit()
{
  RMADataConvDlg *dialog = new RMADataConvDlg(_T("RMAExpress Data Convertor"),
                                 wxPoint(50, 50), wxSize(800, 800));

    // Show it and tell the application that it's our main window
    dialog->Show(TRUE);
    SetTopWindow(dialog);


    


    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned FALSE here, the
    // application would exit immediately.
    return TRUE;
}
