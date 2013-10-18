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

/************************************************************************
 **
 ** file: medianpolish.c
 **
 ** Copyright (C) 2002-2008 Ben Bolstad
 **
 ** created by: B. M. Bolstad   <bolstad@stat.berkeley.edu>
 ** created on: Jan 7, 2003 (but based on code dating back as far as June 2002)
 **
 ** last modified: Jan 7, 2003
 **
 ** License: GPL V2 or later (same as the rest of the Affy package)
 **
 ** Median polish summary measure (used in the RMA expression measure)
 **
 **
 ** History
 **
 ** Jan 7, 2003 - Initial version to fit into the three-step framework.
 ** Jan 13, 2003 - move median() into threestep_common.c
 ** Feb 24, 2003 - make maxiter get used.
 ** Apr 21, 2003 - Changes to get it to work with RMAExpress
 ** Mar 24, 2005 - BufferedMatrix support
 ** Feb 28, 2008 - BufferedMatrix indexing is now via() operator rather than []
 **
 ************************************************************************/


#include <cstdio>
#include <cstdlib>
#include <cmath>
//#include "rma_structures.h"

#include "../rma_common.h"
#include "../threestep_common.h"

#include "medianpolish.h"
/**************************************************************************
 **
 ** double median(double *x, int length)
 **
 ** double *x - vector
 ** int length - length of *x
 **
 ** returns the median of *x
 **
 *************************************************************************/

/*double  median(double *x, int length){
  int i;
  int half;
  double med;
  double *buffer = malloc(length*sizeof(double));
  
  for (i = 0; i < length; i++)
    buffer[i] = x[i];
  
  qsort(buffer,length,sizeof(double), (int(*)(const void*, const void*))sort_double);
  half = (length + 1)/2;
  if (length % 2 == 1){
    med = buffer[half - 1];
  } else {
    med = (buffer[half] + buffer[half-1])/2.0;
  }
  
  free(buffer);
  return med;}

*/

/*******************************************************************************
 **
 ** double sum_abs(double *z, int rows, int cols)
 **
 ** double *z - matrix of doubles
 ** int rows - dimension of matrix
 ** int cols - dimension of matrix
 **
 ** returns the sum of the absolute values of elements of the matrix *z
 **
 ******************************************************************************/

double sum_abs(double *z, int rows, int cols){
 
  int i, j;
  double sum = 0.0;

  for (i=0; i < rows; i++)
    for (j=0; j < cols; j++)
      sum+=fabs(z[j*rows+i]);

  return sum;
}

/********************************************************************************
 **
 ** void get_row_median(double *z, double *rdelta, int rows, int cols)
 **
 ** double *z - matrix of dimension  rows*cols
 ** double *rdelta - on output will contain row medians (vector of length rows)
 ** int rows, cols - dimesion of matrix
 **
 ** get the row medians of a matrix 
 **
 ********************************************************************************/

void get_row_median(double *z, double *rdelta, int rows, int cols){
  int i,j;
  double *buffer = (double *)malloc(cols*sizeof(double));

  for (i = 0; i < rows; i++){ 
    for (j = 0; j < cols; j++){
      buffer[j] = z[j*rows + i];
    }
    rdelta[i] = median(buffer,cols);
  }
  
  free(buffer);
}

/********************************************************************************
 **
 ** void get_col_median(double *z, double *cdelta, int rows, int cols)
 **
 ** double *z - matrix of dimension  rows*cols
 ** double *cdelta - on output will contain col medians (vector of length cols)
 ** int rows, cols - dimesion of matrix
 **
 ** get the col medians of a matrix 
 **
 ********************************************************************************/

void get_col_median(double *z, double *cdelta, int rows, int cols){
  
  int i, j;
  
  double *buffer = (double *)malloc(rows*sizeof(double));
  for (j = 0; j < cols; j++){
    for (i = 0; i < rows; i++){  
      buffer[i] = z[j*rows + i];
    }
    cdelta[j] = median(buffer,rows);
  }
  
  free(buffer);

}

/***********************************************************************************
 **
 ** void subtract_by_row(double *z, double *rdelta, int rows, int cols)
 ** 
 ** double *z - matrix of dimension rows by cols
 ** double *rdelta - vector of length rows
 ** int rows, cols dimensions of matrix
 **
 ** subtract the elements of *rdelta off each row of *z
 **
 ***********************************************************************************/

void subtract_by_row(double *z, double *rdelta, int rows, int cols){
  
  int i,j;

  for (i = 0; i < rows; i++){
    for (j = 0; j < cols; j++){
      z[j*rows +i]-= rdelta[i];
    }
  }
}


/***********************************************************************************
 **
 ** void subtract_by_col(double *z, double *cdelta, int rows, int cols)
 ** 
 ** double *z - matrix of dimension rows by cols
 ** double *cdelta - vector of length rows
 ** int rows, cols dimensions of matrix
 **
 ** subtract the elements of *cdelta off each col of *z
 **
 ***********************************************************************************/

void subtract_by_col(double *z, double *cdelta, int rows, int cols){
  
  int i,j;
  for (j = 0; j < cols; j++){
    for (i = 0; i < rows; i++){
      z[j*rows +i]-= cdelta[j];
    }
  }

}

/***********************************************************************************
 **
 ** void rmod(double *r, double *rdelta, int rows)
 ** 
 ** double *r - vector of length rows
 ** double *rdelta - vector of length rows
 ** int rows, cols dimensions of matrix
 **
 ** add elementwise *rdelta to *r
 **
 ***********************************************************************************/


void rmod(double *r, double *rdelta, int rows){
  int i;

  for (i = 0; i < rows; i++){
    r[i]= r[i] + rdelta[i];
  }
}

/***********************************************************************************
 **
 ** void cmod(double *c, double *cdelta, int cols)
 ** 
 ** double *c - vector of length rows
 ** double *cdelta - vector of length rows
 ** int cols length of vector
 **
 ** add elementwise *cdelta to *c
 **
 ***********************************************************************************/

void cmod(double *c, double *cdelta, int cols){
  int j;

  for (j = 0; j < cols; j++){
    c[j]= c[j] + cdelta[j];
  }
}


/*************************************************************************************
 **
 ** void median_polish(double *data, int rows, int cols, int *cur_rows, double *results, int nprobes)
 **
 ** double *data - a data matrix of dimension rows by cols (the entire PM matrix)
 ** int rows, cols - rows and columns dimensions of matrix
 ** int cur_rows - vector of length nprobes containg row indicies of *data matrix which apply for a 
 **                particular probeset
 ** double *results - a vector of length cols already allocated. on output contains expression values
 ** int nprobes - number of probes in current probeset.
 **
 ** a function to do median polish expression summary.
 **
 *************************************************************************************/
#ifdef BUFFERED
void median_polish(BufferedMatrix *data, int rows, int cols, int *cur_rows, double *results, int nprobes){
#else
void median_polish(double *data, int rows, int cols, int *cur_rows, double *results, int nprobes){
#endif
  int i,j,iter;
  int maxiter = 10;
  double eps=0.01;
  double oldsum = 0.0,newsum = 0.0;
  double t = 0.0;
  double delta;
  double *rdelta = (double *)calloc(nprobes,sizeof(double));
  double *cdelta = (double *)calloc(cols,sizeof(double));
  
  double *r = (double *)calloc(nprobes,sizeof(double));
  double *c = (double *)calloc(cols,sizeof(double));
  double *z = (double *)calloc(nprobes*cols,sizeof(double));

  for (j = 0; j < cols; j++){
    for (i =0; i < nprobes; i++){
      z[j*nprobes + i] = log((*data)(cur_rows[i],j))/log(2.0);  
    }
  } 
  
  
  for (iter = 1; iter <= maxiter; iter++){
    get_row_median(z,rdelta,nprobes,cols);
    subtract_by_row(z,rdelta,nprobes,cols);
    rmod(r,rdelta,nprobes);
    delta = median(c,cols);
    for (j = 0; j < cols; j++){
      c[j] = c[j] - delta;
    }
    t = t + delta;
    get_col_median(z,cdelta,nprobes,cols);
    subtract_by_col(z,cdelta,nprobes,cols);
    cmod(c,cdelta,cols);
    delta = median(r,nprobes);
    for (i =0; i < nprobes; i ++){
      r[i] = r[i] - delta;
    }
    t = t+delta;
    newsum = sum_abs(z,nprobes,cols);
    if (newsum == 0.0 || fabs(1.0 - oldsum/newsum) < eps)
      break;
    oldsum = newsum;
  }
  
  for (j=0; j < cols; j++){
    results[j] =  t + c[j]; 
  }
   
  for (j = 0; j < cols; j++){
    for (i =0; i < nprobes; i++){
      (*data)(cur_rows[i],j) = z[j*nprobes + i];  
    }
  } 
  
  free(rdelta);
  free(cdelta);
  free(r);
  free(c);
  free(z); 
}
