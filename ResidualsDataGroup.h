#ifndef RESIDUALSDATAGROUP_H
#define RESIDUALSDATAGROUP_H

#include <wx/wx.h>
#include "CDFLocMapTree.h"
#include "DataGroup.h"
#include "PMProbeBatch.h"
#include <iostream>
#include "Storage/Matrix.h"
#include "PreferencesDialog.h"

class ResidualsDataGroup{

  
 public:
  ResidualsDataGroup(PMProbeBatch *residuals, DataGroup *originaldata, Preferences *preferences);
  ~ResidualsDataGroup();
#ifndef BUFFERED
  Matrix *GetIntensities();
#else
  BufferedMatrix *GetIntensities();
#endif
  long nrows();
  long ncols();
  wxArrayString GetArrayNames();
  void ResizeBuffer(int rows, int cols);
 private:

  wxArrayString ArrayTypeName;
  wxArrayString probeset_names;
  wxArrayString ArrayNames;

  long array_rows;
  long array_cols;
  long n_probesets;
  long n_arrays;
  long n_probes;

  CDFLocMapTree *cdflocs;

#ifndef BUFFERED
  Matrix *intensitydata;
#else
  BufferedMatrix *intensitydata;

#endif

};



#endif
