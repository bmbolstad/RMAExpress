#ifndef DATAGROUP_H
#define DATAGROUP_H

#include <wx/wx.h>
#include <wx/progdlg.h>
#include "CDFLocMapTree.h"
#include "Storage/Matrix.h"
#include "Storage/BufferedMatrix.h"
#include "PreferencesDialog.h"


class DataGroup
{
 public: 
  DataGroup(wxWindow *parent,const wxString cdf_fname, const wxString cdf_path,
	    const wxArrayString cel_fnames, const wxArrayString cel_paths, Preferences *preferences); // instantiate with CDF and CEL files
  DataGroup(wxWindow *parent,const wxArrayString binary_fnames, const wxArrayString binary_paths, Preferences *preferences); //instantiate with RME files.
  DataGroup(wxWindow *parent,const wxString cdf_fname);       // instantiate with only a CDF file
  DataGroup(wxWindow *parent,const wxArrayString cel_fnames, Preferences *preferences); // instantiate with only CEL files
  ~DataGroup();
  
  void Add(const wxArrayString &fnames, const wxArrayString &paths);
  void Remove(const wxArrayString fnames);

  double &operator[](unsigned int i);
 
  wxArrayString GiveNames();
  LocMapItem *FindLocMapItem(const wxString &);
  wxArrayString GetArrayNames();
  CDFLocMapTree *GiveLocMapTree();

  bool WriteBinaryCDF(wxString path);
  bool WriteBinaryCDF(wxString path, wxString restrictfname ,wxArrayString restrictnames);

  bool WriteBinaryCEL(wxString path);

  int count_pm();
  int count_probesets();
  int count_arrays();
  int nrows();
  int ncols();

  void SetArrayTypeName(wxString new_name);
  wxArrayString GetArrayTypeName();

  void ResizeBuffer(int rows, int cols);

  double operator()(int row, int col);

  void ReadOnlyMode(bool setting);

  void AddCDF_desc(wxString &desc);
 private:

  void ReadCDFFile(const wxString cdf_fname, const wxString cdf_path);
  void ReadCELFile(const wxString cel_path,const int col);
  void ReadBinaryCDF(const wxString cdf_fname, const wxString cdf_path);
  void ReadBinaryCEL(const wxString cel_fname, const wxString cel_path,const int col);
  void ReadBinaryCEL(const wxString cel_path,const int col);

  wxWindow *parent;
  wxArrayString ArrayTypeName;
  wxArrayString probeset_names;
  wxArrayString ArrayNames;
  
  CDFLocMapTree cdflocs;
  wxArrayString  CDF_descStr;


  int array_rows;
  int array_cols;
  int n_probesets;
  int n_arrays;
  int n_probes;
#ifndef BUFFERED
  Matrix *intensitydata;
#else
  BufferedMatrix *intensitydata;

#endif


#if RMA_GUI_APP 
  wxProgressDialog *DataGroupProgress;
#endif
};










#endif
