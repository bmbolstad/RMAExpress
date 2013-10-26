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
 ** File: axes.cpp
 **
 ** Copyright (C) B. M. Bolstad 2006-2007
 **
 ** implements drawing of plot axes
 **
 ** History - Created Sept 29, 2006
 ** Sept 30 - create an plotAxes class
 ** Jan 7, 2007 - add perpendicular labels to LabeledAxes class
 ** Feb 3, 2007 - Play with caption/title/label placement
 **
 **************************************************************************/

#include <wx/dc.h>
#include <wx/colour.h>

#include "axes.h"





plotAxes::plotAxes(wxDC &dc, wxPoint origin_location,
		   wxPoint axes_length,
		   wxRealPoint min_point,
		   wxRealPoint max_point,
		   bool boxformat,
		   wxPoint axespadding,
		   int pen_width,
		   wxColor axis_color,
		   wxBrush background_color,
		   bool left_to_right,
		   bool bottom_to_top){

  int w,h;
	
  dc.GetSize(&w, &h);
	
  m_w = w;
  m_h = h;

  m_origin_location = origin_location;
  m_axes_length = axes_length;

  m_min_point = min_point;
  m_max_point = max_point;

  m_box_format = boxformat;
  
  m_pen_width = pen_width;
  m_axis_color = axis_color;

  m_axespadding = axespadding;

  m_background_color = background_color;

  m_left_to_right = left_to_right;
  m_bottom_to_top = bottom_to_top;
	
}






void plotAxes::activateBoxFormat(){
  m_box_format = true;
}

void plotAxes::deactivateBoxFormat(){
  m_box_format = false;
}


wxRect plotAxes::getDrawingRegion(){
	
	int w=m_w,h=m_h;
	
	
	if (m_left_to_right && !m_bottom_to_top){
		return wxRect(m_origin_location.x, m_origin_location.y, m_axes_length.x, m_axes_length.y);
	} else if (m_left_to_right && m_bottom_to_top){
		return wxRect(m_origin_location.x,h-m_origin_location.y, m_axes_length.x, -m_axes_length.y);
	} else if (!m_left_to_right && m_bottom_to_top){
		return wxRect(w-m_origin_location.x,h-m_origin_location.y, -m_axes_length.x, -m_axes_length.y);
	} else if (!m_left_to_right && !m_bottom_to_top){
		return wxRect(w-m_origin_location.x,m_origin_location.y, -m_axes_length.x, m_axes_length.y);
	}
	
	

}


wxPoint plotAxes::FindPoint(const wxRealPoint &point){

	int w=m_w,h=m_h;


   
  
  double scalefactor_x = (m_axes_length.x -2*m_axespadding.x)/(m_max_point.x - m_min_point.x);
  double scalefactor_y = (m_axes_length.y -2*m_axespadding.y)/(m_max_point.y - m_min_point.y);


  double relativeorigin_x = scalefactor_x*(point.x - m_min_point.x);
  double relativeorigin_y = scalefactor_y*(point.y - m_min_point.y);

  if (m_left_to_right && !m_bottom_to_top){
	  int point_x = (int)relativeorigin_x + m_origin_location.x + m_axespadding.x;
	  int point_y = (int)relativeorigin_y + m_origin_location.y + m_axespadding.y;
    
	  return wxPoint(point_x,point_y);
  } else if (m_left_to_right && m_bottom_to_top){
	  int point_x = (int)relativeorigin_x + m_origin_location.x + m_axespadding.x;
	  int point_y = h-(m_origin_location.y + m_axespadding.y) - (int)relativeorigin_y;
	  
	  return wxPoint(point_x,point_y);
  } else if (!m_left_to_right && m_bottom_to_top){
	  int point_x = w - (m_origin_location.x + m_axespadding.x) - (int)relativeorigin_x;
	  int point_y = h-(m_origin_location.y + m_axespadding.y) - (int)relativeorigin_y;
	  
	  return wxPoint(point_x,point_y);
  } else if (!m_left_to_right && !m_bottom_to_top){
	  int point_x = w - (m_origin_location.x + m_axespadding.x) - (int)relativeorigin_x;
	  int point_y = (int)relativeorigin_y + m_origin_location.y + m_axespadding.y;
     
	  return wxPoint(point_x,point_y);
  }   
}



void plotAxes::Draw(wxDC &dc){

	
  int w=m_w,h=m_h;
	


  dc.SetPen(wxPen(m_axis_color,m_pen_width,wxSOLID));
  dc.SetBrush(wxBrush(*wxWHITE));
  
	if (m_left_to_right && !m_bottom_to_top){
		dc.DrawLine(m_origin_location.x, m_origin_location.y, m_origin_location.x, m_origin_location.y + m_axes_length.y);  // + 2*m_axespadding.y);
		dc.DrawLine(m_origin_location.x, m_origin_location.y, m_origin_location.x + m_axes_length.x, m_origin_location.y);  //     + 2*m_axespadding.x, m_origin_location.y);
      
		// Box format axes
		if (m_box_format){
			dc.DrawLine(m_origin_location.x + m_axes_length.x, // + 2*m_axespadding.x, 
						m_origin_location.y, 
						m_origin_location.x + m_axes_length.x,// + 2*m_axespadding.x,
						m_origin_location.y + m_axes_length.y);// + 2*m_axespadding.y);
    
			dc.DrawLine(m_origin_location.x, 
						m_origin_location.y + m_axes_length.y,// + 2*m_axespadding.y, 
						m_origin_location.x + m_axes_length.x,// + 2*m_axespadding.x, 
						m_origin_location.y + m_axes_length.y);// + 2*m_axespadding.y);
		}
	} else if (m_left_to_right && m_bottom_to_top){
		dc.DrawLine(m_origin_location.x, h-m_origin_location.y, m_origin_location.x, h-(m_origin_location.y + m_axes_length.y));  // + 2*m_axespadding.y);
		dc.DrawLine(m_origin_location.x, h-m_origin_location.y, m_origin_location.x + m_axes_length.x, h-m_origin_location.y);  //     + 2*m_axespadding.x, m_origin_location.y);
		
		// Box format axes
		if (m_box_format){
			dc.DrawLine(m_origin_location.x + m_axes_length.x, // + 2*m_axespadding.x, 
							h-m_origin_location.y, 
							m_origin_location.x + m_axes_length.x,// + 2*m_axespadding.x,
							h-(m_origin_location.y + m_axes_length.y));// + 2*m_axespadding.y);
				
			dc.DrawLine(m_origin_location.x, 
							h - (m_origin_location.y + m_axes_length.y),// + 2*m_axespadding.y, 
							m_origin_location.x + m_axes_length.x,// + 2*m_axespadding.x, 
							h- (m_origin_location.y + m_axes_length.y));// + 2*m_axespadding.y);
		}
		
	} else if (!m_left_to_right && m_bottom_to_top){
		
		dc.DrawLine(w-m_origin_location.x, h-m_origin_location.y, w-m_origin_location.x, h-(m_origin_location.y + m_axes_length.y));  // + 2*m_axespadding.y);
		dc.DrawLine(w-m_origin_location.x, h-m_origin_location.y, w-(m_origin_location.x + m_axes_length.x), h-m_origin_location.y);  //     + 2*m_axespadding.x, m_origin_location.y);
		
		// Box format axes
		if (m_box_format){
			dc.DrawLine(w-(m_origin_location.x + m_axes_length.x), // + 2*m_axespadding.x, 
						h-m_origin_location.y, 
						w-(m_origin_location.x + m_axes_length.x),// + 2*m_axespadding.x,
						h-(m_origin_location.y + m_axes_length.y));// + 2*m_axespadding.y);
			
			dc.DrawLine(w-m_origin_location.x, 
						h - (m_origin_location.y + m_axes_length.y),// + 2*m_axespadding.y, 
						w-(m_origin_location.x + m_axes_length.x),// + 2*m_axespadding.x, 
						h- (m_origin_location.y + m_axes_length.y));// + 2*m_axespadding.y);
		}		
	} else if (!m_left_to_right && !m_bottom_to_top){
		
		dc.DrawLine(w-m_origin_location.x, m_origin_location.y, w-m_origin_location.x, (m_origin_location.y + m_axes_length.y));  // + 2*m_axespadding.y);
		dc.DrawLine(w-m_origin_location.x, m_origin_location.y, w-(m_origin_location.x + m_axes_length.x), m_origin_location.y);  //     + 2*m_axespadding.x, m_origin_location.y);
		
		// Box format axes
		if (m_box_format){
			dc.DrawLine(w-(m_origin_location.x + m_axes_length.x), // + 2*m_axespadding.x, 
						m_origin_location.y, 
						w-(m_origin_location.x + m_axes_length.x),// + 2*m_axespadding.x,
						(m_origin_location.y + m_axes_length.y));// + 2*m_axespadding.y);
			
			dc.DrawLine(w-m_origin_location.x, 
						(m_origin_location.y + m_axes_length.y),// + 2*m_axespadding.y, 
						w-(m_origin_location.x + m_axes_length.x),// + 2*m_axespadding.x, 
					    (m_origin_location.y + m_axes_length.y));// + 2*m_axespadding.y);
		}		
	}
}


void plotAxes::setupDCforPlot(wxDC &dc){

  int w,h;

  dc.GetSize(&w, &h);

  wxRect plot_region = getDrawingRegion();
  	
  dc.SetClippingRegion(plot_region.x, plot_region.y, plot_region.width, plot_region.height);  
  dc.SetBackground(m_background_color); //*wxGREY_BRUSH);
  dc.Clear();
  
  //dc.SetDeviceOrigin(0,0);

 

}



void plotAxes::setBackgroundColor(wxBrush color){


  m_background_color = color;





}

labeledplotAxes::labeledplotAxes(wxDC &dc, wxPoint origin_location,
				 wxPoint axes_length,
				 wxRealPoint min_point,
				 wxRealPoint max_point,
				 bool boxformat,
				 wxPoint axespadding,
				 bool draw_ticks,
				 int tick_length,
				 wxFont axis_label_font,
				 wxFont tick_label_font,
				 wxFont title_font,
				 bool tick_labels_perpendicular):plotAxes(dc, origin_location,
							       axes_length,
							       min_point,
							       max_point,
							       boxformat,
							       axespadding){


  
  m_grid_add = false;
  m_title = wxT("Dummy Title");
  m_draw_ticks = draw_ticks;
  m_tick_length = tick_length;
  m_axis_label_font = axis_label_font;
  m_tick_label_font = tick_label_font;
  m_title_font =  title_font;

  m_tick_labels_perpendicular = tick_labels_perpendicular;
}

void labeledplotAxes::addGrid(double grid_spacing_x, double grid_spacing_y,wxColor linecolor){

  m_grid_add = true;
  

  m_grid_spacing_x = grid_spacing_x;
  m_grid_spacing_y = grid_spacing_y;

  m_grid_color_x = linecolor;
  m_grid_color_y = linecolor;


}

void labeledplotAxes::Draw(wxDC &dc){

  double cur_loc;
  wxPoint cur_point;
  int cur_tick;

  int text_width, text_height;
	
  int w=m_w,h=m_h;
	
	
	
	
  plotAxes::Draw(dc);
  
  	
  if (m_grid_add){
	if(m_left_to_right && !m_bottom_to_top){
		cur_loc = m_min_point.x; 
		dc.SetPen(wxPen(m_grid_color_x,1,wxSOLID));
		while (cur_loc <= m_max_point.x){
			cur_point =  FindPoint(wxRealPoint(cur_loc,m_min_point.y));
			dc.DrawLine(cur_point.x,m_origin_location.y,cur_point.x,m_origin_location.y+m_axes_length.y); //+2*m_axespadding.y);
			cur_loc += m_grid_spacing_x; 
		}

		cur_loc = m_min_point.y; 
		dc.SetPen(wxPen(m_grid_color_y,1,wxSOLID));
		while (cur_loc <= m_max_point.y){
			cur_point =  FindPoint(wxRealPoint(m_min_point.x, cur_loc));
			dc.DrawLine(m_origin_location.x,cur_point.y,m_origin_location.x+m_axes_length.x,cur_point.y); ///+2*m_axespadding.x,cur_point.y);
			cur_loc += m_grid_spacing_y; 
		}

	} else if (m_left_to_right && m_bottom_to_top){
		cur_loc = m_min_point.x; 
		dc.SetPen(wxPen(m_grid_color_x,1,wxSOLID));
		while (cur_loc <= m_max_point.x){
			cur_point =  FindPoint(wxRealPoint(cur_loc,m_min_point.y));
			dc.DrawLine(cur_point.x,h-m_origin_location.y,cur_point.x,h-(m_origin_location.y+m_axes_length.y)); //+2*m_axespadding.y);
			cur_loc += m_grid_spacing_x; 
		}
		
		cur_loc = m_min_point.y; 
		dc.SetPen(wxPen(m_grid_color_y,1,wxSOLID));
		while (cur_loc <= m_max_point.y){
			cur_point =  FindPoint(wxRealPoint(m_min_point.x, cur_loc));
			dc.DrawLine(m_origin_location.x,cur_point.y,m_origin_location.x+m_axes_length.x,cur_point.y); ///+2*m_axespadding.x,cur_point.y);
			cur_loc += m_grid_spacing_y; 
		}
		
		
		
		
	} 
  }

  if (m_x_tick_locations.GetCount() > 0){
    if (m_draw_ticks){
      dc.SetPen(wxPen(m_axis_color,m_pen_width,wxSOLID));
      for (cur_tick =0; cur_tick < (int)m_x_tick_locations.GetCount(); cur_tick++){
		  cur_point =  FindPoint(wxRealPoint(m_x_tick_locations[cur_tick],m_min_point.y));
		  if ((m_x_tick_locations[cur_tick]<= m_max_point.x) && (m_x_tick_locations[cur_tick] >= m_min_point.x)){
			  if (!m_bottom_to_top){
				  dc.DrawLine(cur_point.x,m_origin_location.y,cur_point.x,m_origin_location.y-(int)((double)m_tick_length*(double)(dc.GetPPI().x)/(double)72));
			  } else if (m_bottom_to_top){
				  dc.DrawLine(cur_point.x,h-m_origin_location.y,cur_point.x,h-m_origin_location.y+(int)((double)m_tick_length*(double)(dc.GetPPI().x)/(double)72));

			  }
		  }
	  }
	}
  }
	

  
  if (m_y_tick_locations.GetCount() > 0){
    if (m_draw_ticks){
      dc.SetPen(wxPen(m_axis_color,m_pen_width,wxSOLID));
      for (cur_tick =0; cur_tick < (int)m_y_tick_locations.GetCount(); cur_tick++){
		  cur_point =  FindPoint(wxRealPoint(m_min_point.x,m_y_tick_locations[cur_tick]));	
		  if ((m_y_tick_locations[cur_tick]<= m_max_point.y) && (m_y_tick_locations[cur_tick] >= m_min_point.y)){
			  if (m_left_to_right){
				  dc.DrawLine(m_origin_location.x,cur_point.y,m_origin_location.x-(int)((double)m_tick_length*(double)(dc.GetPPI().x)/(double)72),cur_point.y);
			  } else if (!m_left_to_right){
				  dc.DrawLine(w-m_origin_location.x,cur_point.y,w-m_origin_location.x+(int)((double)m_tick_length*(double)(dc.GetPPI().x)/(double)72),cur_point.y);
			  }
		  }
	  }
	}
  }

  if (m_x_tick_labels.GetCount() > 0){
    wxFont x_tick_label_font = m_tick_label_font;

    x_tick_label_font.SetPointSize(x_tick_label_font.GetPointSize()*(dc.GetPPI().x)/72);
    dc.SetFont(x_tick_label_font);
    for (cur_tick =0; cur_tick < (int)m_x_tick_locations.GetCount(); cur_tick++){	
      if ((m_x_tick_locations[cur_tick]<= m_max_point.x) && (m_x_tick_locations[cur_tick] >= m_min_point.x)){
	cur_point =  FindPoint(wxRealPoint(m_x_tick_locations[cur_tick],m_min_point.y));
	dc.GetTextExtent(m_x_tick_labels[cur_tick],&text_width,&text_height);
	if (m_tick_labels_perpendicular){
		if (m_bottom_to_top){
#ifdef __WXMSW__
			dc.DrawRotatedText(m_x_tick_labels[cur_tick],cur_point.x + text_height/2,h- (m_origin_location.y-(int)((double)15*(double)(dc.GetPPI().x)/(double)72)),270.0);
#else
			dc.DrawRotatedText(m_x_tick_labels[cur_tick],cur_point.x + text_height/2,h- (m_origin_location.y-(int)((double)15*(double)(dc.GetPPI().x)/(double)72)),270.0);
#endif
		} else {
#ifdef __WXMSW__
	  dc.DrawRotatedText(m_x_tick_labels[cur_tick],cur_point.x + text_height/2,m_origin_location.y-(int)((double)15*(double)(dc.GetPPI().x)/(double)72),270.0);
#else
	  dc.DrawRotatedText(m_x_tick_labels[cur_tick],cur_point.x + text_height/2,m_origin_location.y-(int)((double)15*(double)(dc.GetPPI().x)/(double)72),270.0);
#endif
		}
	} else {
		if (m_bottom_to_top){
			dc.DrawText( m_x_tick_labels[cur_tick],cur_point.x-text_width/2,h-(m_origin_location.y-(int)((double)20*(double)(dc.GetPPI().x)/(double)72)));
		} else {
			dc.DrawText( m_x_tick_labels[cur_tick],cur_point.x-text_width/2,m_origin_location.y-(int)((double)20*(double)(dc.GetPPI().x)/(double)72));
		}
	}
      }
    }


  }


  if (m_y_tick_labels.GetCount() > 0){

    wxFont y_tick_label_font = m_tick_label_font;

    y_tick_label_font.SetPointSize(y_tick_label_font.GetPointSize()*(dc.GetPPI().x)/72);
    dc.SetFont(y_tick_label_font);
    for (cur_tick =0; cur_tick < (int)m_y_tick_locations.GetCount(); cur_tick++){
      if ((m_y_tick_locations[cur_tick]<= m_max_point.y) && (m_y_tick_locations[cur_tick] >= m_min_point.y)){
		  cur_point =  FindPoint(wxRealPoint(m_min_point.x,m_y_tick_locations[cur_tick]));
		  dc.GetTextExtent(m_y_tick_labels[cur_tick],&text_width,&text_height);
		  if (m_left_to_right){
			  dc.DrawText( m_y_tick_labels[cur_tick],m_origin_location.x-(int)((double)20*(double)(dc.GetPPI().x)/(double)72)-text_width,cur_point.y - text_height/2);
		  } else {
			  dc.DrawText( m_y_tick_labels[cur_tick],m_origin_location.x-(int)((double)20*(double)(dc.GetPPI().x)/(double)72)-text_width,cur_point.y + text_height/2);   
		  }
      }
    }

  }


  if (!m_x_axis_label.IsEmpty()){
    cur_point = FindPoint(wxRealPoint((m_max_point.x+m_min_point.x)/2,m_min_point.y));
    
    wxFont x_axis_label_font = m_axis_label_font;
    x_axis_label_font.SetPointSize(x_axis_label_font.GetPointSize()*(dc.GetPPI().x)/72);
    
    dc.SetFont(x_axis_label_font);
    dc.GetTextExtent(m_x_axis_label,&text_width,&text_height);
	  if (m_bottom_to_top){
		  dc.DrawText(m_x_axis_label,cur_point.x-text_width/2,h- (m_origin_location.y - (int)((double)55*(double)(dc.GetPPI().x)/(double)72)));
	  } else {
		  dc.DrawText(m_x_axis_label,cur_point.x-text_width/2,m_origin_location.y - (int)((double)55*(double)(dc.GetPPI().x)/(double)72));
	  }
  }

  if (!m_y_axis_label.IsEmpty()){
    cur_point = FindPoint(wxRealPoint(m_min_point.x,(m_max_point.y+m_min_point.y)/2));

    wxFont y_axis_label_font = m_axis_label_font;
    y_axis_label_font.SetPointSize(m_axis_label_font.GetPointSize()*(dc.GetPPI().x)/72);
    
    

    dc.SetFont(y_axis_label_font); 
    dc.GetTextExtent(m_y_axis_label,&text_width,&text_height);
    if (m_y_tick_labels.GetCount() > 0){
		if (m_bottom_to_top){
#ifdef __WXMSW__
			dc.DrawRotatedText(m_y_axis_label,m_origin_location.x - (int)((double)90*(double)(dc.GetPPI().x)/(double)72),cur_point.y+text_width/2,90.0);
#else
			dc.DrawRotatedText(m_y_axis_label,m_origin_location.x - (int)((double)74*(double)(dc.GetPPI().x)/(double)72),h-((int)(m_origin_location.y +m_axes_length.y/2-(double)1.0/2.0*text_width)),90.0);
#endif
			
		} else {
#ifdef __WXMSW__
			dc.DrawRotatedText(m_y_axis_label,m_origin_location.x - (int)((double)90*(double)(dc.GetPPI().x)/(double)72),cur_point.y-text_width/2,90.0);
#else
			dc.DrawRotatedText(m_y_axis_label,m_origin_location.x - (int)((double)74*(double)(dc.GetPPI().x)/(double)72),(int)(m_origin_location.y +m_axes_length.y/2+(double)3/2.0*text_width),90.0);
#endif
		}
    } else {
#ifdef __WXMSW__
      dc.DrawRotatedText(m_y_axis_label,m_origin_location.x - (int)((double)35*(double)(dc.GetPPI().x)/(double)72),cur_point.y-text_width/2,90.0);
#else
      //dc.DrawRotatedText(m_y_axis_label,m_origin_location.x - (int)((double)74*(double)(dc.GetPPI().x)/(double)72),(int)(cur_point.y+(double)3/2.0*text_width),90.0);
      
      dc.DrawRotatedText(m_y_axis_label,m_origin_location.x - (int)((double)37*(double)(dc.GetPPI().x)/(double)72),(int)(m_origin_location.y +m_axes_length.y/2+(double)1.0/2.0*text_width),90.0);
#endif
    }

  }

  wxFont titlefont = m_title_font;
  
  titlefont.SetPointSize(16*(dc.GetPPI().x)/72);
  

  dc.SetFont(titlefont);


  dc.GetTextExtent(m_title,&text_width,&text_height);
  
  dc.DrawText(m_title,m_origin_location.x +m_axes_length.x/2 - text_width/2,h-(m_origin_location.y+m_axes_length.y + (int)((double)10*(double)(dc.GetPPI().x)/(double)72) + text_height));



}

void labeledplotAxes::setTickLocationX(wxArrayDouble x_tick_locations){


  m_x_tick_locations =  x_tick_locations;


}

void labeledplotAxes::setTickLocationY(wxArrayDouble y_tick_locations){

  m_y_tick_locations =  y_tick_locations;
    


}



void labeledplotAxes::setTickLocations(wxArrayDouble x_tick_locations,wxArrayDouble y_tick_locations){

  
  setTickLocationX(x_tick_locations);
  setTickLocationY(y_tick_locations);

}







void labeledplotAxes::setTickLabelsX(wxArrayString x_tick_labels){


  m_x_tick_labels = x_tick_labels;





}




void labeledplotAxes::setTickLabelsY(wxArrayString y_tick_labels){
  m_y_tick_labels = y_tick_labels;
}





void labeledplotAxes::setTickLabels(wxArrayString x_tick_labels,wxArrayString y_tick_labels){
  setTickLabelsX(x_tick_labels);
  setTickLabelsY(y_tick_labels);


}




void labeledplotAxes::setAxisLabelX(wxString x_axis_label){



  m_x_axis_label = x_axis_label;




}


void labeledplotAxes::setAxisLabelY(wxString y_axis_label){



  m_y_axis_label = y_axis_label;




}


void labeledplotAxes::setAxisLabels(wxString x_axis_label,wxString y_axis_label){

  setAxisLabelX(x_axis_label);
  setAxisLabelY(y_axis_label);




}



void labeledplotAxes::setAxisLabelsFont(wxFont axis_label_font){

  m_axis_label_font = axis_label_font;

}

void labeledplotAxes::setTitle(wxString title){

  m_title = title;


}


void labeledplotAxes::setTitleFont(wxFont title_font){

  m_title_font = title_font;


}

void labeledplotAxes::setTickLabelsPerpendicular(bool tick_labels_perpendicular){
 
  m_tick_labels_perpendicular = tick_labels_perpendicular;

}

void Draw_Axes(wxDC &dc,
	       double x_lim_min,
	       double x_lim_max,
	       double y_lim_min,
	       double y_lim_max,
	       int origin_x,
	       int origin_y,
	       int length_x,
	       int length_y,
	       bool drawticks,
	       int tick_length,
	       int number_ticks,
	       int axes_end_buffer,
	       bool boxformat
	       )
{

  //  int curtick;
  int tick_y;
  int tick_x;


  //  double scalefactor_x = (length_x)/(x_lim_max - x_lim_min);
  // double scalefactor_y = (length_y)/(y_lim_max - y_lim_min);


  dc.SetPen(wxPen(*wxBLACK,1,wxSOLID));
  dc.SetBrush(wxBrush(*wxWHITE));
  
  
  dc.DrawLine(origin_x, origin_y, origin_x, origin_y + length_y + 2*axes_end_buffer);
  dc.DrawLine(origin_x, origin_y, origin_x + length_x + 2*axes_end_buffer, origin_y);
  

  // Box format axes
  if (boxformat){
    dc.DrawLine(origin_x + length_x + 2*axes_end_buffer, origin_y, origin_x + length_x + 2*axes_end_buffer, origin_y + length_y + 2*axes_end_buffer);
    dc.DrawLine(origin_x , origin_y + length_y + 2*axes_end_buffer , origin_x + length_x + 2*axes_end_buffer , origin_y + length_y +  2*axes_end_buffer);
  }
  

  // Draw axes ticks


  for (int curtick=0; curtick <= number_ticks; curtick++){
    tick_x = axes_end_buffer + origin_x + (int)(length_x*((double)curtick)/((double)number_ticks)); 
    tick_y = origin_y;
    dc.DrawLine(tick_x, tick_y,tick_x,tick_y-tick_length);
    
    tick_y =  axes_end_buffer + origin_y + (int)(length_y*((double)curtick)/((double)number_ticks)); 
    tick_x = origin_x;
    dc.DrawLine(tick_x, tick_y,tick_x-tick_length,tick_y);
  }

  
  // Draw tick labels
  
  for (int curtick=0; curtick <= number_ticks; curtick++){
    tick_x = axes_end_buffer + origin_x + (int)(length_x*((double)curtick)/((double)number_ticks)); 
    tick_y = origin_y;
    dc.DrawRotatedText(wxT("A"),tick_x-6,tick_y,90);

    tick_y =  axes_end_buffer + origin_y + (int)(length_y*((double)curtick)/((double)number_ticks)); 
    tick_x = origin_x;
    dc.DrawRotatedText(wxT("B"),tick_x-20,tick_y+6,0);
    


  }





}

