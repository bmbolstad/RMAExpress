#ifndef READ_CELFILE_TEXT_H
#define READ_CELFILE_TEXT_H

#include "read_cel_structures.h"

int isTextCelFile(const char *filename);
char *get_header_info(const char *filename, int *dim1, int *dim2, int *err_code);
void get_detailed_header_info(const char *filename, detailed_header_info *header_info);
int read_cel_file_intensities(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows);
int check_cel_file(const char *filename, const char *ref_cdfName, int ref_dim_1, int ref_dim_2);
#if defined(INCLUDE_ALL_AFFYIO)
int read_cel_file_stddev(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows);
int read_cel_file_npixels(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows);
void get_masks_outliers(const char *filename, int *nmasks, short **masks_x, short **masks_y, int *noutliers, short **outliers_x, short **outliers_y);
void apply_masks(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows, int rm_mask, int rm_outliers);
#endif

#if defined(HAVE_ZLIB)
int isgzTextCelFile(const char *filename);
char *gz_get_header_info(const char *filename, int *dim1, int *dim2);
void gz_get_detailed_header_info(const char *filename, detailed_header_info *header_info);
int read_gzcel_file_intensities(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows);
int check_gzcel_file(const char *filename, const char *ref_cdfName, int ref_dim_1, int ref_dim_2);
int read_gzcel_file_stddev(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows);
int read_gzcel_file_npixels(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows);
void gz_get_masks_outliers(const char *filename, int *nmasks, short **masks_x, short **masks_y, int *noutliers, short **outliers_x, short **outliers_y
);
void gz_apply_masks(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows, int rm_mask, int rm_outliers);
#endif





#endif
