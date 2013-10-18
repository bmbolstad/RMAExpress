#ifndef BOXPLOT_H
#define BOXPLOT_H

#include <wx/dc.h>








void Draw_Single_Boxplot(wxDC &dc,
			 double *fivenumsummary,
			 int ylim_min_int,
			 int ylim_max_int,
			 double ylim_min_double,
			 double ylim_max_double,
			 int x,
			 int boxwidth,
			 const wxColor *fillcolor=wxWHITE);


void Draw_Single_Boxplot(wxDC &dc,
			 int y_min,
			 int y_LQ,
			 int y_median,
			 int y_UQ,
			 int y_max,
			 int x,
			 int boxwidth,
			 const wxColor *fillcolor=wxWHITE);

#endif
