#ifndef RMA_BACKGROUND3_H
#define RMA_BACKGROUND3_H

#ifdef BUFFERED
#include "../Storage/BufferedMatrix.h"
void bg_parameters2(BufferedMatrix *PM,BufferedMatrix *MM, double *param, int rows, int cols, int column);
void bg_adjust(BufferedMatrix *PM, BufferedMatrix *MM, double *param, int rows, int cols, int column);
#else
void bg_parameters2(double *PM,double *MM, double *param, int rows, int cols, int column);
void bg_adjust(double *PM,double *MM, double *param, int rows, int cols, int column);
#endif


#endif
