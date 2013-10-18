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
 ** file: rma_background3.cpp
 **
 ** Copyright (C) 2003-2008    B. M. Bolstad
 **
 ** aim: does traditional convolution model RMA background 
 **  
 ** by: B. M. Bolstad  <bmb@bmbolstad.com>
 ** 
 ** Created on: Apr 21, 2003
 **
 ** Description:
 **
 ** this file has a fairly convoluted history. dating back
 ** to parts of 2002.
 ** 
 ** Initially there was a file rma_background.c which
 ** worked with 1.0 version R package affy
 ** significant data structure changes were required
 ** for 1.1 release of affy and too prevent confusion
 ** the updated file was called rma_background2.c
 ** 
 ** this file draws an significant amount of code
 ** from these older files. The rename is to
 ** prevent confusion. rma_background2.c is still
 ** being used in affy (version 1.1-) and AffyExtensions
 ** (version 0.4,0.5-)
 **
 **
 ** History
 ** Apr 21, 2003 - Initial version.
 ** Mar 24, 2005 - Add Support for BufferedMatrix
 ** Feb 6, 2008 - max find_max use an STL based sort operation
 ** Feb 28, 2008 - BufferedMatrix indexing is now via() operator rather than []
 **
 *****************************************************/

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <algorithm>

#include "rma_background3.h"
#include "pnorm.h"
#include "../rma_common.h"
#include "weightedkerneldensity.h"


using namespace std;

typedef std::vector<double> DoubleArray;


/***********************************************************
 **
 ** double find_max(double *x, int length)
 **
 ** this function returns the max of x
 **
 ************************************************************/

static double find_max(double *x,int length){
  int i;
  double max;

  max = x[0];
  for (i=1; i < length; i++){
    if (x[i] > max){
      max = x[i];
    }
  }
  
  return max;
}

/***********************************************************
 **
 ** double find_max(double *x, int length)
 **
 ** this function returns the max of x
 **
 ************************************************************/
 
// double find_max(double *x,int length){
//   int i;
//   double max;
// 
//   DoubleArray buffer;
// 						
//   buffer.reserve(length);
//   for (i=0; i < length; i++){
//     buffer.push_back(x[i]);
//   }
//      
//   nth_element(buffer.begin(),buffer.end(),buffer.end());
// 
//   max = buffer[length-1];
//    
//   /* printf("max is %f \n", max); */
// 
//   
// 
// 
//   return max;
// }
 
/***************************************************************
 **
 ** double get_sd(double *MM, double MMmax, int rows, int cols, int column)
 **
 ** double *PM - pm matrix
 ** double PMmax - value of mode of PMs for column
 ** int rows,cols - dimensions of matrix
 ** int column - column (chip) of interest
 **
 ** estimate the sigma parameter given vector MM value of maximum of density
 ** of MM, dimensions of MM matrix and column of interest
 **
 **
 ***************************************************************/
#ifdef BUFFERED
double get_sd(BufferedMatrix *MM, double MMmax, int rows, int cols, int column){

  double sigma;
  double tmpsum = 0.0;
  int numtop=0;
  int i;
 
 
  for (i=0; i < rows; i++){
    if ((*MM)(i,column) < MMmax){
      tmpsum = tmpsum + ((*MM)(i,column) - MMmax)*((*MM)(i,column) - MMmax);
      numtop++;
    }
  }
  sigma = sqrt(tmpsum/(numtop -1))*sqrt(2.0)/0.85;
  return sigma;
   
}
#else
double get_sd(double *MM, double MMmax, int rows, int cols, int column){

  double sigma;
  double tmpsum = 0.0;
  int numtop=0;
  int i;
 
 
  for (i=0; i < rows; i++){
    if (MM[column*rows + i] < MMmax){
      tmpsum = tmpsum + (MM[column*rows + i] - MMmax)*(MM[column*rows + i] - MMmax);
      numtop++;
    }
  }
  sigma = sqrt(tmpsum/(numtop -1))*sqrt(2.0)/0.85;
  return sigma;
   
}
#endif

 
/*********************************************************************************
 **
 ** double  get_alpha(double *PM,double PMmax, int rows,int cols,int column)
 **
 ** double *PM - pm matrix
 ** double PMmax - value of mode of PMs for column
 ** int rows,cols - dimensions of matrix
 ** int column - column (chip) of interest
 **
 ** estimate the alpha parameter given vector PM value of maximum of density
 ** of PM, dimensions of MM matrix and column of interest
 **
 **
 ***********************************************************************/
#ifdef BUFFERED 
double get_alpha(BufferedMatrix *PM,double PMmax, int rows,int cols,int column){

  double alpha;
  double tmpsum = 0.0;
  int numtop=0;
  int i;
 
  for (i=0; i < rows; i++){
    if ((*PM)(i,column) > PMmax){
      tmpsum = tmpsum + ((*PM)(i,column) - PMmax);
      numtop++;
    }
  }
  alpha = numtop/tmpsum;
  return alpha ;
   
}
#else                                                                                                                                             
double get_alpha(double *PM,double PMmax, int rows,int cols,int column){

  double alpha;
  double tmpsum = 0.0;
  int numtop=0;
  int i;
 
  for (i=0; i < rows; i++){
    if (PM[column*rows + i] > PMmax){
      tmpsum = tmpsum + (PM[column*rows + i] - PMmax);
      numtop++;
    }
  }
  alpha = numtop/tmpsum;
  return alpha ;
   
}
#endif

/**************************************************************************************
 **
 ** double max_density(double *z,int rows,int cols,int column, SEXP fn,SEXP rho)
 **
 ** double *z - matrix of dimension rows*cols
 ** int cols - matrix dimension
 ** int rows - matrix dimension
 ** int column - column of interest
 ** SEXP fn - R function for estimation of density
 ** SEXP rho - an R environment to work within
 **
 *************************************************************************************/
#ifdef BUFFERED
double max_density2(BufferedMatrix *z,int rows,int cols,int column){

  int i;

  double *x;
  double *dens_x;
  double *dens_y;
  double max_y,max_x;
   
  int npts = 16384;

  dens_x = (double *)calloc(npts,sizeof(double));
  dens_y = (double *)calloc(npts,sizeof(double));

  x = (double *)calloc(rows,sizeof(double));
  
  z->GetFullColumn(column, x);
  
  KernelDensity_lowmem(x,&rows,dens_y,dens_x,&npts);

  max_y = find_max(dens_y,16384);
   
  i = 0;
  do {
    if (dens_y[i] == max_y)
      break;
    i++;
 
  } while(1);
   
  max_x = dens_x[i];

  free(dens_x);
  free(dens_y);
  free(x);
  
  return max_x;
 
}




#endif


double max_density(double *z,int rows,int cols,int column){

  int i;

  double *x;
  double *dens_x;
  double *dens_y;
  double max_y,max_x;
   
  int npts = 16384;

  dens_x = (double *)calloc(npts,sizeof(double));
  dens_y = (double *)calloc(npts,sizeof(double));


  //  KernelDensity(double *x, int *nxxx, double *weights, double *output, double *xords, int *nout)
    
  x = (double *)calloc(rows,sizeof(double));

  for (i=0; i< rows; i++){
    x[i] = z[column*rows +i];
  }
  
  
  KernelDensity_lowmem(x,&rows,dens_y,dens_x,&npts);

  max_y = find_max(dens_y,16384);
   
  i = 0;
  do {
    // printf("%d %f %f\n",i,dens_y[i],max_y);
    if (dens_y[i] == max_y)
      break;
    i++;
 
  } while(1);
   
  max_x = dens_x[i];

  free(dens_x);
  free(dens_y);
  free(x);

  return max_x;
 
}
 
/********************************************************************************
 **
 ** void bg_parameters(double *PM,double *MM, double *param, int rows, int cols, int column,SEXP fn,SEXP rho)
 **
 ** estimate the parameters for the background, they will be returned in *param
 ** param[0] is alpha, param[1] is mu, param[2] is sigma.
 **
 ** parameter estimates are same as those given by affy_1.1.1
 **
 **
 *******************************************************************************/
#ifdef BUFFERED
 void bg_parameters(BufferedMatrix *PM,BufferedMatrix *MM, double *param, int rows, int cols, int column){
  double PMmax;
  double MMmax;
  double sd,alpha;
   
#ifdef BUFFERED
  PMmax = max_density2(PM,rows, cols, column);
  MMmax = max_density2(MM,rows, cols, column);
#else
  PMmax = max_density(PM,rows, cols, column);
  MMmax = max_density(MM,rows, cols, column);
#endif
  sd = get_sd(MM,MMmax,rows,cols,column);
  alpha = get_alpha(PM,PMmax,rows,cols,column);
 
  param[0] = alpha;
  param[1] = MMmax;
  param[2] = sd;
  
 }
#else 
void bg_parameters(double *PM,double *MM, double *param, int rows, int cols, int column){

  double PMmax;
  double MMmax;
  double sd,alpha;
   
  PMmax = max_density(PM,rows, cols, column);
  MMmax = max_density(MM,rows, cols, column);

  sd = get_sd(MM,MMmax,rows,cols,column);
  alpha = get_alpha(PM,PMmax,rows,cols,column);
 
  param[0] = alpha;
  param[1] = MMmax;
  param[2] = sd;
 
}
 
#endif
                                                                                                                                              
/**********************************************************************************
 **
 ** double Phi(double x)
 **
 ** Compute the standard normal distribution function
 **
 *********************************************************************************/
 
double Phi(double x){
   return pnorm5(x,0.0,1.0,1,0);
}
 
/***********************************************************************************
 **
 ** double phi(double x)
 **
 ** compute the standard normal density.
 **
 **
 **********************************************************************************/
 
double phi(double x){
  //return dnorm4(x,0.0,1.0,0);
  double pi = 3.14159265358979323846; 
  return 1 / sqrt(2 * pi)* exp(-0.5 * x * x);
  
}
 
/************************************************************************************
 **
 ** void bg_adjust(double *PM,double *MM, double *param, int rows, int cols, int column)
 **
 ** double *PM - PM matrix of dimension rows by cols
 ** double *MM - MM matrix of dimension rows by cols
 ** double *param - background model parameters
 ** int rows, cols - dimension of matrix
 ** int column - which column to adjust
 **
 ** note we will assume that param[0] is alpha, param[1] is mu, param[2] is sigma
 **
 ***********************************************************************************/
#ifdef BUFFERED 
 void bg_adjust(BufferedMatrix *PM, BufferedMatrix *MM, double *param, int rows, int cols, int column){

  int i;
  double a;
   
  for (i=0; i < rows; i++){
    a = (*PM)(i,column) - param[1] - param[0]*param[2]*param[2];
    (*PM)(i,column) = a + param[2] * phi(a/param[2])/Phi(a/param[2]);
  }
 }
#else
void bg_adjust(double *PM,double *MM, double *param, int rows, int cols, int column){


  int i;
  double a;
   
  for (i=0; i < rows; i++){
    a = PM[column*rows + i] - param[1] - param[0]*param[2]*param[2];
    PM[column*rows + i] = a + param[2] * phi(a/param[2])/Phi(a/param[2]);
  }
   
}
#endif

 
/***************************************************************
 **
 ** double  get_alpha2(double *PM,double PMmax, int rows,int cols,int column)
 **
 ** estimate the alpha parameter given vector PM value of maximum of density
 ** of PM, dimensions of MM matrix and column of interest using method proposed
 ** in affy2
 **
 **
 ***************************************************************/
 
double get_alpha2(double *PM, double PMmax, int length){
  double alpha;
  /* double tmpsum = 0.0;
     int numtop=0; */
  int i;
 
  for (i=0; i < length; i++){
    PM[i] = PM[i] - PMmax;
  }
 
  alpha = max_density(PM,length, 1,0);
 
  alpha = 1.0/alpha;
  return alpha ;
}
 
/********************************************************************************
 **
 ** void bg_parameters2(double *PM,double *MM, double *param, int rows, int cols, int column,SEXP fn,SEXP rho)
 **
 ** estimate the parameters for the background, they will be returned in *param
 ** param[0] is alpha, param[1] is mu, param[2] is sigma.
 **
 ** parameter estimates are same as those given by affy in bg.correct.rma (Version 1.1 release of affy)
 **
 *******************************************************************************/
 
#ifdef BUFFERED 
void bg_parameters2(BufferedMatrix *PM,BufferedMatrix *MM, double *param, int rows, int cols, int column){

  int i = 0;
  double PMmax;
  /* double MMmax; */
  double sd,alpha;
  int n_less=0,n_more=0;
  double *tmp_less = (double *)malloc(rows*sizeof(double));
  double *tmp_more = (double *)malloc(rows*sizeof(double));
  double tmp;

  PMmax = max_density2(PM,rows, cols, column);

  for (i=0; i < rows; i++){
    tmp = (*PM)(i,column);
    if (tmp < PMmax){ 
      tmp_less[n_less] = tmp; 
      n_less++;
    }
 
  }
 
  PMmax = max_density(tmp_less,n_less,1,0);
  sd = get_sd(PM,PMmax,rows,cols,column)*0.85;
 
  for (i=0; i < rows; i++){
    tmp = (*PM)(i,column);
    if  (tmp > PMmax){
      tmp_more[n_more] = tmp;
      n_more++;
    }
  }
 
  /* the 0.85 is to fix up constant in above */
  alpha = get_alpha2(tmp_more,PMmax,n_more);
 
  param[0] = alpha;
  param[1] = PMmax;
  param[2] = sd;
 
  /* printf("%f %f %f\n",param[0],param[1],param[2]); */
 
 
  free(tmp_less);
  free(tmp_more);
}
#else
void bg_parameters2(double *PM,double *MM, double *param, int rows, int cols, int column){

  int i = 0;
  double PMmax;
  /* double MMmax; */
  double sd,alpha;
  int n_less=0,n_more=0;
  double *tmp_less = (double *)malloc(rows*sizeof(double));
  double *tmp_more = (double *)malloc(rows*sizeof(double));
   

  PMmax = max_density(PM,rows, cols, column);

  for (i=0; i < rows; i++){
    if (PM[column*rows +i] < PMmax){
      tmp_less[n_less] = PM[column*rows +i];
      n_less++;
    }
 
  }
 
  PMmax = max_density(tmp_less,n_less,1,0);
  sd = get_sd(PM,PMmax,rows,cols,column)*0.85;
 
  for (i=0; i < rows; i++){
    if (PM[column*rows +i] > PMmax) {
      tmp_more[n_more] = PM[column*rows +i];
      n_more++;
    }
  }
 
  /* the 0.85 is to fix up constant in above */
  alpha = get_alpha2(tmp_more,PMmax,n_more);
 
  param[0] = alpha;
  param[1] = PMmax;
  param[2] = sd;
 
  /* printf("%f %f %f\n",param[0],param[1],param[2]); */
 
 
  free(tmp_less);
  free(tmp_more);
}
#endif 
