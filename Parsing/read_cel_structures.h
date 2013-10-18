#ifndef READ_CEL_STRUCTURES_H
#define READ_CEL_STRUCTURES_H



/****************************************************************
 **
 ** A structure for holding full header information
 **
 **
 **
 ***************************************************************/

typedef struct{
  char *cdfName;
  int cols;
  int rows;
  int GridCornerULx,GridCornerULy;      /* XY coordinates of the upper left grid corner in pixel coordinates.*/
  int GridCornerURx,GridCornerURy;      /* XY coordinates of the upper right grid corner in pixel coordinates.*/
  int GridCornerLRx,GridCornerLRy;      /* XY coordinates of the lower right grid corner in pixel coordinates.*/
  int GridCornerLLx,GridCornerLLy;      /* XY coordinates of the lower left grid corner in pixel coordinates.*/ 
  char *DatHeader;
  char *Algorithm;
  char *AlgorithmParameters;
} detailed_header_info;


/****************************************************************
 **
 ** Constant values for parsing errors
 **
 ** Format
 ** General Errors   1XX
 ** Text CEL files   1XXX
 ** Binary/XDA CEL files 2XXX
 ** Calvin/Command Console CEL files 3XXX
 **
 **
 **
 ****************************************************************/

const int CEL_NON_MATCHING_DIMENSIONS = 101;
const int CEL_NON_MATCHING_CDFNAMES = 102;

const int TEXT_HEADER_TRUNCATED=1001;
const int TEXT_HEADER_CDF_NAME_MISSING=1002;
const int TEXT_INTENSITY_TRUNCATED=1003;
const int TEXT_INTENSITY_CORRUPTED=1004;
const int TEXT_MISSING_LINE=1005;
const int TEXT_DID_NOT_FIND_STARTS_WITH = 1006;
const int TEXT_DID_NOT_FIND_SECTION = 1007;
const int TEXT_DID_NOT_FIND_HEADER_SECTION= 1008;
const int TEXT_DID_NOT_FIND_COL_DIMENSION = 1009;
const int TEXT_DID_NOT_FIND_ROW_DIMENSION = 1010;
const int TEXT_DID_NOT_FIND_DAT_HEADER = 1011;
const int TEXT_DID_NOT_FIND_INTENSITY_SECTION = 1012;
const int TEXT_DID_NOT_FIND_CELLHEADER = 1013;
const int TEXT_DID_NOT_MISC_HEADER_FIELD= 1014;


const int BINARY_HEADER_TRUNCATED=2001;
const int BINARY_HEADER_CDF_NAME_MISSING=2002;
const int BINARY_INTENSITY_TRUNCATED=2003;
const int BINARY_INTENSITY_CORRUPTED=2004;

const int CALVIN_HEADER_TRUNCATED=3001;
const int CALVIN_HEADER_CDF_NAME_MISSING=3002;
const int CALVIN_HEADER_FILE_HEADER_TRUNCATED = 3003;
const int CALVIN_HEADER_DATA_HEADER_TRUNCATED = 3004;
const int CALVIN_HEADER_DATA_GROUP_TRUNCATED = 3005;
const int CALVIN_HEADER_DATA_SET_TRUNCATED = 3006;
const int CALVIN_DID_NOT_FIND_COL_DIMENSION = 3009;
const int CALVIN_DID_NOT_FIND_ROW_DIMENSION = 3010;

#endif
