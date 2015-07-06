#ifndef QNORM_H
#define QNORM_H 1

#ifdef BUFFERED
#include "../Storage/BufferedMatrix.h"
#if RMA_GUI_APP
int qnorm_c(BufferedMatrix *data, int *rows, int *cols, int *lowmem, wxProgressDialog *NormalizeProgress);
#else
int qnorm_c(BufferedMatrix *data, int *rows, int *cols, int *lowmem);
#endif
#else
int qnorm_c(double *data, int *rows, int *cols, int *lowmem);
#endif


#endif
