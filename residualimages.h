#include <wx/object.h>
#include <wx/combobox.h>
#include "PMProbeBatch.h"
#include "ResidualsDataGroup.h"
#include "CDFLocMapTree.h"


class ResidualImageDialog;

class MyCanvas: public wxScrolledWindow
{
public:
  MyCanvas(){}
  MyCanvas(ResidualImageDialog *parent,ResidualsDataGroup *resids);
  void OnPaint( wxPaintEvent &event);
  ResidualsDataGroup *GiveMyResids();
  void SetScroll();
  void PaintBackground(wxDC& dc);
  void OnEraseBackground(wxEraseEvent& event);
 private: 
  ResidualsDataGroup *my_resids;
  ResidualImageDialog *my_parent;
  wxBitmap* my_bitmap;
  
  DECLARE_DYNAMIC_CLASS(MyCanvas)
	DECLARE_EVENT_TABLE()
};



class ResidualImageDialog: public wxDialog
{
 public:
  ResidualImageDialog(){}
  ResidualImageDialog(wxWindow *parent, ResidualsDataGroup *resids,
		      wxWindowID id,
		      const wxString &title,
		      const wxPoint &position,
		      const wxSize& size,
		      long style); 
  void ChangeChip(wxCommandEvent &event); 
  void ChangeImageType(wxCommandEvent &event); 
  void SaveCurrentImage(wxCommandEvent &event); 
  void SaveCurrentImageAll(wxCommandEvent &event); 
  void SaveAllImages(wxCommandEvent &event); 

  void ClickNext(wxCommandEvent &event);
  void ClickPrevious(wxCommandEvent &event);


  wxComboBox *whichchip;
  wxRadioBox *whichtype;
  wxRadioBox *whichzoom;
  
  bool needtoredraw;
  
 private:
  void OnOk( wxCommandEvent &event );

  MyCanvas *m_canvas;


  DECLARE_DYNAMIC_CLASS(ResidualImageDialog)
  DECLARE_EVENT_TABLE()
};
    

