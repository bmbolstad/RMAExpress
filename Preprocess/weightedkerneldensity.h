#ifndef WEIGHTEDKERNELDENSITY_H
#define WEIGHTEDKERNELDENSITY_H

void KernelDensity(double *data, int *numRows, double *weights, 
  double *output, double *xords, int *nout);
void KernelDensity_lowmem(double *data, int *numRows, double *output, 
  double *output_x, int *nout);

#endif // WEIGHTEDKERNELDENSITY_H