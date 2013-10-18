#ifndef READ_CLF_H
#define READ_CLF_H


typedef struct clf_file *clf_handle;

void clf_get_probe_id(clf_handle clf, int *probe_id, int x, int y);
void clf_get_x_y(clf_handle clf, int probe_id, int *x, int *y);

clf_handle read_clf(char *filename);
void dealloc_clf_file(clf_handle my_clf);


wxArrayString clf_get_arraytypes(clf_file *my_clf);
wxString clf_get_libsetversion(clf_file *my_clf);
wxString clf_get_libsetname(clf_file *my_clf);

int clf_get_rows(clf_file *my_clf);
int clf_get_cols(clf_file *my_clf);


#endif
