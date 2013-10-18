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
 ** file: PreferencesDialog.cpp
 **
 ** Copyright (C) 2005-2008    B. M. Bolstad
 **
 ** aim: Preference Dialog Box
 **      
 **
 ** Created by: B. M. Bolstad  <bmb@bmbolstad.com>
 ** 
 ** Created on: Mar 20, 2005
 **
 **
 ** 
 ** Mar 20, 2005 - Added preferences dialog box.
 ** Apr 1, 2005 - Added a check that the temporary file location exists
 **               and a test to see if a small file can be written there.
 ** Sept 16, 2006 - fix compile problems with unicode builds of wxWidgets
 ** Feb 5, 2008 - allow minimum of 1 array in Buffer.
 **
 *****************************************************/


#include "PreferencesDialog.h"
#include <wx/slider.h>
#include <wx/textctrl.h>
#include <wx/filename.h>
#include <wx/file.h>
#include <wx/utils.h>

#define ID_ARRAYSBUFFERED 10101
#define ID_PROBESBUFFERED 10102
#define ID_TMPFILELOCATION 10103
#define ID_NAME 10104
#define ID_NAME_VAL 10105
#define ID_CHOOSEDIR 10106

#if _WIN32
static wxString fullname =_T("");
#else
static wxString fullname =_T("RMA");
#endif

/*************************************************************

    PreferencesDialog is where the user can set the settings

*************************************************************/


#if RMA_GUI_APP

PreferencesDialog::PreferencesDialog(wxWindow *parent, 
				     wxWindowID id,
				     const wxString &title,
				     const wxPoint &position,
				     const wxSize& size,
				     long style):wxDialog( parent, id, title, position, size, style)
{


  wxBoxSizer *item0 = new wxBoxSizer( wxVERTICAL ); 
  

  wxBoxSizer *item1 = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText *item1a = new wxStaticText(this, ID_NAME, wxString(_T("Arrays in Buffer:")), wxDefaultPosition, wxDefaultSize,wxALIGN_CENTER| wxALIGN_CENTER_VERTICAL);
  wxSlider *item1b = new wxSlider(this, ID_ARRAYSBUFFERED, 40 , 1, 150,wxDefaultPosition, wxSize(210,25)); //,wxALIGN_CENTER| wxALIGN_CENTER_VERTICAL);
  wxStaticText *item1c = new wxStaticText(this,ID_NAME_VAL,wxString(_T("NULL")), wxDefaultPosition, wxSize(100,20),wxALIGN_LEFT);
  wxString curval = wxString::Format(_T("%d"), item1b->GetValue());

  item1c->SetLabel(curval);
  item1->Add(item1a, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL , 5 );
  item1->Add(item1b, 2, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL , 5 );
  item1->Add(item1c, 1, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5 );


  ArraysSlider = item1b;
  ArraysSliderMsg = item1c;

  wxBoxSizer *item2 = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText *item2a = new wxStaticText(this, ID_NAME, wxString(_T("Probes in Buffer:")),wxDefaultPosition, wxDefaultSize,wxALIGN_CENTER| wxALIGN_CENTER_VERTICAL);
  wxSlider *item2b = new wxSlider(this, ID_PROBESBUFFERED, 40 , 10, 50,wxDefaultPosition, wxSize(210,25)); //wxALIGN_CENTER| wxALIGN_CENTER_VERTICAL );

  wxStaticText *item2c = new wxStaticText(this,ID_NAME_VAL,wxString(_T("NULL")), wxDefaultPosition, wxSize(100,20),wxALIGN_LEFT);
  curval = wxString::Format(_T("%d"), item2b->GetValue()*1000);
  
  item2c->SetLabel(curval);
  item2->Add(item2a, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5 );
  item2->Add(item2b, 2, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5 );
  item2->Add(item2c, 1, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5 );

  ProbesSlider = item2b;
  ProbesSliderMsg = item2c;

  wxBoxSizer *item3 = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText *item3a = new wxStaticText(this,ID_NAME_VAL,wxString(_T("Temporary File Location:")), wxDefaultPosition, wxDefaultSize,wxALIGN_CENTER| wxALIGN_CENTER_VERTICAL);
  wxTextCtrl *item3b = new wxTextCtrl(this, ID_TMPFILELOCATION,_T("/tmp"), wxDefaultPosition,wxSize(250,20));
  wxButton *item3c = new wxButton(this, ID_CHOOSEDIR, wxT("Choose Dir"), wxDefaultPosition, wxDefaultSize, 0);
  item3->Add(item3a, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
  item3->Add(item3b, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
  item3->Add(item3c, 0, wxALIGN_CENTER|wxALL, 5 );
  
  tempfilepath = item3b;

  wxButton *item4 = new wxButton(this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );

  //  wxStaticText * item5 = new wxStaticText(this, ID_NAME, wxString("Blah"), wxDefaultPosition, wxDefaultSize);
  //  wxString *item5 = new wxString("Blah"); 

  item0->Add( item1, 1, wxALIGN_CENTER|wxALL, 5 );
  item0->Add( item2, 1, wxALIGN_CENTER|wxALL, 5 );
  item0->Add( item3, 1, wxALIGN_CENTER|wxALL, 5 );
  item0->Add( item4, 0, wxALIGN_CENTER|wxALL, 5 );
  this->SetAutoLayout( TRUE );
  this->SetSizer( item0 );
  
  item0->Fit( this );
  item0->SetSizeHints( this );

}




void PreferencesDialog::OnOk(wxCommandEvent &event ){
  
  wxString fpath = tempfilepath->GetValue();
  wxString tmpname =_T("RMA_TMP_TESTER");

  if (!wxDirExists(fpath)){
    wxString t=_T("This directory") + fpath +  _T(" does not exist. Try to Create?"); 
    wxMessageDialog
      checkdirDialog
      (0, t, _T("Create Directory?"), wxYES_NO);
      
    if (checkdirDialog.ShowModal() == wxID_NO){
      return;
    } else {
      if (!wxMkdir(fpath)){
	return;
      }
    }

  }

  wxFileName testfilename(fpath,tmpname);
    
  wxFile testfile;

  testfile.Create(testfilename.GetFullPath());
  if (testfile.IsOpened()){
    testfile.Close();
    wxRemoveFile(testfilename.GetFullPath());
  } else {
    wxString t=_T("Can't write to the directory :") + fpath; 
    wxMessageDialog
      checkdirDialog
      (0, t, _T("Problem"), wxOK);
    return;
  }

  event.Skip();

}

void PreferencesDialog::OnChooseDir(wxCommandEvent &event ){
  wxDirDialog BinaryDirDialog( this,_T("Choose directory"),tempfilepath->GetValue()); 
  BinaryDirDialog.ShowModal(); 
  tempfilepath->SetValue(BinaryDirDialog.GetPath());
  event.Skip();

}





PreferencesDialog::~PreferencesDialog(){

}


IMPLEMENT_DYNAMIC_CLASS(PreferencesDialog, wxDialog)

BEGIN_EVENT_TABLE(PreferencesDialog, wxDialog)
  EVT_BUTTON( wxID_OK, PreferencesDialog::OnOk )
  EVT_BUTTON(ID_CHOOSEDIR,PreferencesDialog::OnChooseDir)
  EVT_COMMAND_SCROLL(ID_ARRAYSBUFFERED, PreferencesDialog::MovedArraysSlider)
  EVT_COMMAND_SCROLL(ID_PROBESBUFFERED, PreferencesDialog::MovedProbesSlider)
END_EVENT_TABLE()

  void PreferencesDialog::MovedArraysSlider(wxScrollEvent &event){ // wxUpdateUIEvent &event){

  wxString curval = wxString::Format(_T("%d"), ArraysSlider->GetValue());
#ifdef DEBUG
  wxPrintf("%d\n",ArraysSlider->GetValue());
#endif
  ArraysSliderMsg->SetLabel(curval);
}

void PreferencesDialog::MovedProbesSlider(wxScrollEvent &event){ // wxUpdateUIEvent &event){
  wxString curval = wxString::Format(_T("%d"), ProbesSlider->GetValue()*1000);
#ifdef DEBUG
  wxPrintf("%d\n",ProbesSlider->GetValue()*1000);
#endif
  ProbesSliderMsg->SetLabel(curval);

}


int PreferencesDialog::GetArraysBufSize(){
  
  return ArraysSlider->GetValue();
  
}


int PreferencesDialog::GetProbesBufSize(){

  return ProbesSlider->GetValue()*1000;

}


wxString PreferencesDialog::GetTempFileLoc(){


  return tempfilepath->GetValue();

}

void PreferencesDialog::SetPreferences(Preferences *mypref){
  
  wxString curval,curval2;
  ProbesSlider->SetValue(mypref->GetProbesBufSize()/1000);
  curval = wxString::Format(_T("%d"), ProbesSlider->GetValue()*1000);
  ProbesSliderMsg->SetLabel(curval);

  ArraysSlider->SetValue(mypref->GetArrayBufSize());
  curval2 = wxString::Format(_T("%d"), ArraysSlider->GetValue());
  ArraysSliderMsg->SetLabel(curval2);
  
  tempfilepath->SetValue( mypref->GetFilePath());
  
}

#endif

/*************************************************************

    Preferences is where the settings are stored

*************************************************************/


Preferences::Preferences(){


}


Preferences::Preferences(int ProbesBufSize, int ArraysBufSize, wxString filepath){

  this->filepath = filepath;
  this->ArraysBufSize = ArraysBufSize;
  this->ProbesBufSize = ProbesBufSize;


}


int Preferences::GetArrayBufSize(){
  return ArraysBufSize;
}
  


void Preferences::SetArrayBufSize(int value){
  ArraysBufSize = value;

}

int Preferences::GetProbesBufSize(){
  return ProbesBufSize;
}
  


void Preferences::SetProbesBufSize(int value){
  ProbesBufSize = value;

}

void Preferences::SetFilePath(wxString value){
  filepath = value;


}

wxString &Preferences::GetFilePath(){

  return filepath;

}


wxString Preferences::GetFullFilePath(){
  
  wxString temp(wxFileName(filepath,fullname).GetFullPath());

  return temp;


}
