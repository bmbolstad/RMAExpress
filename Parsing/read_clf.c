/******************************************************************
 **
 ** file: read_clf.c
 ** 
 ** Aim: implement parsing of CLF format files
 **
 ** Copyright (C) 2007    B. M. Bolstad
 **
 ** Created on Nov 4, 2007
 **
 ** History
 ** Dec 14, 2007 - Initial version
 ** Jun 24, 2008 - change char * to const char * where appropriate
 ** Oct 22, 2013 - Force "order" to always be interpreted as row_major
 **
 **
 ** 
 ******************************************************************/

//#include <R.h>

#include <cstdio>
#include <cstdlib>
#include <cstring> 

#define BUFFERSIZE 1024

#include <wx/string.h>
#include <wx/arrstr.h>
#include "read_clf.h"

static void error(const char *msg, const char *msg2="", const char *msg3=""){
  wxString Error = wxString((const char*)msg,wxConvUTF8) +_T(" ") + wxString((const char*)msg2,wxConvUTF8)  +_T(" ") + wxString((const char*)msg3,wxConvUTF8)+ _T("\n");
  throw Error;
}


/*******************************************************************
 *******************************************************************
 **
 ** Structures for dealing with clf file information
 **
 **
 **
 *******************************************************************
 ******************************************************************/

/*******************************************************************
 *******************************************************************
 **
 ** Starting off with the headers
 **
 *******************************************************************
 ******************************************************************/

/* integer (from 0 to n-1) indicates position of header (-1 means header is not present) */

typedef struct{
  int probe_id;
  int x;
  int y;
} header_0;

/*******************************************************************
 **
 ** These are all the headers that appear in CLF files
 **
 ** Note that some are required (chip_type, lib_set_name, lib_set_version, clf_format_version
 **                              rows, cols, header0)
 ** While others are optional  (sequential, order, create_date, guid and others)
 **
 **
 *******************************************************************/

typedef struct{
  char **chip_type;
  int n_chip_type;
  char *lib_set_name;
  char *lib_set_version;
  char *clf_format_version;
  int rows;
  int cols;
  char *header0_str;
  header_0 *header0;
  int sequential;
  char *order;
  char *create_date;
  char *guid;
  char **other_headers_keys;
  char **other_headers_values;
  int n_other_headers;
} clf_headers;

/*******************************************************************
 *******************************************************************
 **
 ** Now the actual data
 **
 ** (only store the probeset ids to save space)
 **
 ** length of probe_id is rows*cols.
 **
 ** Given an x, y it maps to probe_id[index]
 **
 ** index = y*rows + x
 **
 ** Which means that given an index, it maps to
 **
 ** x = index % rows where % means modulo  (ie remainder)
 ** y = index / rows
 **
 ** 
 **
 *******************************************************************
 ******************************************************************/


typedef struct{
  int *probe_id;
} clf_data;


/*******************************************************************
 *******************************************************************
 **
 ** Structure for storing clf file (after it is read from file)
 **
 *******************************************************************
 ******************************************************************/


struct clf_file{
  clf_headers *headers;
  clf_data *data;
};



/*******************************************************************
 *******************************************************************
 **
 **
 ** Code for splitting a string into a series of tokens
 **
 **
 *******************************************************************
 *******************************************************************/


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

#if USE_PTHREADS  
  char *tmp_pointer;
#endif  
  int i=0;

  char *current_token;
  tokenset *my_tokenset = (tokenset *)calloc(1,sizeof(tokenset));
  my_tokenset->n=0;
  
  my_tokenset->tokens = NULL;
#if USE_PTHREADS
  current_token = strtok_r(str,delimiters,&tmp_pointer);
#else
  current_token = strtok(str,delimiters);
#endif
  while (current_token != NULL){
    my_tokenset->n++;
    my_tokenset->tokens = (char **)realloc(my_tokenset->tokens,(my_tokenset->n)*sizeof(char*));
    my_tokenset->tokens[i] = (char *)calloc(strlen(current_token)+1,sizeof(char));
    strcpy(my_tokenset->tokens[i],current_token);
    my_tokenset->tokens[i][(strlen(current_token))] = '\0';
    i++;
#if USE_PTHREADS
    current_token = strtok_r(NULL,delimiters,&tmp_pointer);
#else
    current_token = strtok(NULL,delimiters);
#endif
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


/*******************************************************************
 *******************************************************************
 **
 ** Code for Reading from file
 **
 *******************************************************************
 *******************************************************************/



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
    return 0;
    //error("End of file reached unexpectedly. Perhaps this file is truncated.\n");
  }  
  return 1;
}  


/****************************************************************
 ****************************************************************
 **
 ** Code for identifying what type of information is stored in 
 ** the current line
 **
 ****************************************************************
 ***************************************************************/

/****************************************************************
 **
 ** static int IsHeaderLine(char *buffer)
 **
 ** char *buffer - contains line to evaluate
 **
 ** Checks whether supplied line is a header line (ie starts with #%)
 **
 ** return 1 (ie true) if header line. 0 otherwise
 **
 ***************************************************************/


static int IsHeaderLine(char *buffer){

  if (strncmp("#%",buffer,2) == 0){
    return 1;
  }
  return 0;
}

/****************************************************************
 **
 ** static int IsHeaderLine(char *buffer)
 **
 ** char *buffer - contains line to evaluate
 **
 ** Checks whether supplied line is a comment line (ie starts with #)
 **
 **
 ***************************************************************/

static int IsCommentLine(char *buffer){
  if (strncmp("#",buffer,1) == 0){
    return 1;
  }
  return 0;
}

/****************************************************************
 **
 ** void initialize_clf_header(clf_headers *header)
 **
 ** Initialize all the header values
 **
 **
 **
 ***************************************************************/

void initialize_clf_header(clf_headers *header){

  header->chip_type = NULL;
  header->n_chip_type = 0;
  
  header->lib_set_name= NULL;
  header->lib_set_version= NULL;
  header->clf_format_version= NULL;
  header->header0_str= NULL;
  header->header0= NULL;
  header->order = NULL;
  header->create_date= NULL;
  header->guid= NULL;
  header->other_headers_keys= NULL;
  header->other_headers_values= NULL;
  header->n_other_headers=0;

  header->rows = -1;
  header->cols = -1;
  header->n_other_headers = 0;

}


/****************************************************************
 ****************************************************************
 **
 ** Code for reading in clf header
 **
 ****************************************************************
 ***************************************************************/

static void determine_order_header0(char *header_str, header_0 *header0){

  tokenset *cur_tokenset;
  int i;
  char *temp_str = (char *)calloc(strlen(header_str) +1, sizeof(char));


  strcpy(temp_str,header_str);

  header0->probe_id = -1;
  header0->x = -1;
  header0->y = -1;
  
  cur_tokenset = tokenize(temp_str,"\t\r\n");
  
  for (i=0; i < tokenset_size(cur_tokenset); i++){
    if (strcmp(get_token(cur_tokenset,i),"probe_id")==0){
      header0->probe_id = i;
    } else if (strcmp(get_token(cur_tokenset,i),"x")==0){
      header0->x = i;
    } else if (strcmp(get_token(cur_tokenset,i),"y")==0){
      header0->y = i;
    }
  }
  delete_tokens(cur_tokenset);

  free(temp_str);

}

/****************************************************************
 **
 ** Validate that required headers are present in file. 
 **
 ** Return 0 if an expected header is not present.
 ** Returns 1 otherwise (ie everything looks fine)
 **
 ***************************************************************/

static int validate_clf_header(clf_headers *header){


  /* check that required headers are all there (have been read) */
  if (header->chip_type == NULL)
    return 0;

  if (header->lib_set_name == NULL)
    return 0;

  if (header->lib_set_version == NULL)
    return 0;

  if (header->clf_format_version == NULL)
    return 0;

  if (header->header0_str == NULL)
    return 0;
      
  if (header->rows == -1)
    return 0;
  
  if (header->cols == -1)
    return 0;

  /* Check that format version is 1.0 (only supported version) */

  if (strcmp( header->clf_format_version,"1.0") != 0){
    return 0;
  }

  /* check that header0, header1, header2 (ie the three levels of headers) have required fields */

  if (header->header0->probe_id == -1)
    return 0;

  if (header->header0->x == -1)
    return 0;

  if (header->header0->y == -1)
    return 0;


  return 1;
}

/****************************************************************
 **
 ** static FILE *open_clf_file(const char *filename)
 **
 ** Open the CLF to begin reading from it.
 **
 ***************************************************************/

static FILE *open_clf_file(const char *filename){
  
  const char *mode = "r";
  FILE *currentFile = NULL; 
  
  currentFile = fopen(filename,mode);
  if (currentFile == NULL){
     error("Could not open file %s", filename);
  }   
  return currentFile;

}

/****************************************************************
 **
 ** void read_clf_header(FILE *cur_file, char *buffer, clf_headers *header)
 **
 ** read the CLF header section
 **
 **
 ***************************************************************/

void read_clf_header(FILE *cur_file, char *buffer, clf_headers *header){


  tokenset *cur_tokenset;
  int i;
  char *temp_str;
  
  
  initialize_clf_header(header);
  do {
    ReadFileLine(buffer, 1024, cur_file);
    /* Rprintf("%s\n",buffer); */
    if (IsHeaderLine(buffer)){
      cur_tokenset = tokenize(&buffer[2],"=\r\n");
      /* hopefully token 0 is Key 
	 and token 1 is Value */
      /*   Rprintf("Key is: %s\n",get_token(cur_tokenset,0));
	   Rprintf("Value is: %s\n",get_token(cur_tokenset,1)); */
      /* Decode the Key/Value pair */
      if (strcmp(get_token(cur_tokenset,0),"chip_type") == 0){
	if (header->n_chip_type == 0){
	  header->chip_type = (char **)calloc(1, sizeof(char *));
	} else {
	  header->chip_type = (char **)realloc(header->chip_type, (header->n_chip_type+1)*sizeof(char *));
	}
	temp_str = (char *)calloc(strlen(get_token(cur_tokenset,1))+1,sizeof(char));
	strcpy(temp_str,get_token(cur_tokenset,1));
	header->chip_type[header->n_chip_type] = temp_str;
	header->n_chip_type++;
      } else if (strcmp(get_token(cur_tokenset,0), "lib_set_name") == 0){
	temp_str = (char *)calloc(strlen(get_token(cur_tokenset,1)) + 1,sizeof(char));
	strcpy(temp_str,get_token(cur_tokenset,1));
	header->lib_set_name = temp_str;
      } else if (strcmp(get_token(cur_tokenset,0), "lib_set_version") == 0){
	temp_str = (char *)calloc(strlen(get_token(cur_tokenset,1)) + 1,sizeof(char));
	strcpy(temp_str,get_token(cur_tokenset,1));
	header->lib_set_version = temp_str;
      } else if (strcmp(get_token(cur_tokenset,0), "clf_format_version") == 0) {
	temp_str = (char *)calloc(strlen(get_token(cur_tokenset,1)) + 1,sizeof(char));
	strcpy(temp_str,get_token(cur_tokenset,1));
	header->clf_format_version = temp_str;
      } else if (strcmp(get_token(cur_tokenset,0), "rows") == 0) {
	header->rows = atoi(get_token(cur_tokenset,1));
      } else if (strcmp(get_token(cur_tokenset,0), "cols") == 0) {
	header->cols = atoi(get_token(cur_tokenset,1));
      } else if (strcmp(get_token(cur_tokenset,0), "header0") == 0) {
	temp_str = (char *)calloc(strlen(get_token(cur_tokenset,1)) + 1,sizeof(char));
	strcpy(temp_str,get_token(cur_tokenset,1));
	header->header0_str = temp_str;
	header->header0 = (header_0 *)calloc(1,sizeof(header_0));
	determine_order_header0(header->header0_str,header->header0);
      } else if (strcmp(get_token(cur_tokenset,0), "create_date") == 0) {
	temp_str = (char *)calloc(strlen(get_token(cur_tokenset,1)) + 1,sizeof(char));
	strcpy(temp_str,get_token(cur_tokenset,1));
	header->create_date = temp_str;
      } else if (strcmp(get_token(cur_tokenset,0), "order") == 0) {
	/* temp_str = (char *)calloc(strlen(get_token(cur_tokenset,1)) + 1,sizeof(char));
	   strcpy(temp_str,get_token(cur_tokenset,1)); */

	header->order = (char *)"row_major"; //temp_str;
      } else if (strcmp(get_token(cur_tokenset,0), "sequential") == 0) {
	header->sequential = atoi(get_token(cur_tokenset,1));
      } else if (strcmp(get_token(cur_tokenset,0), "guid") == 0) {
	temp_str = (char *)calloc(strlen(get_token(cur_tokenset,1)) + 1,sizeof(char));
	strcpy(temp_str,get_token(cur_tokenset,1));
	header->guid = temp_str;
      } else {
	/* not one of the recognised header types */
	if ( header->n_other_headers == 0){
	  header->other_headers_keys = (char **)calloc(1, sizeof(char *));
	  header->other_headers_values = (char **)calloc(1, sizeof(char *));
	} else {
	  header->other_headers_keys = (char **)realloc(header->other_headers_keys,(header->n_other_headers+1)*sizeof(char *));
	  header->other_headers_values = (char **)realloc(header->other_headers_values,(header->n_other_headers+1)*sizeof(char *));
	  header->chip_type = (char **)realloc(header->chip_type, (header->n_chip_type+1)*sizeof(char *));
	}
	temp_str = (char *)calloc(strlen(get_token(cur_tokenset,1)) + 1,sizeof(char));
	strcpy(temp_str,get_token(cur_tokenset,1));
	header->other_headers_values[header->n_other_headers] = temp_str;
	temp_str = (char *)calloc(strlen(get_token(cur_tokenset,0)) + 1,sizeof(char));
	strcpy(temp_str,get_token(cur_tokenset,0));
	header->other_headers_keys[header->n_other_headers] = temp_str;
	header->n_other_headers++;

      }
      
      delete_tokens(cur_tokenset);
    }
  } while (IsHeaderLine(buffer));
 
}

/****************************************************************
 **
 ** void read_clf_data(FILE *cur_file, char *buffer, clf_data *data, clf_headers *header)
 **
 ** Read in the data part of the file. Specifically, the x,y, probe_id section.
 ** Note to save space only the probe_id are stored.
 **
 ****************************************************************/

void read_clf_data(FILE *cur_file, char *buffer, clf_data *data, clf_headers *header){
  tokenset *cur_tokenset;
  int x, y, cur_id;

  /* Check to see if the header information includes enough to know that probe_ids are deterministic */
  /* if the are deterministic then don't need to read the rest of the file */


  if (header->sequential > -1){
    data->probe_id = NULL;
    return;
  } else {
    data->probe_id = (int *)calloc((header->rows)*(header->cols), sizeof(int));
    cur_tokenset = tokenize(buffer,"\t\r\n");
    cur_id = atoi(get_token(cur_tokenset,header->header0->probe_id));
    x = atoi(get_token(cur_tokenset,header->header0->x));
    y = atoi(get_token(cur_tokenset,header->header0->y));
    data->probe_id[y*header->rows + x] = cur_id;

    delete_tokens(cur_tokenset);
    while(ReadFileLine(buffer, 1024, cur_file)){
      cur_tokenset = tokenize(buffer,"\t\r\n");
      cur_id = atoi(get_token(cur_tokenset,header->header0->probe_id));
      x = atoi(get_token(cur_tokenset,header->header0->x));
      y = atoi(get_token(cur_tokenset,header->header0->y));
      data->probe_id[y*header->rows + x] = cur_id;

      delete_tokens(cur_tokenset);
    }
  }
}






/****************************************************************
 ****************************************************************
 **
 ** Code for deallocating or initializing header data structures
 **
 ****************************************************************
 ****************************************************************/

void dealloc_clf_headers(clf_headers *header){
  int i;

  if (header->n_chip_type > 0){
    for (i = 0; i < header->n_chip_type; i++){
      free(header->chip_type[i]);
    }
    free(header->chip_type);
  }
      
  if (header->lib_set_name != NULL){
    free(header->lib_set_name);
  }

  if (header->lib_set_version != NULL){
    free(header->lib_set_version);
  }

  if (header->clf_format_version != NULL){
    free(header->clf_format_version);
  }

  if (header->header0_str != NULL){
    free(header->header0_str);
    free(header->header0);
  }
  
  // Not deallocing because forced to row_major
  //if (header->order != NULL){
  //  free(header->order);
  //}
   
  if (header->create_date != NULL){
    free(header->create_date);
  }

  if (header->guid != NULL){
    free(header->guid);
  }

  if (header->n_other_headers > 0){
    for (i = 0; i < header->n_other_headers; i++){
      free(header->other_headers_keys[i]);
      free(header->other_headers_values[i]);
    }
    free(header->other_headers_keys);
    free(header->other_headers_values);
  }
}


void dealloc_clf_data(clf_data *data){
  if (data->probe_id != NULL){
    free(data->probe_id);
  }
}


void dealloc_clf_file(clf_file* my_clf){


  if (my_clf->headers != NULL){
    dealloc_clf_headers(my_clf->headers);
    free(my_clf->headers);
  }

  
  if (my_clf->data !=NULL){
    dealloc_clf_data(my_clf->data);
    free(my_clf->data);
  }

  free(my_clf);
}

/**********************************************************************
 ***
 *** A function for getting the probe_id for a given x,y
 ***
 ***
 *********************************************************************/

void clf_get_probe_id(clf_file *clf, int *probe_id, int x, int y){
 
  if (clf->headers->sequential > -1){
    /* Check if order is "col_major" or "row_major" */

    if (strcmp(clf->headers->order,"col_major") == 0){
      *probe_id = y*clf->headers->cols + x + clf->headers->sequential;
    } else if (strcmp(clf->headers->order,"row_major") == 0){
      *probe_id = x*clf->headers->rows + y + clf->headers->sequential;
    } else {
      *probe_id = -1;  /* ie missing */
    }

  } else {

    *probe_id = clf->data->probe_id[y*clf->headers->rows + x];
  }
}

/**********************************************************************
 ***
 *** A function for getting the x , y for a given probe_id
 ***
 ***
 *********************************************************************/

void clf_get_x_y(clf_file *clf, int probe_id, int *x, int *y){
  int ind;

  if (clf->headers->sequential > -1){
    /* Check if order is "col_major" or "row_major" */

    if (strcmp(clf->headers->order,"col_major") == 0){
      ind = (probe_id - clf->headers->sequential); 
      *x = ind/clf->headers->cols;
      *y = ind%clf->headers->cols;
    } else if (strcmp(clf->headers->order,"row_major") == 0){
      ind = (probe_id - clf->headers->sequential); 
      *x = ind%clf->headers->rows;
      *y = ind/clf->headers->rows;
    } else {
      *x = -1;  /* ie missing */
      *y = -1;
    }
  } else {
    /* Linear Search (this should be improved for routine use) */
    ind = 0;

    while (ind < (clf->headers->cols*clf->headers->rows)){
      if (clf->data->probe_id[ind] == probe_id){
	break;
      }
      ind++;
    }

    if (ind == (clf->headers->cols*clf->headers->rows)){
      *x = -1; *y = -1;
    } else {
      *x = ind/clf->headers->rows;
      *y = ind%clf->headers->rows;
    }
  }
}





/*
 * Note this function is only for testing purposes. It provides no methodology for accessing anything
 * stored in the CLF file in R.
 *
 */

void read_clf_file(char **filename){

  FILE *cur_file;
  clf_file my_clf;
  char *buffer = (char *)calloc(1024, sizeof(char));


  
  cur_file = open_clf_file(filename[0]);
  
  my_clf.headers = (clf_headers *)calloc(1, sizeof(clf_headers));
  my_clf.data = (clf_data *)calloc(1, sizeof(clf_data));

  read_clf_header(cur_file,buffer,my_clf.headers);
  
  read_clf_data(cur_file, buffer, my_clf.data, my_clf.headers);

  free(buffer);
  dealloc_clf_file(&my_clf);
  fclose(cur_file);

}



/****************************************************************
 ****************************************************************
 **
 ** Everything below is specific only to RMAExpress. It is not ported
 ** from/to affyio/BioConductor
 **
 ****************************************************************
 ****************************************************************/

clf_file *read_clf(char *filename){


  FILE *cur_file;
  clf_file *my_clf;
  char *buffer = (char *)calloc(1024, sizeof(char));

  
  cur_file = open_clf_file(filename);

  
  my_clf = (clf_file *)calloc(1,sizeof(clf_file));
  
  my_clf->headers = (clf_headers *)calloc(1, sizeof(clf_headers));
  my_clf->data = (clf_data *)calloc(1, sizeof(clf_data));
  
  read_clf_header(cur_file,buffer,my_clf->headers);

  if (validate_clf_header(my_clf->headers)){
    read_clf_data(cur_file, buffer, my_clf->data, my_clf->headers);
  } else {
     free(buffer);
     dealloc_clf_file(my_clf);
     free(my_clf);

     wxString error = _T("CLF file does not contain all the required headers (defined by version 1.0)\n");

  }


  free(buffer);
  fclose(cur_file);
  return my_clf;
}


wxArrayString clf_get_arraytypes(clf_file *my_clf){

  
  wxArrayString result;
  int i;



  result.Alloc(my_clf->headers->n_chip_type);

  for (i=0; i < my_clf->headers->n_chip_type; i++){
    result[i] = wxString(my_clf->headers->chip_type[i],wxConvUTF8);
  }



  return result;

}

wxString clf_get_libsetversion(clf_file *my_clf){

  
  wxString result;
 
  result = wxString(my_clf->headers->lib_set_version,wxConvUTF8);

  return result;

}

wxString clf_get_libsetname(clf_file *my_clf){

  
  wxString result;
 
  result = wxString(my_clf->headers->lib_set_name,wxConvUTF8);

  return result;

}


int clf_get_rows(clf_file *my_clf){
  return my_clf->headers->rows;
}



int clf_get_cols(clf_file *my_clf){
  return my_clf->headers->cols;
}

