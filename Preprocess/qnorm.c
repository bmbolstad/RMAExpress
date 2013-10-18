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

/**********************************************************
 **
 ** file: qnorm.c
 **
 ** aim: A c implementation of the quantile normalization method 
 **
 ** Copyright (C) 2002-2008    Ben Bolstad
 **
 ** written by: B. M. Bolstad  <bmb@bmbolstad.com>
 **
 ** written: Feb 2, 2002
 ** last modified: Apr 19, 2002
 ** 
 ** This c code implements the quantile normalization method
 ** for normalizing high density oligonucleotide data as discussed
 ** in
 **
 ** Bolstad, B. M., Irizarry R. A., Astrand, M, and Speed, T. P. (2003)(2003) 
 ** A Comparison of Normalization Methods for High 
 ** Density Oligonucleotide Array Data Based on Bias and Variance.
 ** Bioinformatics 19,2,pp 185-193
 **
 ** History
 ** Feb 2, 2002 - Intial c code version from original R code
 ** Apr 19, 2002 - Update to deal more correctly with ties (equal rank)
 ** Jan 2, 2003 - Documentation/Commenting updates reformating
 ** Feb 17, 2003 - add in a free(datvec) to qnorm(). clean up freeing of dimat
 ** Feb 25, 2003 - try to reduce or eliminate compiler warnings (with gcc -Wall)
 ** Feb 28, 2003 - update reference to normalization paper in comments
 ** Mar 25, 2003 - ability to use median, rather than mean in so called "robust" method
 ** Apr 21, 2003 - changes for RMAExpress
 ** Jun 03, 2003 - add a mechanism for indicating failure of qnorm routine.
 **                this would usually be due to memory allocation problems
 ** Oct 31, 2003 - add a low memory overhead method (this will
 **                be slower, but use less memory)
 ** Mar 23, 2003 - make mallocs into callocs
 ** Mar 24, 2005 - BufferedMatrix support
 ** Sep 16, 2006 - fix compile problems related to Unicode wxWidget builds
 ** Jul 13, 2007 - fix ties when number of ties is even (very minor issue)
 ** Feb 5, 2008 - Based on submission from Bahram Parvin change qsort
 **               sorting to STL::sort style sorting
 **               Also remove older !low_mem code
 ** Feb 28, 2008 - BufferedMatrix indexing is now via() operator rather than []
 **
 ***********************************************************/

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>

#include <wx/progdlg.h>
#include "../rma_common.h"
#include "qnorm.h"

using namespace std;

typedef std::vector<double> DoubleArray;



/*************************************************************
 **
 ** the dataitem record is used to keep track of data indicies 
 ** along with data value when sorting and unsorting in the 
 ** quantile algorithm.
 **
 ************************************************************/

typedef struct{
  double data;
  int rank;
} dataitem;
  

typedef std::vector<dataitem> itemVect;

bool itemComp(const dataitem& a, const dataitem& b) {

  return a.data < b.data;
}

/***********************************************************
 **  
 ** int min(int x1, int x2)							    
 **
 ** returns the minimum of x1 and x2
 **		    
 **********************************************************/

int min(int x1,int x2){
  if (x1 > x2)
    return x2;
  else
    return x1;
}

/**********************************************************
 **
 ** int sort_fn(const void *a1,const void *a2)
 **
 ** a comparison function for sorting objects of the dataitem type.
 **
 **
 **********************************************************/

int sort_fn(const void *a1,const void *a2){
  dataitem *s1, *s2;
  s1 = (dataitem *)a1;
  s2 = (dataitem *)a2;
  
  if (s1->data < s2->data)
    return (-1);
  if (s1 ->data > s2->data)
    return (1);
  return 0;
}


/************************************************************
 **
 ** double *get_ranks(dataitem *x,int n)
 **
 ** get ranks in the same manner as R does. Assume that *x is
 ** already sorted
 **
 *************************************************************/

void get_ranks(double *rank, itemVect x,int n){  
  int i,j,k;
   
  i = 0;

  while (i < n) {
    j = i;
    while ((j < n - 1) && (x[j].data  == x[j + 1].data))
      j++;
    if (i != j) {
      for (k = i; k <= j; k++)
	rank[k] = (i + j + 2) / 2.0;
    }
    else
      rank[i] = i + 1;
    i = j + 1;
  }
  /*return rank;*/
}



/*********************************************************
 **
 ** void qnorm_c(double *data, int *rows, int *cols)
 **
 **  this is the function that actually implements the 
 ** quantile normalization algorithm. It is called from R.
 ** 
 ** returns 1 if there is a problem, 0 otherwise
 **
 ********************************************************/
#ifdef BUFFERED
int qnorm_c(BufferedMatrix *data, int *rows, int *cols, int *lowmem){
  int i,j,ind;
  double *row_mean = (double *)calloc((*rows),sizeof(double));
  
  double *ranks = (double *)calloc((*rows),sizeof(double));
  DoubleArray datvec;
  itemVect iv;
  dataitem di;

 
#if RMA_GUI_APP
    wxProgressDialog NormalizeProgress(_T("Normalizing"),_T("Normalizing"),*cols*2,NULL,wxPD_AUTO_HIDE| wxPD_APP_MODAL);
    NormalizeProgress.Update(1);
#endif

    /* Low memory overhead normalization */
   
    memset(row_mean, 0, *rows*sizeof(double));

    
    /* first find the normalizing distribution */
    for (j = 0; j < *cols; j++){
      datvec.reserve(*rows);
      for (i =0; i < *rows; i++){
	datvec.push_back((*data)(i,j));
      }
      sort(datvec.begin(), datvec.end());

      for (i =0; i < *rows; i++){
	row_mean[i] += datvec[i]/((double)*cols);
      }
      datvec.clear();
#if RMA_GUI_APP
      NormalizeProgress.Update(j);
#endif
    }
    
    /* now assign back distribution */
     
    for (j = 0; j < *cols; j++){
      iv.reserve(*rows);
      for (i =0; i < *rows; i++){  
	di.data = (*data)(i,j);
	di.rank = i;
	iv.push_back(di);
      }
      sort(iv.begin(), iv.end(), itemComp);
    
      get_ranks(ranks,iv,*rows);
      for (i =0; i < *rows; i++){
	ind = iv[i].rank;
	if (ranks[i] - floor(ranks[i]) > 0.4){
	  (*data)(ind,j) = 0.5*(row_mean[(int)floor(ranks[i])-1] + row_mean[(int)floor(ranks[i])]);
	} else { 
	  (*data)(ind,j) = row_mean[(int)floor(ranks[i])-1];
	}
      }
      iv.clear();
#if RMA_GUI_APP
  NormalizeProgress.Update(*cols+j);
#endif
    }
    
    free(ranks);
    free(row_mean);
    return 0;

}

#else

int qnorm_c(double *data, int *rows, int *cols, int *lowmem){
  int i,j,ind;
  
  double *row_mean = (double *)calloc((*rows),sizeof(double));
  double *ranks = (double *)calloc((*rows),sizeof(double));
  DoubleArray datavec;
  itemVect iv;
  dataitem di;
  
  /* Low memory overhead normalization */
   
  memset(row_mean, 0, *rows*sizeof(double));

  /* first find the normalizing distribution */
  for (j = 0; j < *cols; j++){ 
    datvec.reserve(*rows);
    for (i =0; i < *rows; i++){
      datvec.push_back((*data)[j*(*rows) + i]);
    }
    sort(datvec.begin(), datvec.end());
    for (i =0; i < *rows; i++){
      row_mean[i] += datvec[i]/((double)*cols);
    } 
    datvec.clear();
  }
  
  /* now assign back distribution */
    
  for (j = 0; j < *cols; j++){ 
    iv.reserve(*rows);
    for (i =0; i < *rows; i++){  
      di.data = data[j*(*rows) + i];
      di.rank = i;
      iv.push_back(di);
    }
    sort(iv.begin(), iv.end(), itemComp);
    
    get_ranks(ranks,iv,*rows);
    iv.clear(); 
    for (i =0; i < *rows; i++){
      ind = iv[i].rank;
	if (ranks[i] - floor(ranks[i]) > 0.4){
	  data[j*(*rows) +ind] = 0.5*(row_mean[(int)floor(ranks[i])-1] + row_mean[(int)floor(ranks[i])]);
	} else { 
	  data[j*(*rows) +ind] = row_mean[(int)floor(ranks[i])-1];
	}
    }
    iv.clear(); 
  }
  
  free(ranks);
  free(row_mean);
  return 0;
  
}
#endif


