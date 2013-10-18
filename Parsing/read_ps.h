#ifndef READ_PS_H
#define READ_PS_H

typedef struct ps_file *ps_handle;


ps_handle read_ps(char *filename);
void dealloc_ps_file(ps_file* my_ps);

wxString ps_get_libsetversion(ps_file *my_ps);
wxString ps_get_libsetname(ps_file *my_pgf);

void sort_probesets(ps_file *my_ps);
bool find_probesets(ps_file *my_ps,int my_id);
int count_unique_probesets(ps_file *my_ps);

#endif
