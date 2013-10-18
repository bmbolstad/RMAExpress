/*********************************************************************
 **
 ** file: threestep_common.c
 **
 ** Aim: Commonly used routines for threestep methods
 **
 ** Copyright (C) 2003-2007 Ben Bolstad 
 **
 ** created by: B. M. Bolstad <bolstad@stat.berkeley.edu>
 ** 
 ** created on: Jan 13, 2003
 **
 ** Last modified: Jan 13, 2003
 **
 ** History
 **
 ** Jan 13, 2003 - Initial version
 ** Feb 16, 2007 - add additional functionality
 ** Oct 25m 2007 - add some defines to help when compiling using VC++
 **
 ********************************************************************/

#include <cstdlib>
#include <cmath>


#include "rma_common.h"
#include "threestep_common.h"


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

double median(double *x, int length){
  int i;
  int half;
  double med;
  double *buffer = (double *)malloc(length*sizeof(double));

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
  return med;
}



double median_nocopy(double *x, int length){
  int i;
  int half;
  double med;
  double *buffer = x;

  for (i = 0; i < length; i++)
    buffer[i] = x[i];

  qsort(buffer,length,sizeof(double), (int(*)(const void*, const void*))sort_double);
  half = (length + 1)/2;
  if (length % 2 == 1){
    med = buffer[half - 1];
  } else {
    med = (buffer[half] + buffer[half-1])/2.0;
  }


  return med;
}


double median_nocopy_hasNA(double *x, int length,int num_na){
  int i;
  int half;
  double med;
  double *buffer = x;

  for (i = 0; i < length; i++)
    buffer[i] = x[i];

  qsort(buffer,length,sizeof(double), (int(*)(const void*, const void*))sort_double);
  half = (length - num_na+ 1)/2;
  if (length % 2 == 1){
    med = buffer[half - 1];
  } else {
    med = (buffer[half] + buffer[half-1])/2.0;
  }


  return med;
}







double quartiles(double *x, int length, double *LQ, double *UQ){

  double lowindex, highindex;
  double lowfloor, highfloor;
  double lowceil, highceil;
  int low_i, high_i;
  double low_h, high_h;


  double qslow, qshigh;
  
  lowindex = (double)(length -1)*0.25;
  highindex = (double)(length -1)*0.75;

  lowfloor = floor(lowindex);
  highfloor = floor(highindex);

  lowceil = ceil(lowindex);
  highceil = ceil(highindex);
  
  low_i = lowindex > lowfloor;
  high_i = highindex > highfloor;
  
  qslow = x[(int)lowfloor];
  qshigh = x[(int)highfloor];
  
  low_h = lowindex - lowfloor;
  high_h = highindex - highfloor;
  
  if (low_h > 1e-10){
    qslow = (1.0 - low_h)*qslow + low_h*x[(int)lowceil];
  }
  if (high_h > 1e-10){
    qshigh = (1.0 - high_h)*qshigh + high_h*x[(int)highceil];
  }


  *UQ = qshigh;
  *LQ = qslow;

  return qshigh - qslow;

}

