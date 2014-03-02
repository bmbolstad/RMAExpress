#ifndef READ_MPS_H
#define READ_MPS_H

#include <vector>

typedef struct mps_file *mps_handle;


mps_handle read_mps(char *filename);
void dealloc_mps_file(mps_file* my_mps);

wxString mps_get_libsetversion(mps_file *my_mps);
wxString mps_get_libsetname(mps_file *my_mps);

int mps_get_number_probesets(mps_file *my_mps);

wxString mps_get_probeset_id(mps_file *my_mps,int index);

std::vector<int> mps_get_probeset_list(mps_file *my_mps,int index);
int mps_get_probe_count(mps_file *my_mps,int index);

#endif
