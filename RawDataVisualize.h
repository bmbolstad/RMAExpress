
#include "PMProbeBatch.h"
#include "Storage/Matrix.h"
#include <wx/print.h>

class RawDataVisualizeFrame: public wxFrame
{
public:
  RawDataVisualizeFrame(const wxString &title, PMProbeBatch *RawData, wxWindow *parent);
  ~RawDataVisualizeFrame();
  void OnQuit(wxCommandEvent& event);
 
  void OnVisualizeRawBoxplot(wxCommandEvent& event);
  void OnVisualizeRawDensityPlot(wxCommandEvent& event); 
  void OnVisualizeIndividualRawDensityPlot(wxCommandEvent& event);
  
  void GenerateBoxplotStatistics();
  void GenerateDensityPlotStatistics();
  
  void DrawRawBoxplot(wxDC& dc);
  void DrawRawDensityPlot(wxDC& dc);
  void DrawIndividualRawDensityPlot(wxDC& dc);
 

  void OnPostscriptCurrentPlot(wxCommandEvent& event);
  void OnPrintPreview(wxCommandEvent& event);

  
  void OnSaveCurrent(wxCommandEvent& event);
  
  void OnKeyPress(wxKeyEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnKeyUp(wxKeyEvent& event);

  PMProbeBatch *GetRawData();
  
  Matrix *BoxplotStatistics;  
  Matrix *DensityPlotStatistics_y;
  Matrix *DensityPlotStatistics_x;
  


  int currentplot;

  int whichdensity;


private:
  wxWindow *MyWindow;

  PMProbeBatch *MyRawData;

  DECLARE_EVENT_TABLE()
};



class RawDataVisualizeDrawingWindow: public wxWindow
{
 public:
  RawDataVisualizeDrawingWindow(RawDataVisualizeFrame *parent);
  void OnPaint(wxPaintEvent &WXUNUSED(event));
  void OnClick(wxMouseEvent& event);
  int currentplot;
  void OnEraseBackground(wxEraseEvent &WXUNUSED(event));
	void OnKeyPress(wxKeyEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
 private:

  RawDataVisualizeFrame *myParent;
  
  DECLARE_EVENT_TABLE()
};



class RawDataVisualizePrintout: public wxPrintout
{
 public:
  RawDataVisualizePrintout(const wxChar *title = _T("My printout")):wxPrintout(title) {}
    bool OnPrintPage(int page);
    bool HasPage(int page);
    bool OnBeginDocument(int startPage, int endPage);
    void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
    
    void DrawPageOne(wxDC *dc);
    void DrawPageTwo(wxDC *dc);
};
