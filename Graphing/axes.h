#ifndef AXES_H
#define AXES_H


#include <wx/dc.h>
#include <wx/colour.h>







class plotAxes {

 public:
  plotAxes(wxDC &DC, wxPoint origin_location,
	   wxPoint axes_length,
	   wxRealPoint min_point,
	   wxRealPoint max_point,
	   bool boxformat=false,
	   wxPoint axespadding = wxPoint(5,5),
	   int pen_width=3,
	   wxColor axis_color = *wxBLACK,
	   wxBrush background_color =*wxGREY_BRUSH,
	   bool left_to_right = true,
	   bool bottom_to_top = true);


  void Draw(wxDC &DC);

  wxPoint FindPoint(const wxRealPoint &point);

  wxRect getDrawingRegion();
  
  void activateBoxFormat();
  void deactivateBoxFormat();

  void setupDCforPlot(wxDC &dc);

  void setBackgroundColor(wxBrush color);

 protected:
  int m_h;
	int m_w;	
  wxPoint m_origin_location;        // Location of bottom left hand corner in a Y vs X type plot
  wxPoint m_axes_length;   // x axis and y axis length
	   
  bool m_box_format;
  
  wxRealPoint  m_min_point;
  wxRealPoint  m_max_point;
  
  wxPoint m_axespadding;

  int m_pen_width;
  wxColor m_axis_color;
  
  wxBrush m_background_color;
  
  bool m_left_to_right;
  bool m_bottom_to_top;

};

#include <wx/dynarray.h> 

#if !wxCHECK_VERSION(2, 8, 0)
  WX_DEFINE_ARRAY_DOUBLE(double, wxArrayDouble);
#endif
class labeledplotAxes : public plotAxes{

 public:
  labeledplotAxes(wxDC &DC, wxPoint origin_location,
		  wxPoint axes_length,
		  wxRealPoint min_point,
		  wxRealPoint max_point,
		  bool boxformat=false,
		  wxPoint axespadding = wxPoint(5,5),
		  bool draw_ticks=true,
		  int tick_length=10,
		  wxFont axis_label_font = wxFont(12,wxSWISS,wxNORMAL,wxBOLD),
		  wxFont tick_label_font=wxFont(10,wxSWISS,wxNORMAL,wxNORMAL),
		  wxFont title_font = wxFont(16, wxSWISS, wxNORMAL, wxBOLD),
		  bool tick_labels_perpendicular=false);
  

  void addGrid(double grid_spacing_x, double grid_spacing_y,wxColor linecolor=*wxLIGHT_GREY);
  // void addGridX(double grid_spacing_x,wxColor *linecolor= wxLIGHT_GREY);
  //void addGridY(double grid_spacing_y,wxColor *linecolor=wxLIGHT_GREY);

  void removeGrid();
  
  void setAxisLabels(wxString x_axis_label,wxString y_axis_label);
  void setAxisLabelX(wxString x_axis_label);
  void setAxisLabelY(wxString y_axis_label);
  void setAxisLabelsFont(wxFont axis_label_font);

  void setTitle(wxString title);
  void setTitleFont(wxFont title_font);


  void setTickLocations(wxArrayDouble x_tick_locations, wxArrayDouble y_tick_locations);
  void setTickLocationX(wxArrayDouble x_tick_locations);
  void setTickLocationY(wxArrayDouble y_tick_locations);


  void setTickLabels(wxArrayString x_tick_labels, wxArrayString y_tick_labels);
  void setTickLabelsX(wxArrayString x_tick_labels);
  void setTickLabelsY(wxArrayString y_tick_labels);

  void setTickLabelsPerpendicular(bool tick_labels_perpendicular);

  void Draw(wxDC &dc);

 private:
  wxArrayString m_x_tick_labels;
  wxArrayString m_y_tick_labels;
  wxFont m_tick_label_font;


  wxArrayDouble m_x_tick_locations;
  wxArrayDouble m_y_tick_locations;
  int m_tick_length;
  bool m_draw_ticks;


  wxString m_title;
  wxFont m_title_font;
  
  wxString m_x_axis_label;
  wxString m_y_axis_label;
  wxFont m_axis_label_font;

  bool m_grid_add;
  wxColor m_grid_color_x;
  wxColor m_grid_color_y;
  double m_grid_spacing_x;
  double m_grid_spacing_y;

  bool m_tick_labels_perpendicular;

};







void Draw_Axes(wxDC &dc,
	       double x_lim_min,
	       double x_lim_max,
	       double y_lim_min,
	       double y_lim_max,
	       int origin_x,
	       int origin_y,
	       int length_x,
	       int length_y,
	       bool drawticks = true,
	       int tick_length = 4,
	       int number_ticks = 5,
	       int axes_end_buffer = 10,
	       bool boxformat = false
	       );

#endif
