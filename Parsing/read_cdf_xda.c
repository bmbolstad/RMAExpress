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

/****************************************************************
 **
 ** File: read_cdf_xda.c
 **
 ** Copyright (C) 2005  B. M. Bolstad
 **
 ** A parser designed to read the binary format cdf files
 **
 ** Implemented based on documentation available from Affymetrix
 **
 ** Modification Dates
 ** Feb 4 - Initial version
 ** Feb 5 - A bunch of hacks for SNP chips.
 **
 ****************************************************************/

/** --- includes --- */
//#include <R.h>
//#include <Rdefines.h>

#include <cstdlib>
#include <cstdio>
#include <cctype>

#include "fread_functions.h"
#include "read_cdf_xda.h"

//#define READ_CDF_DEBUG
//#define READ_CDF_DEBUG_SNP
#define READ_CDF_NOSNP




int read_cdf_qcunit(cdf_qc_unit *my_unit,int filelocation,FILE *instream){
  
  int i;


  fseek(instream,filelocation,SEEK_SET);

  fread_uint16(&(my_unit->type),1,instream);
  fread_uint32(&(my_unit->n_probes),1,instream);


  my_unit->qc_probes = (cdf_qc_probe *)calloc(my_unit->n_probes,sizeof(cdf_qc_probe));

  for (i=0; i < (int)my_unit->n_probes; i++){
    fread_uint16(&(my_unit->qc_probes[i].x),1,instream);
    fread_uint16(&(my_unit->qc_probes[i].y),1,instream);
    fread_uchar(&(my_unit->qc_probes[i].probelength),1,instream);
    fread_uchar(&(my_unit->qc_probes[i].pmflag),1,instream);
    fread_uchar(&(my_unit->qc_probes[i].bgprobeflag),1,instream);
    
     }
  return 1;
}


int read_cdf_unit(cdf_unit *my_unit,int filelocation,FILE *instream){

  int i,j;

  fseek(instream,filelocation,SEEK_SET);

  fread_uint16(&(my_unit->unittype),1,instream);
  fread_uchar(&(my_unit->direction),1,instream);
  

  fread_int32(&(my_unit->natoms),1,instream);
  fread_int32(&(my_unit->nblocks),1,instream);
  fread_int32(&(my_unit->ncells),1,instream);
  fread_int32(&(my_unit->unitnumber),1,instream);
  fread_uchar(&(my_unit->ncellperatom),1,instream);

  my_unit->unit_block = (cdf_unit_block *)calloc(my_unit->nblocks,sizeof(cdf_unit_block));

  for (i=0; i < my_unit->nblocks; i++){
    fread_int32(&(my_unit->unit_block[i].natoms),1,instream);
    fread_int32(&(my_unit->unit_block[i].ncells),1,instream);
    fread_uchar(&(my_unit->unit_block[i].ncellperatom),1,instream);
    fread_uchar(&(my_unit->unit_block[i].direction),1,instream);
    fread_int32(&(my_unit->unit_block[i].firstatom),1,instream);
    fread_int32(&(my_unit->unit_block[i].unused),1,instream);
    fread_char(my_unit->unit_block[i].blockname,64,instream); 

    my_unit->unit_block[i].unit_cells = (cdf_unit_cell *)calloc(my_unit->unit_block[i].ncells,sizeof(cdf_unit_cell));

    for (j=0; j < my_unit->unit_block[i].ncells; j++){
      fread_int32(&(my_unit->unit_block[i].unit_cells[j].atomnumber),1,instream);
      fread_uint16(&(my_unit->unit_block[i].unit_cells[j].x),1,instream);
      fread_uint16(&(my_unit->unit_block[i].unit_cells[j].y),1,instream);
      fread_int32(&(my_unit->unit_block[i].unit_cells[j].indexpos),1,instream);
      fread_char(&(my_unit->unit_block[i].unit_cells[j].pbase),1,instream);
      fread_char(&(my_unit->unit_block[i].unit_cells[j].tbase),1,instream);
    }


  }


  return 1;

}


void dealloc_cdf_xda(cdf_xda *my_cdf){

  int i;

  for (i=0; i < my_cdf->header.n_units; i++){
    free(my_cdf->probesetnames[i]);
  }              
  free(my_cdf->probesetnames);

  free(my_cdf->qc_start);
  free(my_cdf->units_start);

  for (i=0; i < my_cdf->header.n_qc_units; i++){
    free(my_cdf->qc_units[i].qc_probes);
  }

  free(my_cdf->qc_units);


  for (i=0; i < my_cdf->header.n_units; i++){
    free(my_cdf->units[i].unit_block);
  }
  free(my_cdf->units);
  free(my_cdf->header.ref_seq);

} 



/*************************************************************
 **
 ** int read_cdf_xda(char *filename)
 **
 ** filename - Name of the prospective binary cel file
 **
 ** Returns 1 if the file was completely successfully parsed
 ** otherwise 0 (and possible prints a message to screen)
 ** 
 **
 **
 **
 *************************************************************/

int read_cdf_xda(char *filename,cdf_xda *my_cdf){

  FILE *infile;

  int i;

  if ((infile = fopen(filename, "rb")) == NULL)
    {
      //error("Unable to open the file %s",filename);
      return 0;
    }

  if (!fread_int32(&my_cdf->header.magicnumber,1,infile)){
    return 0;
  }

  if (!fread_int32(&my_cdf->header.version_number,1,infile)){
    return 0;
  }


  if (my_cdf->header.magicnumber != 67){
    printf("Magic number is not 67. This is probably not a binary cdf file.\n");
    return 0;
  }

  if (my_cdf->header.version_number != 1){
    printf("Don't know if version %d binary cdf files can be handled.\n",my_cdf->header.version_number);
    return 0;
  } 
  if (!fread_int16(&my_cdf->header.cols,1,infile)){
    return 0;
  }
  if (!fread_int16(&my_cdf->header.rows,1,infile)){
    return 0;
  }
 
  if (!fread_int32(&my_cdf->header.n_units,1,infile)){
    return 0;
  }

  if (!fread_int32(&my_cdf->header.n_qc_units,1,infile)){
    return 0;
  }

  
  if (!fread_int32(&my_cdf->header.len_ref_seq,1,infile)){
    return 0;
  }
  
  my_cdf->header.ref_seq = (char *)calloc(my_cdf->header.len_ref_seq,sizeof(char));

  fread_char(my_cdf->header.ref_seq, my_cdf->header.len_ref_seq, infile);
  my_cdf->probesetnames = (char **)calloc(my_cdf->header.n_units,sizeof(char *));


  for (i =0; i < my_cdf->header.n_units;i++){
    my_cdf->probesetnames[i] = (char *)calloc(64,sizeof(char));
    if (!fread_char(my_cdf->probesetnames[i], 64, infile)){
      return 0;
    }
  }



  my_cdf->qc_start = (int *)calloc(my_cdf->header.n_qc_units,sizeof(int));
  my_cdf->units_start = (int *)calloc(my_cdf->header.n_units,sizeof(int));

  if (!fread_int32(my_cdf->qc_start,my_cdf->header.n_qc_units,infile) 
      || !fread_int32(my_cdf->units_start,my_cdf->header.n_units,infile)){
    return 0;
  }
  

  /* We will read in all the QC and Standard Units, rather than  
     random accessing what we need */
  my_cdf->qc_units = (cdf_qc_unit *)calloc(my_cdf->header.n_qc_units,sizeof(cdf_qc_unit));
  
  
  for (i =0; i < my_cdf->header.n_qc_units; i++){
    if (!read_cdf_qcunit(&my_cdf->qc_units[i],my_cdf->qc_start[i],infile)){
      return 0;
    }
  }
    
  my_cdf->units = (cdf_unit *)calloc(my_cdf->header.n_units,sizeof(cdf_unit));


  for (i=0; i < my_cdf->header.n_units; i++){
    if (!read_cdf_unit(&my_cdf->units[i],my_cdf->units_start[i],infile)){
      return 0;
    }
  }
  

#ifdef READ_CDF_DEBUG
   Rprintf("%d %d %d %d  %d\n",my_cdf->header.cols,my_cdf->header.rows,my_cdf->header.n_units,my_cdf->header.n_qc_units,my_cdf->header.len_ref_seq);
  for (i =0; i < my_cdf->header.n_units;i++){
    Rprintf("%s\n",my_cdf->probesetnames[i]);
  }

  for (i =0; i < my_cdf->header.n_qc_units;i++){
    Rprintf("%d\n",my_cdf->qc_start[i]);
  }
  
  for (i =0; i < my_cdf->header.n_qc_units;i++){
    Rprintf("%d\n",my_cdf->units_start[i]);
  }

  Rprintf("%d %d\n",my_cdf->qc_units[0].type,my_cdf->qc_units[0].n_probes);
  
  for (i=0; i < my_cdf->qc_units[0].n_probes; i++){
    Rprintf("%d %d %d %u %d\n",my_cdf->qc_units[0].qc_probes[i].x,my_cdf->qc_units[0].qc_probes[i].y,
	    my_cdf->qc_units[0].qc_probes[i].probelength,
	    my_cdf->qc_units[0].qc_probes[i].pmflag,
	    my_cdf->qc_units[0].qc_probes[i].bgprobeflag);

  }
  

  Rprintf("%u %u %d %d %d %d %u\n",my_cdf->units[0].unittype,my_cdf->units[0].direction,
	  my_cdf->units[0].natoms,
	  my_cdf->units[0].nblocks,
	  my_cdf->units[0].ncells,
	  my_cdf->units[0].unitnumber,
	  my_cdf->units[0].ncellperatom);

    Rprintf("%d %d %u %u %d %d %s\n",my_cdf->units[0].unit_block[0].natoms,my_cdf->units[0].unit_block[0].ncells,
	    my_cdf->units[0].unit_block[0].ncellperatom,
	    my_cdf->units[0].unit_block[0].direction,
	    my_cdf->units[0].unit_block[0].firstatom,
	    my_cdf->units[0].unit_block[0].unused,
	    my_cdf->units[0].unit_block[0].blockname);
    
    for (i=0; i <my_cdf->units[0].unit_block[0].ncells  ; i++){
      Rprintf("%d %u %u %d %c %c\n",
	      my_cdf->units[0].unit_block[0].unit_cells[i].atomnumber,
	      my_cdf->units[0].unit_block[0].unit_cells[i].x,
	      my_cdf->units[0].unit_block[0].unit_cells[i].y,
	      my_cdf->units[0].unit_block[0].unit_cells[i].indexpos,
	      my_cdf->units[0].unit_block[0].unit_cells[i].pbase,
	      my_cdf->units[0].unit_block[0].unit_cells[i].tbase);
    }
#endif
    
  fclose(infile);
  return 1;

  // fseek()
}






static int check_cdf_xda(char *filename){

  FILE *infile;

  int magicnumber,version_number;

  if ((infile = fopen(filename, "rb")) == NULL)
    {
      // error("Unable to open the file %s",filename);
      return 0;
    }

  if (!fread_int32(&magicnumber,1,infile)){
    //error("File corrupt or truncated?");
    return 0;
  }

  if (!fread_int32(&version_number,1,infile)){ 
    //error("File corrupt or truncated?");
    return 0;
  }


  if (magicnumber != 67){
    // error("Magic number is not 67. This is probably not a binary cdf file.\n");
    return 0;
  }

  if (version_number != 1){
    // error("Don't know if version %d binary cdf files can be handled.\n",my_cdf->header.version_number);
    return 0;
  } 

  return 1;

}


int isPM(char pbase,char tbase){
  /*
  if (Pbase.Cmp(Tbase) == 0){
    *isPM = false;
  } else if (((Pbase.Cmp("A")== 0) && (Tbase.Cmp("T") != 0)) || ((Pbase.Cmp("T")
== 0) && (Tbase.Cmp("A") != 0))){
    *isPM = false;
  } else if (((Pbase.Cmp("C")== 0) && (Tbase.Cmp("G") != 0)) || ((Pbase.Cmp("G")
== 0) && (Tbase.Cmp("C") != 0))){
    *isPM = false;
  } else {
    *isPM = true;
  }
  */

  pbase = toupper(pbase);
  tbase = toupper(tbase);

  if (pbase == tbase){
    return 0;
  } else if ((( pbase == 'A') && (tbase != 'T')) || (( pbase == 'T') && (tbase != 'A'))){
    return 0;
  } else if ((( pbase == 'C') && (tbase != 'G')) || (( pbase == 'G') && (tbase != 'C'))){
    return 0;
  }
  return 1;


}


int is_cdf_xda(char *filename){

  return check_cdf_xda(filename);

}
