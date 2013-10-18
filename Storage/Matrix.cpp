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
 ** file: Matrix.cpp
 **
 ** Copyright (C) 2003    B. M. Bolstad
 **
 ** aim: A class to represent a resizable matrix of doubles. 
 **      Where resizable means we may add columns of data.
 **
 ** Created by: B. M. Bolstad  <bolstad@stat.berkeley.edu>
 ** 
 ** Created on: Aug 11, 2003
 **
 ** History
 ** Aug 11, 2003 - Initial version
 ** Oct 18, 2004 - small memory problem fixed
 **
 **
 *****************************************************/


#include "Matrix.h"


Matrix::Matrix(){

  this->rows = 0;
  this->cols = 0;
  
  this->data = 0;



}




Matrix::Matrix(int rows, int cols){
  
  int i=0,j;

  this->rows = rows;
  this->cols = cols;
  
  this->data = new double*[cols];
  
  for (i=0; i < cols; i++){
    this->data[i] = new double[rows];
    for (j=0; j < rows; j++){
      this->data[i][j] = 0.0;
    }
  }
  
}


Matrix::Matrix(double *x, int rows, int cols){

  int i=0,j=0;

  this->rows = rows;
  this->cols = cols;
  
  this->data = new double*[cols];
  
  for (i=0; i < cols; i++){
    this->data[i] = new double[rows];
  }
  
  for (j=0; j < cols; j++){
    for (i=0; i < rows; i++){
      data[i][j] = x[j*rows + i];
    }
  }
}



void Matrix::SetRows(int rows){

  this->rows = rows;


}








double &Matrix::operator[](unsigned long i){

  int whichcol = i/rows;
  int whichrow = i % rows;

  

  
  return data[whichcol][whichrow];
    

}



void Matrix::AddColumn(){

  int j,i;
  
  double **temp_ptr = new double *[cols+1];
  double **temp_ptr2 = data;

  for (j =0; j < cols; j++){
    temp_ptr[j] = data[j];
  }
  
  temp_ptr[cols] = new double[rows];
  for (i =0; i < rows; i++){
    temp_ptr[cols][j] = 0.0;
  }
  
  data = temp_ptr;
  delete [] temp_ptr2;
  cols++;
}






void Matrix::AddColumn(double *x){

  int j,i;

  double **temp_ptr = new double *[cols+1];
  double **temp_ptr2 = data;

  for (j =0; j < cols; j++){
    temp_ptr[j] = data[j];
  }
  
  temp_ptr[cols] = new double[rows];
  
  for (i =0; i < rows; i++){
    temp_ptr[cols][i] = x[i];
  }

  data = temp_ptr;
  delete [] temp_ptr2;
  cols++;
}


Matrix::~Matrix(){
  int i;

  for (i=0; i < cols; i++){
    delete [] data[i];
  }
  
  delete [] data;

}


int Matrix::Rows(){
  return rows;
}

int Matrix::Cols(){
  return cols;
}

