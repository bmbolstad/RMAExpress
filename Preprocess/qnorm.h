#ifndef QNORM_H
#define QNORM_H 1

#ifdef BUFFERED
#include "../Storage/BufferedMatrix.h"
int qnorm_c(BufferedMatrix *data, int *rows, int *cols, int *lowmem);
#else
int qnorm_c(double *data, int *rows, int *cols, int *lowmem);
#endif


#endif
