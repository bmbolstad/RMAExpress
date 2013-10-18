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
 ** File: boxplot.cpp
 **
 ** Copyright (C) B. M. Bolstad 2006
 **
 ** implements drawing of boxplots
 **
 ** History - Created Sept 28, 2006
 **
 ** Feb 1, 2007 - make median line thicker.
 **
 **************************************************************************/

#include <wx/colour.h>

#include "boxplot.h"



void Draw_Single_Boxplot(wxDC &dc,
			 double *five_num_summary,
			 int ylim_min_int,
			 int ylim_max_int,
			 double ylim_min_double,
			 double ylim_max_double,
			 int x,
			 int boxwidth,
			 const wxColor *fillcolor){

  
  double scalefactor;

  int boxheight;
  int boxbottom;
  int boxmiddle;
  int boxtop;

  int low_whisker_start;
  int high_whisker_start;


  // Set the box line color to black and the fill color to white
  dc.SetPen(wxPen(*wxBLACK,1,wxSOLID));
  dc.SetBrush(wxBrush(*fillcolor));

  // Calculate where things should be drawn  
  scalefactor = (ylim_max_int - ylim_min_int)/(ylim_max_double - ylim_min_double);
  
  boxheight = (int)(scalefactor*(five_num_summary[3] - five_num_summary[1]));
  boxbottom = (int)(scalefactor*(five_num_summary[1] - ylim_min_double)); 
  boxmiddle = (int)(scalefactor*(five_num_summary[2] - ylim_min_double)); 
  boxtop = (int)(scalefactor*(five_num_summary[3] - ylim_min_double)); 

  low_whisker_start = (int)(scalefactor*(five_num_summary[0] - ylim_min_double));
  high_whisker_start = (int)(scalefactor*(five_num_summary[4] - ylim_min_double));


  // Draw the box.
  dc.DrawRectangle(x-boxwidth/2,boxbottom,boxwidth,boxheight);
  

  // Now a line segment for the median
  dc.DrawLine(x-boxwidth/2,boxmiddle   , x+boxwidth/2-1, boxmiddle);

  // now the "whiskers"

  dc.DrawLine(x,boxbottom,x,low_whisker_start);
  dc.DrawLine(x,boxtop,x,high_whisker_start);
  
}



void Draw_Single_Boxplot(wxDC &dc,
			 int y_min,
			 int y_LQ,
			 int y_median,
			 int y_UQ,
			 int y_max,
			 int x,
			 int boxwidth,
			 const wxColor *fillcolor){





  dc.SetPen(wxPen(*wxBLACK,1,wxSOLID));
  dc.SetBrush(wxBrush(*fillcolor));

  // draw the box

  dc.DrawRectangle(x-boxwidth/2,y_LQ,boxwidth,y_UQ-y_LQ);
  
  
  // Now a line segment for the median
  dc.DrawLine(x-boxwidth/2,y_median,x+boxwidth/2, y_median);

  // now the "whiskers"

  dc.DrawLine(x,y_LQ,x,y_min);
  dc.DrawLine(x,y_UQ,x,y_max); 
  
  // Now a line segment for the median
  dc.SetPen(wxPen(*wxBLACK,3,wxSOLID));

  dc.DrawLine(x-boxwidth/2,y_median,x+boxwidth/2, y_median);
  dc.SetPen(wxPen(*wxBLACK,1,wxSOLID));
 
}
