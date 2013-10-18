#ifndef BUFFERED_MATRIX_H
#define BUFFERED_MATRIX_H

#include <wx/wx.h>

class BufferedMatrix
{
 public:
  BufferedMatrix(int max_cols=40, int max_rows=1000,char *prefix="bufmat");
  void SetRows(int);
  void SetPrefix(char *prefix);
  double &operator[](unsigned long i);
  double &operator()(int row, int col);
  
  double getValue(int row, int col);
  
  void AddColumn();
  //void AddColumn(double *x);
  ~BufferedMatrix();
  void ResizeColBuffer(int new_maxcol);
  void ResizeRowBuffer(int new_maxrow);
  void ResizeBuffer(int new_maxrow, int new_maxcol);
  void RowMode();
  void ColMode();
  void ReadOnlyMode(bool setting);

  void Compute5Summary(int col, double *results);

  void GetFullColumn(int col, double *dest);

 private:
  void SetClash(int row, int col);
  void ClearClash();
  bool InRowBuffer(int row, int col);
  bool InColBuffer(int row, int col,int *which_col_index);
  
  void FlushRowBuffer();
  void FlushOldestColumn();
  void FlushAllColumns();

  void LoadNewColumn(int col);
  void LoadRowBuffer(int row);

  void LoadAdditionalColumn(int col, int where);

  int rows;  // number of rows in matrix
  int cols;  // number of cols in matrix

  int max_cols; /* Maximum number of cols kept in RAM
                   in column buffered data */

  int max_rows; /* Maximum number of rows kept in RAM
		   in row buffered data  the maximum 
		   value that this should be is 1000 */
  double **coldata; /* RAM buffer containing stored data
                       its maximum size should be no more 
                       than max_cols*rows, anything else should be
                       written out temporarily to disk. 

		       If cols is less than max_cols, then entire contents
		       are in RAM
		    */

  double **rowdata; /* RAM buffer containing stored data it size will always
		     be max_rows*cols */

  
  int first_rowdata; /* matrix index of first row stored in rowdata  should be from 0 to rows */

  int *which_cols; /* vector containing indices of columns currently in col data. The 
                      "oldest" indice is first indice. Newest indicit is last indice. 
                       Note that the length this will be is min(cols, max_cols) */


  char **filenames; /* contains names of temporary files where data is stored  */

  
  char *fileprefix;

  bool rowcolclash;  /* referenced a cell location that is both in column and row buffer */
  int clash_row;     /* contains row index of potential clash */
  int clash_col;     /* contains column index of potential clash */
			

  bool colmode;      /* If true then in column mode so no rowdata (rows buffer) */
                     /* If false then in row mode, so both column and row buffers */

  bool readonly;     /* If true then no need to to carry out "flushing" activities 
		        but the operator [] cannot be used to set values. this should only be 
			set in situations where it is known that only reads are
			going to occur.
			
			If false then flush as normal (this is the default situation) */
  
};








#endif
