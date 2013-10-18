/* 
   This file is part of RMAExpress.

    RMAExpress is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    RMAExpress is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RMAExpress; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
*/

/*****************************************************
 **
 ** file: Dump_CDFRME.cpp
 **
 ** Copyright (c) B. M. Bolstad 2009
 **
 ** Aim: Dump out CDFRME file contents
 **
 ** History
 ** Dec 15, 2007 - Transfer relevant code from out of DataGroup.cpp
 **
 **
 *****************************************************/

#include <wx/wx.h>
#include <wx/progdlg.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>
#include <wx/datstrm.h>



typedef struct{
  wxArrayString CDF_descStr;
  wxArrayString ArrayTypeName;
  int array_rows;
  int array_cols;
  int n_probesets;
  int n_probes;
  wxArrayString probeset_names;
} RME_CDF_Header;



void ParseRMECDF(const wxString cdf_path,RME_CDF_Header *header){

  int i,j;
  int versionnumber;
  int current_n_probes;
  int current_n_probes_pm;
  int current_n_probes_mm;
  
  wxFileInputStream input(cdf_path);
  wxDataInputStream store(input);
   
  wxString Error;

  wxString filetype;
  wxString current_probeset_name;
  
  int *PMLoc, *MMLoc;
  
  filetype = store.ReadString();
  wxPrintf(filetype);
  wxPrintf(_T("\n"));
  /* Version 1 and 2 start with CDF. Version 3 starts with RMECDF */

  if (filetype.Cmp(_T("CDF"))!=0 && (filetype.Cmp(_T("RMECDF")) != 0)){
    Error=_T("Problem with RME (CDF format). Not correct file format or otherwise malformed?");
    throw Error;

  }

  versionnumber = store.Read32();
  wxPrintf(_T("%d\n"),versionnumber);
  if (versionnumber == 1 || versionnumber == 2){
    header->ArrayTypeName.Add(store.ReadString());
    header->array_rows = store.Read32();
    header->array_cols = store.Read32();
    header->n_probesets = store.Read32();
    wxPrintf(header->ArrayTypeName[1]);
    wxPrintf(_T("\n"));
    wxPrintf(_T("%d\n"),header->array_rows);
    wxPrintf(_T("%d\n"),header->array_cols);
    wxPrintf(_T("%d\n"),header->n_probesets);
  } else if (versionnumber == 3){
    int n_ArrayTypes = store.Read32();
    wxPrintf(_T("%d\n"),n_ArrayTypes);
    for (i =0; i < n_ArrayTypes; i++){
      header->ArrayTypeName.Add(store.ReadString());
      wxPrintf(header->ArrayTypeName[i]);
      wxPrintf(_T("\n"));
    }
    int n_descStr = store.Read32();
    wxPrintf(_T("%d\n"),n_descStr);
    for (i =0; i < n_descStr; i++){
      header->CDF_descStr.Add(store.ReadString()); 
      wxPrintf(header->CDF_descStr[i]);
      wxPrintf(_T("\n"));
    }
    header->array_rows = store.Read32();
    header->array_cols = store.Read32();
    header->n_probesets = store.Read32();  
    wxPrintf(_T("%d\n"),header->array_rows);
    wxPrintf(_T("%d\n"),header->array_cols);
    wxPrintf(_T("%d\n"),header->n_probesets);
  }
  header->n_probes = 0;
  if (versionnumber == 1){
    header->probeset_names.Alloc(header->n_probesets);
    for (i =0; i < header->n_probesets; i++){
      current_probeset_name = store.ReadString();
      header->probeset_names.Add(current_probeset_name);   // ProbesetName;
      current_n_probes = store.Read32(); // number of probe pairs
      header->n_probes+= current_n_probes;
      
      PMLoc = new int[current_n_probes];
      MMLoc = new int[current_n_probes];
      
      for (j = 0; j < current_n_probes; j++){
	PMLoc[j] = store.Read32();
      }
      for (j = 0; j < current_n_probes; j++){
	MMLoc[j] = store.Read32();
      }
      
    }
  } else if (versionnumber ==2){
    header->probeset_names.Alloc(header->n_probesets);

    for (i =0; i < header->n_probesets; i++){
      current_probeset_name = store.ReadString();
      header->probeset_names.Add(current_probeset_name);   // ProbesetName;
      current_n_probes_pm = store.Read32(); // number of PM probes
      header->n_probes+= current_n_probes_pm;
      if (current_n_probes_pm > 0){
	PMLoc = new int[current_n_probes_pm];
	for (j = 0; j < current_n_probes_pm; j++){
	  PMLoc[j] = store.Read32();
	}
      } else {
	PMLoc = NULL;
      }
      current_n_probes_mm = store.Read32();
      if (current_n_probes_mm > 0){
	MMLoc = new int[current_n_probes_mm];
	for (j = 0; j < current_n_probes_mm; j++){
	  MMLoc[j] = store.Read32();
	}
      } else {
	MMLoc = NULL;
      }
    }
  } else if (versionnumber ==3){
    header->probeset_names.Alloc(header->n_probesets);
      
    for (i =0; i < header->n_probesets; i++){
      current_probeset_name = store.ReadString();
      wxPrintf(_T("Probeset: "));
      wxPrintf(current_probeset_name);
      wxPrintf(_T("\n"));
      header->probeset_names.Add(current_probeset_name);   // ProbesetName;
      current_n_probes_pm = store.Read32(); // number of PM probes
      wxPrintf(_T("PM:%d"), current_n_probes_pm);
      header->n_probes+= current_n_probes_pm;
      //    wxPrintf(_T("%s %d\n"),current_probeset_name.c_str(),current_n_probes_pm); 
      if (current_n_probes_pm > 0){
	PMLoc = new int[current_n_probes_pm];
	for (j = 0; j < current_n_probes_pm; j++){
	  PMLoc[j] = store.Read32();
	  wxPrintf(_T("\t%d"),PMLoc[j]);
	}
	delete [] PMLoc;
      } else {
	PMLoc = NULL;
      }
      wxPrintf(_T("\n"));
      current_n_probes_mm = store.Read32();
      wxPrintf(_T("MM:%d"), current_n_probes_mm);
      if (current_n_probes_mm > 0){
	MMLoc = new int[current_n_probes_mm];
	for (j = 0; j < current_n_probes_mm; j++){
	  MMLoc[j] = store.Read32(); 
	  wxPrintf(_T("\t%d"),MMLoc[j]);
	}
	delete [] MMLoc;
      } else {
	MMLoc = NULL;
      }
      wxPrintf(_T("\n"));
    }
  }
}




int main(int argc, char **argv){


  RME_CDF_Header *header;
  
  header = new RME_CDF_Header;
  

  ParseRMECDF(wxString(argv[1], wxConvUTF8),header);


}
