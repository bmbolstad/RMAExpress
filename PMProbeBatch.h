#ifndef PMPROBEBATCH_H
#define PMPROBEBATCH_H
 
#include <wx/wx.h>
#include <wx/string.h>  // wxString
#include <utility>   // for pair
#include <vector>
#include "DataGroup.h"
#include "expressionGroup.h"
#include "Storage/BufferedMatrix.h"
#include "PreferencesDialog.h"

using namespace std;

class PMProbeBatch
{
 public:
  PMProbeBatch(DataGroup &x,Preferences  *preferences);
  ~PMProbeBatch();
  void background_adjust();
  void normalize(bool lowmem);
  expressionGroup *summarize(); 
  expressionGroup *summarize_PLM();
  

  wxArrayString GetArrayNames();
  wxArrayString GetRowNames();
  
  void GetRow(double *buffer, int row);
  void GetValue(double *buffer, int row, int col);

  long count_pm();
  long count_probesets();
  long count_arrays();

  void Compute5Summary(int col,double *results);

 private:
  long n_probes;
  long n_arrays;
  long n_probesets;
#ifndef BUFFERED
  double *intensity;
#else
  BufferedMatrix *intensity;
#endif
  wxArrayString ProbesetRowNames;
  wxArrayString ArrayNames;
  wxArrayString ArrayTypeName;
  

  std::vector<std::pair<wxString, int>> ProbesetRowNames_count;

#if RMA_GUI_APP
 // wxProgressDialog *PreprocessDialog;
#endif


};








#endif
