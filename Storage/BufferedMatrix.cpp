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
 ** file: BufferedMatrix.cpp
 **
 ** Copyright (C) 2005-2008    B. M. Bolstad
 **
 ** aim: A class to represent a resizable matrix of doubles.
 **      Where resizable means we may add columns of data.
 **      In addition data is buffered out to storage space.
 **
 ** Created by: B. M. Bolstad  <bolstad@stat.berkeley.edu>
 **
 ** Created on: Mar 15, 2005
 **
 ** History
 ** Mar 15, 2005 - Initial version
 ** Mar 18, 2005 - some refactoring and heavy testing.
 ** Mar 21, 2005 - Added ability to resize buffers
 ** Mar 27, 2005 - Fixed a bug, increasing buffer size when everything already in buffer
 ** Apr 1, 2005 - Introduce the concept of "column mode". When in column mode, the row
 **               buffer does not exist. Column mode will be the default mode
 **               "row mode" is what the previous state was
 ** Jan 30, 2006 - Add a Flush all Columns function.
 **                Add a readonly mode
 ** Jan 6, 2007 - add Commpute5Summary
 ** Feb 6, 2008 - Add GetFullColumn
 ** Feb 28, 2008 - minor fix to resize buffer. Revise operator()
 **
 *****************************************************/

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <vector>
#include <algorithm>

using namespace std;



#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__CYGWIN__) && !defined(UNICODE)
#include <cstdarg>
#include <windows.h>
#include <windef.h>
#include <winbase.h>
#endif

#include "BufferedMatrix.h"

#include "../rma_common.h"
#include "../threestep_common.h"



#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__CYGWIN__) && !defined(UNICODE)
int MyGetFileName(char *temppath,char* templat)
{
  
  if(GetTempFileName(temppath,"RMA",0,templat)!=0)
    {
      FILE *pFile;
      pFile=fopen(templat,"wb"); 
      fclose(pFile);
      if(pFile!=NULL)
	return (int)pFile;
    }
}

#elif defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__CYGWIN__) && defined(_UNICODE)
int MyGetFileName(wchar_t const *temppath, wchar_t *templat)
{
  
  if(GetTempFileName(temppath,wxT("RMA"),0,templat)!=0)
    {
      FILE *pFile;
      pFile=_wfopen(templat,wxT("wb")); 
      fclose(pFile);
      if(pFile!=NULL)
	return (int)pFile;
    }
}


#endif




/******************************************************
 **
 ** Basic Idea: Store a Matrix partially in memory.
 **             Partially on disk.
 **             Keep all of this hidden as much as
 **             possible from user.
 **
 ** More general comments:
 **            Assumption is that most access to matrix
 **            is systematic either going down a column
 **            or across a row. Random access will probably be
 **            very, very inefficient.
 **         
 **            two buffers 
 **               - small row buffer - this is the primary buffer when 
 **                          in row mode it contains a contiguous set of rows
 **			     across all columns 
 **               - larger column buffer - secondary buffer in row mode
 **                          it contains all the data for a 
 **                          relatively small number of columns
 **
 **            Two modes: column mode (DEFAULT on initialization)
 **                       row mode
 **           
 **            When in column mode the row buffer does not exist.
 **            this means that if the matrix is being accessed across
 **            rows it will be very inefficient. When it is likely that
 **            accessing is going to occur across rows, then a call to:
 **            RowMode() should be called to switch into row mode.
 **            To switch back into column mode use: ColMode()
 **
 **            When switching from column mode to row mode
 **             - allocate space for row buffer
 **             - load data in from files, when data is not 
 **               currently in column buffer
 **             - copy across relevant data that is in column buffer
 **             - set colmode flag to false
 **            
 **            When switching from row mode to column mode
 **            - clear clashes
 **            - flush row buffer (ie write to files)
 **            - deallocate row buffer
 **            - set colmode flag to true
 **
 **
 **            When in "row mode"
 **             the following rules will be used: 
 **           
 **            when data values are being READ from the matrix
 **            - check if it is in small row buffer. If so return value else ...
 **            - check if it is in larger column matrix. If so return value
 **            - fill row buffer with all rows adjacent to row being queried then
 **              remove oldest column from column buffer then put new column into buffer
 **              finally return value
 **
 **            when data values are being written into matrix
 **            - check if location is in small buffer. If it is then
 **              return the address of that location
 **            - If it is not in the row buffer check column buffer.
 **              if it is then return the address of that location
 **            - flush buffers (ie write current row buffer and column buffers
 **              out to file) then refill buffers and return address of cell in row buffer.
 **            
 **            When in "column mode"
 **             the following rules will be used
 **             - check if column is in buffer. If it is return value else ....
 **             - remove oldest column from column buffer then put new column into buffer
 **              finally return value
 **
 **            reads/writes via &operator[]
 **
 **            Add will work like this:
 **              Create a new temporary file name
 **              Open this temporary file and write # of row zeros
 **              Then increase the rowdata buffer
 **              Remove oldest row data from buffer
 **              Add new column to end of column buffer
 **
 *****************************************************/





BufferedMatrix::BufferedMatrix(int max_rows,int max_cols,const char *prefix){

  char endprefix[7] = "XXXXXX";
  char *tmp;
  wchar_t *wtmp;
  this->rows =0;
  this->cols =0;
  this->max_rows = max_rows;
  this->max_cols = max_cols;
  
  this->coldata = 0;
  this->rowdata = 0;
  
  this->which_cols = 0;

  this->filenames = 0;
  
  this->first_rowdata =0;
#if _WIN32
  tmp = new char[strlen(prefix)+1];
  strcpy(tmp,prefix);
#else 
  tmp = new char[strlen(prefix)+7];
  strcpy(tmp,prefix);
  strcat(tmp,endprefix);
#endif

#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__CYGWIN__) && defined(_UNICODE)
  wtmp = new wchar_t[strlen(prefix)+1];
  mbstowcs(wtmp,prefix,strlen(prefix));
  this->fileprefix = wtmp;
#else
  this->fileprefix = tmp;
#endif
  
  this->rowcolclash = false;

  this->colmode = true;

  this->readonly=false;
 
}



void BufferedMatrix::SetRows(int rows){

  this->rows = rows;
  if (this ->rows < this->max_rows){
    this->max_rows = rows;
  }


}



void BufferedMatrix::SetPrefix(char *prefix){
  char endprefix[7] = "XXXXXX";
  char *tmp;
  wchar_t *wtmp;

#if _WIN32
  tmp = new char[strlen(prefix)+1];
  strcpy(tmp,prefix);
#else 
  tmp = new char[strlen(prefix)+7];
  strcpy(tmp,prefix);
  strcat(tmp,endprefix);
#endif
  
  if (this->fileprefix != NULL){
    delete [] this->fileprefix;
  }
#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__CYGWIN__) && defined(_UNICODE)
  wtmp = new wchar_t[strlen(prefix)+1];
  mbstowcs(wtmp,prefix,strlen(prefix));
  this->fileprefix = wtmp;
#else
  this->fileprefix = tmp;
#endif

}














void BufferedMatrix::AddColumn(){
  FILE *myfile;
  int j,i;
  int which_col_num;
  int fd;

  
  /* Handle the housekeeping of indices, clearing buffer if needed etc */
  if (cols < max_cols){
    /* No need to clear out column buffer */
    int *temp_indices = new int[cols +1];
    int *temp_old_indices = which_cols;
    double **temp_ptr = new double *[cols +1];
    double **old_temp_ptr = coldata;

    for (j =0; j < cols; j++){
      temp_indices[j] = which_cols[j];
      temp_ptr[j] = coldata[j];
    }
    temp_indices[cols] =cols;
    temp_ptr[cols] = new double[this->rows];

    coldata = temp_ptr;
    
    for (i =0; i < this->rows; i++){
      coldata[cols][i] = 0.0;  //(cols)*rows + i; 
    }
    which_col_num = cols;
    which_cols = temp_indices;
    delete [] temp_old_indices;
    delete [] old_temp_ptr;

    if (!colmode){
      /* Now handle the row buffer */
      old_temp_ptr = rowdata;
      temp_ptr = new double *[cols +1];
      
      for (j =0; j < cols; j++){
	temp_ptr[j] = rowdata[j];
      }
      temp_ptr[cols] = new double[this->max_rows];
      
      for (i=0; i < max_rows; i++){
	temp_ptr[cols][i] = 0.0;   // (cols)*rows + i; 
      }


      rowdata = temp_ptr;
      delete [] old_temp_ptr;
    }

  } else {
    /* Need to remove oldest column from buffer */
    double **temp_ptr;
    double *temp_col = coldata[0];
    double **old_temp_ptr = rowdata;
 
    /* Before we deallocate, better empty the column buffer */
    myfile = fopen(filenames[which_cols[0]],"rb+");
    fwrite(&temp_col[0],sizeof(double),rows,myfile);
    fclose(myfile); 
    
    for (j =1; j < max_cols; j++){
      which_cols[j-1] = which_cols[j];
      coldata[j-1] = coldata[j];
    }
    which_cols[max_cols-1] = cols;
    coldata[max_cols-1] = temp_col; //new double[this->rows];
    for (i =0; i < this->rows; i++){
      coldata[max_cols-1][i] = 0.0; // (cols)*rows +i;
    }
   
    
    which_col_num = max_cols-1;

   
    

    //delete [] temp_col;
    if (!colmode){
      old_temp_ptr = rowdata;
      temp_ptr = new double *[cols +1];
      
      for (j =0; j < cols; j++){
	temp_ptr[j] = rowdata[j];
      }
      temp_ptr[cols] = new double[this->max_rows];
      
      for (i=0; i < max_rows; i++){
	temp_ptr[cols][i] = 0.0;      //(cols)*rows + i;
      }
      
      
      rowdata = temp_ptr;
      delete [] old_temp_ptr;
    }


  }
  /* now do the file stuff */

  char **temp_filenames = new char *[cols+1];
  //  char *temp_name;
  char **temp_names_ptr = filenames;


  for (j =0; j < cols; j++){
    temp_filenames[j] = filenames[j];
  }
#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__CYGWIN__) && !defined(_UNICODE)
  char *tmp = new char[500];
#elif defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__CYGWIN__) && defined(_UNICODE)
  wchar_t *tmp = new wchar_t[500];
#else
  char *tmp = new char[strlen(fileprefix)+1];
#endif

#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__CYGWIN__) && defined(_UNICODE)
  wcscpy(tmp,fileprefix);
#else
  strcpy(tmp,fileprefix);
#endif


#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__CYGWIN__) && !defined(_UNICODE)
  fd = MyGetFileName(fileprefix,tmp);
#elif defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__CYGWIN__) && defined(_UNICODE)

  fd = MyGetFileName(fileprefix,tmp);

#else
  fd = mkstemp(tmp);
#endif

#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__CYGWIN__) && defined(_UNICODE)
  temp_filenames[cols] = new char[wcslen(tmp)+1];
  wcstombs (temp_filenames[cols], tmp, sizeof(temp_filenames[cols]) );
  //temp_filenames[cols] = strcpy(temp_filenames[cols],tmp);
#else
  temp_filenames[cols] = new char[strlen(tmp)+1];
  temp_filenames[cols] = strcpy(temp_filenames[cols],tmp);
#endif

  filenames = temp_filenames;

  delete [] temp_names_ptr;
  delete [] tmp;


  /* Finally lets write it all out to a file */

  const char *mode = "wb";
  //printf("%s\n", filenames[cols]);
#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__CYGWIN__)
  myfile = fopen(temp_filenames[cols],mode);
#else
  myfile = fdopen(fd,mode);
#endif
  if (!myfile){
    throw "Can't open/create temporary file.\n";
  }
  fwrite(coldata[which_col_num],sizeof(double),  rows, myfile);
  fclose(myfile);
  this->cols++;
  
}



BufferedMatrix::~BufferedMatrix(){
  
  int i;
  int lastcol;
  
  if (cols < max_cols){
    lastcol = cols;
  } else {
    lastcol = max_cols;
  }

  for (i=0; i < cols; i++){
    //printf("%s\n",filenames[i]);
    remove(filenames[i]);
  }

  delete [] which_cols;

  for (i = 0; i < cols; i++){
    delete [] filenames[i];
  }
  delete [] filenames;

  if (!colmode){
    for (i=0; i < cols; i++){
      delete [] rowdata[i];
    }
    delete [] rowdata;
  }


  for (i=0; i < lastcol; i++){
    delete [] coldata[i];
  }
  delete [] coldata;
  

  delete [] fileprefix;

}


double &BufferedMatrix::operator[](unsigned long i)
{
  
  int whichcol = i/rows;
  int whichrow = i % rows;

  int curcol;

  if (!colmode){
    /* Fix up any potential clashes */
    if (rowcolclash){
      ClearClash();
    }
    
    /* check to see if this cell is in row buffer, then column buffer, then read from files */
    
    if (InRowBuffer(whichrow,whichcol)){
      /* In row buffer so return */
      if (InColBuffer(whichrow,whichcol,&curcol)){
	SetClash(whichrow,whichcol);
      }
      
      return rowdata[whichcol][whichrow - first_rowdata];
    } else if (InColBuffer(whichrow,whichcol,&curcol)){
      return coldata[curcol][whichrow];
    } else {
      
      /* looks like we are going to have to go to files */
      //printf("Couldn't find in buffers\n");

      if (!readonly){
	/* Flush buffers */ 
	/* First row buffer */
	FlushRowBuffer();
      
	/* Now flush the column buffer (for oldest column) */
	FlushOldestColumn();
      }
      
      /* Now fill up the buffer */
      /* read in this row and surrounding rows into row buffer */
      LoadRowBuffer(whichrow);
      
      /* read in this column into column buffer */
      LoadNewColumn(whichcol);
      
      
      SetClash(whichrow,whichcol);
      return rowdata[whichcol][whichrow - first_rowdata];
    
    }
  } else {
    if (InColBuffer(whichrow,whichcol,&curcol)){
      return coldata[curcol][whichrow];
    } else {
      if (!readonly)
	FlushOldestColumn(); 
      LoadNewColumn(whichcol);
      return coldata[max_cols -1][whichrow];
    }
  }
}



void BufferedMatrix::SetClash(int row, int col){


  rowcolclash = true;
  clash_row = row;
  clash_col = col;




}


void  BufferedMatrix::ClearClash(){

  // Should mean that row buffer is up to date and column buffer is potentially not
  int curcol,lastcol;


  
  curcol = 0;
  if (cols < max_cols){
    lastcol = cols;
  } else {
    lastcol = max_cols;
  }
  
  while (curcol < lastcol){
    if (which_cols[curcol] == clash_col){
      break;
    }
    curcol++;
  }



  if (rowdata[clash_col][clash_row - first_rowdata] != coldata[curcol][clash_row]){
    /* there is a clash, update coldata with current version in rowdata */
    coldata[curcol][clash_row] = rowdata[clash_col][clash_row - first_rowdata];
  } 

  rowcolclash=false;
}




bool BufferedMatrix::InRowBuffer(int row, int col){
  if ((first_rowdata <= row) && (row < first_rowdata + max_rows)){
    return true;
  } else {
    return false;
  }
}

bool BufferedMatrix::InColBuffer(int row, int col, int *which_col_index){
  int curcol, lastcol;

  /*curcol = 0;
  if (cols < max_cols){
    lastcol = cols;
  } else {
    lastcol = max_cols;
  }
  
  while (curcol < lastcol){
  if (which_cols[curcol] == col){
  *which_col_index = curcol;
  return true;
  }
  curcol++;
  } */
  

  
  
  if (cols < max_cols){
    lastcol = cols;
  } else {
    lastcol = max_cols;
  }
    
    
  curcol = lastcol-1;
  while (curcol >= 0){
    if (which_cols[curcol] == col){
      *which_col_index = curcol;
      return true;
    }
    curcol--;
  } 
  
  return false;
}




void BufferedMatrix::FlushRowBuffer(){
  int j;
  const char *mode2 ="rb+";
  FILE *myfile;

  //printf("flushing rows %d through %d\n",first_rowdata, first_rowdata+max_rows);
  for (j =0; j < cols; j++){
    myfile = fopen(filenames[j],mode2);
    //for (k= first_rowdata; k < first_rowdata + max_rows; k++){
    fseek(myfile,first_rowdata*sizeof(double),SEEK_SET);
    //printf("Writing %f at %d %d to %s\n",rowdata[j][k- first_rowdata],j,k- first_rowdata,filenames[j]);
	                                                          //fwrite(&rowdata[j][k- first_rowdata],sizeof(double),1,myfile);
	                                                          //}
    fwrite(&rowdata[j][0],sizeof(double),max_rows,myfile);
    /* for (k=0; k < max_rows; k++){
      printf("row writing %d %d with value %f\n",j,first_rowdata+k, rowdata[j][k]);
      } */
    
    
      
    fclose(myfile);
  } 

}

/**
 ** writes all columns out to files to keep buffer-file coherency.
 **
 **/

void BufferedMatrix::FlushAllColumns(){

  int k,lastcol;
  const char *mode2 ="rb+";
  FILE *myfile;
 
  if (cols < max_cols){
    lastcol = cols;
  } else {
    lastcol = max_cols;
  }
    
  
  for (k=0; k < lastcol; k++){
    myfile = fopen(filenames[which_cols[k]],mode2);
    fseek(myfile,0,SEEK_SET); 
    fwrite(coldata[k],sizeof(double),rows,myfile);
    fclose(myfile);  
  }


}


void BufferedMatrix::FlushOldestColumn(){
  
  const char *mode2 ="rb+";
  FILE *myfile;
  
  //printf("flushing column %d \n",which_cols[0]);
  myfile = fopen(filenames[which_cols[0]],mode2);
  fseek(myfile,0,SEEK_SET); 
  fwrite(coldata[0],sizeof(double),rows,myfile);
  fclose(myfile);  
  /* for (j=0; j < rows; j++){
    printf("writing %d %d with value %f\n",which_cols[0], j, coldata[0][j]);
    } */
  
}



void BufferedMatrix::LoadNewColumn(int col)
{
  const char *mode = "rb";
  FILE *myfile;
  double *tmpptr;
  int lastcol;
  int j;


  if (cols < max_cols){
    lastcol = cols;
  } else {
    lastcol = max_cols;
  }
  
  tmpptr = coldata[0];

  for (j=1; j < lastcol; j++){
    coldata[j-1] = coldata[j];
    which_cols[j-1] = which_cols[j];
  }
  
  which_cols[lastcol -1] = col;
  coldata[lastcol -1] = tmpptr;
  
  //printf("loading column %d \n",whichcol);
  myfile = fopen(filenames[col],mode);
  fseek(myfile,0,SEEK_SET);
  fread(coldata[lastcol -1],sizeof(double),rows,myfile);
  fclose(myfile);
  
}



void BufferedMatrix::LoadAdditionalColumn(int col, int where)
{
  const char *mode = "rb";
  FILE *myfile;
  //  double *tmpptr;
  //int lastcol;
  //int j;


  //  if (cols < max_cols){
  //    lastcol = cols;
  //  } else {
  //    lastcol = max_cols;
  //  }
  
  //  tmpptr = coldata[0];

  //  for (j=1; j < lastcol; j++){
  //    coldata[j-1] = coldata[j];
  //     which_cols[j-1] = which_cols[j];
  //   }
  
  coldata[where] = new double[rows];
  which_cols[where] = col;
  //printf("loading column %d \n",whichcol);
  myfile = fopen(filenames[col],mode);
  fseek(myfile,0,SEEK_SET);
  fread(coldata[where],sizeof(double),rows,myfile);
  fclose(myfile);
  
}





void BufferedMatrix::LoadRowBuffer(int row){
  
  const char *mode = "rb";
  FILE *myfile;
  int j,k;
  int lastcol;
  int curcol;


  if (cols < max_cols){
    lastcol = cols;
  } else {
    lastcol = max_cols;
  }
  
  if (row > rows - max_rows){
    this->first_rowdata = rows - max_rows;
  } else {
    this->first_rowdata = row;
  }
    
  //   printf("loading rows %d through %d\n",first_rowdata, first_rowdata+max_rows);
  for (j =0; j < cols; j++){
    
    myfile = fopen(filenames[j],mode);
    //for (k= first_rowdata; k < first_rowdata + max_rows; k++){
    //  fseek(myfile,k*sizeof(double),SEEK_SET);
    //  fread(&rowdata[j][k- first_rowdata],sizeof(double),1,myfile);
    //}
    fseek(myfile,first_rowdata*sizeof(double),SEEK_SET);
    fread(&rowdata[j][0],sizeof(double),max_rows,myfile);

    fclose(myfile);  
  }
  
  for (j =0; j < cols; j++){
    curcol =0;
    while (curcol < lastcol){
      if (which_cols[curcol] == j){
	//	printf("curcol is %d j is %d\n",curcol,j);
	for (k= first_rowdata; k < first_rowdata + max_rows; k++){
	  //printf("at %d %d in row buffer %f and at %d %d in column buffer %f\n",j,k,rowdata[j][k- first_rowdata],which_cols[curcol],k,coldata[curcol][k]);
	  rowdata[j][k- first_rowdata] = coldata[curcol][k];
	}	
      }
      curcol++;
    }
  }
}

void BufferedMatrix::ResizeRowBuffer(int new_maxrow){

  int i, j;
  double *tmpptr;
  int new_first_rowdata;

  if (new_maxrow <= 0){
    throw "Can't have 0 OR negative rows in buffer\n";
  }

  if (new_maxrow > rows){
    new_maxrow =rows;
  }

  if (colmode){
    max_rows =new_maxrow;
    return;
  }


  /* Fix up any potential clashes */
  if (rowcolclash){
    ClearClash();
  }

  if (max_rows == new_maxrow){
    // No need to do anything.
    return;
  } else if (max_rows > new_maxrow){
    // Remove rows from the rows buffer
    // Empty out row buffer (at least resync with files)
    FlushRowBuffer();
    
    for (j =0; j < cols; j++){
      // printf("fixing col %d in row buffer\n",j);
      tmpptr = rowdata[j];
      rowdata[j] = new double[new_maxrow];
      for (i=0; i < new_maxrow; i++){
	rowdata[j][i] = tmpptr[i];
      }
      delete [] tmpptr;
    }
    max_rows = new_maxrow;
  } else {
    // Increase number of rows in row buffer
    
    // Empty out row buffer (at least resync with files)
    FlushRowBuffer();

    // Allocate Space
    
    for (j =0; j < cols; j++){ 
      tmpptr = rowdata[j];
      rowdata[j] = new double[new_maxrow];
      delete [] tmpptr;
    }
      

    // Now see if we will be hitting the bottom of the matrix with the added rows
    
    if (first_rowdata + new_maxrow > rows){
      new_first_rowdata = rows - new_maxrow;
    } else {
      new_first_rowdata = rows;
    }
    max_rows = new_maxrow;
    LoadRowBuffer(new_first_rowdata);
    
  }

}


void BufferedMatrix::ResizeColBuffer(int new_maxcol){

  int i,j;
  int lastcol;
  int n_cols_remove=0;
  int n_cols_add=0; 
  double *tmpptr;
  double **tmpptr2;
  int *tmpptr3;

  int *whichadd;

  int curcol;
  int min_j;


    /* Fix up any potential clashes */
  if (rowcolclash){
    ClearClash();
  }
  // First figure out if need to mak any changes

  if (new_maxcol <= 0){
    throw "Can't have 0 or negative columns in buffer\n";
  }

  if (cols < max_cols){
    lastcol = cols;
  } else {
    lastcol = max_cols;
  }
  

  if (max_cols == new_maxcol){
    // No need to do anything.
    return;
  } else if (max_cols > new_maxcol){
    // Remove columns from the column buffer
    // Will remove max_col - new_maxcol oldest columns
    if (new_maxcol < cols){
      if (max_cols < cols){
	n_cols_remove = max_cols - new_maxcol;
      } else{
	n_cols_remove = cols - new_maxcol;
      }


      for (i=0; i < n_cols_remove; i++){
	FlushOldestColumn();
	tmpptr = coldata[0];
	for (j=1; j < lastcol; j++){
	  coldata[j-1] = coldata[j];
	  which_cols[j-1] = which_cols[j];
	}
	delete [] tmpptr;
      }
      
      tmpptr2 = coldata;
      tmpptr3 = which_cols;
      
      coldata = new double *[new_maxcol];
      which_cols = new int[new_maxcol];
      
      for (j=0; j < new_maxcol; j++){
	coldata[j] = tmpptr2[j];
	which_cols[j] = tmpptr3[j];
      }
      delete [] tmpptr2;
      delete [] tmpptr3;
    }
    max_cols = new_maxcol;

  } else {
    // Need to add columns to the column buffer
    
    if (new_maxcol < cols){
      n_cols_add = new_maxcol - max_cols;
    } else if (max_cols < cols){
      n_cols_add = cols - max_cols;
    } else {
      // there are no more columns to add, everything is already in the buffer
      n_cols_add = 0;
      max_cols = new_maxcol;
      return;
    }
    
    // Figure out which columns to add
    // rule will be to add columns in numerical order (ie column 0 if not it, then 1 if not in and so on)
    
    whichadd = new int[n_cols_add];
    
    min_j=0;
    for (i=0; i < n_cols_add; i++){
      for (j=min_j; j < cols; j++){
	//	printf("j is %d %d ",j, min_j);
	if(!InColBuffer(0,j,&curcol)){
	  whichadd[i] = j;
	  //printf("Adding %d\n",j);
	  break;
	}
      }
      min_j = j+1;
    }
    
    // Add columns to end of buffer
    tmpptr2 = coldata;
    tmpptr3 = which_cols;
    
    coldata = new double *[max_cols+ n_cols_add];
    which_cols = new int[new_maxcol+ n_cols_add];  
    for (j=0; j < max_cols; j++){
      coldata[j] = tmpptr2[j];
      which_cols[j] = tmpptr3[j];
    }
    
    for (i=0; i < n_cols_add; i++){
      LoadAdditionalColumn(whichadd[i], max_cols + i);
    }
    delete [] tmpptr2;
    delete [] tmpptr3;
    delete [] whichadd;

    max_cols = new_maxcol;
  }


}



void BufferedMatrix::ResizeBuffer(int new_maxrow, int new_maxcol){


  ResizeColBuffer(new_maxcol);
  if (!colmode){
    ResizeRowBuffer(new_maxrow);
  } else {
    /* No actual row buffer active. So just increase potential size.
       with caveats: Can't: Be smaller than 1 or be bigger than number of rows
       if those cases exist go to nearest possible value. ie 1 or max

    */
    if (new_maxrow < 1){
      max_rows = 1;
    } else if (new_maxrow > rows){
      max_rows = rows;
    } else {
      max_rows = new_maxrow;
    } 
  }

}



double &BufferedMatrix::operator()(int row, int col){
  
  int curcol;

  if (!colmode){
    /* Fix up any potential clashes */
    if (rowcolclash){
      ClearClash();
    }
    
    /* check to see if this cell is in row buffer, then column buffer, then read from files */
    
    if (InRowBuffer(row,col)){
      /* In row buffer so return */
      if (InColBuffer(row,col,&curcol)){
	SetClash(row,col);
      }
      
      return rowdata[col][row - first_rowdata];
    } else if (InColBuffer(row,col,&curcol)){
      return coldata[curcol][row];
    } else {
      
      /* looks like we are going to have to go to files */
      //printf("Couldn't find in buffers\n");

      if (!readonly){
	/* Flush buffers */ 
	/* First row buffer */
	FlushRowBuffer();
      
	/* Now flush the column buffer (for oldest column) */
	FlushOldestColumn();
      }
      
      /* Now fill up the buffer */
      /* read in this row and surrounding rows into row buffer */
      LoadRowBuffer(row);
      
      /* read in this column into column buffer */
      LoadNewColumn(col);
      
      
      SetClash(row,col);
      return rowdata[col][row - first_rowdata];
    
    }
  } else {
    if (InColBuffer(row,col,&curcol)){
      return coldata[curcol][row];
    } else {
      if (!readonly)
	FlushOldestColumn(); 
      LoadNewColumn(col);
      return coldata[max_cols -1][row];
    }
  }

  













}




void BufferedMatrix::RowMode(){
  int j;

  /**            When switching from column mode to row mode
 **             - allocate space for row buffer
 **             - load data in from files, when data is not 
 **               currently in column buffer
 **             - copy across relevant data that is in column buffer
 **             - set colmode flag to false
 */
  rowdata = new double *[cols +1];
  for (j =0; j < cols; j++){
    rowdata[j] = new double[this->max_rows];
  }
  LoadRowBuffer(0); /* this both fills the row buffer and copys across anything in the current column buffer */
  colmode =false;

}


void BufferedMatrix::ColMode(){
  int j;
  /* **            When switching from row mode to column mode
 **            - clear clashes
 **            - flush row buffer (ie write to files)
 **            - deallocate row buffer
 **            - set colmode flag to true
 ** */

  if (rowcolclash){
    ClearClash();
  }
  FlushRowBuffer();

  for (j =0; j < cols; j++){
    delete [] rowdata[j];
  }
  delete [] rowdata;
  colmode = true;
}


void BufferedMatrix::ReadOnlyMode(bool setting=false){

  /* 
     If readonly mode is true we don't need to do anything
     extra except set the flag.

     If however readonly is false and we are changing to
     true then we will need to flush the current
     buffers to ensure coherency.

  */


  if (!readonly & setting){
    if (!colmode){
      if (rowcolclash){
	ClearClash();
      }
      FlushRowBuffer();
    }
    FlushAllColumns();
  } 



  readonly = setting;

}




void BufferedMatrix::Compute5Summary(int col, double *results){

  int i;
  double med, UQ, LQ;
  double *buffer = new double[this->rows];
  
  GetFullColumn(col, buffer);
  
  
  //  qsort(buffer,this->rows,sizeof(double),(int(*)(const void*, const void*))sort_double);
  sort(buffer, buffer+this->rows);

  med = median_nocopy(buffer,this->rows);
  quartiles(buffer,this->rows, &LQ, &UQ);
 
  results[0] = buffer[0];
  results[1] = LQ;
  results[2] = med;
  results[3] = UQ;
  results[4] = buffer[this->rows-1];

  delete [] buffer;

}



void BufferedMatrix::GetFullColumn(int col, double *dest){

  int row = 0;
  int curcol;

  int i;


  if (colmode){
    /* In column mode all we need to do is see if column is in buffer
       and if so copy all the data into the supplied destination, if not in the 
       buffer then we need to load it in first then copy 
    */
       
    if (!InColBuffer(row,col,&curcol)){
      if (!readonly){
	FlushOldestColumn();
      }
      /* read in this column into column buffer */
      LoadNewColumn(col);
      memcpy(dest,&coldata[max_cols -1][0],rows*sizeof(double));
    } else {
      memcpy(dest,&coldata[curcol][0],rows*sizeof(double));
    }
  } else {
    /* we are in row mode */
    /* Need to copy out the data */

    for (i =0; i < rows; i++){
      dest[i] = (true,i,col);
    }
  }

}





double BufferedMatrix::getValue(int row, int col){

  double tmp;

  tmp = (*this)(row,col);
  
  if (!colmode && readonly){
    rowcolclash = 0;  /* If readonly. No need to worry about clashes */
  }


  return tmp;
}
