
/****************************************************************
 **
 ** File: read_celfile_text.c
 **
 ** Copyright (C) 2008 B. M. Bolstad
 **
 ** aim: implement parsing of text formatted CEL files
 **
 ** History
 ** - originally this was a port of the code used in affyio for parsing
 **   text format CEL files
 ** Mar 6-7 - refactor code so that rather than throwing "errors", error codes
 **           are passed back up
 ** Jun 24, 2008 - change char* to const char* where appropriate
 **
 **
 **
 **
 ***************************************************************/

#include <cstdio>
#include <cstdlib>


#include <cmath>  /* for NAN */
#include <cstring>
#include <wx/string.h>

#include "read_cel_structures.h"
#include "read_celfile_text.h"
#include "../threestep_common.h"

//#define HAVE_ZLIB 0

#if defined(HAVE_ZLIB)
#include <zlib.h>
#endif

#define BUF_SIZE 1024


static void error(const char *msg, const char *msg2="", const char *msg3=""){
  wxString Error = wxString((const char*)msg,wxConvUTF8) +_T(" ") + wxString((const char*)msg2,wxConvUTF8)  +_T(" ") + wxString((const char*)msg3,wxConvUTF8)+ _T("\n");
  throw Error;
}


/****************************************************************
 ****************************************************************
 **
 ** Code for spliting strings into tokens. 
 ** Not heavily used anymore
 **
 ***************************************************************
 ***************************************************************/

/***************************************************************
 **
 ** tokenset
 ** 
 ** char **tokens  - a array of token strings
 ** int n - number of tokens in this set.
 **
 ** a structure to hold a set of tokens. Typically a tokenset is
 ** created by breaking a character string based upon a set of 
 ** delimiters.
 **
 **
 **************************************************************/

typedef struct{
  char **tokens;
  int n;
} tokenset;



/******************************************************************
 **
 ** tokenset *tokenize(char *str, char *delimiters)
 **
 ** char *str - a string to break into tokens
 ** char *delimiters - delimiters to use in breaking up the line
 **
 **
 ** RETURNS a new tokenset
 **
 ** Given a string, split into tokens based on a set of delimitors
 **
 *****************************************************************/

static tokenset *tokenize(char *str, const char *delimiters){

  int i=0;

  char *current_token;
  tokenset *my_tokenset = (tokenset *)calloc(1,sizeof(tokenset));
  my_tokenset->n=0;
  
  my_tokenset->tokens = NULL;

  current_token = strtok(str,delimiters);
  while (current_token != NULL){
    my_tokenset->n++;
    my_tokenset->tokens = (char **)realloc(my_tokenset->tokens,(my_tokenset->n)*sizeof(char*));
    my_tokenset->tokens[i] = (char *)calloc(strlen(current_token)+1,sizeof(char));
    strcpy(my_tokenset->tokens[i],current_token);
    my_tokenset->tokens[i][(strlen(current_token))] = '\0';
    i++;
    current_token = strtok(NULL,delimiters);
  }

  return my_tokenset; 
}


/******************************************************************
 **
 ** int tokenset_size(tokenset *x)
 **
 ** tokenset *x - a tokenset
 ** 
 ** RETURNS the number of tokens in the tokenset 
 **
 ******************************************************************/

static int tokenset_size(tokenset *x){
  return x->n;
}


/******************************************************************
 **
 ** char *get_token(tokenset *x, int i)
 **
 ** tokenset *x - a tokenset
 ** int i - index of the token to return
 ** 
 ** RETURNS pointer to the i'th token
 **
 ******************************************************************/

static char *get_token(tokenset *x,int i){
  return x->tokens[i];
}

/******************************************************************
 **
 ** void delete_tokens(tokenset *x)
 **
 ** tokenset *x - a tokenset
 ** 
 ** Deallocates all the space allocated for a tokenset 
 **
 ******************************************************************/

static void delete_tokens(tokenset *x){
  
  int i;

  for (i=0; i < x->n; i++){
    free(x->tokens[i]);
  }
  free(x->tokens);
  free(x);
}

/*******************************************************************
 **
 ** int token_ends_with(char *token, char *ends)
 ** 
 ** char *token  -  a string to check
 ** char *ends_in   - we are looking for this string at the end of token
 **
 **
 ** returns  0 if no match, otherwise it returns the index of the first character
 ** which matchs the start of *ends.
 **
 ** Note that there must be one additional character in "token" beyond 
 ** the characters in "ends". So
 **
 **  *token = "TestStr"
 **  *ends = "TestStr"   
 **  
 ** would return 0 but if 
 **
 ** ends = "estStr"
 **
 ** we would return 1.
 **
 ** and if 
 ** 
 ** ends= "stStr"
 ** we would return 2 .....etc
 **
 **
 ******************************************************************/

static int token_ends_with(char *token, const char *ends_in){
  
  int tokenlength = strlen(token);
  int ends_length = strlen(ends_in);
  int start_pos;
  char *tmp_ptr;
  
  if (tokenlength <= ends_length){
    /* token string is too short so can't possibly end with ends */
    return 0;
  }
  
  start_pos = tokenlength - ends_length;
  
  tmp_ptr = &token[start_pos];

  if (strcmp(tmp_ptr,ends_in)==0){
    return start_pos;
  } else {
    return 0;
  }
}


/****************************************************************
 ****************************************************************
 **
 ** Code for dealing with text CEL files.
 **
 ***************************************************************
 ***************************************************************/

/****************************************************************
 **
 ** void ReadFileLine(char *buffer, int buffersize, FILE *currentFile)
 **
 ** char *buffer  - place to store contents of the line
 ** int buffersize - size of the buffer
 ** FILE *currentFile - FILE pointer to an opened CEL file.
 **
 ** Read a line from a file, into a buffer of specified size.
 ** otherwise die.
 **
 ***************************************************************/

static int ReadFileLine(char *buffer, int buffersize, FILE *currentFile){
  if (fgets(buffer, buffersize, currentFile) == NULL){
    return TEXT_MISSING_LINE;
  }
  return 0;
}	  


/****************************************************************
 **
 ** FILE *open_cel_file(const char *filename)
 **
 ** const char *filename - name of file to open
 **
 **
 ** RETURNS a file pointer to the open file
 **
 ** this file will open the named file and check to see that the 
 ** first characters agree with "[CEL]" 
 **
 ***************************************************************/

static FILE *open_cel_file(const char *filename){
  
  const char *mode = "r";
  FILE *currentFile = NULL; 
  char buffer[BUF_SIZE];

  currentFile = fopen(filename,mode);
  if (currentFile == NULL){
     error("Could not open file %s", filename);
  } else {
    /** check to see if first line is [CEL] so looks like a CEL file**/
    ReadFileLine(buffer, BUF_SIZE, currentFile);
    if (strncmp("[CEL]", buffer, 4) == 0) {
      rewind(currentFile);
    } else {
      error("The file %s does not look like a CEL file",filename);
    }
  }
  
  return currentFile;

}

/******************************************************************
 **
 ** void findStartsWith(FILE *my_file,char *starts, char *buffer)
 **
 ** FILE *my_file - an open file to read from
 ** char *starts - the string to search for at the start of each line
 ** char *buffer - where to place the line that has been read.
 **
 **
 ** Find a line that starts with the specified character string.
 ** At exit buffer should contain that line
 **
 *****************************************************************/


static int findStartsWith(FILE *my_file,const char *starts, char *buffer){

  int starts_len = strlen(starts);
  int match = 1;
  
  int errorCode =0;

  do {
    errorCode = ReadFileLine(buffer, BUF_SIZE, my_file);
    if (errorCode){
      errorCode = TEXT_DID_NOT_FIND_STARTS_WITH;
      break;
    }
    match = strncmp(starts, buffer, starts_len);
  } while (match != 0);

  return errorCode;

}


/******************************************************************
 **
 ** void AdvanceToSection(FILE *my_file,char *sectiontitle, char *buffer)
 **
 ** FILE *my_file - an open file
 ** char *sectiontitle - string we are searching for
 ** char *buffer - return's with line starting with sectiontitle
 **
 **
 *****************************************************************/

static int AdvanceToSection(FILE *my_file,const char *sectiontitle, char *buffer){
  if (findStartsWith(my_file,sectiontitle,buffer)){
    return TEXT_DID_NOT_FIND_SECTION;
  }
  return 0;
}


/******************************************************************
 ** 
 ** int check_cel_file(const char *filename, const char *ref_cdfName, int ref_dim_1, int ref_dim_2)
 **
 ** const char *filename - the file to read
 ** const char *ref_cdfName - the reference CDF filename
 ** int ref_dim_1 - 1st dimension of reference cel file
 ** int ref_dim_2 - 2nd dimension of reference cel file
 **
 ** returns 0 if no problem, 1 otherwise
 **
 ** The aim of this function is to read the header of the CEL file
 ** in particular we will look for the rows beginning "Cols="  and "Rows="
 ** and then for the line DatHeader=  to scope out the appropriate cdf
 ** file. An error() will be flagged if the appropriate conditions
 ** are not met.
 **
 **
 ******************************************************************/

int check_cel_file(const char *filename, const char *ref_cdfName, int ref_dim_1, int ref_dim_2){

  int i;
  int dim1,dim2;

  FILE *currentFile; 
  char buffer[BUF_SIZE];
  tokenset *cur_tokenset;

  currentFile = open_cel_file(filename);
  
  int errCode;

  errCode = AdvanceToSection(currentFile,"[HEADER]",buffer);

  if (errCode){
    fclose(currentFile);
    return TEXT_DID_NOT_FIND_HEADER_SECTION;
  }


  errCode = findStartsWith(currentFile,"Cols",buffer);
  if (errCode){
    fclose(currentFile);
    return TEXT_DID_NOT_FIND_COL_DIMENSION;
  }
  cur_tokenset = tokenize(buffer,"=");
  dim1 = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);

  errCode = findStartsWith(currentFile,"Rows",buffer);
  if (errCode){
    fclose(currentFile);
    return TEXT_DID_NOT_FIND_ROW_DIMENSION;
  }
  cur_tokenset = tokenize(buffer,"=");
  dim2 = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);
  

  if ((dim1 != ref_dim_1) || (dim2 != ref_dim_2)){
    return CEL_NON_MATCHING_DIMENSIONS;
  }
  
  
  errCode = findStartsWith(currentFile,"DatHeader",buffer);
  if (errCode){
    fclose(currentFile);
    return TEXT_DID_NOT_FIND_DAT_HEADER;
  }

  cur_tokenset = tokenize(buffer," ");
  for (i =0; i < tokenset_size(cur_tokenset);i++){
    if (strncasecmp(get_token(cur_tokenset,i),ref_cdfName,strlen(ref_cdfName)) == 0){
      break;
    }
    if (i == (tokenset_size(cur_tokenset) - 1)){
      fclose(currentFile);
      delete_tokens(cur_tokenset);
      return CEL_NON_MATCHING_CDFNAMES;
    }
  }
  delete_tokens(cur_tokenset);
  fclose(currentFile);

  return 0;
}

/************************************************************************
 **
 ** int read_cel_file_intensities(const char *filename, double *intensity, int chip_num, int rows, int cols)
 **
 ** const char *filename - the name of the cel file to read
 ** double *intensity  - the intensity matrix to fill
 ** int chip_num - the column of the intensity matrix that we will be filling
 ** int rows - dimension of intensity matrix
 ** int cols - dimension of intensity matrix
 **
 ** returns 0 if successful, non zero if unsuccessful
 **
 ** This function reads from the specified file the cel intensities for that
 ** array and fills a column of the intensity matrix.
 **
 ************************************************************************/

int read_cel_file_intensities(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows){
  
  int i, cur_x,cur_y,cur_index;
  double cur_mean;
  FILE *currentFile; 
  char buffer[BUF_SIZE];
  /* tokenset *cur_tokenset;*/
  char *current_token;

  int errCode;

  currentFile = open_cel_file(filename);
  
  errCode = AdvanceToSection(currentFile,"[INTENSITY]",buffer);
  if (errCode){
    fclose(currentFile);
    return TEXT_DID_NOT_FIND_INTENSITY_SECTION;
  }
  

  errCode = findStartsWith(currentFile,"CellHeader=",buffer);  
  if (errCode){
    fclose(currentFile);
    return TEXT_DID_NOT_FIND_CELLHEADER;
  }

  
  for (i=0; i < rows; i++){
    errCode = ReadFileLine(buffer, BUF_SIZE,  currentFile);
    if (errCode){
      fclose(currentFile);
      return TEXT_INTENSITY_TRUNCATED;
    }

    if (strlen(buffer) <=2){
      wxPrintf(wxT("Warning: found an empty line where not expected in %s.\nThis means that there is a cel intensity missing from the cel file.\nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, i);
      return TEXT_INTENSITY_TRUNCATED;
      
    }

    current_token = strtok(buffer," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      return TEXT_INTENSITY_TRUNCATED;
    }

    cur_x = atoi(current_token);
    current_token = strtok(NULL," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      return TEXT_INTENSITY_TRUNCATED;
    }

    cur_y = atoi(current_token);
    current_token = strtok(NULL," \t");  
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      return TEXT_INTENSITY_TRUNCATED;
    }

    if (cur_x < 0 || cur_x >= chip_dim_rows){
      return TEXT_INTENSITY_CORRUPTED;
    }
    if (cur_y < 0 || cur_y >= chip_dim_rows){
      return TEXT_INTENSITY_CORRUPTED;
    }

    cur_mean = atof(current_token);

    if (cur_mean < 0 || cur_mean > 65536){
      return TEXT_INTENSITY_CORRUPTED;
    }


    cur_index = cur_x + chip_dim_rows*(cur_y);
    intensity[chip_num*rows + cur_index] = cur_mean;
    /* delete_tokens(cur_tokenset); */
  }

  fclose(currentFile);

  if (i != rows){
    return TEXT_INTENSITY_TRUNCATED;
  }


  return 0;
}


#if defined(INCLUDE_ALL_AFFYIO)

/************************************************************************
 **
 ** int read_cel_file_stddev(const char *filename, double *intensity, int chip_num, int rows, int cols)
 **
 ** const char *filename - the name of the cel file to read
 ** double *intensity  - the intensity matrix to fill
 ** int chip_num - the column of the intensity matrix that we will be filling
 ** int rows - dimension of intensity matrix
 ** int cols - dimension of intensity matrix
 **
 ** returns 0 if successful, non zero if unsuccessful
 **
 ** This function reads from the specified file the cel stddev for that
 ** array and fills a column of the intensity matrix.
 **
 ************************************************************************/

int read_cel_file_stddev(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows){
  
  int i, cur_x,cur_y,cur_index;
  double cur_mean, cur_stddev;
  FILE *currentFile; 
  char buffer[BUF_SIZE];
  /* tokenset *cur_tokenset;*/
  char *current_token;

  currentFile = open_cel_file(filename);
  
  AdvanceToSection(currentFile,"[INTENSITY]",buffer);
  findStartsWith(currentFile,"CellHeader=",buffer);  
  
  for (i=0; i < rows; i++){
    ReadFileLine(buffer, BUF_SIZE,  currentFile);
    /* cur_tokenset = tokenize(buffer," \t");
    cur_x = atoi(get_token(cur_tokenset,0));
    cur_y = atoi(get_token(cur_tokenset,1));
    cur_mean = atof(get_token(cur_tokenset,2)); */
    
    if (strlen(buffer) <=2){
      wxPrintf(wxT("Warning: found an empty line where not expected in %s.\n This means that there is a cel intensity missing from the cel file.\n Sucessfully read to cel intensity %d of %d expected\n"), filename, i-1, i);
      break;
    }

    current_token = strtok(buffer," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    cur_x = atoi(current_token);

    current_token = strtok(NULL," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    cur_y = atoi(current_token);
    
    current_token = strtok(NULL," \t");
     if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    cur_mean = atof(current_token);

    current_token = strtok(NULL," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    cur_stddev = atof(current_token);


    cur_index = cur_x + chip_dim_rows*(cur_y);
    intensity[chip_num*rows + cur_index] = cur_stddev;
    /* delete_tokens(cur_tokenset); */
  }

  fclose(currentFile);

  if (i != rows){
    return 1;
  }

  return 0;
}




/************************************************************************
 **
 ** int read_cel_file_npixels(const char *filename, double *intensity, int chip_num, int rows, int cols)
 **
 ** const char *filename - the name of the cel file to read
 ** double *intensity  - the intensity matrix to fill
 ** int chip_num - the column of the intensity matrix that we will be filling
 ** int rows - dimension of intensity matrix
 ** int cols - dimension of intensity matrix
 **
 ** returns 0 if successful, non zero if unsuccessful
 **
 ** This function reads from the specified file the cel stddev for that
 ** array and fills a column of the intensity matrix.
 **
 ************************************************************************/

int read_cel_file_npixels(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows){
  
  int i, cur_x,cur_y,cur_index,cur_npixels;
  double cur_mean, cur_stddev;
  FILE *currentFile; 
  char buffer[BUF_SIZE];
  /* tokenset *cur_tokenset;*/
  char *current_token;

  currentFile = open_cel_file(filename);
  
  AdvanceToSection(currentFile,"[INTENSITY]",buffer);
  findStartsWith(currentFile,"CellHeader=",buffer);  
  
  for (i=0; i < rows; i++){
    ReadFileLine(buffer, BUF_SIZE,  currentFile);
    /* cur_tokenset = tokenize(buffer," \t");
    cur_x = atoi(get_token(cur_tokenset,0));
    cur_y = atoi(get_token(cur_tokenset,1));
    cur_mean = atof(get_token(cur_tokenset,2)); */
    
    if (strlen(buffer) <=2){
      wxPrintf(wxT("Warning: found an empty line where not expected in %s.\n This means that there is a cel intensity missing from the cel file.\n Sucessfully read to cel intensity %d of %d expected\n"), filename, i-1, i);
      break;
    }

    current_token = strtok(buffer," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    cur_x = atoi(current_token);
    current_token = strtok(NULL," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    
    cur_y = atoi(current_token);
    current_token = strtok(NULL," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    
    cur_mean = atof(current_token);
    current_token = strtok(NULL," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    cur_stddev = atof(current_token);
    
    current_token = strtok(NULL," \t");  
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    
    cur_npixels = atoi(current_token);
    
    cur_index = cur_x + chip_dim_rows*(cur_y);
    intensity[chip_num*rows + cur_index] = (double)cur_npixels;
    /* delete_tokens(cur_tokenset); */
  }

  fclose(currentFile);
  
  if (i != rows){
    return 1;
  }

  return 0;
}














/****************************************************************
 **
 ** void apply_masks(const char *filename, double *intensity, int chip_num, 
 **                   int rows, int cols,int chip_dim_rows, 
 **                   int rm_mask, int rm_outliers)
 **
 ** const char *filename    - name of file to open
 ** double *intensity - matrix of probe intensities
 ** int chip_num - the index 0 ...n-1 of the chip we are dealing with
 ** int rows - dimension of the intensity matrix
 ** int cols - dimension of the intensity matrix
 ** int chip_dim_rows - a dimension of the chip
 ** int rm_mask - if true locations in the MASKS section are set NA
 ** int rm_outliers - if true locations in the OUTLIERS section are set NA
 **
 ** This function sets the MASK and OUTLIER probes to NA
 ** 
 **
 ****************************************************************/

void apply_masks(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows, int rm_mask, int rm_outliers){
  
  int i;
  int numcells, cur_x, cur_y, cur_index;
  FILE *currentFile;
  char buffer[BUF_SIZE];
  tokenset *cur_tokenset;

  if ((!rm_mask) && (!rm_outliers)){
    /* no masking or outliers */
    return;
  }
  
  currentFile = open_cel_file(filename);
  /* read masks section */
  if (rm_mask){

    AdvanceToSection(currentFile,"[MASKS]",buffer);
    findStartsWith(currentFile,"NumberCells=",buffer); 
    cur_tokenset = tokenize(buffer,"=");
    numcells = atoi(get_token(cur_tokenset,1));
    delete_tokens(cur_tokenset);
    findStartsWith(currentFile,"CellHeader=",buffer); 


     for (i =0; i < numcells; i++){
       ReadFileLine(buffer, BUF_SIZE, currentFile);
       
       
       cur_tokenset = tokenize(buffer," \t");
       cur_x = atoi(get_token(cur_tokenset,0));
       cur_y = atoi(get_token(cur_tokenset,1));
       
       cur_index = cur_x + chip_dim_rows*(cur_y);
       intensity[chip_num*rows + cur_index] = NAN;
       delete_tokens(cur_tokenset); 
     }
  }

  /* read outliers section */

  if (rm_outliers){
    
    AdvanceToSection(currentFile,"[OUTLIERS]",buffer);
    findStartsWith(currentFile,"NumberCells=",buffer);
    cur_tokenset = tokenize(buffer,"=");
    numcells = atoi(get_token(cur_tokenset,1));
    delete_tokens(cur_tokenset);
    findStartsWith(currentFile,"CellHeader=",buffer); 
    for (i = 0; i < numcells; i++){
      ReadFileLine(buffer, BUF_SIZE, currentFile);      
      cur_tokenset = tokenize(buffer," \t");
      cur_x = atoi(get_token(cur_tokenset,0));
      cur_y = atoi(get_token(cur_tokenset,1));
      
      cur_index = cur_x + chip_dim_rows*(cur_y);
      intensity[chip_num*rows + cur_index] = NAN;
      delete_tokens(cur_tokenset); 
    }
  }
  
  fclose(currentFile);

}


/****************************************************************
 **
 ** static void get_masks_outliers(const char *filename, 
 **                         int *nmasks, short **masks_x, short **masks_y, 
 **                         int *noutliers, short **outliers_x, short **outliers_y
 ** 
 ** This gets the x and y coordinates stored in the masks and outliers sections
 ** of the cel files.
 **
 ****************************************************************/

void get_masks_outliers(const char *filename, int *nmasks, short **masks_x, short **masks_y, int *noutliers, short **outliers_x, short **outliers_y){
  
  FILE *currentFile;
  char buffer[BUF_SIZE];
  int numcells, cur_x, cur_y; 
  tokenset *cur_tokenset;
  int i;


  currentFile = open_cel_file(filename);
  /* read masks section */
  

  AdvanceToSection(currentFile,"[MASKS]",buffer);
  findStartsWith(currentFile,"NumberCells=",buffer); 
  cur_tokenset = tokenize(buffer,"=");
  numcells = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);
  findStartsWith(currentFile,"CellHeader=",buffer); 
  
  *nmasks = numcells;

  *masks_x = (short *)calloc(numcells,sizeof(short));
  *masks_y = (short *)calloc(numcells,sizeof(short));


  for (i =0; i < numcells; i++){
    ReadFileLine(buffer, BUF_SIZE, currentFile);
    
    
    cur_tokenset = tokenize(buffer," \t");
    cur_x = atoi(get_token(cur_tokenset,0));
    cur_y = atoi(get_token(cur_tokenset,1));
    (*masks_x)[i] = (short)cur_x;
    (*masks_y)[i] = (short)cur_y;
    
    delete_tokens(cur_tokenset); 
  }
  
  
  /* read outliers section */
    
  AdvanceToSection(currentFile,"[OUTLIERS]",buffer);
  findStartsWith(currentFile,"NumberCells=",buffer);
  cur_tokenset = tokenize(buffer,"=");
  numcells = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);
  findStartsWith(currentFile,"CellHeader=",buffer); 

  *noutliers = numcells;
  *outliers_x = (short *)calloc(numcells,sizeof(short));
  *outliers_y = (short *)calloc(numcells,sizeof(short));


  for (i = 0; i < numcells; i++){
    ReadFileLine(buffer, BUF_SIZE, currentFile);      
    cur_tokenset = tokenize(buffer," \t");
    cur_x = atoi(get_token(cur_tokenset,0));
    cur_y = atoi(get_token(cur_tokenset,1));
    /* wxPrintf("%d: %d %d   %d\n",i, cur_x,cur_y, numcells); */
    (*outliers_x)[i] = (short)cur_x;
    (*outliers_y)[i] = (short)cur_y;
    
    delete_tokens(cur_tokenset); 
  }
  
  
  fclose(currentFile);




}

#endif

/*************************************************************************
 **
 ** char *get_header_info(const char *filename, int *dim1, int *dim2)
 **
 ** const char *filename - file to open
 ** int *dim1 - place to store Cols
 ** int *dim2 - place to store Rows
 **
 ** returns a character string containing the CDF name.
 **
 ** gets the header information (cols, rows and cdfname)
 **
 ************************************************************************/

char *get_header_info(const char *filename, int *dim1, int *dim2, int *err_code){
  
  int i,endpos;
  char *cdfName = NULL;
  FILE *currentFile; 
  char buffer[BUF_SIZE];
  tokenset *cur_tokenset;

  int errCode;

  currentFile = open_cel_file(filename);

  errCode = AdvanceToSection(currentFile,"[HEADER]",buffer);
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_FIND_HEADER_SECTION;
    return NULL;
  }
  
  errCode = findStartsWith(currentFile,"Cols",buffer);
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_FIND_COL_DIMENSION;
    return NULL;
  }
  cur_tokenset = tokenize(buffer,"=");
  *dim1 = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);

  errCode = findStartsWith(currentFile,"Rows",buffer);
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_FIND_ROW_DIMENSION; 
    return NULL;
  }

  cur_tokenset = tokenize(buffer,"=");
  *dim2 = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);
  
  errCode = findStartsWith(currentFile,"DatHeader",buffer);
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_FIND_DAT_HEADER;
    return NULL;
  }
  cur_tokenset = tokenize(buffer," ");
  for (i =0; i < tokenset_size(cur_tokenset);i++){
    /* look for a token ending in ".1sq" */
    endpos=token_ends_with(get_token(cur_tokenset,i),".1sq");
    if(endpos > 0){
      /* Found the likely CDF name, now chop of .1sq and store it */
      
      cdfName= (char *)calloc(endpos+1,sizeof(char));
      strncpy(cdfName,get_token(cur_tokenset,i),endpos);
      cdfName[endpos] = '\0';
      
      break;
    }
    if (i == (tokenset_size(cur_tokenset) - 1)){
      delete_tokens(cur_tokenset);
      fclose(currentFile);
      *err_code = TEXT_HEADER_CDF_NAME_MISSING;
      return NULL;
    }
  }
  delete_tokens(cur_tokenset);
  fclose(currentFile);
  return(cdfName);
}


/*************************************************************************
 **
 ** void get_detailed_header_info(const char *filename, detailed_header_info *header_info)
 **
 ** const char *filename - file to open
 ** detailed_header_info *header_info - place to store header information
 **
 ** reads the header information from a text cdf file (ignoring some fields
 ** that are unused).
 **
 ************************************************************************/

void get_detailed_header_info(const char *filename, detailed_header_info *header_info, int *err_code){

  int i,endpos;
  FILE *currentFile; 
  char buffer[BUF_SIZE];
  char *buffercopy;

  tokenset *cur_tokenset;

  int errCode;

  currentFile = open_cel_file(filename);

  errCode = AdvanceToSection(currentFile,"[HEADER]",buffer);
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_FIND_HEADER_SECTION;
    return;
  }
  

  errCode = findStartsWith(currentFile,"Cols",buffer);  
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_FIND_COL_DIMENSION;
    return;
  }
  cur_tokenset = tokenize(buffer,"=");
  header_info->cols = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);

  errCode = findStartsWith(currentFile,"Rows",buffer);
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_FIND_ROW_DIMENSION;
    return;
  }
  cur_tokenset = tokenize(buffer,"=");
  header_info->rows = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);

  errCode = findStartsWith(currentFile,"GridCornerUL",buffer);
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_MISC_HEADER_FIELD;
    return;
  }
  cur_tokenset = tokenize(buffer,"= ");
  header_info->GridCornerULx  = atoi(get_token(cur_tokenset,1));
  header_info->GridCornerULy  = atoi(get_token(cur_tokenset,2));
  delete_tokens(cur_tokenset);

  errCode = findStartsWith(currentFile,"GridCornerUR",buffer);
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_MISC_HEADER_FIELD;
    return;
  }
  cur_tokenset = tokenize(buffer,"= ");
  header_info->GridCornerURx  = atoi(get_token(cur_tokenset,1));
  header_info->GridCornerURy  = atoi(get_token(cur_tokenset,2));
  delete_tokens(cur_tokenset);
  
  errCode = findStartsWith(currentFile,"GridCornerLR",buffer);
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_MISC_HEADER_FIELD;
    return;
  }
  cur_tokenset = tokenize(buffer,"= ");
  header_info->GridCornerLRx  = atoi(get_token(cur_tokenset,1));
  header_info->GridCornerLRy  = atoi(get_token(cur_tokenset,2));
  delete_tokens(cur_tokenset);

  errCode = findStartsWith(currentFile,"GridCornerLL",buffer);
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_MISC_HEADER_FIELD;
    return;
  }
  cur_tokenset = tokenize(buffer,"= ");
  header_info->GridCornerLLx  = atoi(get_token(cur_tokenset,1));
  header_info->GridCornerLLy  = atoi(get_token(cur_tokenset,2));
  delete_tokens(cur_tokenset);

  errCode = findStartsWith(currentFile,"DatHeader",buffer); 
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_FIND_DAT_HEADER;
    return;
  }
  /* first lets copy the entire string over */

  buffercopy =  (char *)calloc(strlen(buffer)+1,sizeof(char));
  strcpy(buffercopy,buffer);
  cur_tokenset = tokenize(buffercopy,"\r\n");
  header_info->DatHeader = (char *)calloc(strlen(get_token(cur_tokenset,0))-8,sizeof(char));
  strcpy(header_info->DatHeader,(get_token(cur_tokenset,0)+10));  /* the +10 is to avoid the starting "DatHeader=" */
  free(buffercopy);
  delete_tokens(cur_tokenset);

  
  /* now pull out the actual cdfname */ 
  cur_tokenset = tokenize(buffer," ");
  for (i =0; i < tokenset_size(cur_tokenset);i++){
    /* look for a token ending in ".1sq" */
    endpos=token_ends_with(get_token(cur_tokenset,i),".1sq");
    if(endpos > 0){
      /* Found the likely CDF name, now chop of .1sq and store it */
      
      header_info->cdfName= (char *)calloc(endpos+1,sizeof(char));
      strncpy( header_info->cdfName,get_token(cur_tokenset,i),endpos);
      header_info->cdfName[endpos] = '\0';
      
      break;
    }
    if (i == (tokenset_size(cur_tokenset) - 1)){  
      delete_tokens(cur_tokenset);
      fclose(currentFile);
      *err_code = TEXT_HEADER_CDF_NAME_MISSING;
      return;
    }
  }
  delete_tokens(cur_tokenset);
  
  errCode = findStartsWith(currentFile,"Algorithm",buffer); 
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_MISC_HEADER_FIELD;
    return;
  }
  cur_tokenset = tokenize(buffer,"=\r\n");
  header_info->Algorithm = (char *)calloc(strlen(get_token(cur_tokenset,1))+1,sizeof(char));
  strcpy(header_info->Algorithm,get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);

  errCode = findStartsWith(currentFile,"AlgorithmParameters",buffer);
  if (errCode){
    fclose(currentFile);
    *err_code = TEXT_DID_NOT_MISC_HEADER_FIELD;
    return;
  }
  cur_tokenset = tokenize(buffer,"=\r\n");
  header_info->AlgorithmParameters = (char *)calloc(strlen(get_token(cur_tokenset,1))+1,sizeof(char));
  strcpy(header_info->AlgorithmParameters,get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);

  fclose(currentFile);

}










/***************************************************************
 **
 ** int isTextCelFile(const char *filename)
 **
 ** test whether the file is a valid text cel file
 ** 
 **
 **************************************************************/


int isTextCelFile(const char *filename){

  const char *mode = "r";

  FILE *currentFile= NULL; 
  char buffer[BUF_SIZE];

  currentFile = fopen(filename,mode);
  if (currentFile == NULL){
    error("Could not open file %s", filename);
  } else {
    /** check to see if first line is [CEL] so looks like a CEL file**/
    ReadFileLine(buffer, BUF_SIZE, currentFile);
    fclose(currentFile);
    if (strncmp("[CEL]", buffer, 4) == 0) {
      return 1;
    }
  }
  return 0;
}


/****************************************************************
 ****************************************************************
 **
 ** Code for GZ files starts here.
 **
 ***************************************************************
 ***************************************************************/



#if defined(HAVE_ZLIB)


/****************************************************************
 **
 ** void ReadgzFileLine(char *buffer, int buffersize, FILE *currentFile)
 **
 ** char *buffer  - place to store contents of the line
 ** int buffersize - size of the buffer
 ** FILE *currentFile - FILE pointer to an opened CEL file.
 **
 ** Read a line from a gzipped file, into a buffer of specified size.
 ** otherwise die.
 **
 ***************************************************************/

static void ReadgzFileLine(char *buffer, int buffersize, gzFile currentFile){
  if (gzgets( currentFile,buffer, buffersize) == NULL){
    error("End of gz file reached unexpectedly. Perhaps this file is truncated.\n");
  }  
}


/****************************************************************
 **
 ** FILE *open_gz_cel_file(const char *filename)
 **
 ** const char *filename - name of file to open
 **
 **
 ** RETURNS a file pointer to the open file
 **
 ** this file will open the named file and check to see that the 
 ** first characters agree with "[CEL]" 
 **
 ***************************************************************/

static gzFile open_gz_cel_file(const char *filename){
  
  const char *mode = "rb";

  gzFile currentFile= NULL; 
  char buffer[BUF_SIZE];

  currentFile = gzopen(filename,mode);
  if (currentFile == NULL){
     error("Could not open file %s", filename);
  } else {
    /** check to see if first line is [CEL] so looks like a CEL file**/
    ReadgzFileLine(buffer, BUF_SIZE, currentFile);
    if (strncmp("[CEL]", buffer, 4) == 0) {
      gzrewind(currentFile);
    } else {
      error("The file %s does not look like a CEL file",filename);
    }
  }
  
  return currentFile;

}



/******************************************************************
 **
 ** void gzfindStartsWith(gzFile *my_file,char *starts, char *buffer)
 **
 ** FILE *my_file - an open file to read from
 ** char *starts - the string to search for at the start of each line
 ** char *buffer - where to place the line that has been read.
 **
 **
 ** Find a line that starts with the specified character string.
 ** At exit buffer should contain that line
 **
 *****************************************************************/


static void  gzfindStartsWith(gzFile my_file,char *starts, char *buffer){

  int starts_len = strlen(starts);
  int match = 1;

  do {
    ReadgzFileLine(buffer, BUF_SIZE, my_file);
    match = strncmp(starts, buffer, starts_len);
  } while (match != 0);
}


/******************************************************************
 **
 ** void gzAdvanceToSection(gzFile my_file,char *sectiontitle, char *buffer)
 **
 ** FILE *my_file - an open file
 ** char *sectiontitle - string we are searching for
 ** char *buffer - return's with line starting with sectiontitle
 **
 **
 *****************************************************************/

static void gzAdvanceToSection(gzFile my_file,char *sectiontitle, char *buffer){
  gzfindStartsWith(my_file,sectiontitle,buffer);
}



/******************************************************************
 ** 
 ** int check_gzcel_file(const char *filename, char *ref_cdfName, int ref_dim_1, int ref_dim_2)
 **
 ** const char *filename - the file to read
 ** char *ref_cdfName - the reference CDF filename
 ** int ref_dim_1 - 1st dimension of reference cel file
 ** int ref_dim_2 - 2nd dimension of reference cel file
 **
 ** returns 0 if no problem, 1 otherwise
 **
 ** The aim of this function is to read the header of the CEL file
 ** in particular we will look for the rows beginning "Cols="  and "Rows="
 ** and then for the line DatHeader=  to scope out the appropriate cdf
 ** file. An error() will be flagged if the appropriate conditions
 ** are not met.
 **
 **
 ******************************************************************/

int check_gzcel_file(const char *filename, const char *ref_cdfName, int ref_dim_1, int ref_dim_2){

  int i;
  int dim1,dim2;

  gzFile currentFile; 
  char buffer[BUF_SIZE];
  tokenset *cur_tokenset;

  currentFile = open_gz_cel_file(filename);
  

  gzAdvanceToSection(currentFile,"[HEADER]",buffer);
  gzfindStartsWith(currentFile,"Cols",buffer);  
  cur_tokenset = tokenize(buffer,"=");
  dim1 = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);

  gzfindStartsWith(currentFile,"Rows",buffer);
  cur_tokenset = tokenize(buffer,"=");
  dim2 = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);
  if ((dim1 != ref_dim_1) || (dim2 != ref_dim_2)){
    error("Cel file %s does not seem to have the correct dimensions",filename);
  }
  
  
  gzfindStartsWith(currentFile,"DatHeader",buffer);
  cur_tokenset = tokenize(buffer," ");
  for (i =0; i < tokenset_size(cur_tokenset);i++){
    if (strncasecmp(get_token(cur_tokenset,i),ref_cdfName,strlen(ref_cdfName)) == 0){
      break;
    }
    if (i == (tokenset_size(cur_tokenset) - 1)){
      error("Cel file %s does not seem to be of %s type",filename,ref_cdfName);
    }
  }
  delete_tokens(cur_tokenset);
  gzclose(currentFile);

  return 0;
}



/************************************************************************
 **
 ** int read_gzcel_file_intensities(const char *filename, double *intensity, int chip_num, int rows, int cols)
 **
 ** const char *filename - the name of the cel file to read
 ** double *intensity  - the intensity matrix to fill
 ** int chip_num - the column of the intensity matrix that we will be filling
 ** int rows - dimension of intensity matrix
 ** int cols - dimension of intensity matrix
 **
 ** returns 0 if successful, non zero if unsuccessful
 **
 ** This function reads from the specified file the cel intensities for that
 ** array and fills a column of the intensity matrix.
 **
 ************************************************************************/

int read_gzcel_file_intensities(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows){
  
  int i, cur_x,cur_y,cur_index;
  double cur_mean;
  gzFile currentFile; 
  char buffer[BUF_SIZE];
  /* tokenset *cur_tokenset;*/
  char *current_token;

  currentFile = open_gz_cel_file(filename);
  
  gzAdvanceToSection(currentFile,"[INTENSITY]",buffer);
  gzfindStartsWith(currentFile,"CellHeader=",buffer);  
  
  for (i=0; i < rows; i++){
    ReadgzFileLine(buffer, BUF_SIZE,  currentFile);
    /* cur_tokenset = tokenize(buffer," \t");
    cur_x = atoi(get_token(cur_tokenset,0));
    cur_y = atoi(get_token(cur_tokenset,1));
    cur_mean = atof(get_token(cur_tokenset,2)); */
    
    current_token = strtok(buffer," \t"); 
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }

    cur_x = atoi(current_token);
    current_token = strtok(NULL," \t"); 
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }

    cur_y = atoi(current_token);
    current_token = strtok(NULL," \t"); 
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    
    if (cur_x < 0 || cur_x >= chip_dim_rows){
      error("It appears that the file %s is corrupted.",filename);
      return 1;
    }
    if (cur_y < 0 || cur_y >= chip_dim_rows){
      error("It appears that the file %s is corrupted.",filename);
      return 1;
    }


    cur_mean = atof(current_token);

    cur_index = cur_x + chip_dim_rows*(cur_y);
    intensity[chip_num*rows + cur_index] = cur_mean;
    /* delete_tokens(cur_tokenset); */
  }

  gzclose(currentFile);
  
  if (i != rows){
    return 1;
  }

  return 0;
}

/************************************************************************
 **
 ** int read_gzcel_file_stddev(const char *filename, double *intensity, int chip_num, int rows, int cols)
 **
 ** const char *filename - the name of the cel file to read
 ** double *intensity  - the intensity matrix to fill
 ** int chip_num - the column of the intensity matrix that we will be filling
 ** int rows - dimension of intensity matrix
 ** int cols - dimension of intensity matrix
 **
 ** returns 0 if successful, non zero if unsuccessful
 **
 ** This function reads from the specified file the cel intensities for that
 ** array and fills a column of the intensity matrix.
 **
 ************************************************************************/

int read_gzcel_file_stddev(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows){
  
  int i, cur_x,cur_y,cur_index;
  double cur_mean, cur_stddev;
  gzFile currentFile; 
  char buffer[BUF_SIZE];
  /* tokenset *cur_tokenset;*/
  char *current_token;

  currentFile = open_gz_cel_file(filename);
  
  gzAdvanceToSection(currentFile,"[INTENSITY]",buffer);
  gzfindStartsWith(currentFile,"CellHeader=",buffer);  
  
  for (i=0; i < rows; i++){
    ReadgzFileLine(buffer, BUF_SIZE,  currentFile);
    /* cur_tokenset = tokenize(buffer," \t");
    cur_x = atoi(get_token(cur_tokenset,0));
    cur_y = atoi(get_token(cur_tokenset,1));
    cur_mean = atof(get_token(cur_tokenset,2)); */
    
    current_token = strtok(buffer," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }

    cur_x = atoi(current_token);
    current_token = strtok(NULL," \t"); 
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }

    cur_y = atoi(current_token);
    current_token = strtok(NULL," \t"); 
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }

    cur_mean = atof(current_token);
  
    current_token = strtok(NULL," \t"); 
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }

    cur_stddev = atof(current_token);
    
    cur_index = cur_x + chip_dim_rows*(cur_y);
    intensity[chip_num*rows + cur_index] = cur_stddev;
    /* delete_tokens(cur_tokenset); */
  }

  gzclose(currentFile);
  
  if (i != rows){
    return 1;
  }


  return 0;
}


/************************************************************************
 **
 ** int read_gzcel_file_npixels(const char *filename, double *intensity, int chip_num, int rows, int cols)
 **
 ** const char *filename - the name of the cel file to read
 ** double *intensity  - the intensity matrix to fill
 ** int chip_num - the column of the intensity matrix that we will be filling
 ** int rows - dimension of intensity matrix
 ** int cols - dimension of intensity matrix
 **
 ** returns 0 if successful, non zero if unsuccessful
 **
 ** This function reads from the specified file the cel npixels for that
 ** array and fills a column of the intensity matrix.
 **
 ************************************************************************/

int read_gzcel_file_npixels(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows){
  
  int i, cur_x,cur_y,cur_index,cur_npixels;
  double cur_mean, cur_stddev;
  gzFile currentFile; 
  char buffer[BUF_SIZE];
  /* tokenset *cur_tokenset;*/
  char *current_token;

  currentFile = open_gz_cel_file(filename);
  
  gzAdvanceToSection(currentFile,"[INTENSITY]",buffer);
  gzfindStartsWith(currentFile,"CellHeader=",buffer);  
  
  for (i=0; i < rows; i++){
    ReadgzFileLine(buffer, BUF_SIZE,  currentFile);
    /* cur_tokenset = tokenize(buffer," \t");
    cur_x = atoi(get_token(cur_tokenset,0));
    cur_y = atoi(get_token(cur_tokenset,1));
    cur_mean = atof(get_token(cur_tokenset,2)); */
    
    current_token = strtok(buffer," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    cur_x = atoi(current_token);
    current_token = strtok(NULL," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    cur_y = atoi(current_token);
    current_token = strtok(NULL," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    cur_mean = atof(current_token);
  
    current_token = strtok(NULL," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    cur_stddev = atof(current_token);
    
    current_token = strtok(NULL," \t");
    if (current_token == NULL){
      wxPrintf(wxT("Warning: found an incomplete line where not expected in %s.\nThe CEL file may be truncated. \nSucessfully read to cel intensity %d of %d expected\n"), filename, i-1, rows);
      break;
    }
    cur_npixels = atoi(current_token);

    cur_index = cur_x + chip_dim_rows*(cur_y);
    intensity[chip_num*rows + cur_index] = (double)cur_npixels;
    /* delete_tokens(cur_tokenset); */
  }

  gzclose(currentFile);
  
  if (i != rows){
    return 1;
  }

  return 0;
}



/****************************************************************
 **
 ** void gz_apply_masks(const char *filename, double *intensity, int chip_num, 
 **                   int rows, int cols,int chip_dim_rows, 
 **                   int rm_mask, int rm_outliers)
 **
 ** const char *filename    - name of file to open
 ** double *intensity - matrix of probe intensities
 ** int chip_num - the index 0 ...n-1 of the chip we are dealing with
 ** int rows - dimension of the intensity matrix
 ** int cols - dimension of the intensity matrix
 ** int chip_dim_rows - a dimension of the chip
 ** int rm_mask - if true locations in the MASKS section are set NA
 ** int rm_outliers - if true locations in the OUTLIERS section are set NA
 **
 ** This function sets the MASK and OUTLIER probes to NA
 ** 
 **
 ****************************************************************/

void gz_apply_masks(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows, int rm_mask, int rm_outliers){
  
  int i;
  int numcells, cur_x, cur_y, cur_index;
  gzFile currentFile;
  char buffer[BUF_SIZE];
  tokenset *cur_tokenset;

  if ((!rm_mask) && (!rm_outliers)){
    /* no masking or outliers */
    return;
  }
  
  currentFile = open_gz_cel_file(filename);
  /* read masks section */
  if (rm_mask){

    gzAdvanceToSection(currentFile,"[MASKS]",buffer);
    gzfindStartsWith(currentFile,"NumberCells=",buffer); 
    cur_tokenset = tokenize(buffer,"=");
    numcells = atoi(get_token(cur_tokenset,1));
    delete_tokens(cur_tokenset);
    gzfindStartsWith(currentFile,"CellHeader=",buffer); 


     for (i =0; i < numcells; i++){
       ReadgzFileLine(buffer, BUF_SIZE, currentFile);
       
       
       cur_tokenset = tokenize(buffer," \t");
       cur_x = atoi(get_token(cur_tokenset,0));
       cur_y = atoi(get_token(cur_tokenset,1));
       
       cur_index = cur_x + chip_dim_rows*(cur_y);
       intensity[chip_num*rows + cur_index] = NAN;
       delete_tokens(cur_tokenset); 
     }
  }

  /* read outliers section */

  if (rm_outliers){
    
    gzAdvanceToSection(currentFile,"[OUTLIERS]",buffer);
    gzfindStartsWith(currentFile,"NumberCells=",buffer);
    cur_tokenset = tokenize(buffer,"=");
    numcells = atoi(get_token(cur_tokenset,1));
    delete_tokens(cur_tokenset);
    gzfindStartsWith(currentFile,"CellHeader=",buffer); 
    for (i = 0; i < numcells; i++){
      ReadgzFileLine(buffer, BUF_SIZE, currentFile);      
      cur_tokenset = tokenize(buffer," \t");
      cur_x = atoi(get_token(cur_tokenset,0));
      cur_y = atoi(get_token(cur_tokenset,1));
      
      cur_index = cur_x + chip_dim_rows*(cur_y);
      intensity[chip_num*rows + cur_index] = NAN;
      delete_tokens(cur_tokenset); 
    }
  }
  
  gzclose(currentFile);

}


/*************************************************************************
 **
 ** char *gz_get_header_info(const char *filename, int *dim1, int *dim2)
 **
 ** const char *filename - file to open
 ** int *dim1 - place to store Cols
 ** int *dim2 - place to store Rows
 **
 ** returns a character string containing the CDF name.
 **
 ** gets the header information (cols, rows and cdfname)
 **
 ************************************************************************/

char *gz_get_header_info(const char *filename, int *dim1, int *dim2){
  
  int i,endpos;
  char *cdfName = NULL;
  gzFile currentFile; 
  char buffer[BUF_SIZE];
  tokenset *cur_tokenset;

  currentFile = open_gz_cel_file(filename);

  gzAdvanceToSection(currentFile,"[HEADER]",buffer);
  gzfindStartsWith(currentFile,"Cols",buffer);  
  cur_tokenset = tokenize(buffer,"=");
  *dim1 = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);

  gzfindStartsWith(currentFile,"Rows",buffer);
  cur_tokenset = tokenize(buffer,"=");
  *dim2 = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);
  
  gzfindStartsWith(currentFile,"DatHeader",buffer);
  cur_tokenset = tokenize(buffer," ");
  for (i =0; i < tokenset_size(cur_tokenset);i++){
    /* look for a token ending in ".1sq" */
    endpos=token_ends_with(get_token(cur_tokenset,i),".1sq");
    if(endpos > 0){
      /* Found the likely CDF name, now chop of .1sq and store it */
      
      cdfName= (char *)calloc(endpos+1,sizeof(char));
      strncpy(cdfName,get_token(cur_tokenset,i),endpos);
      cdfName[endpos] = '\0';

      break;
    }
    if (i == (tokenset_size(cur_tokenset) - 1)){
      error("Cel file %s does not seem to be have cdf information",filename);
    }
  }
  delete_tokens(cur_tokenset);
  gzclose(currentFile);
  return(cdfName);
}




/*************************************************************************
 **
 ** char *gz_get_detailed_header_info(const char *filename, detailed_header_info *header_info)
 **
 ** const char *filename - file to open
 ** detailed_header_info *header_info - place to store header information
 **
 ** reads the header information from a gzipped text cdf file (ignoring some fields
 ** that are unused).
 **
 ************************************************************************/

void gz_get_detailed_header_info(const char *filename, detailed_header_info *header_info){

  int i,endpos;
  gzFile currentFile; 
  char buffer[BUF_SIZE];
  char *buffercopy;

  tokenset *cur_tokenset;

  currentFile = open_gz_cel_file(filename);

  gzAdvanceToSection(currentFile,"[HEADER]",buffer);

  gzfindStartsWith(currentFile,"Cols",buffer);  
  cur_tokenset = tokenize(buffer,"=");
  header_info->cols = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);

  gzfindStartsWith(currentFile,"Rows",buffer);
  cur_tokenset = tokenize(buffer,"=");
  header_info->rows = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);

  gzfindStartsWith(currentFile,"GridCornerUL",buffer);
  cur_tokenset = tokenize(buffer,"= ");
  header_info->GridCornerULx  = atoi(get_token(cur_tokenset,1));
  header_info->GridCornerULy  = atoi(get_token(cur_tokenset,2));
  delete_tokens(cur_tokenset);

  gzfindStartsWith(currentFile,"GridCornerUR",buffer);
  cur_tokenset = tokenize(buffer,"= ");
  header_info->GridCornerURx  = atoi(get_token(cur_tokenset,1));
  header_info->GridCornerURy  = atoi(get_token(cur_tokenset,2));
  delete_tokens(cur_tokenset);
  
  gzfindStartsWith(currentFile,"GridCornerLR",buffer);
  cur_tokenset = tokenize(buffer,"= ");
  header_info->GridCornerLRx  = atoi(get_token(cur_tokenset,1));
  header_info->GridCornerLRy  = atoi(get_token(cur_tokenset,2));
  delete_tokens(cur_tokenset);

  gzfindStartsWith(currentFile,"GridCornerLL",buffer);
  cur_tokenset = tokenize(buffer,"= ");
  header_info->GridCornerLLx  = atoi(get_token(cur_tokenset,1));
  header_info->GridCornerLLy  = atoi(get_token(cur_tokenset,2));
  delete_tokens(cur_tokenset);

  gzfindStartsWith(currentFile,"DatHeader",buffer);
  /* first lets copy the entire string over */

  buffercopy =  (char *)calloc(strlen(buffer)+1,sizeof(char));
  strcpy(buffercopy,buffer);
  cur_tokenset = tokenize(buffercopy,"\r\n");
  header_info->DatHeader = (char *)calloc(strlen(get_token(cur_tokenset,0))-8,sizeof(char));
  strcpy(header_info->DatHeader,(get_token(cur_tokenset,0)+10));  /* the +10 is to avoid the starting "DatHeader=" */
  free(buffercopy);
  delete_tokens(cur_tokenset);

  
  /* now pull out the actual cdfname */ 
  cur_tokenset = tokenize(buffer," ");
  for (i =0; i < tokenset_size(cur_tokenset);i++){
    /* look for a token ending in ".1sq" */
    endpos=token_ends_with(get_token(cur_tokenset,i),".1sq");
    if(endpos > 0){
      /* Found the likely CDF name, now chop of .1sq and store it */
      
      header_info->cdfName= (char *)calloc(endpos+1,sizeof(char));
      strncpy( header_info->cdfName,get_token(cur_tokenset,i),endpos);
       header_info->cdfName[endpos] = '\0';
      
      break;
    }
    if (i == (tokenset_size(cur_tokenset) - 1)){
      error("Cel file %s does not seem to be have cdf information",filename);
    }
  }
  delete_tokens(cur_tokenset);
  
  gzfindStartsWith(currentFile,"Algorithm",buffer);
  cur_tokenset = tokenize(buffer,"=\r\n");
  header_info->Algorithm = (char *)calloc(strlen(get_token(cur_tokenset,1))+1,sizeof(char));
  strcpy(header_info->Algorithm,get_token(cur_tokenset,1));
  

  delete_tokens(cur_tokenset);

  gzfindStartsWith(currentFile,"AlgorithmParameters",buffer);
  cur_tokenset = tokenize(buffer,"=\r\n");
  header_info->AlgorithmParameters = (char *)calloc(strlen(get_token(cur_tokenset,1))+1,sizeof(char));
  strcpy(header_info->AlgorithmParameters,get_token(cur_tokenset,1));
  
  gzclose(currentFile);

}




/****************************************************************
 **
 ** static void gz_get_masks_outliers(const char *filename, 
 **                         int *nmasks, short **masks_x, short **masks_y, 
 **                         int *noutliers, short **outliers_x, short **outliers_y
 ** 
 ** This gets the x and y coordinates stored in the masks and outliers sections
 ** of the cel files. (for gzipped text CEL files)
 **
 ****************************************************************/

void gz_get_masks_outliers(const char *filename, int *nmasks, short **masks_x, short **masks_y, int *noutliers, short **outliers_x, short **outliers_y){
  
  gzFile currentFile;
  char buffer[BUF_SIZE];
  int numcells, cur_x, cur_y;
  tokenset *cur_tokenset;
  int i;


  currentFile = open_gz_cel_file(filename);
  /* read masks section */
  

  gzAdvanceToSection(currentFile,"[MASKS]",buffer);
  gzfindStartsWith(currentFile,"NumberCells=",buffer); 
  cur_tokenset = tokenize(buffer,"=");
  numcells = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);
  gzfindStartsWith(currentFile,"CellHeader=",buffer); 
  
  *nmasks = numcells;

  *masks_x = (short *)calloc(numcells,sizeof(short));
  *masks_y = (short *)calloc(numcells,sizeof(short));


  for (i =0; i < numcells; i++){
    ReadgzFileLine(buffer, BUF_SIZE, currentFile);
    
    
    cur_tokenset = tokenize(buffer," \t");
    cur_x = atoi(get_token(cur_tokenset,0));
    cur_y = atoi(get_token(cur_tokenset,1));
    (*masks_x)[i] = (short)cur_x;
    (*masks_y)[i] = (short)cur_y;
    
    delete_tokens(cur_tokenset); 
  }
  
  
  /* read outliers section */
    
  gzAdvanceToSection(currentFile,"[OUTLIERS]",buffer);
  gzfindStartsWith(currentFile,"NumberCells=",buffer);
  cur_tokenset = tokenize(buffer,"=");
  numcells = atoi(get_token(cur_tokenset,1));
  delete_tokens(cur_tokenset);
  gzfindStartsWith(currentFile,"CellHeader=",buffer); 

  *noutliers = numcells;
  *outliers_x = (short *)calloc(numcells,sizeof(short));
  *outliers_y = (short *)calloc(numcells,sizeof(short));


  for (i = 0; i < numcells; i++){
    ReadgzFileLine(buffer, BUF_SIZE, currentFile);      
    cur_tokenset = tokenize(buffer," \t");
    cur_x = atoi(get_token(cur_tokenset,0));
    cur_y = atoi(get_token(cur_tokenset,1));
    /* wxPrintf("%d: %d %d   %d\n",i, cur_x,cur_y, numcells); */
    (*outliers_x)[i] = (short)cur_x;
    (*outliers_y)[i] = (short)cur_y;
    
    delete_tokens(cur_tokenset); 
  }
  
  
  gzclose(currentFile);




}








#endif



/***************************************************************
 **
 ** int isgzTextCelFile(const char *filename)
 **
 ** test whether the file is a valid gzipped text cel file
 ** 
 **
 **************************************************************/

int isgzTextCelFile(const char *filename){
  
#if defined HAVE_ZLIB
  const char *mode = "rb"; 
 gzFile currentFile = NULL; 
 char buffer[BUF_SIZE];
 currentFile = gzopen(filename,mode);
 if (currentFile == NULL){
   error("Could not open file %s", filename);
 } else {
   /** check to see if first line is [CEL] so looks like a CEL file**/
   ReadgzFileLine(buffer, BUF_SIZE, currentFile);
   gzclose(currentFile);    /* fixed by WH 28 Dec 2003 */
   if (strncmp("[CEL]", buffer, 4) == 0) {
     return 1;
   }
 }
#endif 
 return 0;
}

