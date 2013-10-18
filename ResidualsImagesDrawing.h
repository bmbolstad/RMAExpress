#ifndef RESIDUALSIMAGESDRAWING_H
#define RESIDUALSIMAGESDRAWING_H



#include <wx/wx.h>
#include <wx/file.h>
#include <wx/image.h>

#if RMA_GUI_APP
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#endif

#include <wx/filename.h>
#include "ResidualsDataGroup.h"






#if RMA_GUI_APP
void drawPseudoChipImage(wxDC *dc,wxString name, wxString type, ResidualsDataGroup *resids);
void drawPseudoChipImage(wxDC *dc,wxString name, wxString type, ResidualsDataGroup *resids,int startx, int starty, int width, int height);
#else
void drawPseudoChipImage( wxImage *Image, wxString name, wxString type, ResidualsDataGroup *resids);
#endif


#endif
