/*********************************************************************
 **
 ** file: rlm_anova.c
 **
 ** Aim: implement robust linear models specialized to samples + probes model.
 **
 ** Copyright (C) 2004-2008 Ben Bolstad
 **
 ** created by: B. M. Bolstad <bolstad@stat.berkeley.edu>
 ** 
 ** created on: June 23, 2003
 **
 ** Last modified: June 23, 2003
 **
 ** History:
 ** July 29, 2004 - change routines so output order is the same as 
 **                 in new structure.
 ** Mar 1, 2006 - change comment style to ansi
 ** Jan 25, 2007 - adapt code from affyPLM to work with RMAExpress
 ** Jan 28, 2007 - add PLM_summarize which wraps rlm_anova 
 ** Feb 28, 2008 - BufferedMatrix indexing is now via() operator rather than []
 **
 *********************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "psi_fns.h"
#include "matrix_functions.h"

#include "../threestep_common.h"
#include "../Storage/BufferedMatrix.h"



/**********************************************************************************
 **
 ** double med_abs(double *x, int length)
 **
 ** double *x - a vector of data
 ** int length - length of the vector.
 ** 
 ** returns the median of the absolute values.
 **
 ** computes the median of the absolute values of a given vector.
 **
 **********************************************************************************/

static double med_abs(double *x, int length){
  int i;
  double med_abs;
  double *buffer = (double *)calloc(length,sizeof(double));

  for (i = 0; i < length; i++)
    buffer[i] = fabs(x[i]);
  
  med_abs = median(buffer,length);
    
  free(buffer);
  return(med_abs);
}



/***************************************************************
 **
 ** double irls_delta(double *old, double *new, int length)
 **
 ** double *old - previous value of a vector
 ** double *new - new value of a vector
 ** int length - length of vector
 **
 ** this function computes the sum of the difference of two vectors
 ** divides this by the sum squared of the old datavector.
 **
 ** the aim of this function is compute something to test for 
 ** convergence in the iteratively reweighted least squares (IRLS)
 ** 
 **
 **************************************************************/

static double irls_delta(double *old, double *newv, int length){
  int i=0;
  double sum = 0.0;
  double sum2 =0.0;
  double divisor=1e-20;

  for (i=0; i < length; i++){
    sum = sum + (old[i] - newv[i])*(old[i]-newv[i]);
    sum2 = sum2 + old[i]*old[i];
  }
  
  if(sum2 >= divisor){
    divisor = sum2;
  }

  return sqrt(sum/divisor); 
} 


static void XTWY(int y_rows, int y_cols, double *wts,double *y, double *xtwy){

  int i,j;
  /* sweep columns (ie chip effects) */
   
  for (j=0; j < y_cols; j++){
    xtwy[j] = 0.0;
    for (i=0; i < y_rows; i++){
      xtwy[j] += wts[j*y_rows + i]* y[j*y_rows + i];
    }
  }
  
  /* sweep rows  (ie probe effects) */
 
  for (i=0; i < y_rows; i++){
    xtwy[i+y_cols] = 0.0;
    for (j=0; j < y_cols; j++){
      xtwy[i+y_cols] += wts[j*y_rows + i]* y[j*y_rows + i]; 
    }
  }

  for (i=0; i < y_rows-1; i++){
    xtwy[i+y_cols] = xtwy[i+y_cols] - xtwy[y_cols+y_rows-1];
  }
  
}


static void XTWX(int y_rows, int y_cols, double *wts, double *xtwx){

  int Msize = y_cols +y_rows-1;
  int i,j,k;

  /* diagonal elements of first part of matrix ie upper partition */
  for (j =0; j < y_cols;j++){
    for (i=0; i < y_rows; i++){
      xtwx[j*Msize + j]+=wts[j*y_rows + i];
    }
  }


  /* diagonal portion of lower partition matrix: diagonal elements*/ 
  for (j =0; j < y_cols;j++){
    for (i = 0; i < y_rows-1;i++){
      xtwx[(y_cols +i)*Msize + (y_cols +i)]+= wts[j*y_rows + i];
    }
  }
  /* diagonal portion of lower partition matrix: off diagonal elements*/ 
  for (j =0; j < y_cols;j++){
    for (i = 0; i < y_rows-1;i++){
      for (k=i ; k <  y_rows-1;k++){
	xtwx[(y_cols +k)*Msize + (y_cols +i)] = xtwx[(y_cols +i)*Msize + (y_cols +k)]+= wts[j*y_rows + (y_rows-1)];
      }
    }
  }

  /* the two other portions of the matrix */
  for (j =0; j < y_cols;j++){
    for (i= 0; i < y_rows-1;i++){
       xtwx[j*Msize + (y_cols + i)] = xtwx[(y_cols + i)*Msize + j] = wts[j*y_rows + i] - wts[j*y_rows + (y_rows-1)];
    }
  }

}



static void XTWXinv(int y_rows, int y_cols,double *xtwx){
  int i,j,k;
  int Msize = y_cols +y_rows-1;
  double *P= (double *)calloc(y_cols,sizeof(double));
  double *RP = (double *)calloc(y_cols*(y_rows-1),sizeof(double));
  double *RPQ = (double *)calloc((y_rows-1)*(y_rows-1),sizeof(double));
  double *S = (double *)calloc((y_rows-1)*(y_rows-1),sizeof(double));
  double *work = (double *)calloc((y_rows-1)*(y_rows-1),sizeof(double));
  
  for (j=0;j < y_cols;j++){
    for (i=0; i < y_rows -1; i++){
      RP[j*(y_rows-1) + i] = xtwx[j*Msize + (y_cols + i)]*(1.0/xtwx[j*Msize+j]);
    }
  } 
  
  for (i=0; i < y_rows -1; i++){
    for (j=i;j <  y_rows -1; j++){
      for (k=0; k < y_cols;k++){
	RPQ[j*(y_rows-1) + i] +=  RP[k*(y_rows-1) + j]*xtwx[k*Msize + (y_cols + i)];
      }
      RPQ[i*(y_rows-1) + j] = RPQ[j*(y_rows-1) + i];
    }
  }
  
  for (j=0; j < y_rows-1;j++){
    for (i=j; i < y_rows-1;i++){
      RPQ[i*(y_rows-1) + j] = RPQ[j*(y_rows-1)+i] =  xtwx[(y_cols + j)*Msize + (y_cols + i)] - RPQ[j*(y_rows-1) + i]; 
    }
  } 
  
  
  /*for (i =0; i<  y_rows-1; i++){
    for (j=0; j <  y_cols; j++){ 
      printf("%4.4f ",RP[j*(y_rows-1) + i]);
    }
    printf("\n");
    }
  
    for (j=0;j <  y_rows -1; j++){
    for (i=0; i < y_rows -1; i++){
    printf("%4.4f ",RPQ[j*(y_rows-1) + i]);
    }
    printf("\n");
    }
  

    for (i=0; i < y_rows -1; i++){
    for (j=0;j <  y_rows -1; j++){
    printf("%4.4f ",S[j*(y_rows-1) + i]);
    }
    printf("\n");
    }
  */
  



  /* Lets start making the inverse */

  Choleski_inverse(RPQ, S, work, y_rows-1, 0);

  for (j=0; j< y_cols;j++){
    for (i=0; i < y_rows -1; i++){
      xtwx[j*Msize + (y_cols + i)] = 0.0;
      for (k=0; k < y_rows -1; k++){
	xtwx[j*Msize + (y_cols + i)]+= -1.0*(S[i*(y_rows-1) + k])*RP[j*(y_rows-1) + k];
      }
      xtwx[(y_cols + i)*Msize + j]=xtwx[j*Msize + (y_cols + i)];
    }
  }


  for (j=0;j < y_cols;j++){
      P[j] = 1.0/xtwx[j*Msize+j];
  } 


  for (j=0; j < y_cols; j++){
    for (i=j; i < y_cols;i++){
      xtwx[i*Msize + j]=0.0;
      for (k=0;k < y_rows-1; k++){
	xtwx[i*Msize + j]+= RP[i*(y_rows-1) + k]*xtwx[j*Msize + (y_cols + k)];
      }
      xtwx[i*Msize + j]*=-1.0;
      xtwx[j*Msize + i] =  xtwx[i*Msize + j];
    }
    xtwx[j*Msize + j]+=P[j];
  }


  for (j=0; j < y_rows-1;j++){
    for (i=0; i < y_rows-1;i++){
      xtwx[(y_cols + j)*Msize + (y_cols + i)] = S[j*(y_rows-1)+i];
    }
  }


  free(P);
  free(work);
  free(RP);
  free(RPQ);
  free(S);

}


/**********************************************************************************
 **
 ** void rlm_fit_anova(double *y, int rows, int cols,double *out_beta, 
 **                double *out_resids, double *out_weights,
 **                double (* PsiFn)(double, double, int), double psi_k,int max_iter, 
 **                int initialized))
 **
 ** double *y - matrix of response variables (stored by column, with rows probes, columns chips
 ** int rows - dimensions of y
 ** int cols - dimensions of y
 **
 ** specializes procedure so decomposes matrix more efficiently
 ** note that routine is not as numerically stable as above.
 **
 ** fits a row + columns model
 **
 **********************************************************************************/


void rlm_fit_anova(double *y, int y_rows, int y_cols,double *out_beta, double *out_resids, double *out_weights,double (* PsiFn)(double, double, int), double psi_k,int max_iter, int initialized){

  int i,j,iter;
  /* double tol = 1e-7; */
  double acc = 1e-4;
  double scale =0.0;
  double conv;
  double endprobe;

  double *wts = out_weights; 

  double *resids = out_resids; 
  double *old_resids = (double *)calloc(y_rows*y_cols,sizeof(double));
  
  double *rowmeans = (double *)calloc(y_rows,sizeof(double));

  double *xtwx = (double *)calloc((y_rows+y_cols-1)*(y_rows+y_cols-1),sizeof(double));
  double *xtwy = (double *)calloc((y_rows+y_cols),sizeof(double));

  double sumweights;
  
  int rows;

  rows = y_rows*y_cols;
  
  if (!initialized){
    
    /* intially use equal weights */
    for (i=0; i < rows; i++){
      wts[i] = 1.0;
    }
  }

  /* starting matrix */
  
  for (i=0; i < y_rows; i++){
    for (j=0; j < y_cols; j++){
      resids[j*y_rows + i] = y[j*y_rows + i];
    }
  }
  
  /* sweep columns (ie chip effects) */

  for (j=0; j < y_cols; j++){
    out_beta[j] = 0.0;
    sumweights = 0.0;
    for (i=0; i < y_rows; i++){
      out_beta[j] += wts[j*y_rows + i]* resids[j*y_rows + i];
      sumweights +=  wts[j*y_rows + i];
    }
    out_beta[j]/=sumweights;
    for (i=0; i < y_rows; i++){
      resids[j*y_rows + i] = resids[j*y_rows + i] -  out_beta[j];
    }
  }


 /* sweep rows  (ie probe effects) */
  
  for (i=0; i < y_rows; i++){
    rowmeans[i] = 0.0;
    sumweights = 0.0;
    for (j=0; j < y_cols; j++){
      rowmeans[i] += wts[j*y_rows + i]* resids[j*y_rows + i]; 
      sumweights +=  wts[j*y_rows + i];
    }
    rowmeans[i]/=sumweights;
    for (j=0; j < y_cols; j++){
       resids[j*y_rows + i] =  resids[j*y_rows + i] - rowmeans[i];
    }
  }
  for (i=0; i < y_rows-1; i++){
    out_beta[i+y_cols] = rowmeans[i];
  }



  for (iter = 0; iter < max_iter; iter++){
    
    scale = med_abs(resids,rows)/0.6745;
    
    if (fabs(scale) < 1e-10){
      /*printf("Scale too small \n"); */
      break;
    }
    for (i =0; i < rows; i++){
      old_resids[i] = resids[i];
    }

    for (i=0; i < rows; i++){
      wts[i] = PsiFn(resids[i]/scale,psi_k,0);  /*           psi_huber(resids[i]/scale,k,0); */
    }
   
    /* printf("%f\n",scale); */


    /* weighted least squares */
    
    memset(xtwx,0,(y_rows+y_cols-1)*(y_rows+y_cols-1)*sizeof(double));


    XTWX(y_rows,y_cols,wts,xtwx);
    XTWXinv(y_rows, y_cols,xtwx);
    XTWY(y_rows, y_cols, wts,y, xtwy);

    
    for (i=0;i < y_rows+y_cols-1; i++){
      out_beta[i] = 0.0;
       for (j=0;j < y_rows+y_cols -1; j++){
    	 out_beta[i] += xtwx[j*(y_rows+y_cols -1)+i]*xtwy[j];
       }
    }

    /* residuals */
    
    for (i=0; i < y_rows-1; i++){
      for (j=0; j < y_cols; j++){
	resids[j*y_rows +i] = y[j*y_rows + i]- (out_beta[j] + out_beta[i + y_cols]); 
      }
    }

    for (j=0; j < y_cols; j++){
      endprobe=0.0;
      for (i=0; i < y_rows-1; i++){
	endprobe+= out_beta[i + y_cols];
      }
      resids[j*y_rows + y_rows-1] = y[j*y_rows + y_rows-1]- (out_beta[j] - endprobe);
    }

    /*check convergence  based on residuals */
    
    conv = irls_delta(old_resids,resids, rows);
    
    if (conv < acc){
      /*    printf("Converged \n");*/
      break; 

    }



  }
    
  /* order output in probes, samples order */
  /*
    for (i=0;i < y_rows+y_cols-1; i++){
    old_resids[i] = out_beta[i];
    }  
    for (i=0; i <y_rows-1;i++){
    out_beta[i] = old_resids[i+y_cols];
    }
    for (i=0; i < y_cols; i++){
    out_beta[i+(y_rows-1)] = old_resids[i];
    }
  */



  free(xtwx);
  free(xtwy);
  free(old_resids);
  free(rowmeans);


}

/*************************************************************************
 **
 ** void RLM_SE_Method_1_anova(double residvar, double *XTX, int p, double *se_estimates)
 **
 ** double residvar - residual variance estimate
 ** double *XTX - t(Design matrix)%*% Design Matrix
 ** double p - number of parameters
 ** double *se_estimates - on output contains standard error estimates for each of
 **                        the parametes
 **
 ** this function computes the parameter standard errors using the first
 ** method described in Huber (1981)
 ** 
 ** ie k^2 (sum psi^2/(n-p))/(sum psi'/n)^2 *(XtX)^(-1)
 **
 **
 ************************************************************************/


static void RLM_SE_Method_1_anova(double residvar, double *XTX, int y_rows,int y_cols, double *se_estimates,double *varcov){
  int i,j;
  int p = y_rows + y_cols -1;
  
  XTWXinv(y_rows, y_cols,XTX);


  for (i =0; i < p; i++){
    se_estimates[i] = sqrt(residvar*XTX[i*p + i]);
  }
  

  /*** for (i =0; i < y_rows-1; i++){
       se_estimates[i] = sqrt(residvar*XTX[(i+y_cols)*p + (i+y_cols)]);
       }
       for (i =0; i < y_cols; i++){
       se_estimates[i+(y_rows -1)] = sqrt(residvar*XTX[i*p + i]);
       }  ***/

  if (varcov != NULL)
    for (i =0; i < p; i++){
      for (j = i; j < p; j++){
	varcov[j*p +i]= residvar*XTX[j*p +i];
      }
    }
  

  /*** if (varcov != NULL){
       // copy across varcov matrix in right order 
       for (i = 0; i < y_rows-1; i++)
       for (j = i; j < y_rows-1; j++)
       varcov[j*p + i] =  residvar*XTX[(j+y_cols)*p + (i+y_cols)];
       
       for (i = 0; i < y_cols; i++)
       for (j = i; j < y_cols; j++)
       varcov[(j+(y_rows-1))*p + (i+(y_rows -1))] =  residvar*XTX[j*p + i];
       
       
       
       for (i = 0; i < y_cols; i++)
       for (j = y_cols; j < p; j++)
       varcov[(i+ y_rows -1)*p + (j - y_cols)] =  residvar*XTX[j*p + i];
       
       }  **/

}



/*************************************************************************
 **
 ** void RLM_SE_Method_2(double residvar, double *W, int p, double *se_estimates)
 **
 ** double residvar - residual variance estimate
 ** double *XTX - t(Design matrix)%*% Design Matrix
 ** double p - number of parameters
 ** double *se_estimates - on output contains standard error estimates for each of
 **                        the parametes
 **
 ** this function computes the parameter standard errors using the second
 ** method described in Huber (1981)
 ** 
 ** ie K*(sum psi^2/(n-p))/(sum psi'/n) *(W)^(-1)
 **
 **
 ************************************************************************/

static void RLM_SE_Method_2_anova(double residvar, double *W, int y_rows,int y_cols, double *se_estimates,double *varcov){
  int i,j; /* l,k; */
  int p = y_rows + y_cols -1;
  double *Winv = (double *)calloc(p*p,sizeof(double));
  double *work = (double *)calloc(p*p,sizeof(double));
  
  
  if (!Choleski_inverse(W,Winv,work,p,1)){
   for (i =0; i < p; i++){
      se_estimates[i] = sqrt(residvar*Winv[i*p + i]);
      /* printf("%f ", se_estimates[i]); */
    }
   /*for (i =0; i < y_rows-1; i++){
      se_estimates[i] = sqrt(residvar*Winv[(i+y_cols)*p + (i+y_cols)]);
      }
      for (i =0; i < y_cols; i++){
      se_estimates[i+(y_rows -1)] = sqrt(residvar*Winv[i*p + i]);
      } */
  } else {
    /* printf("Using a G-inverse\n"); */
    SVD_inverse(W, Winv,p);
    for (i =0; i < p; i++){
      se_estimates[i] = sqrt(residvar*Winv[i*p + i]);
      /* printf("%f ", se_estimates[i]); */
    }
    /* for (i =0; i < y_rows-1; i++){
       se_estimates[i] = sqrt(residvar*Winv[(i+y_cols)*p + (i+y_cols)]);
       }
       for (i =0; i < y_cols; i++){
       se_estimates[i+(y_rows -1)] = sqrt(residvar*Winv[i*p + i]);
       } */
  }
  
  if (varcov != NULL)
    for (i =0; i < p; i++){
      for (j = i; j < p; j++){
	varcov[j*p +i]= residvar*Winv[j*p +i];
      }
    }
  /** if (varcov != NULL){
       copy across varcov matrix in right order 
      for (i = 0; i < y_rows-1; i++)
      for (j = i; j < y_rows-1; j++)
      varcov[j*p + i] =  residvar*Winv[(j+y_cols)*p + (i+y_cols)];
      
      for (i = 0; i < y_cols; i++)
      for (j = i; j < y_cols; j++)
      varcov[(j+(y_rows-1))*p + (i+(y_rows -1))] =  residvar*Winv[j*p + i];
      
      
      
      for (i = 0; i < y_cols; i++)
      for (j = y_cols; j < p; j++)
      varcov[(i+ y_rows -1)*p + (j - y_cols)] =  residvar*Winv[j*p + i];
      } **/
  
  free(work);
  free(Winv);

}



/*************************************************************************
 **
 ** void RLM_SE_Method_3(double residvar, double *XTX, double *W, int p, double *se_estimates)
 **
 ** double residvar - residual variance estimate
 ** double *XTX - t(Design matrix)%*% Design Matrix
 ** double p - number of parameters
 ** double *se_estimates - on output contains standard error estimates for each of
 **                        the parametes
 **
 ** this function computes the parameter standard errors using the third
 ** method described in Huber (1981)
 ** 
 ** ie 1/(K)*(sum psi^2/(n-p))*(W)^(-1)(XtX)W^(-1)
 **
 **
 ************************************************************************/

static int RLM_SE_Method_3_anova(double residvar, double *XTX, double *W,  int y_rows,int y_cols, double *se_estimates,double *varcov){
  int i,j,k;   /* l; */
  int rv;
  int p = y_rows + y_cols -1;
  double *Winv = (double *)calloc(p*p,sizeof(double));
  double *work = (double *)calloc(p*p,sizeof(double));
  

  /***************** 

  double *Wcopy = Calloc(p*p,double);

  
  for (i=0; i <p; i++){
    for (j=0; j < p; j++){
      Wcopy[j*p + i] = W[j*p+i];
    }
    } **********************/
  
  if(Choleski_inverse(W,Winv,work,p,1)){
    SVD_inverse(W, Winv,p);
  }
  
  /*** want W^(-1)*(XtX)*W^(-1) ***/

  /*** first Winv*(XtX) ***/

  for (i=0; i <p; i++){
    for (j=0; j < p; j++){
      work[j*p + i] = 0.0;
      for (k = 0; k < p; k++){
	work[j*p+i]+= Winv[k*p +i] * XTX[j*p + k];
      }
    }
  }
 
  /* now again by W^(-1) */
  
   for (i=0; i <p; i++){
    for (j=0; j < p; j++){
      W[j*p + i] =0.0;
      for (k = 0; k < p; k++){
	W[j*p+i]+= work[k*p +i] * Winv[j*p + k];
      }
    }
   }
     

   /* make sure in right order */

   /*  for (i =0; i < y_rows-1; i++){
       se_estimates[i] = sqrt(residvar*W[(i+y_cols)*p + (i+y_cols)]);
       }
       for (i =0; i < y_cols; i++){
       se_estimates[i+(y_rows -1)] = sqrt(residvar*W[i*p + i]);
       }*/
   
   for (i =0; i < p; i++){
     se_estimates[i] = sqrt(residvar*W[i*p + i]);
     /*  printf("%f ", se_estimates[i]); */
   }

   rv = 0;
   
   if (varcov != NULL)
     for (i =0; i < p; i++){
       for (j = i; j < p; j++){
	 varcov[j*p +i]= residvar*W[j*p +i];
       }
     }

   /* if (varcov != NULL){
       copy across varcov matrix in right order 
      for (i = 0; i < y_rows-1; i++)
      for (j = i; j < y_rows-1; j++)
      varcov[j*p + i] =  residvar*W[(j+y_cols)*p + (i+y_cols)];
      
      for (i = 0; i < y_cols; i++)
      for (j = i; j < y_cols; j++)
      varcov[(j+(y_rows-1))*p + (i+(y_rows -1))] =  residvar*W[j*p + i];
      
      
      
      for (i = 0; i < y_cols; i++)
      for (j = y_cols; j < p; j++)
      varcov[(i+ y_rows -1)*p + (j - y_cols)] =  residvar*W[j*p + i];
      } */
   free(work);
   free(Winv);

   return rv;

}





/*********************************************************************
 **
 ** void rlm_compute_se(double *X,double *Y, int n, int p, double *beta, double *resids,double *weights,double *se_estimates, int method)
 **
 ** specializes to the probes + arrays model the method for computing the standard errors of the parameter estimates
 **
 *********************************************************************/

void rlm_compute_se_anova(double *Y, int y_rows,int y_cols, double *beta, double *resids,double *weights,double *se_estimates, double *varcov, double *residSE, int method,double (* PsiFn)(double, double, int), double psi_k){
  
  int i,j; /* counter/indexing variables */
  double k1 = psi_k;   /*  was 1.345; */
  double sumpsi2=0.0;  /* sum of psi(r_i)^2 */
  /*  double sumpsi=0.0; */
  double sumderivpsi=0.0; /* sum of psi'(r_i) */
  double Kappa=0.0;      /* A correction factor */
  double scale=0.0;
  int n = y_rows*y_cols;
  int p = y_rows + y_cols -1;
  double *XTX = (double *)calloc(p*p,sizeof(double));
  double *W = (double *)calloc(p*p,sizeof(double));
  double *work = (double *)calloc(p*p,sizeof(double));
  double RMSEw = 0.0;
  double vs=0.0,m,varderivpsi=0.0; 
  double *W_tmp=(double *)calloc(n,sizeof(double));


  if (method == 4){
    for (i=0; i < n; i++){
      RMSEw+= weights[i]*resids[i]*resids[i];
    }
    
    RMSEw = sqrt(RMSEw/(double)(n-p));

    residSE[0] =  RMSEw;


    XTWX(y_rows,y_cols,weights,XTX);
    if (y_rows > 1){
      XTWXinv(y_rows, y_cols,XTX);
    } else {
      for (i=0; i < p; i++){
	XTX[i*p + i] = 1.0/XTX[i*p + i];
      }
    }
    /* make sure in right order 
       
    for (i =0; i < y_rows-1; i++){
    se_estimates[i] = RMSEw*sqrt(XTX[(i+y_cols)*p + (i+y_cols)]);
    }
    for (i =0; i < y_cols; i++){
    se_estimates[i+(y_rows -1)] = RMSEw*sqrt(XTX[i*p + i]);
    } */
    
    for (i =0; i < p; i++){
      se_estimates[i] = RMSEw*sqrt(XTX[i*p + i]);
    }
    
    
    if (varcov != NULL)
      for (i = 0; i < p; i++)
	for (j = i; j < p; j++)
	  varcov[j*p + i] =  RMSEw*RMSEw*XTX[j*p + i];
    


    /*     if (varcov != NULL){
	   copy across varcov matrix in right order 
	   for (i = 0; i < y_rows-1; i++)
	   for (j = i; j < y_rows-1; j++)
	   varcov[j*p + i] =  RMSEw*RMSEw*XTX[(j+y_cols)*p + (i+y_cols)];
	   
	   for (i = 0; i < y_cols; i++)
	   for (j = i; j < y_cols; j++)
	   varcov[(j+(y_rows-1))*p + (i+(y_rows -1))] =  RMSEw*RMSEw*XTX[j*p + i];
	   
	   
      
	   for (i = 0; i < y_cols; i++)
	   for (j = y_cols; j < p; j++)
	   varcov[(i+ y_rows -1)*p + (j - y_cols)] =  RMSEw*RMSEw*XTX[j*p + i];
	   } */


  } else {
    scale = med_abs(resids,n)/0.6745;
    
    residSE[0] =  scale;
    
    /* compute most of what we will need to do each of the different standard error methods */
    for (i =0; i < n; i++){
      sumpsi2+= PsiFn(resids[i]/scale,k1,2)*PsiFn(resids[i]/scale,k1,2); 
      /* sumpsi += psi_huber(resids[i]/scale,k1,2); */
      sumderivpsi+= PsiFn(resids[i]/scale,k1,1);
    }
    
    m = (sumderivpsi/(double) n);

    for (i = 0; i < n; i++){
      varderivpsi+=(PsiFn(resids[i]/scale,k1,1) - m)*(PsiFn(resids[i]/scale,k1,1) - m);
    }
    varderivpsi/=(double)(n);

    /*    Kappa = 1.0 + (double)p/(double)n * (1.0-m)/(m); */


    Kappa = 1.0 + ((double)p/(double)n) *varderivpsi/(m*m);

    
    /* prepare XtX and W matrices */

    for (i=0; i < n; i++){
      W_tmp[i] = 1.0;
    }
    XTWX(y_rows,y_cols,W_tmp,XTX);
    
     for (i=0; i < n; i++){
       W_tmp[i] = PsiFn(resids[i]/scale,k1,1);
    }
    XTWX(y_rows,y_cols,W_tmp,W);

    if (method==1) {
      Kappa = Kappa*Kappa;
      vs = scale*scale*sumpsi2/(double)(n-p);
      Kappa = Kappa*vs/(m*m);
      RLM_SE_Method_1_anova(Kappa, XTX, y_rows,y_cols, se_estimates,varcov);
    } else if (method==2){
      vs = scale*scale*sumpsi2/(double)(n-p);
      Kappa = Kappa*vs/m;
      RLM_SE_Method_2_anova(Kappa, W, y_rows,y_cols, se_estimates,varcov);
      
    } else if (method==3){
      
      vs = scale*scale*sumpsi2/(double)(n-p);
      Kappa = 1.0/Kappa*vs;
      i = RLM_SE_Method_3_anova(Kappa, XTX, W, y_rows,y_cols, se_estimates,varcov);
      if (i){
	for (i=0; i <n; i++){
	  printf("%2.1f ", PsiFn(resids[i]/scale,k1,1));
	} 
	printf("\n");
      }
    } 
  }
  free(W_tmp);
  free(work);
  free(XTX);
  free(W);

}





void PLM_summarize(BufferedMatrix *data, int rows, int cols, int *cur_rows, double *results, double *results_se,int nprobes){


  int i,j;

  double *weights = (double *)calloc(nprobes*cols,sizeof(double));
  double *resids = (double *)calloc(nprobes*cols,sizeof(double));
  double *z = (double *)calloc(nprobes*cols,sizeof(double));

  double *beta = (double *)calloc(cols+nprobes,sizeof(double));
  double *se = (double *)calloc(cols+nprobes,sizeof(double));

  double residSE;


  for (j = 0; j < cols; j++){
    for (i =0; i < nprobes; i++){
      z[j*nprobes + i] = log((*data)(cur_rows[i],j))/log(2.0);  
    }
  } 
  

  rlm_fit_anova(z, nprobes, cols, beta, resids, weights,&psi_huber, 1.345,20, 0);
  rlm_compute_se_anova(z, nprobes, cols, beta,resids, weights, se, (double *)NULL, &residSE, 4, &psi_huber,1.345);
  
  for (j=0; j < cols; j++){
    results[j]=beta[j];
    results_se[j]=se[j];
  }




  for (j = 0; j < cols; j++){
    for (i =0; i < nprobes; i++){
      (*data)(cur_rows[i],j) = resids[j*nprobes + i];  
    }
  }
  
  
  free(weights);
  free(resids);
  free(z);
  free(beta);
  free(se);


}
