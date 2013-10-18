#ifndef PMPROBEBATCH_H
#define PMPROBEBATCH_H
 
#include <wx/wx.h>
#include "DataGroup.h"
#include "expressionGroup.h"
#include "Storage/BufferedMatrix.h"
#include "PreferencesDialog.h"

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
  



};








#endif
