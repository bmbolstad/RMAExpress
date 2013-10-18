#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H 1

#include <wx/wx.h>
#include <wx/object.h>
#include <wx/combobox.h>



class Preferences{
 public:
  
  Preferences();
  Preferences(int ProbesBufSize, int ArraysBufSize, wxString filepath);
  int GetArrayBufSize();
  void SetArrayBufSize(int value);
  
  int GetProbesBufSize();
  void SetProbesBufSize(int value);


  void SetFilePath(wxString value);
  wxString &GetFilePath();

  wxString GetFullFilePath();


 private:
  wxString filepath;
  int ArraysBufSize;  // ie number of columns 
  int ProbesBufSize;  // ie number of rows;
};

#if RMA_GUI_APP
class PreferencesDialog: public wxDialog
{
 public:
  PreferencesDialog(){}
  PreferencesDialog(wxWindow *parent, 
		    wxWindowID id,
		    const wxString &title,
		    const wxPoint &position,
		    const wxSize& size,
		    long style);
  ~PreferencesDialog();
  int GetArraysBufSize();
  int GetProbesBufSize();
  wxString GetTempFileLoc();
  void SetPreferences(Preferences *mypref);

  
 private:
  void OnOk( wxCommandEvent &event );
  void OnChooseDir(wxCommandEvent &event );
  void MovedArraysSlider(wxScrollEvent &event); //wxUpdateUIEvent &event);
  void MovedProbesSlider(wxScrollEvent &event); //xUpdateUIEvent &event);


  wxSlider *ArraysSlider;
  wxStaticText *ArraysSliderMsg;
  wxSlider *ProbesSlider;
  wxStaticText *ProbesSliderMsg;
  wxTextCtrl *tempfilepath;
  


  DECLARE_DYNAMIC_CLASS(PreferencesDialog)
  DECLARE_EVENT_TABLE()
    };
#endif


#endif
