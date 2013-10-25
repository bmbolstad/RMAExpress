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
 ** File: residualsimages.cpp
 **
 ** Copyright (C) B. M. Bolstad 2004-2007
 **
 ** Aim is to implement residuals images dialog
 **
 ** History - Created Jan 20, 2004
 ** Feb 28, 2004 - added radiobox for selecting, both, positive or negative residuals
 **               and altered drawing routine to draw them.
 **
 ** Feb 29, 2004 - fixed save buttons.
 **
 ** Jul 11, 2005 - added sign of residuals, fixed colors which were around wrong way (why no one noticed until now is somewhat of a mystery).m
 ** Sep 30, 2005 - moved getBlue, getRed, getGreen, drawPseudoChipImage to tiheir own file 
 ** Jan 30, 2005 - fixed scroll so can get to edge of large images. Stays at same position if you change the image type.
 ** Sept 16. 2006 - fix potential compile problems on unicode builds of wxWidgets
 ** Feb 22, 2007 - add prev and next buttons
 ** Jan 4, 2007 - improve speed of drawing image to window
 ** Oct 22, 2013 - Default to saving images as jpg
 **
 ***************************************************************************/


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
#include <wx/filename.h>
#include <wx/dcbuffer.h>
#include <wx/bitmap.h>

#include "residualimages.h"
#include "ResidualsDataGroup.h"
#include "Storage/Matrix.h"
#include "ResidualsImagesDrawing.h"


#define ID_IMAGE 10003
#define ID_WHICHCHIP 10004
#define ID_WHICHTYPE 10005
#define ID_WHICHZOOM 10006
#define ID_SAVETHISIMAGE 10007
#define ID_SAVETHISCHIP 10008
#define ID_SAVEALLIMAGE 10009

#define ID_PREV_IMAGE 10020
#define ID_NEXT_IMAGE 10025




IMPLEMENT_DYNAMIC_CLASS(MyCanvas, wxScrolledWindow)

BEGIN_EVENT_TABLE(MyCanvas, wxScrolledWindow)
  EVT_PAINT(MyCanvas::OnPaint)
EVT_ERASE_BACKGROUND(MyCanvas::OnEraseBackground)
END_EVENT_TABLE()
 

void MyCanvas::OnPaint( wxPaintEvent &WXUNUSED(event) )
{

  int x, y;
  int width, height;

  wxMemoryDC memdc;
  memdc.SelectObject(*my_bitmap);
  
  if (my_parent->needtoredraw){
  
    PrepareDC( memdc );	
  	memdc.Clear();
  
  	wxImage tempimage = my_bitmap->ConvertToImage();
  	drawPseudoChipImage(&tempimage,my_parent->whichchip->GetValue(),my_parent->whichtype->GetStringSelection(),my_resids);
  	
  	*my_bitmap = tempimage;	
  	//PrepareDC( memdc );	
  	//memdc.Clear();
  	//drawPseudoChipImage(&memdc,my_parent->whichchip->GetValue(),my_parent->whichtype->GetStringSelection(),my_resids);
  	
  	
  	my_parent->needtoredraw = false;
  }
  
  //wxMemoryDC memdc;
  memdc.SelectObject(*my_bitmap);
 
  
  if (my_parent->whichzoom->GetSelection() != 3){
  		wxBitmap resize_bitmap(*my_bitmap);	
  		int orig_width = resize_bitmap.GetWidth();
  		int orig_height = resize_bitmap.GetHeight();
  		
  		wxImage resize_image = resize_bitmap.ConvertToImage();
  	
  	
  		double rescale_fac;
  		
  		if (my_parent->whichzoom->GetSelection() == 0){
  		  rescale_fac = 0.3162278;
  		} else if (my_parent->whichzoom->GetSelection() == 1){
  		   rescale_fac = 0.5;
  		} else if (my_parent->whichzoom->GetSelection() == 2){
  		   rescale_fac = 0.7071068;
  		} else  if (my_parent->whichzoom->GetSelection() == 4){
  		   rescale_fac = 1.414214;
  		}
  		resize_image.Rescale(orig_width*rescale_fac,orig_height*rescale_fac);
  		
  		resize_bitmap = resize_image;
  		
  		memdc.SelectObject(resize_bitmap);
   }
  
  wxBufferedPaintDC dc(this);
  PrepareDC(dc);
  
  PaintBackground(dc);

  GetViewStart(&x, &y);
  x = x*10;
  y = y*10;
  GetClientSize(&width, &height);

  dc.Blit(x,y,width,height,&memdc,x,y);
 //  drawPseudoChipImage(&dc,my_parent->whichchip->GetValue(),my_parent->whichtype->GetStringSelection(),my_resids,y,x,width+y,height+x);
  
}


void MyCanvas::PaintBackground(wxDC& dc){

  wxColour backgroundColour = GetBackgroundColour();

  if (!backgroundColour.Ok())
    backgroundColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);

  dc.SetBrush(wxBrush(backgroundColour));
  dc.SetPen(wxPen(backgroundColour,1));

  wxRect windowRect(wxPoint(0,0),GetClientSize());

  CalcUnscrolledPosition(windowRect.x, windowRect.y, &windowRect.x, &windowRect.y);

  dc.DrawRectangle(windowRect);
  

}


void MyCanvas::OnEraseBackground(wxEraseEvent& event){

}



ResidualsDataGroup *MyCanvas::GiveMyResids(){
  
  return my_resids;

}


void MyCanvas::SetScroll(){

  this->SetScrollbars( 10, 10, (my_resids->ncols()*2+50)/10, (my_resids->nrows()*2+50)/10, 0, 0 );

}


MyCanvas::MyCanvas(ResidualImageDialog *Parent, ResidualsDataGroup *resids):wxScrolledWindow(Parent, -1, wxPoint(10,10), wxSize(600,600), wxHSCROLL|wxVSCROLL|wxSUNKEN_BORDER){
  my_parent = Parent;
  my_resids  = resids;
  //this->SetScrollbars( 10, 10, (my_resids->nrows()+50)/10, (my_resids->ncols()+50)/10, 0, 0 );
  this->SetScroll();
  this->SetBackgroundColour(*wxWHITE );
  this->Refresh();
  
  long horizontalsize = my_resids->ncols()+ 40;
  long verticalsize = my_resids->nrows() + 40;
  
  my_bitmap = new wxBitmap(horizontalsize*2,verticalsize*2);
  
  
}


void ResidualImageDialog::OnOk(wxCommandEvent &event ){
  
  event.Skip();

};

IMPLEMENT_DYNAMIC_CLASS(ResidualImageDialog, wxDialog)

BEGIN_EVENT_TABLE(ResidualImageDialog, wxDialog)
  EVT_BUTTON( wxID_OK, ResidualImageDialog::OnOk )
  EVT_COMBOBOX(ID_WHICHCHIP, ResidualImageDialog::ChangeChip)
  EVT_RADIOBOX(ID_WHICHTYPE, ResidualImageDialog::ChangeImageType)
  EVT_RADIOBOX(ID_WHICHZOOM, ResidualImageDialog::ChangeZoom)
  EVT_BUTTON(ID_SAVETHISIMAGE,ResidualImageDialog::SaveCurrentImage)
  EVT_BUTTON(ID_SAVETHISCHIP,ResidualImageDialog::SaveCurrentImageAll)
  EVT_BUTTON(ID_SAVEALLIMAGE,ResidualImageDialog::SaveAllImages)
  EVT_BUTTON(ID_PREV_IMAGE,ResidualImageDialog::ClickPrevious)
  EVT_BUTTON(ID_NEXT_IMAGE,ResidualImageDialog::ClickNext)
END_EVENT_TABLE()
  

  
void ResidualImageDialog::ClickNext(wxCommandEvent &event){
  if (whichchip->GetSelection() < (int)whichchip->GetCount()-1){
    whichchip->SetSelection(whichchip->GetSelection()+1); 
  } else {
    whichchip->SetSelection(0); 
  }
  ChangeChip(event);
};
  



void  ResidualImageDialog::ClickPrevious(wxCommandEvent &event){
  if (whichchip->GetSelection() > 0){
    whichchip->SetSelection(whichchip->GetSelection()-1); 
  } else {
    whichchip->SetSelection(whichchip->GetCount()-1); 
  }


  ChangeChip(event);
};




void ResidualImageDialog::ChangeChip(wxCommandEvent &event){ 
 
 needtoredraw=true; 
 
 m_canvas->Refresh();
 
}
  


void ResidualImageDialog::ChangeImageType(wxCommandEvent &event){ 
  
  needtoredraw=true;
  
  m_canvas->Refresh();

}


void ResidualImageDialog::ChangeZoom(wxCommandEvent &event){ 
 
 m_canvas->Refresh();
 
}


void ResidualImageDialog::SaveCurrentImage(wxCommandEvent &event){ 

  wxString defaultname = whichchip->GetValue();


  defaultname.Replace(_T(".cel"),_T(""));

  if (whichtype->GetStringSelection().Cmp(_T("Positive")) == 0){
    defaultname += wxT("_resid_pos");
  } else if (whichtype->GetStringSelection().Cmp(_T("Negative")) == 0){
    defaultname += wxT("_resid_neg");
  } else if (whichtype->GetStringSelection().Cmp(_T("Negative")) == 0){
    defaultname += wxT("_resid_sign");
  } else {
    defaultname += wxT("_resid");
  }

  defaultname+= wxT(".jpg");


  wxString savefilename = wxFileSelector( wxT("Save Image"),
					  wxT(""),
					  defaultname,
					  wxT(".jpg"),
					  wxT("PNG files (*.png)|*.png|JPEG files (*.jpg)|*.jpg")
					  ,
					  wxFD_SAVE);

  if ( savefilename.empty() )
    return;


  
  if (wxFileExists(savefilename)){      
    wxString t=savefilename + _T(" already exists. Overwrite?"); 
      wxMessageDialog
	aboutDialog
	(0, t, _T("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	return;
      }
  }

  long horizontalsize = (m_canvas->GiveMyResids()->ncols())+ 40;
  long verticalsize = m_canvas->GiveMyResids()->nrows() + 40;

  wxBitmap bitmap( horizontalsize,verticalsize );
  wxMemoryDC memdc;
  memdc.SelectObject( bitmap );
  
  PrepareDC( memdc );
  memdc.Clear();
  drawPseudoChipImage(&memdc,whichchip->GetValue(),whichtype->GetStringSelection(),m_canvas->GiveMyResids());

  wxImage image = bitmap.ConvertToImage();
  image.SaveFile(savefilename); //,wxBITMAP_TYPE_PNG);
}


void ResidualImageDialog::SaveCurrentImageAll(wxCommandEvent &event){ 

  wxString defaultname = whichchip->GetValue();


  defaultname.Replace(_T(".cel"),_T(""));

  defaultname+= wxT(".jpg");

  wxString savefilename = wxFileSelector( wxT("Save Image"),
					  wxT(""),
					  defaultname,
					   wxT(".jpg"),
					  wxT("PNG files (*.png)|*.png|JPEG files (*.jpg)|*.jpg")
					  ,
					  wxFD_SAVE);

  if ( savefilename.empty() )
    return;

  wxString extension = savefilename.AfterLast('.').Lower();

  long horizontalsize = (m_canvas->GiveMyResids()->ncols())+ 40;
  long verticalsize = m_canvas->GiveMyResids()->nrows() + 40;
  
  wxBitmap bitmap( horizontalsize,verticalsize );
  wxMemoryDC memdc;
  memdc.SelectObject( bitmap );
  
  PrepareDC( memdc );
  memdc.Clear();
  drawPseudoChipImage(&memdc,whichchip->GetValue(),_T("Both"),m_canvas->GiveMyResids());

  
  defaultname = savefilename.BeforeLast('.');
  savefilename = defaultname;
  savefilename+= wxT("_resid.");
  savefilename+= extension;

  
  if (wxFileExists(savefilename)){      
    wxString t=savefilename + _T(" already exists. Overwrite?"); 
      wxMessageDialog
	aboutDialog
	(0, t, _T("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	return;
      }
  }

  wxImage image = bitmap.ConvertToImage();
  image.SaveFile(savefilename); //,wxBITMAP_TYPE_PNG);

   
  PrepareDC( memdc );
  memdc.Clear();
  drawPseudoChipImage(&memdc,whichchip->GetValue(),_T("Positive"),m_canvas->GiveMyResids());

  savefilename = defaultname;
  savefilename += wxT("_resid_pos.");
  savefilename+= extension;
  
  if (wxFileExists(savefilename)){      
    wxString t=savefilename + _T(" already exists. Overwrite?"); 
      wxMessageDialog
	aboutDialog
	(0, t, _T("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	return;
      }
  }

  image = bitmap.ConvertToImage();
  image.SaveFile(savefilename); //,wxBITMAP_TYPE_PNG);


  PrepareDC( memdc );
  memdc.Clear();
  drawPseudoChipImage(&memdc,whichchip->GetValue(),_T("Negative"),m_canvas->GiveMyResids());

  savefilename = defaultname;
  savefilename += wxT("_resid_neg.");
  savefilename+= extension;

  if (wxFileExists(savefilename)){      
    wxString t=savefilename + _T(" already exists. Overwrite?"); 
      wxMessageDialog
	aboutDialog
	(0, t, _T("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	return;
      }
  }
  
  image = bitmap.ConvertToImage();
  image.SaveFile(savefilename); //,wxBITMAP_TYPE_PNG);

  
  PrepareDC( memdc );
  memdc.Clear();
  drawPseudoChipImage(&memdc,whichchip->GetValue(),_T("Sign"),m_canvas->GiveMyResids());

  savefilename = defaultname;
  savefilename += wxT("_resid_sign.");
  savefilename+= extension;

  if (wxFileExists(savefilename)){      
    wxString t=savefilename + _T(" already exists. Overwrite?"); 
      wxMessageDialog
	aboutDialog
	(0, t, _T("Overwrite?"), wxYES_NO);
      
      if (aboutDialog.ShowModal() == wxID_NO){
	return;
      }
  }
  
  image = bitmap.ConvertToImage();
  image.SaveFile(savefilename); //,wxBITMAP_TYPE_PNG);
  
}



void ResidualImageDialog::SaveAllImages(wxCommandEvent &event){ 

  
  wxString defaultname;
  wxString currentname;
  wxString savefilename;


  int numberchips=whichchip->GetCount();


  const wxString& dir = wxDirSelector(_T("Choose an output directory"));
  if (dir.empty())
    {
      return;
    }

  wxProgressDialog ImageProgress(_T("Image writing Progress"),_T("Writing Images"),numberchips,this,wxPD_AUTO_HIDE);


  
  for (int i=0; i < numberchips; i++){

    currentname = whichchip->GetString(i);
    ImageProgress.Update(i,currentname);

    wxFileName currentPath(dir,currentname); 
    defaultname = currentPath.GetFullPath();
    defaultname.Replace(_T(".cel"),_T(""));
    
    /*    wxString savefilename = wxFileSelector( wxT("Save Image"),
					    wxT(""),
					    defaultname,
					    (const wxChar *)NULL,
					    wxT("PNG files (*.png)|*.png")
					    wxT("JPEG files (*.jpg)|*.jpg")
					    ,
					    wxSAVE);
    
    if ( savefilename.empty() )
    return; */
    
    wxString extension = wxT("jpg"); //savefilename.AfterLast('.').Lower();
    
    long horizontalsize = (m_canvas->GiveMyResids()->ncols())+ 40;
    long verticalsize = m_canvas->GiveMyResids()->nrows() + 40;
    
    wxBitmap bitmap( horizontalsize,verticalsize );
    wxMemoryDC memdc;
    memdc.SelectObject( bitmap );
    
    PrepareDC( memdc );
    memdc.Clear();
    drawPseudoChipImage(&memdc,currentname,_T("Both"),m_canvas->GiveMyResids());
    
    
    // defaultname = savefilename.BeforeLast('.');
    savefilename = defaultname;
    savefilename+= wxT("_resid.");
    savefilename+= extension;
    
    wxImage image = bitmap.ConvertToImage();
    image.SaveFile(savefilename); //,wxBITMAP_TYPE_PNG);
    
    
    PrepareDC( memdc );
    memdc.Clear();
    drawPseudoChipImage(&memdc,currentname,_T("Positive"),m_canvas->GiveMyResids());
    
    savefilename = defaultname;
    savefilename += wxT("_resid_pos.");
    savefilename+= extension;
    
    image = bitmap.ConvertToImage();
    image.SaveFile(savefilename); //,wxBITMAP_TYPE_PNG);
    
    
    PrepareDC( memdc );
    memdc.Clear();
    drawPseudoChipImage(&memdc,currentname,_T("Negative"),m_canvas->GiveMyResids());
    
    savefilename = defaultname;
    savefilename += wxT("_resid_neg.");
    savefilename+= extension;
    
    image = bitmap.ConvertToImage();
    image.SaveFile(savefilename); //,wxBITMAP_TYPE_PNG); 
    
    PrepareDC( memdc );
    memdc.Clear();
    drawPseudoChipImage(&memdc,currentname,_T("Sign"),m_canvas->GiveMyResids());
    
    savefilename = defaultname;
    savefilename += wxT("_resid_sign.");
    savefilename+= extension;
    
    image = bitmap.ConvertToImage();
    image.SaveFile(savefilename); //,wxBITMAP_TYPE_PNG);
  }

}



ResidualImageDialog::ResidualImageDialog(wxWindow* parent,ResidualsDataGroup *resids,
   wxWindowID id,
  const wxString &title,
  const wxPoint &position,
  const wxSize& size,
  long style
  ):
  wxDialog( parent, id, title, position, size, style)
{                                                     
  
  m_canvas = new MyCanvas(this,resids);

  wxBoxSizer *item0 = new wxBoxSizer( wxVERTICAL ); 
  wxBoxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );
  
  item1->Add(m_canvas, 1, wxEXPAND | wxGROW | wxALL , 5 );

  wxBoxSizer *sidebar = new wxBoxSizer( wxVERTICAL ); 

  wxBoxSizer *buttonbar = new wxBoxSizer( wxHORIZONTAL ); 
  wxButton *previmage =  new wxButton(this, ID_PREV_IMAGE,wxT("Prev"),wxDefaultPosition, wxDefaultSize, 0 );
  wxButton *nextimage =  new wxButton(this, ID_NEXT_IMAGE,wxT("Next"),wxDefaultPosition, wxDefaultSize, 0 );
  buttonbar->Add(previmage, 0, wxALIGN_CENTER | wxALL, 10 );
  buttonbar->Add(nextimage, 0, wxALIGN_CENTER | wxALL, 10 );
  sidebar->Add(buttonbar, 0, wxALIGN_CENTER | wxALL, 10 );


  wxString *strs3 = (wxString*) NULL;
  whichchip = new wxComboBox(this, ID_WHICHCHIP, wxT(""), wxDefaultPosition, wxDefaultSize, 0, strs3, wxCB_DROPDOWN|wxCB_READONLY );
  sidebar->Add( whichchip, 0, wxALIGN_CENTER | wxALL, 10 );

  wxString whichtypetitle = _T("Residual type");
  wxString whichtypechoices[] =
    {
      wxT("Both"),
      wxT("Positive"),
      wxT("Negative"),
      wxT("Sign")
    };

  whichtype = new wxRadioBox(this, 
			    ID_WHICHTYPE, 
			    whichtypetitle, 
			    wxDefaultPosition,
			    wxDefaultSize,
			    4,
			    whichtypechoices,
			    1,
			    wxHORIZONTAL,
			    wxDefaultValidator,
			    wxRadioBoxNameStr);

  sidebar->Add(whichtype, 0, wxALIGN_CENTER|wxALL, 10 );
  
  wxString whichzoomtitle = _T("Zoom level");
  wxString whichzoomchoices[] =
    {
      wxT("10%"),
      wxT("25%"),
      wxT("50%"),
      wxT("100%"),
      wxT("200%")
    };  

  whichzoom = new wxRadioBox(this, 
			    ID_WHICHZOOM, 
			    whichzoomtitle, 
			    wxDefaultPosition,
			    wxDefaultSize,
			    5,
			    whichzoomchoices,
			    1,
			    wxHORIZONTAL,
			    wxDefaultValidator,
			    wxRadioBoxNameStr);
			    
  whichzoom->SetSelection(3);
  
  sidebar->Add(whichzoom, 0, wxALIGN_CENTER|wxALL, 10 );
  
  
  wxButton *thisimage = new wxButton(this,ID_SAVETHISIMAGE,wxT("Save this image"), wxDefaultPosition, wxDefaultSize, 0 );
  wxButton *allthreeimages = new wxButton(this,ID_SAVETHISCHIP,wxT("Save all images for this chip"),wxDefaultPosition, wxDefaultSize, 0 );
  wxButton *allimages = new wxButton(this,ID_SAVEALLIMAGE,wxT("Save all images"),wxDefaultPosition, wxDefaultSize, 0 );

  sidebar->Add(thisimage, 0, wxALIGN_CENTER|wxALL, 10 );
  sidebar->Add(allthreeimages, 0, wxALIGN_CENTER|wxALL, 10 );
  sidebar->Add(allimages,0,wxALIGN_CENTER|wxALL, 10 );


  
  item1->Add( sidebar, 0, wxALIGN_CENTER | wxALL, 5 );
  

  item0->Add( item1, 1, wxEXPAND | wxALIGN_CENTER|wxALL, 5 );
  
  wxButton *item4 = new wxButton(this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
  item0->Add( item4, 0, wxALIGN_CENTER|wxALL, 5 );
  
  this->SetAutoLayout( TRUE );
  this->SetSizer( item0 );
  
  item0->Fit( this );
  item0->SetSizeHints( this );
  
   
}
