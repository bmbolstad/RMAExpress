#ifndef READ_PGF_H
#define READ_PGF_H



typedef struct pgf_file *pgf_handle;


/*******************************************************************
 *******************************************************************
 **
 ** Structure for counting probeset types
 **
 *******************************************************************
 ******************************************************************/

typedef struct{
  char *type;
  int count;
} probeset_type_list;





/*******************************************************************
 *******************************************************************
 **
 **
 ** Code for actually dealing with PGF files
 **
 **
 *******************************************************************
 ******************************************************************/



pgf_handle read_pgf(char *filename);
void dealloc_pgf_file(pgf_handle my_pgf);

probeset_type_list *pgf_count_probeset_types(pgf_handle my_pgf, int *number);
void dealloc_probeset_type_list(probeset_type_list *my_type_list, int length);

wxArrayString pgf_get_arraytypes(pgf_file *my_pgf);
wxString pgf_get_libsetversion(pgf_file *my_pgf);
wxString pgf_get_libsetname(pgf_file *my_pgf);

bool pgf_set_cur_to_first(pgf_file *my_pgf, wxArrayString &probeset_types);
bool pgf_set_cur_to_next(pgf_file *my_pgf, wxArrayString &probeset_types);
int pgf_get_cur_probeset_id(pgf_file *my_pgf);
void pgf_get_cur_PM_probe_ids(pgf_file *my_pgf, wxArrayInt &result);

#endif
