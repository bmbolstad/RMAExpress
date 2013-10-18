#ifndef READ_CDF_XDA_H
#define READ_CDF_XDA_H






/************************************************************************
 **
 ** Structures for holding the CDF file information
 **
 ************************************************************************/

typedef struct {
  int magicnumber;
  int version_number;
  short rows,cols;
  int n_units,n_qc_units;
  int len_ref_seq;
  int i;
  char *ref_seq;
} cdf_xda_header;


/*   QC information, repeated for each QC unit:

Type - unsigned short
Number of probes - integer

Probe information, repeated for each probe in the QC unit:
X coordinate - unsigned short
Y coordinate - unsigned short
Probe length - unsigned char
Perfect match flag - unsigned char
Background probe flag - unsigned char
*/

typedef struct{
  unsigned short x;
  unsigned short y;
  unsigned char probelength;
  unsigned char pmflag;
  unsigned char bgprobeflag;

} cdf_qc_probe;

typedef struct{
  unsigned short type;
  unsigned int n_probes;

  cdf_qc_probe *qc_probes;
  
} cdf_qc_unit;


/* Unit information, repeated for each unit:

UnitType - unsigned short (1 - expression, 2 - genotyping, 3 - CustomSeq, 3 - tag)
Direction - unsigned char
Number of atoms - integer
Number of blocks - integer (always 1 for expression units)
Number of cells - integer
Unit number (probe set number) - integer
Number of cells per atom - unsigned char

Block information, repeated for each block in the unit:

Number of atoms - integer
Number of cells - integer
Number of cells per atom - unsigned char
Direction - unsigned char
The position of the first atom - integer
<unused integer value> - integer
The block name - char[64]

Cell information, repeated for each cell in the block:

Atom number - integer
X coordinate - unsigned short
Y coordinate - unsigned short
Index position (relative to sequence for resequencing units, for expression and mapping units this value is just the atom number) - integer
Base of probe at substitution position - char
Base of target at interrogation position - char

*/


typedef struct{
  int atomnumber;
  unsigned short x;
  unsigned short y;
  int indexpos;
  char pbase;
  char tbase;
} cdf_unit_cell;


typedef struct{
  int natoms;
  int ncells;
  unsigned char ncellperatom;
  unsigned char direction;
  int firstatom;
  int unused;         /* in the docs this is called "unused" but by the looks of it it is actually the lastatom */
  char blockname[64];

  cdf_unit_cell  *unit_cells;

} cdf_unit_block;


typedef struct{
  unsigned short unittype;
  unsigned char direction;
  int natoms;
  int nblocks;
  int ncells;
  int unitnumber;
  unsigned char ncellperatom;

  cdf_unit_block *unit_block;
  
} cdf_unit;







typedef struct {
  
  cdf_xda_header header;  /* Header information */
  char **probesetnames;   /* Names of probesets */
  
  int *qc_start;          /* These are used for random access */
  int *units_start;
  
  cdf_qc_unit *qc_units;
  cdf_unit *units;


} cdf_xda;



int is_cdf_xda(char *filename);
int read_cdf_xda(char *filename,cdf_xda *my_cdf);
void dealloc_cdf_xda(cdf_xda *my_cdf);
int isPM(char pbase,char tbase);

#endif
