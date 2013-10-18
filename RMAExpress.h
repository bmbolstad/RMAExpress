#ifndef RMAEXPRESS_H
#define RMAEXPRESS_H

#include "DataGroup.h"
#include "expressionGroup.h"
#include "PMProbeBatch.h"
#include "ResidualsDataGroup.h"
#include "PreferencesDialog.h"



#define ID_BACKGROUNDBOX 10000
#define ID_NORMALBOX 10001
#define ID_RESIDUALS 10002










enum
{ RMAEXPRESS_EXIT = 1,
  RMAEXPRESS_READ = 100,
  RMAEXPRESS_BINARYREAD = 125,
  RMAEXPRESS_ADDNEWCEL = 150,
  RMAEXPRESS_ABOUT = 200,
  RMAEXPRESS_COMPUTE = 300,
  RMAEXPRESS_WRITE = 400,
  RMAEXPRESS_WRITENATURAL = 410,
  RMAEXPRESS_EXPORT = 420,
  RMAEXPRESS_BINARYWRITE = 425,
  RMAEXPRESS_LOGFILEWRITE = 450,
  RMAEXPRESS_SHOW_DATAGROUP = 500,
  RMAEXPRESS_SHOW_RESIDUALS = 510,
  RMAEXPRESS_SHOW_PREFERENCES = 520,
  RMAEXPRESS_VISUALIZE_RAWDATA = 530,
  RMAEXPRESS_VISUALIZE_QC = 540
};

static const wxChar
  *CDFFILETYPES = _T( "CDF files|*.cdf;*.CDF;*.Cdf;*.cdf.rme;*.cdfrme;*.CDF.RME;*.CDFRME" );

static const wxChar
*CELFILETYPES = _T( "CEL files|*.cel;*.Cel;*.CEL;*.rme;*.Rme;*.RME;*.celrme;*.cel.rme;*.CEL.RME; *.CELRME");

static const wxChar
  *RMEFILETYPES = _T( "RMA Express files|*.rme;*.Rme;*.RME" );


class RMAExpress : public wxApp
{
  public:
	virtual bool OnInit();	

 private:
	wxConfig *mysettings;
};

class RMAExpressFrame : public wxFrame
{
  public:
	RMAExpressFrame( const wxChar *title, 
                int xpos, int ypos, 
                int width, int height);
	~RMAExpressFrame();
	
	wxMenuBar  *menuBar;
	wxMenu     *fileMenu;
	wxMenu     *showMenu;
	wxMenu     *aboutMenu;

	wxTextCtrl *Messages;


	void SetSettings(wxConfig *settings);
	void SetPreferences(int narrays, int nprobes, wxString tmppath);


	void OnRead (wxCommandEvent & event);
	void OnCompute (wxCommandEvent & event);
	void OnWrite (wxCommandEvent & event);
	void OnWriteNatural (wxCommandEvent & event);
	void OnAbout (wxCommandEvent & event);
	void OnExit (wxCommandEvent & event);
	void OnLogFileWrite (wxCommandEvent & event);
	void OnBinaryWrite(wxCommandEvent&);
	void OnBinaryRead(wxCommandEvent&);
	void OnAddCEL(wxCommandEvent&);
	void OnShowDataGroup(wxCommandEvent&);
	void OnShowResidualImages(wxCommandEvent&);
	void OnShowPreferencesDialog(wxCommandEvent & event);
	void OnExportExpressionValues(wxCommandEvent & event);
	void OnVisualizeRawData(wxCommandEvent & event);
	void OnVisualizeQCStatistics(wxCommandEvent & event);

	Preferences *myprefs;
 private:
	DataGroup* currentexperiment;
	expressionGroup *myexprs;
	ResidualsDataGroup *myresids;
	wxConfig *mysettings;
	
	DECLARE_EVENT_TABLE()	
	  
};



class PreprocessDialog: public wxDialog
{
   public:
  PreprocessDialog
      ( wxWindow *parent,
	wxWindowID id,
       const wxString &title,
       const wxPoint& pos = wxDefaultPosition,
       const wxSize& size = wxDefaultSize,
       long style = wxDEFAULT_DIALOG_STYLE
     );
    PreprocessDialog(wxWindow *parent);
    wxTextCtrl * dialogText;
    wxString GetText(); 
    wxRadioBox *BgMethod;
    wxRadioBox *NormMethod;
    wxRadioBox *SummaryMethod;
    wxCheckBox *StoreResiduals;
   private:
    void OnOk( wxCommandEvent &event );
    void OnCancel( wxCommandEvent &event );
    DECLARE_EVENT_TABLE()
};





#endif

