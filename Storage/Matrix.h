#ifndef MATRIX_H
#define MATRIX_H

class Matrix
{
 public:
  Matrix();
  Matrix(int rows, int cols);
  Matrix(double *x, int rows, int cols);
  void SetRows(int);
  double &operator[](unsigned long i);
  void AddColumn();
  void AddColumn(double *x);
  int Rows();
  int Cols();
  ~Matrix();
 private:
  int rows;
  int cols;
  double **data;
};








#endif
