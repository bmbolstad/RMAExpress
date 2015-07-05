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



/*****************************************************************************
 **
 ** File ResidualsDataGroup.cpp
 **
 ** Copyright (C) 2004-2008 B. M. Bolstad
 **
 **
 ** Mar 18, 2005 - initial support for BufferedMatrix class
 ** Mar 19, 2005 - column wise processing in instantiation if BufferedMatrix
 ** Mar 21, 2005 - change intensitydata to be a pointer. Added a ResizeBuffer method
 ** Sept 16, 2006 - Fix possible compile problems with Unicode wxWidget builds
 ** Feb 28, 2008 - BufferedMatrix indexing is now via() operator rather than []
 **
 *****************************************************************************/

#include <vector>

#include <wx/wx.h>
#include "CDFLocMapTree.h"
#include "DataGroup.h"
#include "PMProbeBatch.h"
#include <iostream>
#include "Storage/Matrix.h"
#include "PreferencesDialog.h"

#include "ResidualsDataGroup.h"

 
using namespace std;


ResidualsDataGroup::ResidualsDataGroup(PMProbeBatch *residuals, DataGroup *originaldata, Preferences *preferences){

  bool done=false;
  int i,j,k,l;
  wxArrayString ProbeNames;
  wxString CurrentName;
  LocMapItem *CurrentLocMapItem;

  int *CurrentMMLocs;
  int *CurrentPMLocs;

  array_rows = originaldata->nrows();
  array_cols = originaldata->ncols();
  n_probesets = originaldata->count_probesets();
  n_arrays = originaldata->count_arrays();
  n_probes = originaldata->count_pm();


  cdflocs = originaldata->GiveLocMapTree();

  
  ArrayNames = originaldata->GetArrayNames();
  probeset_names = originaldata->GiveNames();
  ArrayTypeName = originaldata->GetArrayTypeName();
#ifdef BUFFERED
  wxString tempFullPath = preferences->GetFullFilePath();
  const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(tempFullPath);
  const char *tmp_str = (const char*) tmp_buf;
  
  intensitydata = new BufferedMatrix(preferences->GetProbesBufSize(),preferences->GetArrayBufSize(),(char *)tmp_str);
#else
  intensitydata = new Matrix();
#endif

#if RMA_GUI_APP
  wxProgressDialog ResidualProgress(_T("Finding Residuals"),_T("Storing"),n_arrays,NULL,wxPD_AUTO_HIDE| wxPD_APP_MODAL);
#endif


  vector<double> buffer(n_arrays);
  
  intensitydata->SetRows(array_rows*array_cols);

  for (i =0; i < n_arrays; i++){
    intensitydata->AddColumn();
  }   

#ifndef BUFFERED 
  // Go through PMProbeBatch row by row.
  // Get name of probeset, look up probeset in cdflocs
  // then set 
  ProbeNames = residuals->GetRowNames();
  
  i = 0;
  while (!done){
    CurrentName = ProbeNames[i];
    
  

    CurrentLocMapItem = cdflocs->Find(CurrentName);
    
    CurrentPMLocs = CurrentLocMapItem->GetPMLocs();
    CurrentMMLocs = CurrentLocMapItem->GetMMLocs();

    for (j =0; j < CurrentLocMapItem->GetSize(); j++){
      residuals->GetRow(buffer,i+j);
      for (k = 0; k <  n_arrays; k++){
	(*intensitydata)[k*(array_rows*array_cols) + CurrentPMLocs[j]] = buffer[k];
	if (CurrentMMLocs != NULL){
	 (*intensitydata)[k*(array_rows*array_cols) + CurrentMMLocs[j]] = buffer[k];
	}
      }
    } 

    i = i+ CurrentLocMapItem->GetSize();
    if (i >= n_probes)
      done = true;
  }
#else
  // Go through PMProbeBatch column by column.

  ProbeNames = residuals->GetRowNames();
  
  vector<int> PMLocations(n_probes);
  vector<int> MMLocations(n_probes);
  
  l=0;
  for (i=0; i < n_probesets; i++){
    CurrentLocMapItem = cdflocs->Find(probeset_names[i]);
    CurrentPMLocs = CurrentLocMapItem->GetPMLocs();
    CurrentMMLocs = CurrentLocMapItem->GetMMLocs();
    for (j =0; j < CurrentLocMapItem->GetPMSize(); j++){
      PMLocations[l+j] =  CurrentPMLocs[j];
    }
    

    if (CurrentLocMapItem->GetMMSize() == CurrentLocMapItem->GetPMSize()){
      for (j =0; j < CurrentLocMapItem->GetMMSize(); j++){
		MMLocations[l+j] =  CurrentMMLocs[j];
      }
    } else {
      for (j =0; j < CurrentLocMapItem->GetPMSize(); j++){
		MMLocations[l+j] = -1;
      }
    }

    l = l + CurrentLocMapItem->GetPMSize();

  }

  for (k=0; k < n_arrays; k++){
    for (i=0; i < n_probes; i++){
      residuals->GetValue(&buffer[0],i,k);
      (*intensitydata)(PMLocations[i],k) = buffer[0];
      if (MMLocations[i] != -1){
		(*intensitydata)(MMLocations[i],k) = buffer[0];
      }

    }   


    

#if RMA_GUI_APP
    if (k%2 == 0){
      ResidualProgress.Update(k);
    }
#endif
  }
  intensitydata->ReadOnlyMode(true);


#endif
  //cdflocs = NULL;

 

}



#ifndef BUFFERED
Matrix *ResidualsDataGroup::GetIntensities(){
#else
BufferedMatrix *ResidualsDataGroup::GetIntensities(){  
#endif

  return &(*intensitydata);

}






/******************************************************
 **
 ** long DataGroup::nrows()
 **
 **
 **
 **
 **
 ******************************************************/

long ResidualsDataGroup::nrows(){
  return array_rows;
  
}
 

/******************************************************
 **
 ** long DataGroup::ncols()
 **
 **
 **
 **
 **
 ******************************************************/

long ResidualsDataGroup::ncols(){
  return array_cols;

}



wxArrayString ResidualsDataGroup::GetArrayNames(){

  return ArrayNames;


}
 


void ResidualsDataGroup::ResizeBuffer(int rows, int cols){
#ifdef BUFFERED
  intensitydata->ResizeBuffer(rows, cols);
#endif
}



ResidualsDataGroup::~ResidualsDataGroup(){

   delete intensitydata;
}
