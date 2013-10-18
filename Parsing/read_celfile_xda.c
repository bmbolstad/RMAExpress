#include <cstdio>
#include <cstdlib>

#if defined(HAVE_ZLIB)
#include <zlib.h>
#endif

#include <cmath>  /* for NAN */

#include <cstring>

#include "../threestep_common.h"
#include "read_cel_structures.h"
#include "fread_functions.h"

#include "read_celfile_xda.h"

/****************************************************************
 ****************************************************************
 **
 ** These is the code for reading binary CEL files. 
 ** 
 **
 ***************************************************************
 ***************************************************************/

typedef struct{
  int magic_number;
  int version_number;
  int cols;
  int rows;
  int n_cells;
  int header_len;
  char *header;
  int alg_len;
  char *algorithm;
  int alg_param_len;
  char *alg_param;
  int celmargin;
  unsigned int n_outliers;
  unsigned int n_masks;
  int n_subgrids;
  FILE *infile;
#if defined(HAVE_ZLIB)
  gzFile *gzinfile;
#endif
} binary_header;



typedef  struct{
    float cur_intens;
    float cur_sd;
    short npixels;
} celintens_record;


typedef struct{
  short x;
  short y;
} outliermask_loc;


#include <wx/string.h>

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
    my_tokenset->tokens = (char **)realloc(my_tokenset->tokens,my_tokenset->n*sizeof(char*));
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



/*************************************************************
 **
 ** int isBinaryCelFile(const char *filename)
 **
 ** filename - Name of the prospective binary cel file
 **
 ** Returns 1 if we find the appropriate parts of the 
 ** header (a magic number of 64 followed by version number of 
 ** 4)
 **
 **
 **
 *************************************************************/

int isBinaryCelFile(const char *filename){

  FILE *infile;

  int magicnumber;
  int version_number;
  
  if ((infile = fopen(filename, "rb")) == NULL)
    {
      error("Unable to open the file %s",filename);
      return 0;
    }
  
  if (!fread_int32(&magicnumber,1,infile)){
    fclose(infile);
    return 0;
  }
  
  if (!fread_int32(&version_number,1,infile)){
    fclose(infile);
    return 0;
  }


  if (magicnumber != 64){
    fclose(infile);
    return 0;
  }

  if (version_number != 4){
    fclose(infile);
    return 0;
  }

  
  fclose(infile);
  return 1;
}


/*************************************************************
 **
 ** static void delete_binary_header(binary_header *my_header)
 **
 ** binary_header *my_header
 **
 ** frees memory allocated for binary_header structure
 **
 *************************************************************/

static void delete_binary_header(binary_header *my_header){

  if (my_header->header != NULL)
    free(my_header->header);
  if (my_header->algorithm != NULL)
    free(my_header->algorithm);
  if (my_header->alg_param != NULL)
    free(my_header->alg_param);
  free(my_header);
}


/*************************************************************
 **
 ** static binary_header *read_binary_header(const char *filename, int return_stream, FILE *infile)
 **
 ** const char *filename - name of binary cel file
 ** int return_stream - if 1 return the stream as part of the header, otherwise close the
 **              file at end of function.
 **
 *************************************************************/

static binary_header *read_binary_header(const char *filename, int return_stream, int *err_code){  /* , FILE *infile){ */
  
  FILE *infile;

  binary_header *this_header = (binary_header *)calloc(1,sizeof(binary_header));
  
  /* Pass through all the header information */
  
  if ((infile = fopen(filename, "rb")) == NULL)
    {
      error("Unable to open the file %s\n",filename);
      return 0;
    }
  
  if (!fread_int32(&(this_header->magic_number),1,infile)){
    error("The binary file %s does not have the appropriate magic number\n",filename);
    fclose(infile);
    return 0;
  }
  
  if (this_header->magic_number != 64){
    error("The binary file %s does not have the appropriate magic number\n",filename);
    fclose(infile);
    return 0;
  }
  
  if (!fread_int32(&(this_header->version_number),1,infile)){
    fclose(infile);
    return 0;
  }

  if (this_header->version_number != 4){
    error("The binary file %s is not version 4. Cannot read\n",filename);
    fclose(infile);
    return 0;
  }

  /*** NOTE THE DOCUMENTATION ON THE WEB IS INCONSISTENT WITH THE TRUTH IF YOU LOOK AT THE FUSION SDK */

  /** DOCS - cols then rows , FUSION - rows then cols */
  
  /** We follow FUSION here (in the past we followed the DOCS **/

  if (!fread_int32(&(this_header->rows),1,infile)){
    *err_code = BINARY_HEADER_TRUNCATED;
    fclose(infile);
    return this_header;
    
  }
  
  
  if (!fread_int32(&(this_header->cols),1,infile)){
    *err_code = BINARY_HEADER_TRUNCATED;
    fclose(infile);
    return this_header;
  }
  

  if (!fread_int32(&(this_header->n_cells),1,infile)){
    *err_code = BINARY_HEADER_TRUNCATED;
    fclose(infile);
    return this_header;

  }
  
  if (this_header->n_cells != (this_header->cols)*(this_header->rows)){
    *err_code = BINARY_HEADER_TRUNCATED;
    fclose(infile);
    return this_header;
  }

  
  if (!fread_int32(&(this_header->header_len),1,infile)){
    *err_code = BINARY_HEADER_TRUNCATED;
    fclose(infile);
    return this_header;

  }

  this_header->header = (char *)calloc(this_header->header_len+1,sizeof(char));
  
  if (!fread(this_header->header,sizeof(char),this_header->header_len,infile)){
    *err_code = BINARY_HEADER_TRUNCATED;
    fclose(infile);
    return this_header;
  }
  
  if (!fread_int32(&(this_header->alg_len),1,infile)){
    *err_code = BINARY_HEADER_TRUNCATED;
    fclose(infile);
    return this_header;
  }
  
  this_header->algorithm = (char *)calloc(this_header->alg_len+1,sizeof(char));
  
  if (!fread_char(this_header->algorithm,this_header->alg_len,infile)){
    *err_code = BINARY_HEADER_TRUNCATED;
    return this_header;
  }
  
  if (!fread_int32(&(this_header->alg_param_len),1,infile)){
    *err_code = BINARY_HEADER_TRUNCATED;
    fclose(infile);
    return this_header;
 
  }
  
  this_header->alg_param = (char *)calloc(this_header->alg_param_len+1,sizeof(char));
  
  if (!fread_char(this_header->alg_param,this_header->alg_param_len,infile)){
    *err_code = BINARY_HEADER_TRUNCATED;
    return this_header;
    
  }
    
  if (!fread_int32(&(this_header->celmargin),1,infile)){
    *err_code = BINARY_HEADER_TRUNCATED;
    fclose(infile);
    return this_header;
  
  }
  
  if (!fread_uint32(&(this_header->n_outliers),1,infile)){
    *err_code = BINARY_HEADER_TRUNCATED;
    return this_header;
  }
  
  if (!fread_uint32(&(this_header->n_masks),1,infile)){
    *err_code = BINARY_HEADER_TRUNCATED;
    fclose(infile);
    return this_header;
  
  }

  if (!fread_int32(&(this_header->n_subgrids),1,infile)){
    *err_code = BINARY_HEADER_TRUNCATED;
    fclose(infile);
    return this_header;
  
  } 


  if (!return_stream){
    fclose(infile);
  } else {
    this_header->infile = infile;
  }
  
  
  return this_header;




}

/*************************************************************
 **
 ** static char *binary_get_header_info(const char *filename, int *dim1, int *dim2)
 **
 ** this function pulls out the rows, cols and cdfname
 ** from the header of a binary cel file
 **
 *************************************************************/

char *binary_get_header_info(const char *filename, int *dim1, int *dim2, int *err_code){
  

  char *cdfName =0;
  tokenset *my_tokenset;

  int i = 0,endpos;
  
  binary_header *my_header;

  *err_code = 0; /* At this point no error has been found */
  
  my_header = read_binary_header(filename,0, err_code);

  if (*err_code){ 
    delete_binary_header(my_header);
    return NULL;
  }

  *dim1 = my_header->cols;
  *dim2 = my_header->rows;

  my_tokenset = tokenize(my_header->header," ");
    
  for (i =0; i < tokenset_size(my_tokenset);i++){
    /* look for a token ending in ".1sq" */
    endpos=token_ends_with(get_token(my_tokenset,i),".1sq");
    if(endpos > 0){
      /* Found the likely CDF name, now chop of .1sq and store it */      
      cdfName= (char *)calloc(endpos+1,sizeof(char));
      strncpy(cdfName,get_token(my_tokenset,i),endpos);
      cdfName[endpos] = '\0';
      
      break;
    }
    if (i == (tokenset_size(my_tokenset) - 1)){
      *err_code= BINARY_HEADER_CDF_NAME_MISSING;
      break;
    }
  }
  
  delete_binary_header(my_header);
  delete_tokens(my_tokenset);
  return(cdfName);
  
}





/*************************************************************************
 **
 ** void binary_get_detailed_header_info(const char *filename, detailed_header_info *header_info)
 **
 ** const char *filename - file to open
 ** detailed_header_info *header_info - place to store header information
 **
 ** reads the header information from a binary cdf file (ignoring some fields
 ** that are unused).
 **
 ************************************************************************/

void binary_get_detailed_header_info(const char *filename, detailed_header_info *header_info,int *err_code){

  /* char *cdfName =0; */
  tokenset *my_tokenset;
  tokenset *temp_tokenset;

  char *header_copy;
  char *tmpbuffer;


  
  int i = 0,endpos;
  
  binary_header *my_header;


  my_header = read_binary_header(filename,0, err_code);
  if (*err_code){
    delete_binary_header(my_header);
    return;
  }



  header_info->cols = my_header->cols;
  header_info->rows = my_header->rows;


  header_info->Algorithm = (char *)calloc(strlen(my_header->algorithm)+1,sizeof(char));
  
  strcpy(header_info->Algorithm,my_header->algorithm);

  header_info->AlgorithmParameters = (char *)calloc(strlen(my_header->alg_param)+1,sizeof(char));
  strncpy(header_info->AlgorithmParameters,my_header->alg_param,strlen(my_header->alg_param)-1);
  

  /* Rprintf("%s\n\n\n",my_header->header); */


  header_copy = (char *)calloc(strlen(my_header->header) +1,sizeof(char));
  strcpy(header_copy,my_header->header);
  my_tokenset = tokenize(header_copy,"\n");

  /** Looking for GridCornerUL, GridCornerUR, GridCornerLR, GridCornerLL and DatHeader */


  for (i =0; i < tokenset_size(my_tokenset);i++){
    /* Rprintf("%d: %s\n",i,get_token(my_tokenset,i)); */
    if (strncmp("GridCornerUL",get_token(my_tokenset,i),12) == 0){
      tmpbuffer = (char *)calloc(strlen(get_token(my_tokenset,i))+1,sizeof(char));
      strcpy(tmpbuffer,get_token(my_tokenset,i));

      temp_tokenset = tokenize(tmpbuffer,"= ");
      header_info->GridCornerULx  = atoi(get_token(temp_tokenset,1));
      header_info->GridCornerULy  = atoi(get_token(temp_tokenset,2));
      delete_tokens(temp_tokenset);
      free(tmpbuffer);
    }
    if (strncmp("GridCornerUR",get_token(my_tokenset,i),12) == 0){
      tmpbuffer = (char *)calloc(strlen(get_token(my_tokenset,i))+1,sizeof(char));
      strcpy(tmpbuffer,get_token(my_tokenset,i));

      temp_tokenset = tokenize(tmpbuffer,"= ");
      header_info->GridCornerURx  = atoi(get_token(temp_tokenset,1));
      header_info->GridCornerURy  = atoi(get_token(temp_tokenset,2));
      delete_tokens(temp_tokenset);
      free(tmpbuffer);
    }
    if (strncmp("GridCornerLR",get_token(my_tokenset,i),12) == 0){
      tmpbuffer = (char *)calloc(strlen(get_token(my_tokenset,i))+1,sizeof(char));
      strcpy(tmpbuffer,get_token(my_tokenset,i));

      temp_tokenset = tokenize(tmpbuffer,"= ");
      header_info->GridCornerLRx  = atoi(get_token(temp_tokenset,1));
      header_info->GridCornerLRy  = atoi(get_token(temp_tokenset,2));
      delete_tokens(temp_tokenset);
      free(tmpbuffer);
    }
    if (strncmp("GridCornerLL",get_token(my_tokenset,i),12) == 0){
      tmpbuffer = (char *)calloc(strlen(get_token(my_tokenset,i))+1,sizeof(char));
      strcpy(tmpbuffer,get_token(my_tokenset,i));

      temp_tokenset = tokenize(tmpbuffer,"= ");
      header_info->GridCornerLLx  = atoi(get_token(temp_tokenset,1));
      header_info->GridCornerLLy  = atoi(get_token(temp_tokenset,2));
      delete_tokens(temp_tokenset);
      free(tmpbuffer);
    }
    if (strncmp("DatHeader",get_token(my_tokenset,i),9) == 0){
      header_info->DatHeader = (char *)calloc(strlen(get_token(my_tokenset,i))+1, sizeof(char));
      strcpy(header_info->DatHeader,(get_token(my_tokenset,i)+10));
    }
  }
    

  delete_tokens(my_tokenset);


  free(header_copy);

  header_copy = (char *)calloc(my_header->header_len +1,sizeof(char));
  strcpy(header_copy,my_header->header);
  my_tokenset = tokenize(header_copy," ");
    
  for (i =0; i < tokenset_size(my_tokenset);i++){
    /* look for a token ending in ".1sq" */
    endpos=token_ends_with(get_token(my_tokenset,i),".1sq");
    if(endpos > 0){
      /* Found the likely CDF name, now chop of .1sq and store it */      
      header_info->cdfName= (char *)calloc(endpos+1,sizeof(char));
      strncpy(header_info->cdfName,get_token(my_tokenset,i),endpos);
      header_info->cdfName[endpos] = '\0';
      
      break;
    }
    if (i == (tokenset_size(my_tokenset) - 1)){
      *err_code = BINARY_HEADER_CDF_NAME_MISSING;
      return;
    }
  }
  

  delete_tokens(my_tokenset);
  delete_binary_header(my_header);
  free(header_copy);


}

/***************************************************************
 **
 ** int check_binary_cel_file(const char *filename, char *ref_cdfName, int ref_dim_1, int ref_dim_2)
 ** 
 ** This function checks a binary cel file to see if it has the 
 ** expected rows, cols and cdfname
 **
 **************************************************************/

int check_binary_cel_file(const char *filename, const char *ref_cdfName, int ref_dim_1, int ref_dim_2){



  char *cdfName =0;
  tokenset *my_tokenset;

  int i = 0,endpos;
  
  binary_header *my_header;
  int err_code;

  my_header = read_binary_header(filename,0, &err_code);  

  if (err_code){
    delete_binary_header(my_header);
    return err_code;
  }

  if ((my_header->cols != ref_dim_1) || (my_header->rows != ref_dim_2)){
    delete_binary_header(my_header);
    return CEL_NON_MATCHING_DIMENSIONS;
  }
  
  my_tokenset = tokenize(my_header->header," ");



    
  for (i =0; i < tokenset_size(my_tokenset);i++){
    /* look for a token ending in ".1sq" */
    endpos=token_ends_with(get_token(my_tokenset,i),".1sq");
    if(endpos > 0){
      /* Found the likely CDF name, now chop of .1sq and store it */      
      cdfName= (char *)calloc(endpos+1,sizeof(char));
      strncpy(cdfName,get_token(my_tokenset,i),endpos);
      cdfName[endpos] = '\0';
      
      break;
    }
    if (i == (tokenset_size(my_tokenset) - 1)){
      delete_tokens(my_tokenset);
      delete_binary_header(my_header);
      return BINARY_HEADER_CDF_NAME_MISSING;
    }
  }

  if (strncasecmp(cdfName,ref_cdfName,strlen(ref_cdfName)) != 0){
    delete_tokens(my_tokenset);
    delete_binary_header(my_header);
    free(cdfName);
    return CEL_NON_MATCHING_CDFNAMES;
  }

  
  delete_binary_header(my_header);
  delete_tokens(my_tokenset);
  free(cdfName);



  return 0;
}


//inline bool isnan(double x) {
//	return __builtin_isnanf(x);
//}


/***************************************************************
 **
 ** static int read_binarycel_file_intensities(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows)
 **
 ** 
 ** This function reads binary cel file intensities into the data matrix
 **
 **************************************************************/

int read_binarycel_file_intensities(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows){

  int i=0, j=0;
  int cur_index;
  
  int fread_err=0;


  celintens_record *cur_intensity = (celintens_record *)calloc(1,sizeof(celintens_record));
  binary_header *my_header;

  int err_code = 0;

  my_header = read_binary_header(filename,1, &err_code);
  
  if (err_code){
    delete_binary_header(my_header);
    return err_code;
  }

  for (i = 0; i < my_header->rows; i++){
    for (j =0; j < my_header->cols; j++){
      cur_index = j + my_header->cols*i; /* i + my_header->rows*j; */
      fread_err = fread_float32(&(cur_intensity->cur_intens),1,my_header->infile);
      fread_err+= fread_float32(&(cur_intensity->cur_sd),1,my_header->infile);
      fread_err+=fread_int16(&(cur_intensity->npixels),1,my_header->infile);
      if (fread_err < 3){
	fclose(my_header->infile);
	delete_binary_header(my_header);
	free(cur_intensity);
	return BINARY_INTENSITY_TRUNCATED;
      }   
      if (cur_intensity->cur_intens < 0 || cur_intensity->cur_intens > 65536 || isnan(cur_intensity->cur_intens)){
        fclose(my_header->infile);
        delete_binary_header(my_header);
        free(cur_intensity);
        return BINARY_INTENSITY_CORRUPTED;
      }
 
      fread_err=0;
      intensity[chip_num*my_header->n_cells + cur_index] = (double )cur_intensity->cur_intens;
    }
  }
  
  fclose(my_header->infile);
  delete_binary_header(my_header);
  free(cur_intensity);
  return(0);
}


#if defined(INCLUDE_ALL_AFFYIO)

/***************************************************************
 **
 ** static int read_binarycel_file_stdev(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows)
 **
 ** 
 ** This function reads binary cel file stddev values into the data matrix
 **
 **************************************************************/

int read_binarycel_file_stddev(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows){

  int i=0, j=0;
  int cur_index;

  int fread_err=0;
  
  celintens_record *cur_intensity = (celintens_record *)calloc(1,sizeof(celintens_record));
  binary_header *my_header;
  
  int err_code;

  my_header = read_binary_header(filename,1, &err_code);
  
  if (err_code){ 
    delete_binary_header(my_header);
    return err_code;
  }
  
  for (i = 0; i < my_header->rows; i++){
    for (j =0; j < my_header->cols; j++){
      cur_index = j + my_header->cols*i; /* i + my_header->rows*j; */
      fread_err = fread_float32(&(cur_intensity->cur_intens),1,my_header->infile);
      fread_err+= fread_float32(&(cur_intensity->cur_sd),1,my_header->infile);
      fread_err+= fread_int16(&(cur_intensity->npixels),1,my_header->infile);
      if (fread_err < 3){
	fclose(my_header->infile);
	delete_binary_header(my_header);
	free(cur_intensity);
	return 1;
      }
      fread_err=0;
      intensity[chip_num*my_header->n_cells + cur_index] = (double )cur_intensity->cur_sd;
    }
  }
  
  fclose(my_header->infile);
  delete_binary_header(my_header);
  free(cur_intensity);
  return(0);
}




/***************************************************************
 **
 ** static int read_binarycel_file_npixels(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows)
 **
 ** 
 ** This function reads binary cel file npixels values into the data matrix
 **
 **************************************************************/

int read_binarycel_file_npixels(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows){

  int i=0, j=0;
  int cur_index;

  int fread_err=0;
 
  celintens_record *cur_intensity = (celintens_record *)calloc(1,sizeof(celintens_record));
  binary_header *my_header;

  int err_code = 0;

  my_header = read_binary_header(filename,1, &err_code);

  if (err_code){ 
    delete_binary_header(my_header);
    return err_code;
  }
  
  for (i = 0; i < my_header->rows; i++){
    for (j =0; j < my_header->cols; j++){
      cur_index = j + my_header->cols*i; /* i + my_header->rows*j; */
      fread_err = fread_float32(&(cur_intensity->cur_intens),1,my_header->infile);
      fread_err+= fread_float32(&(cur_intensity->cur_sd),1,my_header->infile);
      fread_err+= fread_int16(&(cur_intensity->npixels),1,my_header->infile);  
      if (fread_err < 3){
	fclose(my_header->infile);
	delete_binary_header(my_header);
	free(cur_intensity);
	return 1;
      }
      fread_err=0;
      intensity[chip_num*my_header->n_cells + cur_index] = (double )cur_intensity->npixels;
    }
  }
  
  fclose(my_header->infile);
  delete_binary_header(my_header);
  free(cur_intensity);
  return(0);
}





/***************************************************************
 **
 ** static void binary_apply_masks(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows, int rm_mask, int rm_outliers)
 **
 ** 
 **
 **************************************************************/

void binary_apply_masks(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows, int rm_mask, int rm_outliers){
  
  int i=0;

  int cur_index;

  int sizeofrecords;

  outliermask_loc *cur_loc= (outliermask_loc *)calloc(1,sizeof(outliermask_loc));
  binary_header *my_header;

  int err_code;

  my_header = read_binary_header(filename,1, &err_code);
  
  if (err_code){ 
    delete_binary_header(my_header);
    return;
  }

  sizeofrecords = 2*sizeof(float) + sizeof(short); /* sizeof(celintens_record) */
  
  fseek(my_header->infile,my_header->n_cells*sizeofrecords,SEEK_CUR);

  if (rm_mask){
    for (i =0; i < (int)my_header->n_masks; i++){
      fread_int16(&(cur_loc->x),1,my_header->infile);
      fread_int16(&(cur_loc->y),1,my_header->infile);
      cur_index = (int)cur_loc->x + my_header->rows*(int)cur_loc->y; 
      /* cur_index = (int)cur_loc->y + my_header->rows*(int)cur_loc->x; */
      /*   intensity[chip_num*my_header->rows + cur_index] = NAN; */
      intensity[chip_num*rows + cur_index] =  NAN;
      

    }
  } else {
    fseek(my_header->infile,my_header->n_masks*sizeof(cur_loc),SEEK_CUR);

  }

  if (rm_outliers){
    for (i =0; i < (int)my_header->n_outliers; i++){
      fread_int16(&(cur_loc->x),1,my_header->infile);
      fread_int16(&(cur_loc->y),1,my_header->infile);
      cur_index = (int)cur_loc->x + my_header->rows*(int)cur_loc->y; 
      /* intensity[chip_num*my_header->n_cells + cur_index] = NAN; */
      intensity[chip_num*rows + cur_index] =  NAN;
    }
  } else {
    fseek(my_header->infile,my_header->n_outliers*sizeof(cur_loc),SEEK_CUR);
  }
  
  fclose(my_header->infile);
  delete_binary_header(my_header);
 
  free(cur_loc);

}

/****************************************************************
 **
 ** static void binary_get_masks_outliers(const char *filename, 
 **                         int *nmasks, short **masks_x, short **masks_y, 
 **                         int *noutliers, short **outliers_x, short **outliers_y
 ** 
 ** This gets the x and y coordinates stored in the masks and outliers sections
 ** of the cel files. (for binary CEL files)
 **
 ****************************************************************/

void binary_get_masks_outliers(const char *filename, int *nmasks, short **masks_x, short **masks_y, int *noutliers, short **outliers_x, short **outliers_y){

  
  int i=0;

  int sizeofrecords;

  outliermask_loc *cur_loc= (outliermask_loc *)calloc(1,sizeof(outliermask_loc));
  binary_header *my_header;

  int err_code = 0;
  my_header = read_binary_header(filename,1, &err_code);

  if (err_code){
    delete_binary_header(my_header);
    return;
  }

  sizeofrecords = 2*sizeof(float) + sizeof(short);

  fseek(my_header->infile,my_header->n_cells*sizeofrecords,SEEK_CUR);
 

  *nmasks = my_header->n_masks;
  *masks_x = (short *)calloc(my_header->n_masks,sizeof(short));
  *masks_y = (short *)calloc(my_header->n_masks,sizeof(short));

  for (i =0; i < (int)my_header->n_masks; i++){
    fread_int16(&(cur_loc->x),1,my_header->infile);
    fread_int16(&(cur_loc->y),1,my_header->infile);
    (*masks_x)[i] = (cur_loc->x);
    (*masks_y)[i] = (cur_loc->y);
  }


  *noutliers = my_header->n_outliers;
  *outliers_x = (short *)calloc(my_header->n_outliers,sizeof(short));
  *outliers_y = (short *)calloc(my_header->n_outliers,sizeof(short));
  


  for (i =0; i < (int)my_header->n_outliers; i++){
    fread_int16(&(cur_loc->x),1,my_header->infile);
    fread_int16(&(cur_loc->y),1,my_header->infile);
    (*outliers_x)[i] = (cur_loc->x);
    (*outliers_y)[i] = (cur_loc->y);


  }
      
  fclose(my_header->infile);
  delete_binary_header(my_header);
 
  free(cur_loc);
  

}

#endif

/****************************************************************
 ****************************************************************
 **
 ** The following code is for supporting gzipped binary
 ** format CEL files.
 **
 ****************************************************************
 ***************************************************************/


#if defined(HAVE_ZLIB)

/*************************************************************
 **
 ** int isgzBinaryCelFile(const char *filename)
 **
 ** filename - Name of the prospective gzipped binary cel file
 **
 ** Returns 1 if we find the appropriate parts of the 
 ** header (a magic number of 64 followed by version number of 
 ** 4)
 **
 **
 **
 *************************************************************/

int isgzBinaryCelFile(const char *filename){

  gzFile infile;

  int magicnumber;
  int version_number;
  
  if ((infile = gzopen(filename, "rb")) == NULL)
    {
      error("Unable to open the file %s",filename);
      return 0;
    }
  
  if (!gzread_int32(&magicnumber,1,infile)){
    gzclose(infile);
    return 0;
  }
  
  if (!gzread_int32(&version_number,1,infile)){
    gzclose(infile);
    return 0;
  }


  if (magicnumber != 64){
    gzclose(infile);
    return 0;
  }

  if (version_number != 4){
    gzclose(infile);
    return 0;
  }

  
  gzclose(infile);
  return 1;
}


/*************************************************************
 **
 ** static binary_header *gzread_binary_header(const char *filename, int return_stream, FILE *infile)
 **
 ** const char *filename - name of binary cel file
 ** int return_stream - if 1 return the stream as part of the header, otherwise close the
 **              file at end of function.
 **
 *************************************************************/

static binary_header *gzread_binary_header(const char *filename, int return_stream){  /* , FILE *infile){ */
  
  gzFile infile;

  binary_header *this_header = (binary_header *)calloc(1,sizeof(binary_header));
  
  /* Pass through all the header information */
  
  if ((infile = gzopen(filename, "rb")) == NULL)
    {
      error("Unable to open the file %s\n",filename);
      return 0;
    }
  
  if (!gzread_int32(&(this_header->magic_number),1,infile)){
    error("The binary file %s does not have the appropriate magic number\n",filename);
    return 0;
  }
  
  if (this_header->magic_number != 64){
    error("The binary file %s does not have the appropriate magic number\n",filename);
    return 0;
  }
  
  if (!gzread_int32(&(this_header->version_number),1,infile)){
    return 0;
  }

  if (this_header->version_number != 4){
    error("The binary file %s is not version 4. Cannot read\n",filename);
    return 0;
  }
    
  /*** NOTE THE DOCUMENTATION ON THE WEB IS INCONSISTENT WITH THE TRUTH IF YOU LOOK AT THE FUSION SDK */

  /** DOCS - cols then rows , FUSION - rows then cols */
  
  /** We follow FUSION here (in the past we followed the DOCS **/
  
  if (!gzread_int32(&(this_header->rows),1,infile)){
    error("Binary file corrupted? Could not read any further\n");
  }
  
  if (!gzread_int32(&(this_header->cols),1,infile)){
    error("Binary file corrupted? Could not read any further\n");
    return 0;
  }

  if (!gzread_int32(&(this_header->n_cells),1,infile)){
    error("Binary file corrupted? Could not read any further\n");
  }
  
  if (this_header->n_cells != (this_header->cols)*(this_header->rows)){
    error("The number of cells does not seem to be equal to cols*rows in %s.\n",filename);
  }

  
  if (!gzread_int32(&(this_header->header_len),1,infile)){
    error("Binary file corrupted? Could not read any further\n");
  }

  this_header->header = (char *)calloc(this_header->header_len+1,sizeof(char));
  
  if (!gzread(infile,this_header->header,sizeof(char)*this_header->header_len)){
    error("binary file corrupted? Could not read any further.\n");
  }
  
  if (!gzread_int32(&(this_header->alg_len),1,infile)){
    error("Binary file corrupted? Could not read any further\n");
  }
  
  this_header->algorithm = (char *)calloc(this_header->alg_len+1,sizeof(char));
  
  if (!gzread_char(this_header->algorithm,this_header->alg_len,infile)){
    error("binary file corrupted? Could not read any further.\n");
  }
  
  if (!gzread_int32(&(this_header->alg_param_len),1,infile)){
    error("Binary file corrupted? Could not read any further\n");
  }
  
  this_header->alg_param = (char *)calloc(this_header->alg_param_len+1,sizeof(char));
  
  if (!gzread_char(this_header->alg_param,this_header->alg_param_len,infile)){
    error("binary file corrupted? Could not read any further.\n");
  }
    
  if (!gzread_int32(&(this_header->celmargin),1,infile)){
    error("Binary file corrupted? Could not read any further\n");
  }
  
  if (!gzread_uint32(&(this_header->n_outliers),1,infile)){
    error("Binary file corrupted? Could not read any further\n");
  }
  
  if (!gzread_uint32(&(this_header->n_masks),1,infile)){
    error("Binary file corrupted? Could not read any further\n");
  }

  if (!gzread_int32(&(this_header->n_subgrids),1,infile)){
    error("Binary file corrupted? Could not read any further\n");
  } 


  if (!return_stream){
    gzclose(infile);
  } else {
    this_header->gzinfile = &infile;
  }
  
  
  return this_header;




}



/*************************************************************
 **
 ** static char *binary_get_header_info(const char *filename, int *dim1, int *dim2)
 **
 ** this function pulls out the rows, cols and cdfname
 ** from the header of a binary cel file
 **
 *************************************************************/

char *gzbinary_get_header_info(const char *filename, int *dim1, int *dim2){
  

  char *cdfName =0;
  tokenset *my_tokenset;

  int i = 0,endpos;
  
  binary_header *my_header;


  my_header = gzread_binary_header(filename,0);

  *dim1 = my_header->cols;
  *dim2 = my_header->rows;

  my_tokenset = tokenize(my_header->header," ");
    
  for (i =0; i < tokenset_size(my_tokenset);i++){
    /* look for a token ending in ".1sq" */
    endpos=token_ends_with(get_token(my_tokenset,i),".1sq");
    if(endpos > 0){
      /* Found the likely CDF name, now chop of .1sq and store it */      
      cdfName= (char *)calloc(endpos+1,sizeof(char));
      strncpy(cdfName,get_token(my_tokenset,i),endpos);
      cdfName[endpos] = '\0';
      
      break;
    }
    if (i == (tokenset_size(my_tokenset) - 1)){
      error("Cel file %s does not seem to be have cdf information",filename);
    }
  }
  
  delete_binary_header(my_header);
  delete_tokens(my_tokenset);
  return(cdfName);
  
}



/*************************************************************************
 **
 ** void gzbinary_get_detailed_header_info(const char *filename, detailed_header_info *header_info)
 **
 ** const char *filename - file to open
 ** detailed_header_info *header_info - place to store header information
 **
 ** reads the header information from a gzipped binary cdf file (ignoring some fields
 ** that are unused).
 **
 ************************************************************************/





void gzbinary_get_detailed_header_info(const char *filename, detailed_header_info *header_info){

  /* char *cdfName =0; */
  tokenset *my_tokenset;
  tokenset *temp_tokenset;

  char *header_copy;
  char *tmpbuffer;


  
  int i = 0,endpos;
  
  binary_header *my_header;
  

  my_header = gzread_binary_header(filename,0);


  header_info->cols = my_header->cols;
  header_info->rows = my_header->rows;


  header_info->Algorithm = (char *)calloc(strlen(my_header->algorithm)+1,sizeof(char));
  
  strcpy(header_info->Algorithm,my_header->algorithm);

  header_info->AlgorithmParameters = (char *)calloc(strlen(my_header->alg_param)+1,sizeof(char));
  strncpy(header_info->AlgorithmParameters,my_header->alg_param,strlen(my_header->alg_param)-1);
  

  /* Rprintf("%s\n\n\n",my_header->header); */


  header_copy = (char *)calloc(strlen(my_header->header) +1,sizeof(char));
  strcpy(header_copy,my_header->header);
  my_tokenset = tokenize(header_copy,"\n");

  /** Looking for GridCornerUL, GridCornerUR, GridCornerLR, GridCornerLL and DatHeader */


  for (i =0; i < tokenset_size(my_tokenset);i++){
    /* Rprintf("%d: %s\n",i,get_token(my_tokenset,i)); */
    if (strncmp("GridCornerUL",get_token(my_tokenset,i),12) == 0){
      tmpbuffer = (char *)calloc(strlen(get_token(my_tokenset,i))+1,sizeof(char));
      strcpy(tmpbuffer,get_token(my_tokenset,i));

      temp_tokenset = tokenize(tmpbuffer,"= ");
      header_info->GridCornerULx  = atoi(get_token(temp_tokenset,1));
      header_info->GridCornerULy  = atoi(get_token(temp_tokenset,2));
      delete_tokens(temp_tokenset);
      free(tmpbuffer);
    }
    if (strncmp("GridCornerUR",get_token(my_tokenset,i),12) == 0){
      tmpbuffer = (char *)calloc(strlen(get_token(my_tokenset,i))+1,sizeof(char));
      strcpy(tmpbuffer,get_token(my_tokenset,i));

      temp_tokenset = tokenize(tmpbuffer,"= ");
      header_info->GridCornerURx  = atoi(get_token(temp_tokenset,1));
      header_info->GridCornerURy  = atoi(get_token(temp_tokenset,2));
      delete_tokens(temp_tokenset);
      free(tmpbuffer);
    }
    if (strncmp("GridCornerLR",get_token(my_tokenset,i),12) == 0){
      tmpbuffer = (char *)calloc(strlen(get_token(my_tokenset,i))+1,sizeof(char));
      strcpy(tmpbuffer,get_token(my_tokenset,i));

      temp_tokenset = tokenize(tmpbuffer,"= ");
      header_info->GridCornerLRx  = atoi(get_token(temp_tokenset,1));
      header_info->GridCornerLRy  = atoi(get_token(temp_tokenset,2));
      delete_tokens(temp_tokenset);
      free(tmpbuffer);
    }
    if (strncmp("GridCornerLL",get_token(my_tokenset,i),12) == 0){
      tmpbuffer = (char *)calloc(strlen(get_token(my_tokenset,i))+1,sizeof(char));
      strcpy(tmpbuffer,get_token(my_tokenset,i));

      temp_tokenset = tokenize(tmpbuffer,"= ");
      header_info->GridCornerLLx  = atoi(get_token(temp_tokenset,1));
      header_info->GridCornerLLy  = atoi(get_token(temp_tokenset,2));
      delete_tokens(temp_tokenset);
      free(tmpbuffer);
    }
    if (strncmp("DatHeader",get_token(my_tokenset,i),9) == 0){
      header_info->DatHeader = (char *)calloc(strlen(get_token(my_tokenset,i))+1, sizeof(char));
      strcpy(header_info->DatHeader,(get_token(my_tokenset,i)+10));
    }
  }
    

  delete_tokens(my_tokenset);


  free(header_copy);

  header_copy = (char *)calloc(my_header->header_len +1,sizeof(char));
  strcpy(header_copy,my_header->header);
  my_tokenset = tokenize(header_copy," ");
    
  for (i =0; i < tokenset_size(my_tokenset);i++){
    /* look for a token ending in ".1sq" */
    endpos=token_ends_with(get_token(my_tokenset,i),".1sq");
    if(endpos > 0){
      /* Found the likely CDF name, now chop of .1sq and store it */      
      header_info->cdfName= (char *)calloc(endpos+1,sizeof(char));
      strncpy(header_info->cdfName,get_token(my_tokenset,i),endpos);
      header_info->cdfName[endpos] = '\0';
      
      break;
    }
    if (i == (tokenset_size(my_tokenset) - 1)){
      error("Cel file %s does not seem to be have cdf information",filename);
    }
  }
  

  delete_tokens(my_tokenset);
  delete_binary_header(my_header);
  free(header_copy);


}



/***************************************************************
 **
 ** static int check_binary_cel_file(const char *filename, char *ref_cdfName, int ref_dim_1, int ref_dim_2)
 ** 
 ** This function checks a binary cel file to see if it has the 
 ** expected rows, cols and cdfname
 **
 **************************************************************/

int check_gzbinary_cel_file(const char *filename, const char *ref_cdfName, int ref_dim_1, int ref_dim_2){



  char *cdfName =0;
  tokenset *my_tokenset;

  int i = 0,endpos;
  
  binary_header *my_header;


  my_header = gzread_binary_header(filename,0);  

  if ((my_header->cols != ref_dim_1) || (my_header->rows != ref_dim_2)){
    error("Cel file %s does not seem to have the correct dimensions",filename);
  }
  
  my_tokenset = tokenize(my_header->header," ");



    
  for (i =0; i < tokenset_size(my_tokenset);i++){
    /* look for a token ending in ".1sq" */
    endpos=token_ends_with(get_token(my_tokenset,i),".1sq");
    if(endpos > 0){
      /* Found the likely CDF name, now chop of .1sq and store it */      
      cdfName= (char *)calloc(endpos+1,sizeof(char));
      strncpy(cdfName,get_token(my_tokenset,i),endpos);
      cdfName[endpos] = '\0';
      
      break;
    }
    if (i == (tokenset_size(my_tokenset) - 1)){
      error("Cel file %s does not seem to be have cdf information",filename);
    }
  }

  if (strncasecmp(cdfName,ref_cdfName,strlen(ref_cdfName)) != 0){
    error("Cel file %s does not seem to be of %s type",filename,ref_cdfName);
  }

  
  delete_binary_header(my_header);
  delete_tokens(my_tokenset);
  free(cdfName);



  return 0;
}



/***************************************************************
 **
 ** static int gzread_binarycel_file_intensities(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows)
 **
 ** 
 ** This function reads gzipped binary cel file intensities into the data matrix
 **
 **************************************************************/

int gzread_binarycel_file_intensities(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows){

  int i=0, j=0;
  int cur_index;
  
  int fread_err=0;


  celintens_record *cur_intensity = (celintens_record *)calloc(1,sizeof(celintens_record));
  binary_header *my_header;

  my_header = gzread_binary_header(filename,1);
  
  for (i = 0; i < my_header->rows; i++){
    for (j =0; j < my_header->cols; j++){
      cur_index = j + my_header->cols*i; /* i + my_header->rows*j; */
      fread_err = gzread_float32(&(cur_intensity->cur_intens),1,my_header->gzinfile);
      fread_err+= gzread_float32(&(cur_intensity->cur_sd),1,my_header->gzinfile);
      fread_err+= gzread_int16(&(cur_intensity->npixels),1,my_header->gzinfile);
      if (fread_err < 3){
	gzclose(my_header->gzinfile);
	delete_binary_header(my_header);
	free(cur_intensity);
	return 1;
      } 
      if (cur_intensity->cur_intens < 0 || cur_intensity->cur_intens > 65536 || isnan(cur_intensity->cur_intens)){
        gzclose(my_header->infile);
        delete_binary_header(my_header);
        free(cur_intensity);
        return 1;
      }
      fread_err=0;
      intensity[chip_num*my_header->n_cells + cur_index] = (double )cur_intensity->cur_intens;
    }
  }
  
  gzclose(my_header->gzinfile);
  delete_binary_header(my_header);
  free(cur_intensity);
  return(0);
}




/***************************************************************
 **
 ** static int gzread_binarycel_file_stdev(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows)
 **
 ** 
 ** This function reads binary cel file stddev values into the data matrix
 **
 **************************************************************/

int gzread_binarycel_file_stddev(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows){

  int i=0, j=0;
  int cur_index;

  int fread_err=0;
  
  celintens_record *cur_intensity = (celintens_record *)calloc(1,sizeof(celintens_record));
  binary_header *my_header;

  my_header = gzread_binary_header(filename,1);
  
  for (i = 0; i < my_header->rows; i++){
    for (j =0; j < my_header->cols; j++){
      cur_index = j + my_header->cols*i; /* i + my_header->rows*j; */
      fread_err = gzread_float32(&(cur_intensity->cur_intens),1,my_header->gzinfile);
      fread_err+= gzread_float32(&(cur_intensity->cur_sd),1,my_header->gzinfile);
      fread_err+= gzread_int16(&(cur_intensity->npixels),1,my_header->gzinfile);
      if (fread_err < 3){
	gzclose(my_header->gzinfile);
	delete_binary_header(my_header);
	free(cur_intensity);
	return 1;
      }
      fread_err=0;
      intensity[chip_num*my_header->n_cells + cur_index] = (double )cur_intensity->cur_sd;
    }
  }
  
  gzclose(my_header->gzinfile);
  delete_binary_header(my_header);
  free(cur_intensity);
  return(0);
}





/***************************************************************
 **
 ** static int read_binarycel_file_npixels(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows)
 **
 ** 
 ** This function reads binary cel file npixels values into the data matrix
 **
 **************************************************************/

int gzread_binarycel_file_npixels(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows){

  int i=0, j=0;
  int cur_index;

  int fread_err=0;
 
  celintens_record *cur_intensity = (celintens_record *)calloc(1,sizeof(celintens_record));
  binary_header *my_header;

  my_header = gzread_binary_header(filename,1);
  
  for (i = 0; i < my_header->rows; i++){
    for (j =0; j < my_header->cols; j++){
      cur_index = j + my_header->cols*i; /* i + my_header->rows*j; */
      fread_err = gzread_float32(&(cur_intensity->cur_intens),1,my_header->gzinfile);
      fread_err+= gzread_float32(&(cur_intensity->cur_sd),1,my_header->gzinfile);
      fread_err+= gzread_int16(&(cur_intensity->npixels),1,my_header->gzinfile);  
      if (fread_err < 3){
	gzclose(my_header->infile);
	delete_binary_header(my_header);
	free(cur_intensity);
	return 1;
      }
      fread_err=0;
      intensity[chip_num*my_header->n_cells + cur_index] = (double )cur_intensity->npixels;
    }
  }
  
  gzclose(my_header->gzinfile);
  delete_binary_header(my_header);
  free(cur_intensity);
  return(0);
}




/***************************************************************
 **
 ** static void gz_binary_apply_masks(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows, int rm_mask, int rm_outliers)
 **
 ** 
 **
 **************************************************************/

void gz_binary_apply_masks(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows, int rm_mask, int rm_outliers){
  
  int i=0;

  int cur_index;

  int sizeofrecords;

  outliermask_loc *cur_loc= (outliermask_loc *)calloc(1,sizeof(outliermask_loc));
  binary_header *my_header;

  my_header = gzread_binary_header(filename,1);
  
  sizeofrecords = 2*sizeof(float) + sizeof(short); /* sizeof(celintens_record) */
  
  //fseek(my_header->infile,my_header->n_cells*sizeofrecords,SEEK_CUR);
  gzseek(my_header->infile,my_header->n_cells*sizeofrecords,SEEK_CUR);
  if (rm_mask){
    for (i =0; i < my_header->n_masks; i++){
      gzread_int16(&(cur_loc->x),1,my_header->gzinfile);
      gzread_int16(&(cur_loc->y),1,my_header->gzinfile);
      cur_index = (int)cur_loc->x + my_header->rows*(int)cur_loc->y; 
      /* cur_index = (int)cur_loc->y + my_header->rows*(int)cur_loc->x; */
      /*   intensity[chip_num*my_header->rows + cur_index] = NAN; */
      intensity[chip_num*rows + cur_index] =  NAN;
      

    }
  } else {
    gzseek(my_header->gzinfile,my_header->n_masks*sizeof(cur_loc),SEEK_CUR);

  }

  if (rm_outliers){
    for (i =0; i < my_header->n_outliers; i++){
      gzread_int16(&(cur_loc->x),1,my_header->gzinfile);
      gzread_int16(&(cur_loc->y),1,my_header->gzinfile);
      cur_index = (int)cur_loc->x + my_header->rows*(int)cur_loc->y; 
      /* intensity[chip_num*my_header->n_cells + cur_index] = NAN; */
      intensity[chip_num*rows + cur_index] =  NAN;
    }
  } else {
    gzseek(my_header->gzinfile,my_header->n_outliers*sizeof(cur_loc),SEEK_CUR);
  }
  
  gzclose(my_header->gzinfile);
  delete_binary_header(my_header);
 
  free(cur_loc);

}


/****************************************************************
 **
 ** static void gzbinary_get_masks_outliers(const char *filename, 
 **                         int *nmasks, short **masks_x, short **masks_y, 
 **                         int *noutliers, short **outliers_x, short **outliers_y
 ** 
 ** This gets the x and y coordinates stored in the masks and outliers sections
 ** of the cel files. (for binary CEL files)
 **
 ****************************************************************/

void gzbinary_get_masks_outliers(const char *filename, int *nmasks, short **masks_x, short **masks_y, int *noutliers, short **outliers_x, short **outliers_y){

  
  int i=0;

  int sizeofrecords;

  outliermask_loc *cur_loc= (outliermask_loc *)calloc(1,sizeof(outliermask_loc));
  binary_header *my_header;

  my_header = gzread_binary_header(filename,1);

  sizeofrecords = 2*sizeof(float) + sizeof(short);

  gzseek(my_header->gzinfile,my_header->n_cells*sizeofrecords,SEEK_CUR);
 

  *nmasks = my_header->n_masks;
  *masks_x = (short *)calloc(my_header->n_masks,sizeof(short));
  *masks_y = (short *)calloc(my_header->n_masks,sizeof(short));

  for (i =0; i < my_header->n_masks; i++){
    gzread_int16(&(cur_loc->x),1,my_header->gzinfile);
    gzread_int16(&(cur_loc->y),1,my_header->gzinfile);
    (*masks_x)[i] = (cur_loc->x);
    (*masks_y)[i] = (cur_loc->y);
  }


  *noutliers = my_header->n_outliers;
  *outliers_x = (short *)calloc(my_header->n_outliers,sizeof(short));
  *outliers_y = (short *)calloc(my_header->n_outliers,sizeof(short));
  


  for (i =0; i < my_header->n_outliers; i++){
    gzread_int16(&(cur_loc->x),1,my_header->gzinfile);
    gzread_int16(&(cur_loc->y),1,my_header->gzinfile);
    (*outliers_x)[i] = (cur_loc->x);
    (*outliers_y)[i] = (cur_loc->y);


  }
      
  gzclose(my_header->gzinfile);
  delete_binary_header(my_header);
 
  free(cur_loc);
  

}

#endif
