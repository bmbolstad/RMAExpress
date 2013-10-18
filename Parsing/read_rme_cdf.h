#ifndef READ_RME_CDF_H
#define READ_RME_CDF_H


#include <wx/wx.h>



typedef struct{
  wxArrayString CDF_descStr;
  wxArrayString ArrayTypeName;
  int array_rows;
  int array_cols;
  int n_probesets;
  int n_probes;
  wxArrayString probeset_names;
} RME_CDF_Header;




void ReadRMECDF(const wxString cdf_path,
		CDFLocMapTree *cdflocs,
		RME_CDF_Header *header);

int is_cdf_RME(const wxString cdf_path);

#endif
