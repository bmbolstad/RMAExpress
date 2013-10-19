
#ifndef RMADataConvWidget_H
#define RMADataConvWidget_H

// Include wxWindows' headers

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif


#include <wx/dialog.h>
#include <wx/image.h>
#include <wx/statline.h>
#include <wx/spinbutt.h>
#include <wx/spinctrl.h>
#include <wx/splitter.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/grid.h>
#ifdef BUFFERED
#include "PreferencesDialog.h"
#endif

// Declare window functions

#define ID_TEXT 10000
#define ID_TEXTCTRL 10001

#define ABOUT_BUTTON 10003
#define CONVERT_BUTTON 10004
#define QUIT_BUTTON 10005
#define PREF_BUTTON 10006

#define CELBROWSE_BUTTON 10010
#define CDFBROWSE_BUTTON 10011
#define RESTRICTBROWSE_BUTTON 10012
#define OUTPUTBROWSE_BUTTON 10013
#define PGFBROWSE_BUTTON 10014
#define CLFBROWSE_BUTTON 10015
#define PSBROWSE_BUTTON 10016
#define MPSBROWSE_BUTTON 10017

static const wxChar
  *CDFFILETYPES = _T( "CDF files|*.cdf;*.CDF;*.Cdf" );
static const wxChar
  *CELFILETYPES = _T( "CEL files|*.cel;*.Cel;*.CEL" );

static const wxChar
  *PGFFILETYPES = _T( "PGF files|*.pgf;*.PGF;*.Pgf" );

static const wxChar
  *CLFFILETYPES = _T( "CLF files|*.clf;*.CLF;*.Clf" );

static const wxChar
  *PSFILETYPES = _T( "PS files|*.ps;*.PS;*.Ps" );

static const wxChar
  *MPSFILETYPES = _T( "MPS files|*.mps;*.MPS;*.Mps" );




class RMADataConvDlg : public wxDialog
{
 public:
  RMADataConvDlg(const wxString& title, const wxPoint& pos, const wxSize& size);
  ~RMADataConvDlg();
  void  OnAbout(wxCommandEvent& WXUNUSED(event));
  void  OnQuit(wxCommandEvent& WXUNUSED(event));
  void  OnQuit2(wxCloseEvent& WXUNUSED(event));
  void  OnCelBrowse(wxCommandEvent& WXUNUSED(event));
  void  OnCdfBrowse(wxCommandEvent& WXUNUSED(event));
  void  OnCLFBrowse(wxCommandEvent& WXUNUSED(event));
  void  OnPGFBrowse(wxCommandEvent& WXUNUSED(event));
  void  OnPSBrowse(wxCommandEvent& WXUNUSED(event));
  void  OnMPSBrowse(wxCommandEvent& WXUNUSED(event));
  void  OnRestrictBrowse(wxCommandEvent& WXUNUSED(event));
  void  OnOutputBrowse(wxCommandEvent& WXUNUSED(event));
  void  OnConvert(wxCommandEvent& WXUNUSED(event));
  void  OnPreferences(wxCommandEvent& WXUNUSED(event));
  wxTextCtrl *CelDirectory;
  wxTextCtrl *CdfFile;
  wxTextCtrl *RestrictFile;
  wxTextCtrl *OutputDirectory;
  wxTextCtrl *ForceBox;  
  wxTextCtrl *PGFFile;
  wxTextCtrl *CLFFile;
  wxTextCtrl *PSFile;
  wxTextCtrl *MPSFile;


 private:
#ifdef BUFFERED  
  Preferences *myprefs;
#endif

  DECLARE_EVENT_TABLE()
};


class RMADataConv: public wxApp
{
virtual bool OnInit();
};

#endif
