#include "expressionGroup.h"
#include "Storage/Matrix.h"
#include <wx/wx.h>
#include <wx/print.h>


#if RMA_GUI_APP
class QCStatsVisualizeFrame: public wxFrame
{
public:
  QCStatsVisualizeFrame(const wxString &title, expressionGroup *ExpressionVals,wxWindow *parent);
  ~QCStatsVisualizeFrame();
  void OnQuit(wxCommandEvent& event);
 
  void OnVisualizeRLE(wxCommandEvent& event);
  void OnVisualizeNUSE(wxCommandEvent& event);
  void OnVisualizeRLEMedians(wxCommandEvent& event);
  void OnVisualizeRLEIQRs(wxCommandEvent& event);
  void OnVisualizeNUSEMedians(wxCommandEvent& event);
  void OnVisualizeNUSEIQRs(wxCommandEvent& event);
  void OnVisualizeRLENUSEMultiplot(wxCommandEvent& event);
  void OnVisualizeMultivariateControlChart(wxCommandEvent& event);

  void ToggleSCCLimits(wxCommandEvent& event);
  void ToggleIQRLimits(wxCommandEvent& event);

#else 
class QCStatsVisualizeFrame
{
 public:
  QCStatsVisualizeFrame(expressionGroup *ExpressionVals); 
  ~QCStatsVisualizeFrame();
#endif
  void GenerateRLE();
  void GenerateNUSE();
  void WriteNUSEStats(wxString path, wxString filename);
  void WriteRLEStats(wxString path, wxString filename);

  void ComputeRLENUSE_T2();
#if RMA_GUI_APP  
  void DrawRLEBoxplot(wxDC& dc);
  void DrawNUSEBoxplot(wxDC& dc);
  void DrawRLEMedians(wxDC& dc); 
  void DrawRLEIQRs(wxDC& dc);
  void DrawNUSEMedians(wxDC& dc);
  void DrawNUSEIQRs(wxDC& dc);
  void DrawRLENUSEMultiPlot(wxDC& dc);
  void DrawMultivariateControlChart(wxDC& dc);
  void OnPostscriptCurrentPlot(wxCommandEvent& event);

  void OnSaveQCRLE(wxCommandEvent& event);
  void OnSaveQCNUSE(wxCommandEvent& event);
  
  void OnSaveQCRLEStats(wxCommandEvent& event);
  void OnSaveQCNUSEStats(wxCommandEvent& event);
 
  void OnSaveCurrent(wxCommandEvent& event);
#endif
  Matrix *RLEBoxplotStatistics;  
  Matrix *NUSEBoxplotStatistics;

  Matrix *T2;

  int currentplot;
private:
#if RMA_GUI_APP  
  wxWindow *MyWindow; 
  wxMenu *fileMenu;
  wxMenu *drawMenu;
  wxMenuBar *menuBar;
#endif
  expressionGroup *ExpressionValues;
#if RMA_GUI_APP  
  DECLARE_EVENT_TABLE()
#endif
};


#if RMA_GUI_APP  
class QCStatsVisualizeDrawingWindow: public wxWindow
{
 public:
  QCStatsVisualizeDrawingWindow(QCStatsVisualizeFrame *parent);
  void OnPaint(wxPaintEvent &WXUNUSED(event));
  int currentplot;
  void OnEraseBackground(wxEraseEvent &WXUNUSED(event));

 private:

  QCStatsVisualizeFrame *myParent;
  
  DECLARE_EVENT_TABLE()
};




class QCStatsVisualizePrintout: public wxPrintout
{
 public:
  QCStatsVisualizePrintout(wxChar *title = _T("My printout")):wxPrintout(title) {}
    bool OnPrintPage(int page);
    bool HasPage(int page);
    bool OnBeginDocument(int startPage, int endPage);
    void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
    
    void DrawPageOne(wxDC *dc);
};

#endif
