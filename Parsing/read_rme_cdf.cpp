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
 ** file: read_rme_cdf.cpp
 **
 ** Copyright (c) B. M. Bolstad 2007 
 **
 ** Aim: Implement parsing of RME format CDF
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

#include "../CDFLocMapTree.h"
#include "read_rme_cdf.h"


void ReadRMECDF(const wxString cdf_path,
		CDFLocMapTree *cdflocs,
		RME_CDF_Header *header){

  int i,j;
  int versionnumber;
  int current_n_probes;
  int current_n_probes_pm;
  int current_n_probes_mm;
  
  int *PMLoc, *MMLoc;
  LocMapItem *currentitem;

  
  wxFFileInputStream input(cdf_path);
  wxDataInputStream store(input);
   
  wxString Error;

  wxString filetype;
  wxString current_probeset_name;
  
  filetype = store.ReadString();
  
  /* Version 1 and 2 start with CDF. Version 3 starts with RMECDF */

  if (filetype.Cmp(_T("CDF"))!=0 && (filetype.Cmp(_T("RMECDF")) != 0)){
    Error=_T("Problem with RME (CDF format). Not correct file format or otherwise malformed?");
    throw Error;

  }

  versionnumber = store.Read32();

  if (versionnumber == 1 || versionnumber == 2){
    header->ArrayTypeName.Add(store.ReadString());
    header->array_rows = store.Read32();
    header->array_cols = store.Read32();
    header->n_probesets = store.Read32();
  } else if (versionnumber == 3){
    int n_ArrayTypes = store.Read32();
    for (i =0; i < n_ArrayTypes; i++){
      header->ArrayTypeName.Add(store.ReadString());
    }
    int n_descStr = store.Read32();
    for (i =0; i < n_descStr; i++){
      header->CDF_descStr.Add(store.ReadString());
    }
    header->array_rows = store.Read32();
    header->array_cols = store.Read32();
    header->n_probesets = store.Read32();
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
      
      currentitem = new LocMapItem(current_probeset_name,current_n_probes,current_n_probes,PMLoc,MMLoc); 
      cdflocs->Insert(currentitem);
    }
  } else if (versionnumber ==2){
    header->probeset_names.Alloc(header->n_probesets);

    for (i =0; i < header->n_probesets; i++){
      current_probeset_name = store.ReadString();
      header->probeset_names.Add(current_probeset_name);   // ProbesetName;
      current_n_probes_pm = store.Read32(); // number of PM probes
      header->n_probes+= current_n_probes_pm;
      //    wxPrintf(_T("%s %d\n"),current_probeset_name.c_str(),current_n_probes_pm); 
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
      currentitem = new LocMapItem(current_probeset_name,current_n_probes_pm,current_n_probes_mm,PMLoc,MMLoc); 
      cdflocs->Insert(currentitem);
    }
  } else if (versionnumber ==3){
	header->probeset_names.Alloc(header->n_probesets);

	for (i =0; i < header->n_probesets; i++){
      current_probeset_name = store.ReadString();
      header->probeset_names.Add(current_probeset_name);   // ProbesetName;
      current_n_probes_pm = store.Read32(); // number of PM probes
      header->n_probes+= current_n_probes_pm;
      //    wxPrintf(_T("%s %d\n"),current_probeset_name.c_str(),current_n_probes_pm); 
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
      currentitem = new LocMapItem(current_probeset_name,current_n_probes_pm,current_n_probes_mm,PMLoc,MMLoc); 
      cdflocs->Insert(currentitem);
    }
  }
}




int is_cdf_RME(const wxString cdf_path){


  wxFileInputStream input(cdf_path);
  wxDataInputStream store(input);
   
  wxString Error;

  wxString filetype;
  wxString current_probeset_name;
  
  filetype = store.ReadString();
  
  if ((filetype.Cmp(_T("CDF"))!=0) && (filetype.Cmp(_T("RMECDF")) != 0)){
    return 0;
  }
  wxPrintf(filetype);


  return 1;
}
