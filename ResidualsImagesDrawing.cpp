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
 ** File: ResidualsImagesDrawing.cpp
 **
 ** Copyright (C) B. M. Bolstad 2004-2008
 **
 ** Aim: Separates out the drawing components from the dialog componenets.
 **
 ** Code in this file was originally in residualsimages.cpp
 **
 ** History
 ** Sep 30, 2005 - created as individual file
 ** Oct 1, 2005 - added functionality for drawing to an wxImage for the console application
 **             - change some preprocessor directives
 **
 ** History of residualsimages.cpp - Created Jan 20, 2004
 ** Feb 28, 2004 - added radiobox for selecting, both, positive or negative residuals
 **               and altered drawing routine to draw them.
 **
 ** Feb 29, 2004 - fixed save buttons.
 **
 ** Jul 11, 2005 - added sign of residuals, fixed colors which were around wrong way (why no one noticed until now is somewhat of a mystery).m
 **
 ** Oct 29, 2005 - make sure that empty regions on signs images are white now.
 **
 ** Jan 30, 2006 - Add additional functionality for only redrawing part of the image
 ** Sep 16, 2006 - fix compile problems on unicode builds of wxWidgets
 ** Feb 28, 2008 - BufferedMatrix indexing is now via() operator rather than []
 **
 ***************************************************************************/


#include "ResidualsImagesDrawing.h"
#include "ResidualsDataGroup.h"

#include <wx/image.h>

char getBlue(double x){

  if (x > 2.0){
    return 0;
  }  else if (x <= 0.05){
    return 255;
  } else {
    return (char)(255 - x/2.5*255);
  }
}


char getRed(double x){

  if (x < -2.0){
    return 0;
  } else if (x >= -0.05){
    return 255;
  } else {
    return (char)(255 - x/-2.5*255);
  }
}


char getGreen(double x){

  if (x > 0){
    return getBlue(x);
  } else {
    return getRed(x);
  }
}


#if RMA_GUI_APP

void drawPseudoChipImage(wxDC *dc,wxString name, wxString type, ResidualsDataGroup *resids){

#ifndef BUFFERED
  Matrix *thedata;
#else
  BufferedMatrix *thedata;
#endif
  char red,green,blue;
  int whichchip;

  //cout << name << endl;

  if (name.Cmp(_T(""))==0){
    return;
  }




  
  dc->DrawText( name, 20, 5);
  //dc->SetBrush( wxBrush( wxT("orange"), wxSOLID ) );
  dc->SetPen( *wxBLACK_PEN );
  //dc->DrawRectangle( 150, 30, 100, 100 );
  //dc->SetBrush( *wxWHITE_BRUSH );
  //dc->DrawRectangle( 170, 50, 60, 60 );
  
  dc->DrawRectangle(19,19,resids->ncols()+2,resids->nrows()+2);
  thedata = resids->GetIntensities();
  

  whichchip = (resids->GetArrayNames()).Index(name);
  

  //whichchip = whichchip*(resids->ncols()*resids->nrows());

  
  // Note the flip for orientation purposes
  
  if (type.Cmp(_T("Positive")) == 0){
    for (int i =0; i < resids->nrows(); i++){
      for (int j=0; j < resids->ncols(); j++){  
	if ((*thedata)(i*resids->ncols() + j,whichchip) > 0){            //[whichchip + i*resids->ncols() + j] > 0){
	  red = getRed((*thedata)(i*resids->ncols() + j,whichchip));
	  blue = getBlue((*thedata)(i*resids->ncols() + j,whichchip));
	  green =getGreen((*thedata)(i*resids->ncols() + j,whichchip));
	} else {
	  red = 255;
	  blue = 255;
	  green = 255;
	} 
	dc->SetPen(wxPen(wxColor(red,green,blue) ,1,wxSOLID));
	dc->DrawPoint(20+j,20+i);
      }
    }
  } else if (type.Cmp(_T("Negative")) ==0){
    for (int i =0; i < resids->nrows(); i++){
      for (int j=0; j < resids->ncols(); j++){  
	if ((*thedata)(i*resids->ncols() + j,whichchip) < 0){
	  red = getRed((*thedata)(i*resids->ncols() + j,whichchip));
	  blue = getBlue((*thedata)(i*resids->ncols() + j,whichchip));
	  green = getGreen((*thedata)(i*resids->ncols() + j,whichchip));
	} else {
	  red = 255;
	  blue = 255;
	  green = 255;
	}
	dc->SetPen(wxPen(wxColor(red,green,blue) ,1,wxSOLID));
	dc->DrawPoint(20+j,20+i);
      }
    }
  } else if (type.Cmp(_T("Sign")) ==0){
    for (int i =0; i < resids->nrows(); i++){
      for (int j=0; j < resids->ncols(); j++){  
	if ((*thedata)(i*resids->ncols() + j,whichchip) > 0){
	  red = 255;
	  blue = 0;
	  green = 0;
	} else if ((*thedata)(i*resids->ncols() + j,whichchip) < 0) {
	  red = 0;
	  blue = 255;
	  green = 0;
	} else {
	  red = 255;
	  blue = 255;
	  green = 255;
	}
	dc->SetPen(wxPen(wxColor(red,green,blue) ,1,wxSOLID));
	dc->DrawPoint(20+j,20+i);
      }
    }
  } else {
    for (int i =0; i < resids->nrows(); i++){
      for (int j=0; j < resids->ncols(); j++){  
	red = getRed((*thedata)(i*resids->ncols() + j,whichchip));
	blue = getBlue((*thedata)(i*resids->ncols() + j,whichchip));
	green = getGreen((*thedata)(i*resids->ncols() + j,whichchip));
	dc->SetPen(wxPen(wxColor(red,green,blue) ,1,wxSOLID));
	dc->DrawPoint(20+j,20+i);
      }
    }
  }

}

// Draws only a partial region (for redrawing only what should be visible)

void drawPseudoChipImage(wxDC *dc,wxString name, wxString type, ResidualsDataGroup *resids,int startx, int starty, int width, int height){


  
#ifndef BUFFERED
  Matrix *thedata;
#else
  BufferedMatrix *thedata;
#endif
  char red,green,blue;
  int whichchip;
  int wherestartx, wherestarty;
  int wherestopx, wherestopy;


  //cout << name << endl;
  
  if (name.Cmp(_T(""))==0){
    return;
  }

  
   
  // will title and rebox every time (even if not visible)
 
  dc->DrawText( name, 20, 5);
  //dc->SetBrush( wxBrush( wxT("orange"), wxSOLID ) );
  dc->SetPen( *wxBLACK_PEN );
  //dc->DrawRectangle( 150, 30, 100, 100 );
  //dc->SetBrush( *wxWHITE_BRUSH );
  //dc->DrawRectangle( 170, 50, 60, 60 );
  
  dc->DrawRectangle(19,19,resids->ncols()+2,resids->nrows()+2);

  thedata = resids->GetIntensities();
  

  // Check which chip we are drawing residuals image for
  whichchip = (resids->GetArrayNames()).Index(name);
  

  //  whichchip = whichchip*(resids->ncols()*resids->nrows());

  if (startx < 20){
    wherestartx = 0;
  } else {
    wherestartx = startx-20;
  }

  
  if (starty < 20){
    wherestarty = 0;
  } else {
    wherestarty = starty-20;
  }


  if (startx+width > resids->nrows()){
    wherestopx = resids->nrows(); 
  } else {
    wherestopx =  startx+width-20;
  }

  if (starty+width > resids->nrows()){
    wherestopy = resids->ncols(); 
  } else {
    wherestopy =  starty+width-20;
  }


  // Note the flip for orientation purposes
  for (int i =(wherestartx); i < wherestopx; i++){
    for (int j=(wherestarty); j < wherestopy; j++){  
      if (type.Cmp(_T("Positive")) == 0){
	if ((*thedata)(i*resids->ncols() + j,whichchip) > 0){
	  red = getRed((*thedata)(i*resids->ncols() + j,whichchip));
	  blue = getBlue((*thedata)(i*resids->ncols() + j,whichchip));
	  green =getGreen((*thedata)(i*resids->ncols() + j,whichchip));
	} else {
	  red = 255;
	  blue = 255;
	  green = 255;
	}
      } else if (type.Cmp(_T("Negative")) ==0){
	if ((*thedata)(i*resids->ncols() + j,whichchip) < 0){
	  red = getRed((*thedata)(i*resids->ncols() + j,whichchip));
	  blue = getBlue((*thedata)(i*resids->ncols() + j,whichchip));
	  green = getGreen((*thedata)(i*resids->ncols() + j,whichchip));
	} else {
	  red = 255;
	  blue = 255;
	  green = 255;
	}
      } else if (type.Cmp(_T("Sign")) ==0){
	if ((*thedata)(i*resids->ncols() + j,whichchip) > 0){
	  red = 255;
	  blue = 0;
	  green = 0;
	} else if ((*thedata)(i*resids->ncols() + j,whichchip) < 0){
	  red = 0;
	  blue = 255;
	  green = 0;
	} else {
	  red = 255;
	  blue = 255;
	  green = 255;
	}
      } else {
	red = getRed((*thedata)(i*resids->ncols() + j,whichchip));
	blue = getBlue((*thedata)(i*resids->ncols() + j,whichchip));
	green = getGreen((*thedata)(i*resids->ncols() + j,whichchip));
      }
      dc->SetPen(wxPen(wxColor(red,green,blue) ,1,wxSOLID));
      dc->DrawPoint(20+j,20+i);
    }
  }

  
  

}



#else

#include <wx/image.h>
void drawPseudoChipImage( wxImage *Image, wxString name, wxString type, ResidualsDataGroup *resids){
#ifndef BUFFERED
  Matrix *thedata;
#else
  BufferedMatrix *thedata;
#endif
  int whichchip;
  char red,green,blue;
  int offset = 20;    // note the 20/40 in here is a hard coded constant

  wxImage subimage;

  unsigned char *imagedata;
  
  whichchip = (resids->GetArrayNames()).Index(name);
  //  whichchip = whichchip*(resids->ncols()*resids->nrows());

  thedata = resids->GetIntensities();



  
  imagedata =  Image->GetData();


  // Initial band of white
  for (int i =0; i < 3*20 + (20-1)*3*((resids->ncols())+40) - 3; i++){
    imagedata[i] = 255;
  }

  // short strip after top line but before text
  offset=3*20 + 20*3*(resids->ncols()+40);

  for (int i =offset-4; i >= offset-3*39; i--){
    imagedata[i] = 255;
  }









  // Note the flip for orientation purposes
  for (int i =0; i < resids->nrows(); i++){
    for (int j=0; j < resids->ncols(); j++){  
      if (type.Cmp(_T("Positive")) == 0){
	if ((*thedata)(i*resids->ncols() + j,whichchip) > 0){
	  red = getRed((*thedata)(i*resids->ncols() + j,whichchip));
	  blue = getBlue((*thedata)(i*resids->ncols() + j,whichchip));
	  green =getGreen((*thedata)(i*resids->ncols() + j,whichchip));
	} else {
	  red = 255;
	  blue = 255;
	  green = 255;
	}
      } else if (type.Cmp(_T("Negative")) ==0){
	if ((*thedata)(i*resids->ncols() + j,whichchip) < 0){
	  red = getRed((*thedata)(i*resids->ncols() + j,whichchip));
	  blue = getBlue((*thedata)(i*resids->ncols() + j,whichchip));
	  green = getGreen((*thedata)(i*resids->ncols() + j,whichchip));
	} else {
	  red = 255;
	  blue = 255;
	  green = 255;
	}
      } else if (type.Cmp(_T("Sign")) ==0){
	if ((*thedata)(i*resids->ncols() + j,whichchip) > 0){
	  red = 255;
	  blue = 0;
	  green = 0;
	} else if ((*thedata)(i*resids->ncols() + j,whichchip) < 0){
	  red = 0;
	  blue = 255;
	  green = 0;
	} else {
	  red = 255;
	  blue = 255;
	  green = 255;
	}
      } else {
	red = getRed((*thedata)(i*resids->ncols() + j,whichchip));
	blue = getBlue((*thedata)(i*resids->ncols() + j,whichchip));
	green = getGreen((*thedata)(i*resids->ncols() + j,whichchip));
      }
      //  dc->SetPen(wxPen(wxColor(red,green,blue) ,1,wxSOLID));
      //dc->DrawPoint(20+j,20+i);

      imagedata[offset] = red;
      imagedata[offset + 1] = green;
      imagedata[offset + 2] = blue;
      /*     imagedata[i*(3*resids->ncols()) + 3*j ] = red;
      imagedata[i*(3*resids->ncols()) + 3*j + 1 ] = green;
      imagedata[i*(3*resids->ncols()) + 3*j + 2 ] = blue; */
      offset+=3;

    }
    // leave jsut enough room for borders
    for (int k=offset+3; k <  offset+ 3*40-3;k++){
      imagedata[k] = 255;
    }

    offset+= 3*40;
  }


  // Empty white area at end of image

  offset+=3*((resids->ncols()));
  for (int i =offset+3; i < offset+3*20 + (20-1)*3*((resids->ncols())+40); i++){
    imagedata[i] = 255;
  }






}



#endif
