/*****************************************************
 **
 ** file: PMProbeBatch.cpp
 **
 ** Copyright (C) 2003-2006   B. M. Bolstad
 **
 ** aim: A DataGroup is a collection of related PM intensities. 
 **      and appropriate indexing information.
 **
 ** Created by: B. M. Bolstad  <bolstad@stat.berkeley.edu>
 ** 
 ** Created on: Apr 20, 2003
 **
 ** Notes: Some code based on code in AffyExtensions
 **   from 2002-2003
 **
 ** History
 ** Apr 20 Initial version
 ** Apr 22 Background adjustment implementation
 ** Oct 31 introduce low memory flag for normalization step
 ** June 24, 2004 - adjust so compiles without GUI
 ** Nov 9, 2004 - change core processing routine to better
 **               deal with 1-probe probesets
 ** Mar 19, 2005 - Added a method for getting back a single cell
 **                Class constructor can deal more efficiently with the buffered matrix datagroup
 ** Mar 23, 2005 - Add Message Dialogs when there is a GUI (so user is kept slightly more updated)
 ** Oct 1, 2005 - change some preprocessor directives
 ** Mar 29, 2006 - changes for dealing with CDFLocMap structure
 ** Sept 16, 2006 - fix compile problems with Unicode builds ow wxWidgets
 ** Jan 6, 2007 - add Commpute5Summary
 ** Jan 27, 2007 - add summarize_PLM() method
 **
 *****************************************************/

#include <wx/wx.h>
#include <cstdlib>
#include <vector>

#include "DataGroup.h"
#include "PMProbeBatch.h"
#include "CDFLocMapTree.h"
#include "Preprocess/qnorm.h"
#include "expressionGroup.h"
#include "Preprocess/medianpolish.h"
#include "Preprocess/rma_background3.h"
#include "Preprocess/rlm_anova.h"

#include "Storage/BufferedMatrix.h"
//#include <iostream.h>
//#define DEBUG 1 
#define DEBUG_PLM 1

using namespace std;


PMProbeBatch::PMProbeBatch(DataGroup &x,Preferences *preferences){
  
  int i; // iterator on probesets
  int j; // iterator on locations
  int k; // iterator on arrays
  int x_length;
  int current_row=0;

  wxString current_name;
  int current_n_probes=0;
  int *current_PMLocs;
  wxArrayString probeset_names = x.GiveNames();
  LocMapItem *current_item;

  int n_remove = 0;


  n_probes = x.count_pm();
  n_arrays = x.count_arrays();
  n_probesets = x.count_probesets();
  
  ProbesetRowNames.Alloc(n_probes);

  x_length = x.nrows()*x.ncols();

#ifdef BUFFERED
  wxString tempFullPath = preferences->GetFullFilePath();
  const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(tempFullPath);
  const char *tmp_str = (const char*) tmp_buf;
  
  intensity = new BufferedMatrix(preferences->GetProbesBufSize(),preferences->GetArrayBufSize(),(char *)tmp_str);
  intensity->SetRows(n_probes);
  for (k=0; k < n_arrays;k++){
		intensity->AddColumn();
  }
#else
  intensity = new double[n_probes*n_arrays];
#endif


  if (intensity == NULL){
#if RMA_GUI_APP
    wxString t= _T("Failed to allocate adequate memory. You need more RAM and swap space."); 
    wxMessageDialog
      aboutDialog
      (0, t, _T("Memory problem"), wxOK);
    aboutDialog.ShowModal();
    
    return;
#else
    return;
#endif
  }
#ifndef BUFFERED
  for (i =0; i < n_probesets; i++){
   
    current_item =  x.FindLocMapItem(probeset_names[i]);
    current_name = current_item->GetName();
    current_n_probes = current_item->GetPMSize();
    current_PMLocs = current_item->GetPMLocs(); 
    //#if DEBUG
    //wxPrintf(current_name+"\n");
    //#endif
    for (j =0; j < current_n_probes; j++){
      for (k =0; k < n_arrays; k++){
		intensity[k*n_probes + current_row] = x[k*x_length + current_PMLocs[j]];
      }
      current_row++;
    }
    ProbesetRowNames.Add(current_name,current_n_probes);
  }
#else

#if RMA_GUI_APP  
  wxProgressDialog  InitializeProgress(_T("Preparing"),_T("Preparing Data Structures"),n_arrays,NULL,wxPD_AUTO_HIDE| wxPD_APP_MODAL);
#endif
  int l = 0;

  for (i =0; i < n_probesets; i++){ 
    current_item =  x.FindLocMapItem(probeset_names[i]);
    current_name = current_item->GetName();
    current_n_probes = current_item->GetPMSize();
    ProbesetRowNames.Add(current_name,current_n_probes);
#ifdef DEBUG
    wxPrintf(current_name+_T(" this_p: %d  this_ps: %d tot_ps: %d   tot_p:%d    lofn: %d\n"),current_n_probes,i, n_probesets,n_probes,ProbesetRowNames.GetCount());
#endif
  }
  
  // Make a vector containing indices of PM probes
  std::vector<int> PMLocations(ProbesetRowNames.GetCount()); 
  for (i =0; i < n_probesets; i++){
    current_item =  x.FindLocMapItem(probeset_names[i]);
    current_n_probes = current_item->GetPMSize();
    current_PMLocs = current_item->GetPMLocs(); 
    for (j =0; j < current_n_probes; j++){
      PMLocations[l + j] = current_PMLocs[j];
    }
    l = l +  current_n_probes;
    if (current_n_probes == 0){
      n_remove++;
    }
  }
#ifdef DEBUG
  wxPrintf(_T("ps: %d   p:%d\n"),n_probesets,n_probes);
#endif

  n_probesets = n_probesets - n_remove;
#ifdef DEBUG
  wxPrintf(_T("ps: %d   p:%d    lofn: %d\n"),n_probesets,n_probes,ProbesetRowNames.GetCount());
#endif

  for (k =0; k < n_arrays; k++){
    //intensity->AddColumn();
    for (current_row=0; current_row < l; current_row++){
      (*intensity)[k*n_probes + current_row] =  x(PMLocations[current_row],k);
    }
#if RMA_GUI_APP
    InitializeProgress.Update(k);
#endif    
  }
  


#endif
  ArrayNames = x.GetArrayNames();

  ArrayTypeName = x.GetArrayTypeName();

}






PMProbeBatch::~PMProbeBatch(){
#ifdef BUFFERED
  delete intensity;
#else
  delete[] intensity;
#endif

}

void PMProbeBatch::normalize(bool lowmem){
  
  int lowmemflag = (int)lowmem;
  int nprobes = (int)n_probes;
  int narrs = (int)n_arrays;

  if (qnorm_c(intensity, &nprobes, &narrs, &lowmemflag)){
    wxString t=_T("Failed to allocate adequate memory in normalization step. You may need more RAM and swap space."); 
#if RMA_GUI_APP
    wxMessageDialog
      aboutDialog
      (0, t, _T("Memory problem"), wxOK);
    aboutDialog.ShowModal();
#endif
  }
  
}

void PMProbeBatch::background_adjust(){

  int j=0;
  double param[3];
#if RMA_GUI_APP
  wxProgressDialog BackGroundProgress(_T("Background Adjusting"),_T("Background Adjusting"),n_arrays,NULL,wxPD_AUTO_HIDE| wxPD_APP_MODAL);
#endif

#ifdef BUFFERED
  intensity->ReadOnlyMode(true);
#endif


  for (j = n_arrays-1; j >=0; j--){
    bg_parameters2(intensity,intensity,param,n_probes,n_arrays,j);
    bg_adjust(intensity,intensity,param,n_probes,n_arrays,j);
#if RMA_GUI_APP
    if (j%2 == 0){
      BackGroundProgress.Update(n_arrays - j);
    }
#endif
#ifdef BUFFERED
  intensity->ReadOnlyMode(false);
#endif

  }
}



expressionGroup *PMProbeBatch::summarize(){

  expressionGroup *myexprs = new expressionGroup(n_probesets, n_arrays,ArrayNames,ArrayTypeName[0],false);

  long i=0,j=0,k=0;

  wxString CurrentName;

  int first_ind;
  int max_nrows = 1000;
  /* buffers of size 1000 should be enough. */

  vector<int> cur_rows(max_nrows);
  int cur_nprobes=0;
  vector<double> cur_exprs(n_arrays);

#if RMA_GUI_APP
  wxProgressDialog SummarizeProgress(_T("Summarization"),_T("Summarization"),n_probesets,NULL,wxPD_AUTO_HIDE| wxPD_APP_MODAL);
#endif

#ifdef BUFFERED
  intensity->RowMode();
#endif

  CurrentName = ProbesetRowNames.Item(0);
  i = 0;     /* indexes current probeset */
  j = 0;    /* indexes current row in PM matrix */
  k = 0;    /* indexes current probe in probeset */
  while ( j < n_probes){
    if (CurrentName.Cmp(ProbesetRowNames[j]) == 0){
      if (k >= max_nrows){
		max_nrows = 2*max_nrows;
		cur_rows.resize(max_nrows); 
      }
      cur_rows[k] = j;
      k++;
      j++;
      
    } else {
      cur_nprobes = k;
#ifdef DEBUG
      wxPrintf(CurrentName+_T("\n"));
#endif      
#ifdef BUFFERED
      median_polish(intensity, n_probes, n_arrays, &cur_rows[0], &cur_exprs[0], cur_nprobes);
#else
      median_polish(intensity, n_probes, n_arrays, cur_rows, cur_exprs, cur_nprobes);
#endif
      for (k =0; k < n_arrays; k++){
		(*myexprs)[k*n_probesets + i] = cur_exprs[k];
      } 
       myexprs->AddName(CurrentName);
      CurrentName = ProbesetRowNames[j];
      i++;
      k = 0;
    }
#if RMA_GUI_APP
    if (i%1000 == 0){
      SummarizeProgress.Update(i);
    }
#endif
  }
  cur_nprobes = k;
#ifdef DEBUG
  wxPrintf(CurrentName+_T("\n"));
#endif      
#ifdef BUFFERED
  median_polish(intensity, n_probes, n_arrays, &cur_rows[0], &cur_exprs[0], cur_nprobes);
#else
  median_polish(intensity, n_probes, n_arrays, cur_rows, cur_exprs, cur_nprobes);
#endif

  for (k =0; k < n_arrays; k++){
    (*myexprs)[k*n_probesets + i] = cur_exprs[k];
  } 
  myexprs->AddName(CurrentName);
  
 
#ifdef BUFFERED
  intensity->ColMode();
#endif
  return myexprs;

}






expressionGroup *PMProbeBatch::summarize_PLM(){

  expressionGroup *myexprs = new expressionGroup(n_probesets, n_arrays,ArrayNames,ArrayTypeName[0],true);

  long i=0,j=0,k=0;

  wxString CurrentName;

  int first_ind;
  int max_nrows = 1000;
  /* buffers of size 1000 should be enough. */

  vector<int> cur_rows(max_nrows);
  int cur_nprobes=0;
  
  vector<double> cur_exprs(n_arrays);
  vector<double> cur_se_exprs(n_arrays);


#if RMA_GUI_APP
  wxProgressDialog SummarizeProgress(_T("Summarization"),_T("Summarization"),n_probesets,NULL,wxPD_AUTO_HIDE| wxPD_APP_MODAL);
#endif

#ifdef BUFFERED
  intensity->RowMode();
#endif

  CurrentName = ProbesetRowNames.Item(0);
  i = 0;     /* indexes current probeset */
  j = 0;    /* indexes current row in PM matrix */
  k = 0;    /* indexes current probe in probeset */
  while ( j < n_probes){
    if (CurrentName.Cmp(ProbesetRowNames[j]) == 0){
      if (k >= max_nrows){
		max_nrows = 2*max_nrows;
		cur_rows.resize(max_nrows);
	  }
      cur_rows[k] = j;
      k++;
      j++;
      
    } else {
      cur_nprobes = k;
      //#ifdef DEBUG_PLM
      ///if (cur_nprobes ==1){
      //wxPrintf(CurrentName+wxT(" %d\t %d \t %d \t %d\n"),  cur_nprobes,i,j,n_probes);
      //}
      //#endif      
#ifdef BUFFERED
	if (cur_nprobes < 500){
		PLM_summarize(intensity, n_probes, n_arrays, &cur_rows[0], &cur_exprs[0], &cur_se_exprs[0], cur_nprobes);
	} else {
		median_polish(intensity, n_probes, n_arrays, &cur_rows[0], &cur_exprs[0], cur_nprobes);
	   for (k =0; k < n_arrays; k++){
	     cur_se_exprs[k] = 1.0;
	   }
	}

	//Ugly Hackery. TO BE FIXED
	if (cur_nprobes == 1){
	  for (k =0; k < n_arrays; k++){
	    cur_se_exprs[k] = -1.0;  // Really should be NA
	  }	

	}
	

#endif
      


      for (k =0; k < n_arrays; k++){
		(*myexprs)[k*n_probesets + i] = cur_exprs[k];
		myexprs->SE(k*n_probesets + i) = cur_se_exprs[k];
      } 
       myexprs->AddName(CurrentName);
      CurrentName = ProbesetRowNames[j];
      i++;
      k = 0;
    }
#if RMA_GUI_APP
    if (i%1000 == 0){
      SummarizeProgress.Update(i);
    }
#endif
  }
  cur_nprobes = k;
  //#ifdef DEBUG_PLM
  //wxPrintf(CurrentName+wxT("\n"));
  //#endif      
#ifdef BUFFERED
    if (cur_nprobes < 1000){
		PLM_summarize(intensity, n_probes, n_arrays, &cur_rows[0], &cur_exprs[0], &cur_se_exprs[0], cur_nprobes);
    } else {
		median_polish(intensity, n_probes, n_arrays, &cur_rows[0], &cur_exprs[0], cur_nprobes);
      for (k =0; k < n_arrays; k++){
	cur_se_exprs[k] = 1.0;
      }
    }	
    if (cur_nprobes == 1){
      for (k =0; k < n_arrays; k++){
		cur_se_exprs[k] = 1.0;
      }
    }
#endif

  for (k =0; k < n_arrays; k++){
    (*myexprs)[k*n_probesets + i] = cur_exprs[k];
    myexprs->SE(k*n_probesets + i) = cur_se_exprs[k];
  } 
  myexprs->AddName(CurrentName);
  
#ifdef BUFFERED
  intensity->ColMode();
#endif
  return myexprs;

}





















wxArrayString PMProbeBatch::GetArrayNames(){
  return ArrayNames;
}

wxArrayString PMProbeBatch::GetRowNames(){
  return ProbesetRowNames;
}


void PMProbeBatch:: GetRow(double *buffer, int row){

  int i;

  for (i=0; i < n_arrays; i++){
#ifdef BUFFERED
    buffer[i] = (*intensity)[i*n_probes + row];
#else
    buffer[i] = intensity[i*n_probes + row]; 
#endif
  }


}



void PMProbeBatch::GetValue(double *buffer, int row, int col){

  int i;

  //for (i=0; i < n_arrays; i++){
#ifdef BUFFERED
    buffer[0] = (*intensity)[col*n_probes + row];
#else
    buffer[0] = intensity[col*n_probes + row];
#endif
    //}


}








long PMProbeBatch::count_pm(){
  return n_probes;
}


long PMProbeBatch::count_probesets(){
  return n_probesets;
}


long PMProbeBatch::count_arrays(){
  return n_arrays;
}




void  PMProbeBatch::Compute5Summary(int col, double *results){

  intensity->Compute5Summary(col,results);

}
