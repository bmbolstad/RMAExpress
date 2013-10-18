#include "BufferedMatrix.h"
#include <iostream.h>






int main(){

  int rows = 20;
  int cols = 20;
  int i;
  BufferedMatrix test(10,10);
  

  test.SetPrefix("/tmp/BHMMAMA");
  test.SetRows(rows);


  for (i =0; i < cols; i++){
    test.AddColumn();
  }


  //for (i=0; i < rows*cols;i++){
  //  cout << i << " " << test[i] << endl;
  //}



  //for (i=0; i < rows; i++)
  //   for (int j=0; j < cols; j++)
  //     cout << i << " " << j  << " "<< test[j*rows + i] << endl;
  

  for (i=0; i < rows*cols;i++){
    //cout << "About to write " << i/rows <<" " << i%rows << " with value " << i << endl;
    test[i] = i; 
  }
   
  for (i=0; i < rows*cols;i++){
    cout << i << " " << test[i] << endl;
  }


  test.AddColumn(); // add another column;
  cols ++;
  
  /* run down columns */
  for (i=0; i < rows*cols;i++){
    cout << i << " " << test[i] << endl;
  }
 

  /* run across rows - forwards */
  for (i=0; i < rows; i++)
    for (int j=0; j < cols; j++)
      cout << i << " " << j   << " " <<  j*rows + i << " "<< test[j*rows + i] << endl;

  /* run across rows - backwards */
  for (i=rows-1; i >= 0; i--)
    for (int j=cols-1; j >= 0; j--)
      cout << i << " " << j   << " " <<  j*rows + i << " "<< test[j*rows + i] << endl;


  /* assign across rows - backwards */

  for (i=rows-1; i >= 0; i--)
    for (int j=cols-1; j >= 0; j--)
      test[j*rows + i]  = (j*rows + i)*10;

  /* run down columns */
  for (i=0; i < rows*cols;i++){
    cout << i << " " << test[i] << endl;
  }


  /* Play with resizing buffer */

  
  test.ResizeColBuffer(5);


    /* run down columns */
  for (i=0; i < rows*cols;i++){
    cout << "resized cols down" << i << " " << test[i] << endl;
  }


  test.ResizeColBuffer(50);

  for (i=0; i < rows*cols;i++){
    cout << "resized cols up" << i << " " << test[i] << endl;
  }
  
  test.ResizeRowBuffer(5);
  for (i=0; i < rows*cols;i++){
    cout << "resized rows down" << i << " " << test[i] << endl;
  }
  
  test.ResizeColBuffer(5);
  test.ResizeRowBuffer(8);
  for (i=0; i < rows*cols;i++){
    cout << "resized rows up" << i << " " << test[i] << endl;
  }

  test.ResizeRowBuffer(50);
  for (i=0; i < rows*cols;i++){
    cout << "resized rows up Again" << i << " " << test[i] << endl;
  }

  
  /* run across rows - backwards */
  for (i=rows-1; i >= 0; i--)
    for (int j=cols-1; j >= 0; j--)
      cout << i << " " << j   << " " <<  j*rows + i << " "<< test[j*rows + i] << endl;


  
  test.ResizeBuffer(10,10);

  for (i=rows-1; i >= 0; i--)
    for (int j=cols-1; j >= 0; j--)
      cout << i << " " << j   << " " <<  j*rows + i << " "<< test[j*rows + i] << endl;


  test.AddColumn(); // add another column;
  cols ++;
  test.ResizeRowBuffer(8);
  for (i=0; i < rows*cols;i++){
    cout << "resized rows up" << i << " " << test[i] << endl;
  }

  test.ResizeBuffer(1,1); 
  for (i=rows-1; i >= 0; i--)
    for (int j=cols-1; j >= 0; j--)
      cout << i << " " << j   << " " <<  j*rows + i << " "<< test[j*rows + i] << endl;



  // Testing ReadOnlyMode
  
  test.ReadOnlyMode(true);



  









}
