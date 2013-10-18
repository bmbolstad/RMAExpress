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
 ** File: BitmapSettingDialog.cpp
 **
 ** Copyright (C) B. M. Bolstad 2007
 **
 ** A Dialog box where user can change a bitmap images type
 ** and size
 **
 ** History
 ** Feb 8, 2007 
 **
 **
 **************************************************************************/


#include <wx/slider.h>
#include <wx/textctrl.h>
#include <wx/filename.h>
#include <wx/file.h>
#include <wx/utils.h>
#include <wx/config.h>

#define ID_HORIZONTAL_PIXELS 10101
#define ID_VERTICAL_PIXELS 10102
#define ID_TMPFILELOCATION 10103
#define ID_NAME 10104
#define ID_HORIZONTAL_VAL 10105
#define ID_VERTICAL_VAL 10107
#define ID_CHOOSEDIR 10106

#include "BitmapSettingDialog.h"


extern wxConfig *g_mysettings;


BitmapSettingDialog::BitmapSettingDialog(wxWindow *parent, 
					 wxWindowID id,
					 const wxString &title,
					 const wxPoint &position,
					 const wxSize& size,
					 long style):wxDialog( parent, id, title, position, size, style)
{

  long horizontal_pixels,vertical_pixels;
  long bitmap_format;



  if (g_mysettings->Exists(wxT("bitmap.horizontal.pixels"))){
    g_mysettings->Read(wxT("bitmap.horizontal.pixels"),&horizontal_pixels);
  } else {
    horizontal_pixels = 80;
    g_mysettings->Write(wxT("bitmap.horizontal.pixels"),horizontal_pixels);
  }

  if (g_mysettings->Exists(wxT("bitmap.vertical.pixels"))){
    g_mysettings->Read(wxT("bitmap.vertical.pixels"),&vertical_pixels);
  } else {
    vertical_pixels= 60;
    g_mysettings->Write(wxT("bitmap.vertical.pixels"),vertical_pixels);
  }

  if (g_mysettings->Exists(wxT("bitmap.file.format"))){
    g_mysettings->Read(wxT("bitmap.file.format"),&bitmap_format);
  } else {
    bitmap_format=0;
    g_mysettings->Write(wxT("bitmap.file.format"),bitmap_format);
  }

  g_mysettings->Flush();



  wxBoxSizer *item0 = new wxBoxSizer( wxVERTICAL ); 
  
  wxBoxSizer *item1 = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText *item1a = new wxStaticText(this, ID_NAME, wxString(_T("Horizontal Pixels:")), wxDefaultPosition, wxSize(150,25),wxALIGN_CENTER| wxALIGN_CENTER_VERTICAL);
  wxSlider *item1b = new wxSlider(this, ID_HORIZONTAL_PIXELS, horizontal_pixels , 30, 300,wxDefaultPosition, wxSize(210,25)); //,wxALIGN_CENTER| wxALIGN_CENTER_VERTICAL);
  wxTextCtrl *item1c = new wxTextCtrl(this,ID_HORIZONTAL_VAL,wxString(_T("NULL")), wxDefaultPosition, wxSize(40,20),wxALIGN_LEFT);
  wxString curval = wxString::Format(_T("%d"), item1b->GetValue()*10);

  item1c->SetEditable(false);

  item1c->SetValue(curval);
  item1->Add(item1a, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL , 5 );
  item1->Add(item1b, 2, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL , 5 );
  item1->Add(item1c, 1, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5 );

  wxBoxSizer *item2 = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText *item2a = new wxStaticText(this, ID_NAME, wxString(_T("Vertical Pixels:")),wxDefaultPosition, wxSize(150,25),wxALIGN_CENTER| wxALIGN_CENTER_VERTICAL);
  wxSlider *item2b = new wxSlider(this, ID_VERTICAL_PIXELS, vertical_pixels , 30, 300,wxDefaultPosition, wxSize(210,25)); //wxALIGN_CENTER| wxALIGN_CENTER_VERTICAL );

  wxTextCtrl *item2c = new wxTextCtrl(this,ID_VERTICAL_VAL,wxString(_T("NULL")), wxDefaultPosition, wxSize(40,20),wxALIGN_LEFT);
  curval = wxString::Format(_T("%d"), item2b->GetValue()*10);
  
  item2c->SetEditable(false);

  item2c->SetValue(curval);
  item2->Add(item2a, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5 );
  item2->Add(item2b, 2, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5 );
  item2->Add(item2c, 1, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5 );
 
  WidthInput = item1c;
  HeightInput = item2c;
  
  HorizontalPixelsSlider = item1b;
  VerticalPixelsSlider = item2b;
 
  wxString graphicstitle = _T("Output Format");
  wxString graphicschoices[] =
    {
      wxT("PNG"),
      wxT("JPG"),
      wxT("TIFF"),
    };


  wxRadioBox *item3 = new wxRadioBox(this, 
				     -1, 
				     graphicstitle, 
				     wxDefaultPosition,
				     wxDefaultSize,
				     3,
				     graphicschoices,
				     3,
				     wxHORIZONTAL,
				     wxDefaultValidator,
				     wxRadioBoxNameStr);


  ImageFormat = item3;
  ImageFormat->SetSelection(bitmap_format);

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

BitmapSettingDialog::~BitmapSettingDialog(){
  
  g_mysettings->Write(wxT("bitmap.horizontal.pixels"),HorizontalPixelsSlider->GetValue());
  g_mysettings->Write(wxT("bitmap.vertical.pixels"),VerticalPixelsSlider->GetValue());
  g_mysettings->Write(wxT("bitmap.file.format"),ImageFormat->GetSelection());
  g_mysettings->Flush();

}


IMPLEMENT_DYNAMIC_CLASS(BitmapSettingDialog, wxDialog)

BEGIN_EVENT_TABLE(BitmapSettingDialog, wxDialog)
// EVT_BUTTON( wxID_OK, PreferencesDialog::OnOk )
//  EVT_BUTTON(ID_CHOOSEDIR,PreferencesDialog::OnChooseDir)
  EVT_COMMAND_SCROLL(ID_HORIZONTAL_PIXELS, BitmapSettingDialog::MovedHorizontalPixelsSlider)
  EVT_COMMAND_SCROLL(ID_VERTICAL_PIXELS, BitmapSettingDialog::MovedVerticalPixelsSlider)
//  EVT_TEXT(ID_VERTICAL_VAL,BitmapSettingDialog::EditedVerticalPixels)
END_EVENT_TABLE()




void BitmapSettingDialog::MovedHorizontalPixelsSlider(wxScrollEvent &event){ // wxUpdateUIEvent &event){

  wxString curval = wxString::Format(_T("%d"), HorizontalPixelsSlider->GetValue()*10);
#ifdef DEBUG
  wxPrintf("%d\n",ArraysSlider->GetValue()*10);
#endif
  WidthInput->SetValue(curval);
}

void BitmapSettingDialog::MovedVerticalPixelsSlider(wxScrollEvent &event){ 
  wxString curval = wxString::Format(_T("%d"), VerticalPixelsSlider->GetValue()*10);
#ifdef DEBUG
  wxPrintf("%d\n",ProbesSlider->GetValue()*10);
#endif
  HeightInput->SetValue(curval);

}



void BitmapSettingDialog::EditedVerticalPixels(wxCommandEvent &event){

  wxString curval =  HeightInput->GetValue();

  //  wxPrintf(wxT("%s\n"), (HeightInput->GetValue()).c_str());

  // HeightInput->SetValue(wxT("1000"));



}

int BitmapSettingDialog::GetWidth(){
  return HorizontalPixelsSlider->GetValue()*10;
}

int BitmapSettingDialog::GetHeight(){
  return VerticalPixelsSlider->GetValue()*10;
}



wxString  BitmapSettingDialog::GetImageType(){

  wxString temp;
  
  temp=ImageFormat->GetStringSelection();

  if (temp.CmpNoCase(wxT("TIFF")) == 0){
    return wxString(wxT("TIF")).Lower();
  } else {
    return temp.Lower();
  }
}
