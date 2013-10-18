#ifndef READ_CELFILE_XDA_H
#define READ_CELFILE_XDA_H

#include "read_cel_structures.h"

#if defined(HAVE_ZLIB)
#include <zlib.h>
#endif

int isBinaryCelFile(const char *filename);
char *binary_get_header_info(const char *filename, int *dim1, int *dim2, int *err_code);
void binary_get_detailed_header_info(const char *filename, detailed_header_info *header_info, int *err_code);
int read_binarycel_file_intensities(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows);
int check_binary_cel_file(const char *filename, const char *ref_cdfName, int ref_dim_1, int ref_dim_2);
#if defined(INCLUDE_ALL_AFFYIO)
int read_binarycel_file_stddev(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows);
int read_binarycel_file_npixels(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows);
void binary_get_masks_outliers(const char *filename, int *nmasks, short **masks_x, short **masks_y, int *noutliers, short **outliers_x, short **outliers_y);
void binary_apply_masks(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows, int rm_mask, int rm_outliers);
#endif
#if defined(HAVE_ZLIB)
int isgzBinaryCelFile(const char *filename);
char *gzbinary_get_header_info(const char *filename, int *dim1, int *dim2);
void gzbinary_get_detailed_header_info(const char *filename, detailed_header_info *header_info);
int gzread_binarycel_file_intensities(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows);
int check_gzbinary_cel_file(const char *filename, const char *ref_cdfName, int ref_dim_1, int ref_dim_2);
int gzread_binarycel_file_stddev(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows);
int gzread_binarycel_file_npixels(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows);
void gz_binary_get_masks_outliers(const char *filename, int *nmasks, short **masks_x, short **masks_y, int *noutliers, short **outliers_x, short **outliers_y);
void gzbinary_apply_masks(const char *filename, double *intensity, int chip_num, int rows, int cols,int chip_dim_rows, int rm_mask, int rm_outliers);
#endif





#endif
