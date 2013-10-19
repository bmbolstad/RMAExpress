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
 ** file: DataGroup.cpp
 **
 ** Copyright (C) 2003-2008    B. M. Bolstad
 **
 ** aim: A DataGroup is a collection of related cel intensities. 
 **      and appropriate indexing information.
 **
 ** Created by: B. M. Bolstad  <bolstad@stat.berkeley.edu>
 ** 
 ** Created on: Apr 16, 2003
 **
 ** Description:
 ** 
 ** A DataGroup is the basic raw data structure for this
 ** program. It stores the equivalent of 1 cdf and many cel
 ** files altogether into a single data structure.
 ** Basically all the data you should need to analyze a single
 ** experiment
 **
 ** Things we want to do with a DataGroup
 ** Instantiate/Delete (must instantiate with at least CDF information)
 ** Add or Remove CEL file data
 ** Access data (by probe, by chip, CDF information)
 ** Write information into a binary processed format
 **
 **
 ** History
 ** Apr 16 Initial version
 ** Apr 17-20   Build up Parsing of input files
 ** Apr 21      Add in an array of probeset names.
 ** Apr 22      Remove the "allocated" and "is" dialog boxes
 ** Apr 24 Have a parent pointer so can use modal progress bars.
 ** Apr 30 Investigate behaviour of hgu133b cdf files which seem to
 **        crash at end of input routine.
 ** May 20 Fix/Fill in comments. Make memory allocation
 **        more secure.
 ** Jun 29 Make the progress bars update less frequently.
 **        This is in hopes it speeds things up a little more.
 ** Jul 20 Cross reference CDF names against CEL files.
 **        Binary output routine.
 ** Jul 21 More work on binary output routine, binary input routines
 **        Can create a DataGroup from binary files.
 ** Aug 11 convert intensitydata from *double to *Matrix
 **        more documentation/comments
 ** Sep 12 New constructors       
 **        A few modifications to allow WriteBinary to
 **        output only CDF or CEL information (if that is all
 **        that exists)
 **        ReadCDFFile now "throws" its errors
 ** Sep 18 ReadCDFFile now expects the fullpath with filename
 ** Oct  8 Use buffered filestreams for a little more speed.
 **        some optimizations in the parse functions
 ** Oct 18 Binary cel file support
 ** Oct 19 More work on binary cel file support. make cdfName
 **        default to name of the cdf file (with the .CDF removed)
 ** Oct 30 Clean up code. Note that the binary code is non
 **        portable outside of 32 bit ix86 machines.
 ** Nov 16 Fix binary CEL file support
 ** Jan 25, 2004 Add GiveLocMapTree()
 ** May 6, 2004 Fix Destructor
 ** June 24, 2004 - add stuff so works with console application
 ** July 7, 2004 - can read PM only chips
 ** Oct 13, 2004 - deal better with sense transcript chips
 ** Oct 15, 2004 - small memory leak fixes
 ** Feb 19, 2005 - Binary cdf file support added.
 ** Mar 18-19, 2005 - Preliminary work for BufferedMatrix
 ** Mar 21, 2005 - change how progress bars are shown in cel file reading code
 **                Converted intensitymatrix to *intensitymatrix
 ** May 31, 2005 - remove case comparison problem
 ** Aug 23, 2005 - WriteBinary (the RMAExpress internal format) fixed for dealing with PM only chips
 ** Oct 1, 2005 - changed some preprocessor directives
 ** Mar 27-29, 2006 - changes so that probesets (or more strictly speaking Unit_blocks) can ahve only MM
 ** Jul 24, 2006 - a truncation check for binary cel files.
 ** Aug 3, 2006 - fix problem with not being able to read beyond a certain number of binary cel files.
 ** Aug 31, 2006 - Fix a problem with modal dialog box not being properly closed in error situation
 ** Sept 16, 2006 - fix problems with strings due to unicode (mostly means wrap with _T())
 ** Oct 15, 2007 - restructure cel file parsing code
 ** Oct 16, 2007 - continued cleaning of the parsing code. 
 ** Dec 15, 2007 - Binary RME CDF parsing code moved to its own separate file
 ** Jan 5, 2008  - member variable ArrayTypeName changed to wxArrayString
 ** Jan 6, 2008  - separate writeBinary into separate functions for each of CDF and CEL
 ** Jan 7, 2008  - CDF_desc strings added
 ** Jan 14-17, 2008 - RME format CDF and CEL files are now parsed/read in like any other format (ie no special reading in)
 ** Feb 2, 2008 - fix isRMECEL. Was crashing on XP, 2000 
 ** Feb 28, 2008 - BufferedMatrix indexing is now via() operator rather than []
 ** Mar 6, 2008 - Refine parsing error detection
 ** May 16, 2009 - Repair binary CDF file parsing
 **
 *****************************************************/

#include "DataGroup.h"
#include "Parsing/read_cdf_xda.h"


#include <wx/wx.h>
#include <wx/progdlg.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>
#include <wx/datstrm.h>
#include <math.h>


#include "Parsing/read_celfile_generic.h"
#include "Parsing/read_celfile_xda.h"
#include "Parsing/read_celfile_text.h"
#include "Parsing/read_rme_cdf.h"
#include "Parsing/read_cel_structures.h"
#include "version_number.h"

#define BUF_SIZE 1024
#define DEBUG 0 

#if DEBUG
#include <iostream.h>
#endif



wxString parsingErrorCode(int err_code){

  wxString ErrorString;

  if (err_code == BINARY_INTENSITY_CORRUPTED){
    ErrorString = wxT("xda format file - intensity value is corrupted or otherwise outside expected range");
  } else if (err_code == BINARY_INTENSITY_TRUNCATED){
    ErrorString = wxT("xda format file - intensity section appears to be truncated");
  } else if (err_code == BINARY_HEADER_CDF_NAME_MISSING){
    ErrorString = wxT("xda format file - could not seem to find CDF information in header"); 
  } else if (err_code == BINARY_HEADER_TRUNCATED){
    ErrorString = wxT("text format file - header section truncated or missing expected information"); 
  } else if (err_code == TEXT_INTENSITY_CORRUPTED){
    ErrorString = wxT("text format file - [INTENSITY] section has invalid or corrupt data values");
  } else if (err_code == TEXT_INTENSITY_TRUNCATED){
    ErrorString = wxT("text format file - [INTENSITY] section appears to be truncated");
  } else if (err_code == TEXT_HEADER_CDF_NAME_MISSING){
    ErrorString = wxT("text format file - [HEADER] section does not seem to have CDF information");
  } else if (err_code == TEXT_HEADER_TRUNCATED){
    ErrorString = wxT("text format file - [HEADER] section seems to be truncated"); 
  } else if (err_code == TEXT_MISSING_LINE){
    ErrorString = wxT("text format file - missing line found where one expected");
  } else if (err_code == TEXT_DID_NOT_FIND_STARTS_WITH){
    ErrorString = wxT("text format file - did not find line starting with specified string");
  } else if (err_code == TEXT_DID_NOT_FIND_SECTION){
    ErrorString = wxT("text format file - did not find specified section");
  } else if (err_code == TEXT_DID_NOT_FIND_HEADER_SECTION){
    ErrorString = wxT("text format file - did not find [HEADER] section");
  } else if (err_code == TEXT_DID_NOT_FIND_COL_DIMENSION){
    ErrorString = wxT("text format file - did not find Cols= in [HEADER]");
  } else if (err_code == TEXT_DID_NOT_FIND_ROW_DIMENSION){
    ErrorString = wxT("text format file - did not find Rows= in [HEADER]");
  } else if (err_code == TEXT_DID_NOT_FIND_DAT_HEADER){
    ErrorString = wxT("text format file - did not find DatHeader= in [HEADER]");
  } else if (err_code == TEXT_DID_NOT_FIND_INTENSITY_SECTION){
    ErrorString = wxT("text format file - did not find [INTENSITY] section");
  } else if (err_code == TEXT_DID_NOT_FIND_CELLHEADER){
    ErrorString = wxT("text format file - did not find CellHeader= in [INTENSITY] section");
  } else if (err_code == TEXT_DID_NOT_MISC_HEADER_FIELD){
    ErrorString = wxT("text format file - did not find an expected field in [HEADER] section");
  } else if (err_code == CALVIN_HEADER_TRUNCATED){
    ErrorString = wxT("command console generic file - header truncated");
  } else if (err_code == CALVIN_HEADER_CDF_NAME_MISSING){
    ErrorString = wxT("command console generic file - could not seem to find CDF information in header");
  } else if (err_code == CALVIN_HEADER_FILE_HEADER_TRUNCATED){
    ErrorString = wxT("command console generic file - file header section truncated or otherwise missing expected information");
  } else if (err_code == CALVIN_HEADER_DATA_HEADER_TRUNCATED){
    ErrorString = wxT("command console generic file - data header section truncated or otherwise missing expected information");
  }  else if (err_code == CALVIN_HEADER_DATA_GROUP_TRUNCATED){
    ErrorString = wxT("command console generic file - data group section truncated or otherwise missing expected information");
  } else if (err_code == CALVIN_HEADER_DATA_SET_TRUNCATED){
    ErrorString = wxT("command console generic file - data set section truncated or otherwise missing expected information");
  } else if (err_code == CALVIN_DID_NOT_FIND_COL_DIMENSION){
    ErrorString = wxT("command console generic file - did not find cols information");
  } else if (err_code == CALVIN_DID_NOT_FIND_ROW_DIMENSION){
    ErrorString = wxT("command console generic file - did not find rows information");
  } else if (err_code == CEL_NON_MATCHING_DIMENSIONS){
    ErrorString = wxT("cel file does not seem to have dimensions matching those of previously seen files");
  } else if (err_code == CEL_NON_MATCHING_CDFNAMES){
    ErrorString = wxT("cel file does not seem to have same CDF as those of previously seen files");
  } else {
	  ErrorString = wxT("unknown parsing problem ");// + err_code;
  }



  return ErrorString;

}


/******************************************************
 **
 ** bool StopAtNextOccurance(wxTextInputStream *in_stream, 
 **                          wxString *lineBuffer, 
 **                          wxString searchstring)
 **
 ** wxTextInputStream *in_stream
 ** wxString *lineBuffer
 ** wxString searchstring
 **
 ******************************************************/

bool StopAtNextOccurance(wxTextInputStream *in_stream, wxString *lineBuffer, wxString searchstring, wxFFileInputStream &stream){
  *lineBuffer = (wxChar *)"";
  while ((lineBuffer)->Contains(searchstring) == 0){
    if (stream.Eof()){
      return false;
    }
    *lineBuffer = in_stream->ReadLine();   
  }
  
  return true;

} 


/******************************************************
 **
 ** bool parse_CDF_CEL_LINE(wxString LineBuffer, 
 **                         int *xloc,int *yloc, 
 **                         bool *isPM)
 **
 ** wxString LineBuffer
 ** int *xloc,int *yloc
 ** bool *isPM
 **
 **
 ******************************************************/


void parse_CDF_CEL_LINE(wxString &LineBuffer, int *xloc,int *yloc, bool *isPM){
  wxString CurrentToken;
  wxString Pbase;
  wxString Tbase;

  wxStringTokenizer tkz(LineBuffer,_T("=\t\n"));
  
  long tempbuffer;

  // first element is a Cell* so skip
  tkz.GetNextToken();

  CurrentToken = tkz.GetNextToken();
  
  //CurrentToken.ToLong((long int*)xloc,10);
  CurrentToken.ToLong(&tempbuffer,10);
  *xloc = (int)tempbuffer;

  CurrentToken = tkz.GetNextToken();
  //CurrentToken.ToLong((long int*)yloc,10);
  CurrentToken.ToLong(&tempbuffer,10);
  *yloc = (int)tempbuffer;
  
  // go to ninth position
  for (int skip=2; skip <9; skip++)
    CurrentToken = tkz.GetNextToken();
 
  Pbase = CurrentToken;
  Tbase = tkz.GetNextToken();
  
  if (Pbase.Cmp(Tbase) == 0){
    *isPM = false;
  } else if (((Pbase.Cmp(_T("A"))== 0) && (Tbase.Cmp(_T("T")) != 0)) || ((Pbase.Cmp(_T("T"))== 0) && (Tbase.Cmp(_T("A")) != 0))){
    *isPM = false;
  } else if (((Pbase.Cmp(_T("C"))== 0) && (Tbase.Cmp(_T("G")) != 0)) || ((Pbase.Cmp(_T("G"))== 0) && (Tbase.Cmp(_T("C")) != 0))){
    *isPM = false;
  } else {
    *isPM = true;
  }
}

 
/******************************************************
 **
 ** int xy2i(int xloc, int yloc, int sidelength)
 **
 ** int xloc
 ** int yloc
 ** int sidelength
 **
 **
 ******************************************************/

static int xy2i(int xloc, int yloc, int sidelength){

  return yloc*sidelength + xloc;

}




int is_cdf_text(const wxString cdf_path){
  wxString LineBuffer;
  wxFFileInputStream my_CDF_file_stream(cdf_path);
  wxTextInputStream my_CDF_file(my_CDF_file_stream);
  LineBuffer.Alloc(1000);    
  LineBuffer = my_CDF_file.ReadLine();
  
  if (LineBuffer.StartsWith(_T("[CDF]")) != true){
    return 0;
  } 
  return 1;
  

}



/******************************************************
 **
 ** bool DataGroup::ReadCDFFile(const wxString cdf_fname, 
 **                             const wxString cdf_path)
 **
 ** const wxString cdf_fname - just the filename
 ** const wxString cdf_path - full path including filename
 **                           
 **
 **
 ******************************************************/

void DataGroup::ReadCDFFile(const wxString cdf_fname, 
			    const wxString cdf_path){
  int i,j,k;


  int *PMLoc;
  int *MMLoc;

  LocMapItem *currentitem;
  wxString currentName;
  
  if (is_cdf_xda((char *)cdf_path.char_str())){ 

    cdf_xda my_cdf;
    int cur_blocks,cur_cells, cur_atoms;
    cdf_unit_cell *current_cell;



    if (!read_cdf_xda((char *)cdf_path.char_str(),&my_cdf)){
      dealloc_cdf_xda(&my_cdf);
      wxString Error = _T("Problem reading binary cdf file ") + cdf_fname + _T(". Possibly corrupted or truncated?\n");
      throw Error;
    }
    // Use the name of the cdf file as the ArrayTypeName
    ArrayTypeName.Add(cdf_fname.Mid(0, cdf_fname.length()-4));

    
    array_rows = (int )my_cdf.header.rows;
    array_cols = (int )my_cdf.header.cols;
    n_probesets  = my_cdf.header.n_units;
#if RMA_GUI_APP
    wxProgressDialog CDFProgress(_T("CDF Progress"),_T("Reading in CDF file"),n_probesets,this->parent,wxPD_AUTO_HIDE);
#endif

    for (i=0; i < my_cdf.header.n_units; i++){
      cur_blocks = my_cdf.units[i].nblocks;

      if (my_cdf.units[i].unittype ==1){
	/* Expression analysis */
	for (j=0; j < cur_blocks; j++){

	  cur_cells = my_cdf.units[i].unit_block[j].ncells;
	  cur_atoms = my_cdf.units[i].unit_block[j].natoms;
	  n_probes+=cur_atoms;
	  currentName =  wxString(my_cdf.units[i].unit_block[j].blockname,wxConvUTF8);
	  
	  PMLoc = new int[cur_atoms];
	
	  if (cur_cells == 2*cur_atoms){
	    MMLoc = new int[cur_atoms];
	  } else {
	    MMLoc = NULL;
	  }
	  
	  for (k=0; k < cur_cells; k++){
	    current_cell = &(my_cdf.units[i].unit_block[j].unit_cells[k]);
	    
	    if(isPM(current_cell->pbase,current_cell->tbase)){
	      PMLoc[current_cell->atomnumber] = current_cell->x + current_cell->y*(my_cdf.header.rows);        
	    } else {
	      MMLoc[current_cell->atomnumber] = current_cell->x + current_cell->y*(my_cdf.header.rows);
	    }
	  }
	  
	  if (MMLoc == NULL){
	    currentitem = new LocMapItem(currentName,cur_atoms,0,PMLoc,MMLoc); 
	  } else {
	    currentitem = new LocMapItem(currentName,cur_atoms,cur_atoms,PMLoc,MMLoc); 
	  }
	  cdflocs.Insert(currentitem);
	  probeset_names.Add(currentName);
	} 
      } else {
	wxString Error = _T("This looks like a cdf file for an non-expression array. Genotyping array? Not handled.\n");
	throw Error;
      }
#if RMA_GUI_APP   
      if (i % (n_probesets/200) == 0){
	CDFProgress.Update(i);
      }
#endif
    }
    currentitem = NULL;
    probeset_names.Sort();
    dealloc_cdf_xda(&my_cdf);
  } else if (is_cdf_text(cdf_path)){

    int n_cells = 0, n_blocks = 0, unit_type=0;
    int xloc, yloc;
    bool isPM;
    int whichPM=0;
    int whichMM=0;
    int current_npp;
    //    int nremoved = 0;

    long tempbuffer;
  
    
    wxString LineBuffer;
    wxString restofline;


    wxFFileInputStream my_CDF_file_stream(cdf_path);
    wxTextInputStream my_CDF_file(my_CDF_file_stream);
    LineBuffer.Alloc(1000);    
    LineBuffer = my_CDF_file.ReadLine();
    
    if (LineBuffer.StartsWith(_T("[CDF]")) != true){
      wxString Error = _T("The file ") + cdf_fname + _T(" doesn't look like a valid CDF file.\n");
      throw Error;
      
    } 
    
    while (LineBuffer.StartsWith(_T("[Chip]")) != true){
      LineBuffer = my_CDF_file.ReadLine();
    }
    
    // readinchip  name
    LineBuffer = my_CDF_file.ReadLine();
    
    //ArrayTypeName = LineBuffer.After('=');
    
    ArrayTypeName.Add(cdf_fname.Mid(0, cdf_fname.length()-4));
    
    
    while (LineBuffer.StartsWith(_T("Rows="),&restofline) != true)
      LineBuffer = my_CDF_file.ReadLine();
    restofline.ToLong(&tempbuffer,10);
    array_rows = (int)tempbuffer;
    
    while (LineBuffer.StartsWith(_T("Cols="),&restofline) != true){
      LineBuffer = my_CDF_file.ReadLine();
#if DEBUG
      wxPrintf(LineBuffer+" \n", i);
#endif
    }
    // restofline.ToLong((long int*)&array_cols,10);
    restofline.ToLong(&tempbuffer,10);
    array_cols = (int)tempbuffer;

    while (LineBuffer.StartsWith(_T("NumberOfUnits="),&restofline) != true)
      LineBuffer = my_CDF_file.ReadLine();
    // restofline.ToLong((long int*)&n_probesets,10);
    restofline.ToLong(&tempbuffer,10);
    n_probesets = (int)tempbuffer;

#if RMA_GUI_APP
    wxProgressDialog CDFProgress(_T("CDF Progress"),_T("Reading in CDF file"),n_probesets,this->parent,wxPD_AUTO_HIDE);
#endif
    
    probeset_names.Alloc(n_probesets);
    
    for (i =0; i < n_probesets; i++){
      // Look for appearance of a line containing "[Unit"

      
      if (!StopAtNextOccurance(&my_CDF_file, &LineBuffer,_T("[Unit"),my_CDF_file_stream)){
	wxString Error =_T("Unexpectely reached the end of this cdf file. Perhaps it is corrupted.\n"); 
    	throw Error;
      }
      
      // Check unit type. 1 is CustomSeq, 2 is genotyping, 3 is expression, 7 is tag/genflex. All but 3 are unsupported 
      
      if (!StopAtNextOccurance(&my_CDF_file, &LineBuffer,_T("UnitType"),my_CDF_file_stream)){
	wxString Error =_T("Unexpectely reached the end of this cdf file. Perhaps it is corrupted.\n"); 
	throw Error;
      }
      
      LineBuffer.StartsWith(_T("UnitType="),&restofline);
      //restofline.ToLong((long int*)&unit_type,10);
      restofline.ToLong(&tempbuffer,10);
      unit_type = (int)tempbuffer;


      if ((unit_type !=3) && (unit_type > 0)){
	wxString Error = _T("The file ") + cdf_fname + _T(" does not appear to be for an expression array. This format not currently handled by RMAExpress.\n");
	throw Error;
      }
      
      // now find number of blocks within unit
      if (!StopAtNextOccurance(&my_CDF_file, &LineBuffer,_T("NumberBlocks"),my_CDF_file_stream)){
	wxString Error = _T("Unexpectely reached the end of this cdf file. Perhaps it is corrupted.\n"); 
	throw Error;
	
      }
      
      LineBuffer.StartsWith(_T("NumberBlocks="),&restofline);
      //restofline.ToLong((long int*)&n_blocks,10);
      restofline.ToLong(&tempbuffer,10);
      n_blocks = (int)tempbuffer;
      for (j = 0; j < n_blocks; j++){
	// Go through each block
	StopAtNextOccurance(&my_CDF_file, &LineBuffer,_T("Name"),my_CDF_file_stream);
	LineBuffer.StartsWith(_T("Name="),&restofline);
	currentName=restofline;
	
	StopAtNextOccurance(&my_CDF_file, &LineBuffer,_T("NumAtoms"),my_CDF_file_stream);
	LineBuffer.StartsWith(_T("NumAtoms="),&restofline);
	restofline.ToLong(&tempbuffer,10);
	current_npp = (int)tempbuffer;
	StopAtNextOccurance(&my_CDF_file, &LineBuffer,_T("NumCells"),my_CDF_file_stream);
	LineBuffer.StartsWith(_T("NumCells="),&restofline);
	restofline.ToLong(&tempbuffer,10);
	n_cells = (int)tempbuffer;
	
	if (current_npp > 0){
	  PMLoc = NULL;
	  MMLoc = NULL;
	
	  if (n_cells == 2*current_npp){ 
	    PMLoc = new int[current_npp];
	    MMLoc = new int[current_npp];
	  } else {
	    PMLoc = new int[n_cells];
	    MMLoc = new int[n_cells];
	  }
	  
	  StopAtNextOccurance(&my_CDF_file, &LineBuffer,_T("CellHeader"),my_CDF_file_stream);
	  //LineBuffer = my_CDF_file.ReadLine();
	  //n_cells = 2*current_npp;
	  
	  //cout << "Ncels :" << n_cells << endl;
	  whichPM=whichMM=0;
	  for (k = 0; k < n_cells; k++){
	    // Go through reading each cell= line
	    LineBuffer = my_CDF_file.ReadLine();
	    
	    parse_CDF_CEL_LINE(LineBuffer,&xloc,&yloc,&isPM);
	    //cout << xloc << " " << yloc << " " << isPM << endl;
	    if (isPM){
	      PMLoc[whichPM] = xy2i(xloc,yloc,array_rows);
	      whichPM++;
	    } else {
	      MMLoc[whichMM] =  xy2i(xloc,yloc,array_rows);
	      whichMM++;
	    }
	  }
	  if (whichMM == 0){
	    delete [] MMLoc;
	    MMLoc = NULL;
	  }
	  if (whichPM == 0){
	    delete [] PMLoc;
	    PMLoc = NULL;
	  }
	  if (n_cells != 2*current_npp){
	    int *tmpptr;
	    if ((whichMM < n_cells) && (whichMM > 0)){
	      tmpptr = new int[whichMM];
	      for (k=0; k < whichMM; k++){
		tmpptr[k] = MMLoc[k];
	      }
	      delete [] MMLoc;
	      MMLoc = tmpptr;
	    }
	    if ((whichPM < n_cells) && (whichPM > 0)){
	      tmpptr = new int[whichPM];
	      for (k=0; k < whichPM; k++){
		tmpptr[k] = PMLoc[k];
	      }
	      delete [] PMLoc;
	      PMLoc = tmpptr;
	    }
	  }



	  n_probes+=whichPM;
	  /* if (whichPM > 0){
	     wxPrintf("*** ");
	     }
	     wxPrintf(currentName+" %d %d %d\n",whichPM,whichMM,n_probes);
	  */
		 
	  currentitem = new LocMapItem(currentName,whichPM,whichMM,PMLoc,MMLoc); 
	  cdflocs.Insert(currentitem); 
	  if (j==0){
	    probeset_names.Add(currentName);
	  }
	  
#if DEBUG
	  if ( i > 2400000){
	    wxPrintf(currentName+_T(" %d\n"), i);
	  }
#endif
	  
	  //currentitem->MM
	  //delete currentitem;
	} else {

	  PMLoc = NULL;
	  MMLoc = NULL;
	  currentitem = new LocMapItem(currentName,0,0,PMLoc,MMLoc); 	  
	  cdflocs.Insert(currentitem);
	  if (j==0){
	    probeset_names.Add(currentName);
	  }
	}
	
      }
      
#if RMA_GUI_APP   
      if (i % (n_probesets/400) == 0){   // was 200
	CDFProgress.Update(i);
      }
#endif
      
    }
      
    currentitem = NULL;
    probeset_names.Sort();
#if DEBUG
    wxPrintf("Done with CDF\n");
    wxPrintf("ps: %d   p:%d\n",n_probesets,n_probes);
#endif    
  } else if (is_cdf_RME(cdf_path)){
    ReadBinaryCDF(cdf_fname, cdf_path);
  } else {
    wxString Error = _T("The file ") + cdf_fname + _T(" does not looks like a a valid CDF file. Tried both text, xda (binary) and RMECDF formats.\n");
    throw Error;
  }
  
  
  
  
}


static bool isRMECEL(wxString cel_path){
  
  int strlength = 0;
  wxString filetype;
  wxFileInputStream input(cel_path);
  wxDataInputStream store(input);
  
  /* this prevents reading a file which clearly is not of the right type causing a crash */
  strlength = store.Read32();
  if (strlength  > 10){
    return false;
  }
  input.SeekI(0);
  

  filetype = store.ReadString();


  if (filetype.Cmp(_T("CEL")) != 0 && filetype.Cmp(_T("RMECEL")) != 0){
    return false;
  }

  return true;
}

char *rmecel_get_header_info(wxString cel_path, int *cur_dim1, int *cur_dim2){

  wxString filetype;
  wxString ArrayType;

  int versionnumber;

  char *cdfName, *curCDFName;

  wxFileInputStream input(cel_path);
  wxDataInputStream store(input);
  filetype = store.ReadString();
  
  if (filetype.Cmp(_T("CEL")) != 0 && filetype.Cmp(_T("RMECEL")) != 0){
    wxString Error=_T("Problem with RME (CEL). Malformed?");
    throw Error;
  }
  
  versionnumber = store.Read32();
  store.ReadString();  // the arrayname
  ArrayType = store.ReadString();
  *cur_dim1 =  store.Read32();
  *cur_dim2 =  store.Read32();
  
  const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(ArrayType.c_str());
  cdfName = (char *)(const char*) tmp_buf;
 
  curCDFName= (char *)calloc(strlen(cdfName)+1,sizeof(char));
  
  strcpy(curCDFName,cdfName);

  return curCDFName;
}


/*****************************************************************
 **
 ** This function checks the supplied CEL files and verifies that
 ** they are all of the same kind (via the CDF information) and
 ** of the same dimensions
 **
 **
 **
 ****************************************************************/

static void checkCelHeaders(const wxArrayString cel_paths){

  char *cur_cdfName;
  int cur_dim1;
  int cur_dim2;

  char *ref_cdfName=NULL;
  int ref_dim1=0;
  int ref_dim2=0;
  int err_code = 0;
  
  for (int i =0; i < (int)cel_paths.GetCount(); i++){
    if (isTextCelFile(cel_paths[i].mb_str())){
      cur_cdfName = get_header_info(cel_paths[i].mb_str(), &cur_dim1, &cur_dim2, &err_code);
    } else if (isBinaryCelFile(cel_paths[i].mb_str())){
      cur_cdfName = binary_get_header_info(cel_paths[i].mb_str(), &cur_dim1, &cur_dim2, &err_code);
    } else if (isGenericCelFile(cel_paths[i].mb_str())){
      cur_cdfName = generic_get_header_info(cel_paths[i].mb_str(), &cur_dim1, &cur_dim2, &err_code);
    } else if (isRMECEL(cel_paths[i])){
      cur_cdfName = rmecel_get_header_info(cel_paths[i], &cur_dim1, &cur_dim2);
    } else {
      wxString Error = wxT("Format for file ") + cel_paths[i] + wxT(" was not recognized.\n");
      throw Error;
    }
    if (err_code){
      wxString Error = cel_paths[i] + wxT(": ") + parsingErrorCode(err_code) + wxT("\n"); 
      throw Error;
    }


    if (i == 0){
      ref_dim1 = cur_dim1;
      ref_dim2 = cur_dim2;
      ref_cdfName = cur_cdfName;
    } else {
      if (strncmp(cur_cdfName,ref_cdfName,100) != 0){
	wxString Error = wxString(cur_cdfName,wxConvUTF8) + wxT(" does not match ") + wxString(ref_cdfName,wxConvUTF8) + wxT(" for file ") +  cel_paths[i] + wxT("\n");
	free(cur_cdfName);
	free(ref_cdfName);
	throw Error;
      }
      if ((ref_dim1 != cur_dim1) && (ref_dim2 != cur_dim2)){
	free(cur_cdfName);
	free(ref_cdfName);
	wxString Error = wxT("The dimensions of ") + cel_paths[i] + wxT(" were ") << cur_dim1 << wxT(" by ") <<  cur_dim2 << wxT(" while ") << ref_dim1 << wxT(" by ") << ref_dim2  << wxT(" was expected.\n");
	throw Error;
      } 
      free(cur_cdfName);
    }
  }

  free(ref_cdfName);
}




static void checkCDFCelAgreement(const wxArrayString &cel_paths, const wxArrayString &cdfNames,  int array_rows, int array_cols){
  
  char *cur_cdfName=NULL;
  int cur_dim1;
  int cur_dim2;
  int j;
  int err_code = 0;

  for (int i =0; i < (int)cel_paths.GetCount(); i++){
    if (isTextCelFile(cel_paths[i].mb_str())){
      cur_cdfName = get_header_info(cel_paths[i].mb_str(), &cur_dim1, &cur_dim2, &err_code);
    } else if (isBinaryCelFile(cel_paths[i].mb_str())){
      cur_cdfName = binary_get_header_info(cel_paths[i].mb_str(), &cur_dim1, &cur_dim2, &err_code);
    } else if (isGenericCelFile(cel_paths[i].mb_str())){
      cur_cdfName = generic_get_header_info(cel_paths[i].mb_str(), &cur_dim1, &cur_dim2, &err_code);
    } else if (isRMECEL(cel_paths[i])){
      cur_cdfName = rmecel_get_header_info(cel_paths[i], &cur_dim1, &cur_dim2);
    } else {
      wxString Error = wxT("Format for file ") + cel_paths[i] + wxT(" was not recognized.\n");
      throw Error;
    }
    if (err_code){
      wxString Error = cel_paths[i] + wxT(": ") + parsingErrorCode(err_code) + wxT("\n"); 
      throw Error;
    }

    for (j =0; j < cdfNames.GetCount(); j++){
      if (wxString(cur_cdfName,wxConvUTF8).CmpNoCase(cdfNames[j]) == 0){
	break;
      }
    }
    
    if (j == cdfNames.GetCount()){
      wxString Error = wxString(cur_cdfName,wxConvUTF8) + wxT(" does not match: ");
      for (j=0; j < cdfNames.GetCount(); j++){
	Error = Error + _T("  ") + cdfNames[j];
      }
      Error = Error + wxT(" for file ") +  cel_paths[i] + wxT("\n");
      free(cur_cdfName);
      throw Error;
    }
  }
  free(cur_cdfName);
}


static wxString getCelType(const wxString cel_path, int *dim1, int *dim2){

  wxString cdfName;

  char *cur_cdfName;
  int cur_dim1;
  int cur_dim2;
  int err_code = 0;

  if (isTextCelFile(cel_path.mb_str())){
    cur_cdfName = get_header_info(cel_path.mb_str(), &cur_dim1, &cur_dim2, &err_code);
  } else if (isBinaryCelFile(cel_path.mb_str())){
    cur_cdfName = binary_get_header_info(cel_path.mb_str(), &cur_dim1, &cur_dim2, &err_code);
  } else if (isGenericCelFile(cel_path.mb_str())){
    cur_cdfName = generic_get_header_info(cel_path.mb_str(), &cur_dim1, &cur_dim2, &err_code);
  } else {
    wxString Error = wxT("Format for file ") + cel_path + wxT(" was not recognized.\n");
    throw Error;
  }
  if (err_code){
    wxString Error = cel_path + wxT(": ") + parsingErrorCode(err_code) + wxT("\n"); 
    throw Error;
  }

  *dim1 = cur_dim1;
  *dim2 = cur_dim2;
  
  cdfName = wxString(cur_cdfName,wxConvUTF8);
  
  //  free(cur_cdfName);
  
  return cdfName;
}






/******************************************************
 **
 ** bool DataGroup::ReadCELFile(const wxString cel_fname, 
 * 			        const wxString cel_path,
 **                             const int col)
 **
 ** const wxString cel_fname
 ** const wxString cel_path
 ** const int col
 **
 ** Reads in an arbitary format CEL file into the DataGroup object
 **
 **
 ******************************************************/

void DataGroup::ReadCELFile(const wxString cel_path,
			    const int col){

  int i;
  double *cur_intensities = new double[array_rows*array_cols];
  
  int err_code=0;

  if (isTextCelFile(cel_path.mb_str())){
    err_code=read_cel_file_intensities(cel_path.mb_str(), cur_intensities, 0 , array_rows*array_cols, 1, array_rows);
  } else if (isBinaryCelFile(cel_path.mb_str())){
    err_code=read_binarycel_file_intensities(cel_path.mb_str(), cur_intensities, 0 , array_rows*array_cols, 1, array_rows);
  } else if (isGenericCelFile(cel_path.mb_str())){
    err_code=read_genericcel_file_intensities(cel_path.mb_str(), cur_intensities, 0 , array_rows*array_cols, 1, array_rows);
  } else if (isRMECEL(cel_path)){
    ReadBinaryCEL(cel_path, col);
  } else {
    wxString Error = wxT("Format for file ") + cel_path + wxT(" was not recognized.\n");
    throw Error;
  }

  if (err_code){
    wxString Error = cel_path + wxT(": ") + parsingErrorCode(err_code) + wxT("\n"); 
    throw Error;
  }

  
  if (!isRMECEL(cel_path)){
    for (i=0; i < array_rows*array_cols; i++){
      (*intensitydata)(i,col) = cur_intensities[i];
    }
  }
  delete [] cur_intensities;
  
}


/** 
 ** Reads in an RME binary cdf file.
 **
 **/


void DataGroup::ReadBinaryCDF(const wxString cdf_fname, const wxString cdf_path){

  RME_CDF_Header *header;
  
  header = new RME_CDF_Header;


  ReadRMECDF(cdf_path,
	     &cdflocs,
	     header);

  ArrayTypeName = header->ArrayTypeName;
  array_rows = header->array_rows;
  array_cols = header->array_cols;
  n_probesets = header->n_probesets;
  probeset_names = header->probeset_names;
  n_probes  = header->n_probes; 

  delete header;

}







/** 
 ** Reads in an RME binary cel file.
 **
 **/


void DataGroup::ReadBinaryCEL(const wxString cel_fname, const wxString cel_path,const int col){

  wxString ArrayName;
  wxString currentName;
  //int numberofcells;
  int j; //i,j;
  //int xloc, yloc;
  //double current_intensity;

  int rows, cols;
  int numbercells;
  int versionnumber;
  wxString filetype;
  wxString Error;
  
  
  //LocMapItem *currentitem;
  
  wxFileInputStream input(cel_path);
  wxDataInputStream store(input);
  filetype = store.ReadString();
  
  if (filetype.Cmp(_T("CEL")) != 0 && filetype.Cmp(_T("RMECEL")) != 0){
    Error=_T("Problem with RME (CEL). Malformed?");
    throw Error;
  }
  
  versionnumber = store.Read32();
  store.ReadString();   //ArrayNames.Add(store.ReadString());
  store.ReadString();
  rows =  store.Read32();
  cols =  store.Read32();
  
  numbercells = rows*cols;

  for (j =0; j < numbercells ; j++){
    (*intensitydata)(j,col) = store.ReadDouble();
  }
}


void DataGroup::ReadBinaryCEL(const wxString cel_path,const int col){

  wxString ArrayName;
  wxString currentName;
  //int numberofcells;
  int j; //i,j;
  //int xloc, yloc;
  //double current_intensity;

  int rows, cols;
  int numbercells;
  int versionnumber;
  wxString filetype;
  wxString Error;
  
  
  //LocMapItem *currentitem;
  
  wxFileInputStream input(cel_path);
  wxDataInputStream store(input);
  filetype = store.ReadString();
  
  if (filetype.Cmp(_T("CEL")) != 0 && filetype.Cmp(_T("RMECEL")) != 0){
    Error=_T("Problem with RME (CEL). Malformed?");
    throw Error;
  }
  
  versionnumber = store.Read32();
  store.ReadString(); //ArrayNames.Add(store.ReadString());
  store.ReadString();
  rows =  store.Read32();
  cols =  store.Read32();
  
  numbercells = rows*cols;

  for (j =0; j < numbercells ; j++){
    (*intensitydata)(j,col) = store.ReadDouble();
  }
}











/******************************************************
 **
 ** DataGroup::DataGroup
 **
 ** class constructor: takes the names of a cdf file 
 **   (and appropriate path) and a set of cel files (and paths). 
 **   then reads in the appropriate files and builds appropriate
 **   data structures. The data structure will hold the location 
 **   and probeset mapping information for the probes. In addition
 **   we will hold the cel intensities in a big array
 **
 ******************************************************/

DataGroup::DataGroup(wxWindow *parent,const wxString cdf_fname, 
		     const wxString cdf_path,
		     const wxArrayString cel_fnames, 
		     const wxArrayString cel_paths, Preferences *preferences){

  int i;// j;
  wxString Error;
  

  this->parent = parent;

  n_probes = 0;

  /* Check that CEL files are all of the same type */
  checkCelHeaders(cel_paths);

  ReadCDFFile(cdf_fname,cdf_path);  

  /* check that CEL files and CDF agree */

  checkCDFCelAgreement(cel_paths, ArrayTypeName, array_cols, array_rows);


#ifdef BUFFERED
  wxString tempFullPath = preferences->GetFullFilePath();
  const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(tempFullPath);
  const char *tmp_str = (const char*) tmp_buf;
  
  intensitydata = new BufferedMatrix(preferences->GetProbesBufSize(),preferences->GetArrayBufSize(),(char *)tmp_str);
#else
  intensitydata = new Matrix();
#endif


  intensitydata->SetRows(array_rows*array_cols);
#if RMA_GUI_APP
  DataGroupProgress = new wxProgressDialog(_T("Reading in .........."),_T(""),cel_fnames.GetCount(),this->parent,wxPD_AUTO_HIDE );
#endif
  
  try{
    for (i =0; i < (int)cel_fnames.GetCount(); i++){
#if RMA_GUI_APP 
      DataGroupProgress->Update(i, cel_fnames[i]);
#endif
      intensitydata->AddColumn();
      ReadCELFile(cel_paths[i],i);
    }
    
    n_arrays = cel_fnames.GetCount();
    ArrayNames = cel_fnames;
#if RMA_GUI_APP  
    delete DataGroupProgress;
#endif
  }
  catch(wxString& Problem){
#if RMA_GUI_APP  
    delete DataGroupProgress;
#endif
    throw Problem;
  }


}

/**

This function instantiates a DataGroup from RME binary files


***/




DataGroup::DataGroup(wxWindow *parent,
		     const wxArrayString binary_fnames, 
		     const wxArrayString binary_paths, Preferences *preferences){


  int i, j;
  int cdfindex = -1;
  wxString Error;
  wxString cdf_fname, cdf_path;
  wxArrayString cel_fnames,cel_paths;
  wxString filetype;
  //  bool read_ok;

  this->parent = parent;

  n_probes = 0;
#ifdef BUFFERED
  wxString tempFullPath = preferences->GetFullFilePath();
  const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(tempFullPath);
  const char *tmp_str = (const char*) tmp_buf;
  
  intensitydata = new BufferedMatrix(preferences->GetProbesBufSize(),preferences->GetArrayBufSize(),(char *)tmp_str);

#else
  intensitydata = new Matrix();
#endif
  
  // lets go through the RME files checking for a CDF file

  for (i =0 ; i < (int)binary_fnames.GetCount(); i++){
    
    wxFileInputStream input(binary_paths[i]);
    wxDataInputStream store(input);
    filetype = store.ReadString();
    
    if ((filetype.Cmp(_T("CDF")) == 0) && (cdfindex == -1)){
      cdfindex = i;
      cdf_fname = binary_paths[i];
    } else if ((filetype.Cmp(_T("CDF")) == 0) && (cdfindex != -1)){
      Error = _T("You have more than one file containing CDF information.");
      throw Error;
    }
  }
  
  if(cdfindex == -1){
    Error=_T("None of your RME files contain CDF information.");
    throw Error;
  } else {
    cdf_fname = binary_fnames[cdfindex];
    cdf_path = binary_paths[cdfindex];
    
    if (binary_fnames.GetCount() -1 <= 0){
      Error = _T("No RME files with CEL information specified.");
      throw Error;
    }

    for (j =0; j < (int)binary_fnames.GetCount(); j++){
      if (j != cdfindex){
	cel_fnames.Add(binary_fnames[j]);
	cel_paths.Add(binary_paths[j]);
      }
    }
  }
  
  


  if (cel_paths.Count() == 0){
    return;
  }
#if RMA_GUI_APP 
  wxProgressDialog RMEProgress(_T("RME Progress"),_T("Reading in RME files"),binary_fnames.GetCount(),this->parent,wxPD_AUTO_HIDE);
#endif
  ReadBinaryCDF(cdf_fname,cdf_path);  

#if RMA_GUI_APP 
  RMEProgress.Update(1);
#endif
  
  intensitydata->SetRows(array_rows*array_cols);  

  ArrayNames.Alloc(cel_fnames.GetCount());
  for (i =0; i < (int)cel_fnames.GetCount(); i++){
    intensitydata->AddColumn();
    ReadBinaryCEL(cel_fnames[i],cel_paths[i],i);
#if RMA_GUI_APP
    RMEProgress.Update(i+2);
#endif
  }

  n_arrays = cel_fnames.GetCount();
  
}


/******************************************************************************
 **
 **
 **
 **
 ** Instantiate with only a CDF file
 **
 *****************************************************************************/

DataGroup::DataGroup(wxWindow *parent,const wxString cdf_fname){
  
  this->parent = parent;
  
  n_probes = 0;

  wxFileName my_cdf(cdf_fname);  
  // ReadCDFFile(my_cdf.GetFullName(),my_cdf.GetPath()); 
  ReadCDFFile(my_cdf.GetFullName(), cdf_fname);
  n_arrays = 0;
  
  intensitydata = NULL;
}

/******************************************************************************
 **
 **
 **
 **
 **  Instantiate with only CEL files
 **
 *****************************************************************************/

DataGroup::DataGroup(wxWindow *parent,const wxArrayString cel_paths, Preferences *preferences){

  int i;


  this->parent = parent;
  

  checkCelHeaders(cel_paths);
  
  wxArrayString cel_fnames;
#ifdef BUFFERED  
  wxString tempFullPath = preferences->GetFullFilePath();
  const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(tempFullPath);
  const char *tmp_str = (const char*) tmp_buf;
  
  intensitydata = new BufferedMatrix(preferences->GetProbesBufSize(),preferences->GetArrayBufSize(),(char *)tmp_str);
#else
  intensitydata = new Matrix();
#endif
#if RMA_GUI_APP
  DataGroupProgress = new wxProgressDialog(_T("Reading in .........."),_T(""),cel_paths.GetCount(),this->parent,wxPD_AUTO_HIDE );
#endif  
  for (i=0; i < (int)cel_paths.GetCount(); i++){
    wxFileName my_cel(cel_paths[i]);
    cel_fnames.Add(my_cel.GetFullName());
  }


  //ArrayTypeName = _T("IGNORE");
  n_arrays=0;

  ArrayTypeName.Add(getCelType(cel_paths[0], &array_cols, &array_rows));


  intensitydata->SetRows(array_rows*array_cols);

  for (i =0; i < (int)cel_fnames.GetCount(); i++){
#if RMA_GUI_APP 
    DataGroupProgress->Update(i, cel_fnames[i]);
#endif
    intensitydata->AddColumn();
    ReadCELFile(cel_paths[i],i);
    n_arrays++;
  }
  
  n_arrays = cel_fnames.GetCount();
  ArrayNames = cel_fnames;
#if RMA_GUI_APP  
  delete DataGroupProgress;
#endif 


}


void DataGroup::Add(const wxArrayString &fnames, const wxArrayString &paths){

  int i;

  checkCelHeaders(paths);
  checkCDFCelAgreement(paths, ArrayTypeName, array_cols, array_rows);
  
#if RMA_GUI_APP
  DataGroupProgress = new wxProgressDialog(_T("Reading in .........."),_T(""),fnames.GetCount(),this->parent,wxPD_AUTO_HIDE );
#endif

  for (i =0; i < (int)fnames.GetCount(); i++){
#if RMA_GUI_APP 
    DataGroupProgress->Update(i, fnames[i]);
#endif
    intensitydata->AddColumn();
    ReadCELFile(paths[i],n_arrays);
    n_arrays++;
    ArrayNames.Add(fnames[i]);
  }
#if RMA_GUI_APP  
  delete DataGroupProgress;
#endif
}







/******************************************************
 **
 ** int DataGroup::nrows()
 **
 **
 **
 **
 **
 ******************************************************/

int DataGroup::nrows(){
  return array_rows;
  
}
 

/******************************************************
 **
 ** int DataGroup::ncols()
 **
 **
 **
 **
 **
 ******************************************************/

int DataGroup::ncols(){
  return array_cols;

}


/******************************************************
 **
 ** int DataGroup::count_pm()
 **
 **
 **
 **
 **
 ******************************************************/

int DataGroup::count_pm(){

  return n_probes;

}

/******************************************************
 **
 ** int DataGroup::count_arrays()
 **
 **
 **
 **
 **
 ******************************************************/

int DataGroup::count_arrays(){

  return n_arrays;

}

/******************************************************
 **
 ** int DataGroup::count_probesets()
 **
 **
 **
 **
 **
 ******************************************************/

int DataGroup::count_probesets(){

  return n_probesets;

}

/******************************************************
 **
 ** double &DataGroup::operator[](unsigned int i)
 **
 **
 **
 **
 **
 ******************************************************/

double &DataGroup::operator[](unsigned int i){

  return (*intensitydata)[i];
}

/******************************************************
 **
 ** wxArrayString DataGroup::GiveNames()
 **
 **
 **
 **
 ******************************************************/

wxArrayString DataGroup::GiveNames(){
  
  return probeset_names;

}

/******************************************************
 **
 ** LocMapItem *DataGroup::FindLocMapItem(const wxString &name)
 **  
 ** const wxString &name
 **
 **
 **
 **
 **
 *****************************************************/

LocMapItem *DataGroup::FindLocMapItem(const wxString &name){
  
  return cdflocs.Find(name);
}

/******************************************************
 **
 ** DataGroup::~DataGroup()
 **
 **
 **
 **
 *****************************************************/

DataGroup::~DataGroup(){
  if (intensitydata !=NULL)
    delete intensitydata;
}



/******************************************************
 **
 ** wxArrayString DataGroup::GetArrayNames()
 **
 **
 **
 *****************************************************/

wxArrayString DataGroup::GetArrayNames(){

  return ArrayNames;

}


/**** 
 both CEL like and CDF like binary formats will start with
 
 string  - CEL or CDF which will define what 
 version number  - 32 bit integer   which will define format we will define what version 1 means here.

 for CEL files

 string - name of cel file
 int rows 
 int cols
 
 then rows * cols probes from the appropriate column of *intensity data
 

**/


bool DataGroup::WriteBinaryCDF(wxString path){

  int i,j;
  int numbercells;
  wxString currentName;
  LocMapItem *current_item;
  
  // Store the CEL file data
#if RMA_GUI_APP 
  wxProgressDialog RMEProgress(_T("RME Progress"),_T("Writing CEL files"),n_arrays,this->parent,wxPD_AUTO_HIDE);
#endif  


#if RMA_GUI_APP
  RMEProgress.Update(0,_T("Writing CDF file"));
#endif
 
 
  /*  this is the code that was used for writing version 1 RME CDF files 
  
  wxFileName currentPath(path,ArrayTypeName + ".RME");
  currentName =currentPath.GetFullPath();
  wxFileOutputStream output(currentName);
  wxDataOutputStream store(output);
  store.WriteString("CDF");
  store.Write32(1);
  store.WriteString(ArrayTypeName);
  store.Write32(array_rows);
  store.Write32(array_cols);
  store.Write32(n_probesets);
  
  for (i = 0; i < n_probesets; i++){
    current_item = cdflocs.Find(probeset_names[i]);
    store.WriteString(current_item->GetName());
    store.Write32(current_item->GetPMSize());
    for (j = 0; j < current_item->GetPMSize(); j++){
      store.Write32(current_item->GetPMLocs()[j]); 
    }
    if (current_item->GetMMLocs() != NULL){
      for (j= 0; j < current_item->GetMMSize(); j++){
	store.Write32(current_item->GetMMLocs()[j]);
      }
    }
  }


  
  */

  /* this code writes version 2 format RME cdf files (first introduced in the 0.5 sequence */
  
  /****

  wxFileName currentPath(path,ArrayTypeName + _T(".RME"));

  currentName =currentPath.GetFullPath();
  wxFileOutputStream output(currentName);
  wxDataOutputStream store(output);
  store.WriteString(_T("CDF"));
  store.Write32(2);
  store.WriteString(ArrayTypeName);
  store.Write32(array_rows);
  store.Write32(array_cols);
  store.Write32(n_probesets);
  for (i = 0; i < n_probesets; i++){
    current_item = cdflocs.Find(probeset_names[i]);
    store.WriteString(current_item->GetName());
    store.Write32(current_item->GetPMSize());
    for (j = 0; j < current_item->GetPMSize(); j++){
      store.Write32(current_item->GetPMLocs()[j]); 
    }
    store.Write32(current_item->GetMMSize());
    for (j= 0; j < current_item->GetMMSize(); j++){
	store.Write32(current_item->GetMMLocs()[j]);
    }
  }
  **/

  /* this code is for format 3 introduced at 1.0 beta 4 */
  wxFileName currentPath(path,ArrayTypeName[0] + _T(".CDFRME"));
  currentName =currentPath.GetFullPath();
  
  wxFileOutputStream output(currentName);
  wxDataOutputStream store(output);
  store.WriteString(wxString(_T("RMECDF")));
  store.Write32(3);
  store.Write32(ArrayTypeName.GetCount());
  for (j = 0; j < ArrayTypeName.GetCount(); j++){
    store.WriteString(ArrayTypeName[j]);
  }  
  store.Write32(CDF_descStr.GetCount());   // Number of Strings 

  for (j = 0; j < CDF_descStr.GetCount(); j++){
    store.WriteString(CDF_descStr[i]);
  }
  /* store.WriteString(wxString(_T("Created using RMADataConv ")) + version_number);
     
  // work out the current system date 
  
  wxDateTime now = wxDateTime::Now();
  store.WriteString(wxString(_T("Creation Date: ")) + now.FormatDate() + wxString(_T(" ")) + now.FormatTime());
  store.WriteString(wxString(_T("CDF file: ")) + ArrayTypeName[0]);
  */

  store.Write32(array_rows);
  store.Write32(array_cols);
  store.Write32(n_probesets);  
  for (i = 0; i < n_probesets; i++){
    current_item = cdflocs.Find(probeset_names[i]);
    store.WriteString(current_item->GetName());
    store.Write32(current_item->GetPMSize());
    for (j = 0; j < current_item->GetPMSize(); j++){
      store.Write32(current_item->GetPMLocs()[j]); 
    }
    store.Write32(current_item->GetMMSize());
    for (j= 0; j < current_item->GetMMSize(); j++){
	store.Write32(current_item->GetMMLocs()[j]);
    }
  }

  return true;


}


void DataGroup::SetArrayTypeName(wxString new_name){


  ArrayTypeName.Add(new_name);

}



bool DataGroup::WriteBinaryCEL(wxString path){
 
  int i, j; 
  int numbercells;
  wxString currentName;
  // Store the CEL file data
#if RMA_GUI_APP
  wxProgressDialog RMEProgress(_T("RME Progress"),_T("Writing CEL files"),n_arrays,this->parent,wxPD_AUTO_HIDE);
#endif
  
  for (i =0; i < n_arrays; i++){
#if RMA_GUI_APP
    RMEProgress.Update(i,_T("Writing ")+ ArrayNames[i]);
#endif
    wxFileName currentPath(path,ArrayNames[i] + _T(".RME"));
    currentName =currentPath.GetFullPath();
    wxFileOutputStream output(currentName);
    wxDataOutputStream store(output);
    store.WriteString(_T("CEL"));
    store.Write32(1);
    store.WriteString(ArrayNames[i]);
    store.WriteString(ArrayTypeName[0]);
    store.Write32(array_rows);
    store.Write32(array_cols);
    numbercells = array_rows*array_cols;
    for (j =0; j < numbercells ; j++){
      store.WriteDouble((*intensitydata)(j,i));
    }
  }
  return true;
}





bool DataGroup::WriteBinaryCDF(wxString path, wxString restrictfname, wxArrayString restrictlist){
  
  int i,j;
  
  wxString currentName;
  LocMapItem *current_item;
  
#if RMA_GUI_APP
  wxProgressDialog RMEProgress(_T("RME Progress"),_T("Writing CEL files"),n_arrays,this->parent,wxPD_AUTO_HIDE);
#endif

  n_probesets = restrictlist.Count();
#if RMA_GUI_APP
  RMEProgress.Update(0,_T("Writing CDF file"));
#endif



  /** this is the code that was used for  version 1 RME cdf files
  wxFileName currentPath(path,ArrayTypeName + ".RME");
  
  currentName =currentPath.GetFullPath();
  wxFileOutputStream output(currentName);
  wxDataOutputStream store(output);
  store.WriteString("CDF");
  store.Write32(1);
  store.WriteString(ArrayTypeName);
  store.Write32(array_rows);
  store.Write32(array_cols);
  store.Write32(n_probesets);
  
  for (i = 0; i < restrictlist.Count(); i++){
    current_item = cdflocs.Find(restrictlist[i]);
    if (current_item == 0){

      wxString ErrorMessage = "Can't find it " + restrictlist[i] + "\n";
      throw ErrorMessage;
    }
    store.WriteString(current_item->GetName());
    store.Write32(current_item->GetPMSize());
    for (j = 0; j < current_item->GetPMSize(); j++){
      store.Write32(current_item->GetPMLocs()[j]); 
    }
    if (current_item->GetMMLocs() != NULL){
      for (j= 0; j < current_item->GetMMSize(); j++){
	store.Write32(current_item->GetMMLocs()[j]);
      }
    }
  }
  **/


  /* this code is for format 2 introduced in the 0.5 sequence */

  /****
  wxFileName currentPath(path,ArrayTypeName + _T(".RME"));
  currentName =currentPath.GetFullPath();

  wxFileOutputStream output(currentName);
  wxDataOutputStream store(output);
  store.WriteString(_T("CDF"));
  store.Write32(2);
  store.WriteString(ArrayTypeName);
  store.Write32(array_rows);
  store.Write32(array_cols);
  store.Write32(n_probesets);
  for (i = 0; i < (int)restrictlist.Count(); i++){
    current_item = cdflocs.Find(restrictlist[i]);
    if (current_item == 0){

      wxString ErrorMessage = _T("Can't find it ") + restrictlist[i] + _T("\n");
      throw ErrorMessage;
    }
    store.WriteString(current_item->GetName());
    store.Write32(current_item->GetPMSize());
    for (j = 0; j < current_item->GetPMSize(); j++){
      store.Write32(current_item->GetPMLocs()[j]); 
    }  
    store.Write32(current_item->GetMMSize());
    for (j= 0; j < current_item->GetMMSize(); j++){
      store.Write32(current_item->GetMMLocs()[j]);
    }
  }
  **/

  /* this code is for format 3 introduced at 1.0 beta 4 */
  wxFileName currentPath(path,ArrayTypeName[0] + _T(".CDFRME"));
  currentName =currentPath.GetFullPath();
  
  wxFileOutputStream output(currentName);
  wxDataOutputStream store(output);
  store.WriteString(wxString(_T("RMECDF")));
  store.Write32(3);
  store.Write32(ArrayTypeName.GetCount());
  for (j = 0; j < ArrayTypeName.GetCount(); j++){
    store.WriteString(ArrayTypeName[j]);
  }  
  store.Write32(CDF_descStr.GetCount());   // Number of Strings 

  for (j = 0; j < CDF_descStr.GetCount(); j++){
    store.WriteString(CDF_descStr[i]);
  }
  store.Write32(array_rows);
  store.Write32(array_cols);
  store.Write32(n_probesets);
  for (i = 0; i < (int)restrictlist.Count(); i++){
    current_item = cdflocs.Find(restrictlist[i]);
    if (current_item == 0){

      wxString ErrorMessage = _T("Can't find it ") + restrictlist[i] + _T("\n");
      throw ErrorMessage;
    }
    store.WriteString(current_item->GetName());
    store.Write32(current_item->GetPMSize());
    for (j = 0; j < current_item->GetPMSize(); j++){
      store.Write32(current_item->GetPMLocs()[j]); 
    }  
    store.Write32(current_item->GetMMSize());
    for (j= 0; j < current_item->GetMMSize(); j++){
      store.Write32(current_item->GetMMLocs()[j]);
    }
  }


  return true;


}



wxArrayString DataGroup::GetArrayTypeName(){

  return ArrayTypeName;

}



CDFLocMapTree *DataGroup::GiveLocMapTree(){

  return &cdflocs;
}




void DataGroup::ResizeBuffer(int rows, int cols){

#ifdef BUFFERED
  intensitydata->ResizeBuffer(rows, cols);
#endif
}





double DataGroup::operator()(int row, int col){

  //  return (*intensitydata)(row, col);
  return intensitydata->getValue(row, col);
}


void DataGroup::ReadOnlyMode(bool setting=false){
  
  
#ifdef BUFFERED
  intensitydata->ReadOnlyMode(setting);
#endif
  

}


void DataGroup::AddCDF_desc(wxString &desc){


  CDF_descStr.Add(desc);
  


}
