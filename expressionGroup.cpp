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
 ** file: expressionGroup.cpp
 **
 ** Copyright (C) 2003-3007    B. M. Bolstad
 **
 ** aim: An object to hold computed expression measures (and related standard errors if needed)
 **  
 ** by: B. M. Bolstad  <bolstad@stat.berkeley.edu>
 ** 
 ** Created on: Apr 21, 2003
 **
 ** Description:
 **
 ** RMAExpress is a GUI program for generating RMA
 ** expression estimates. It is implemented in c++
 ** and will use the cross-platform toolkit WxWindows
 **
 ** this file implements the interface part of the program.
 **
 **
 ** History
 ** Apr 21, 2003 - Initial version, which amounts to
 **                opening an empty window.
 ** Oct 13, 2004 - add ability to convert to natural scale in output
 ** Apr  2, 2006 - add a binary format output (intended for exporting to
 **                R where it will be read in using a function in RMAExpress)
 **                Added ArrayTypeName (cdf name)
 ** Sept 16, 2006 - fix compile problems on unicode wxWidgets 
 ** Jan 28, 2007 - Add functionality for also including standard error estimates. Copy constructor. Functions to get dimensions.
 ** Feb 4, 2007 - output writeSEtofile
 **
 *****************************************************/

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <math.h>
#include <wx/datstrm.h>

#include "expressionGroup.h"
#include "version_number.h"

expressionGroup::expressionGroup(long n_probesets, long n_arrays, wxArrayString Names,wxString cdfName, bool hasSE){
  
  this->n_probesets = n_probesets;
  this->n_arrays = n_arrays;
  this->ProbesetNames.Alloc(n_probesets);

  this->expressionvals = new double[n_probesets*n_arrays];
  this->ArrayNames = Names;
  this->ArrayTypeName = cdfName;

  if (hasSE){
    this->se_expressionvals = new double[n_probesets*n_arrays];
  } else {
    this->se_expressionvals = NULL;
  }

}


expressionGroup::~expressionGroup(){
  delete [] expressionvals;

  if (se_expressionvals != NULL){
    delete [] se_expressionvals;
  }


}

expressionGroup::expressionGroup(const expressionGroup &e){

  int i;

  this->n_probesets = e.n_probesets;
  this->n_arrays = e.n_arrays;
  this->ProbesetNames = e.ProbesetNames;

  this->expressionvals = new double[n_probesets*n_arrays];

  for (i=0; i < n_probesets*n_arrays; i++)
    this->expressionvals[i] = e.expressionvals[i];


  this->ArrayNames = e.ArrayNames;
  this->ArrayTypeName = e.ArrayTypeName;

  if (e.se_expressionvals != NULL){
    this->se_expressionvals = new double[n_probesets*n_arrays];
    for (i=0; i < n_probesets*n_arrays; i++)
      this->se_expressionvals[i] = e.se_expressionvals[i];


  } else {
    this->se_expressionvals = NULL;
  }




}


double &expressionGroup::operator[](unsigned long i){

  return expressionvals[i];


}



double &expressionGroup::SE(unsigned long i){

  return se_expressionvals[i];

}



void expressionGroup::AddName(const wxString &name){
  
  ProbesetNames.Add(name);


}




void expressionGroup::writetofile(const wxString output_fname, const wxString output_path,const int naturalscale){

  int i, j;

  wxFileName my_output_fname(output_path, output_fname, wxPATH_NATIVE);
  wxFileOutputStream my_output_file_stream(my_output_fname.GetFullPath());

  wxTextOutputStream my_output_file(my_output_file_stream);


  my_output_file << _T("Probesets");
  for (j=0; j < n_arrays; j++){
    my_output_file << _T("\t") << ArrayNames[j];
  }
  my_output_file << _T("\n");

  for (i =0; i < n_probesets; i++){
    my_output_file << ProbesetNames[i];
    for (j =0; j < n_arrays; j++){
      my_output_file  << _T("\t");
      if (naturalscale){
	my_output_file.WriteDouble(pow(2.0,expressionvals[j*n_probesets + i]));
      } else {
	my_output_file.WriteDouble(expressionvals[j*n_probesets + i]);
      }
    }
    my_output_file << _T("\n");
  }

  

}




void expressionGroup::writetobinaryfile(const wxString output_fname, const wxString output_path,const int naturalscale){


  int i, j;

  wxFileName my_output_fname(output_path, output_fname, wxPATH_NATIVE);
  
  wxFileOutputStream output(my_output_fname.GetFullPath());
  wxDataOutputStream store(output);

  
  store.WriteString(_T("RMAExpressionValues")); // Header to Say that these are indeed RMAExpressionValues
  
  store.Write32(1); // version number for this file type
  store.WriteString(version_number);   // version of RMAExpress used to get these expression values
  store.WriteString(ArrayTypeName);          // Name of the cdf file used
  store.Write32(n_arrays); // number of arrays
  store.Write32(n_probesets);

  for (j=0; j < n_arrays; j++){
    store.WriteString(ArrayNames[j]);
  }
  
  for (i=0; i < n_probesets; i++){
    store.WriteString(ProbesetNames[i]);
  }

  for (j=0; j < n_arrays; j++){
    for (i=0; i < n_probesets; i++){
      output.Write(&expressionvals[j*n_probesets+i],sizeof(double));
    }
  }

}

void expressionGroup::writeSEtofile(const wxString output_fname, const wxString output_path){

  int i, j;

  wxFileName my_output_fname(output_path, output_fname, wxPATH_NATIVE);
  wxFileOutputStream my_output_file_stream(my_output_fname.GetFullPath());

  wxTextOutputStream my_output_file(my_output_file_stream);


  my_output_file << _T("Probesets");
  for (j=0; j < n_arrays; j++){
    my_output_file << _T("\t") << ArrayNames[j];
  }
  my_output_file << _T("\n");

  for (i =0; i < n_probesets; i++){
    my_output_file << ProbesetNames[i];
    for (j =0; j < n_arrays; j++){
      my_output_file  << _T("\t");
      my_output_file.WriteDouble(se_expressionvals[j*n_probesets + i]);
    }
    my_output_file << _T("\n");
  }

  

}



int expressionGroup::count_arrays(){
  return n_arrays;
}

int expressionGroup::count_probesets(){
  return n_probesets;
}








wxArrayString expressionGroup::GetArrayNames(){
  return ArrayNames;
}
