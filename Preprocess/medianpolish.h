#ifndef MEDIANPOLISH_H
#define MEDIANPOLISH_H 1

#ifdef BUFFERED
#include "../Storage/BufferedMatrix.h"
void median_polish(BufferedMatrix *data, int rows, int cols, int *cur_rows, double *results, int nprobes);
#else
void median_polish(double *data, int rows, int cols, int *cur_rows, double *results, int nprobes);
#endif

#endif
