#ifndef BITMAPSETTINGDIALOG_H
#define BITMAPSETTINGDIALOG_H 1

#include <wx/wx.h>
#include <wx/object.h>
#include <wx/combobox.h>



class BitmapSettingDialog: public wxDialog
{
 public:
  BitmapSettingDialog(){};
  BitmapSettingDialog(wxWindow *parent, 
		      wxWindowID id,
		      const wxString &title,
		      const wxPoint &position,
		      const wxSize& size,
		      long style);
    
  ~BitmapSettingDialog();
    
  wxString GetImageType();
    
  int GetHeight();
  int GetWidth();
  void MovedHorizontalPixelsSlider(wxScrollEvent &event);
  void MovedVerticalPixelsSlider(wxScrollEvent &event);

  void EditedVerticalPixels(wxCommandEvent &event);
    
 private:
    int height;
    int width; 
    
    wxRadioBox *ImageFormat;
    
    wxTextCtrl *HeightInput;
    wxTextCtrl *WidthInput;
  
    wxSlider *HorizontalPixelsSlider;
    wxSlider *VerticalPixelsSlider;
   


  DECLARE_DYNAMIC_CLASS(BitmapSettingDialog)
  DECLARE_EVENT_TABLE()
};

#endif
