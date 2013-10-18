#ifndef EXPRESSIONGROUP_H
#define EXPRESSIONGROUP_H


class expressionGroup{

 public:
  expressionGroup(long n_probesets, long n_arrays,wxArrayString Names,wxString cdfName, bool hasSE=false);
  ~expressionGroup();
  expressionGroup(const expressionGroup &e);
  void writetofile(const wxString output_fname, const wxString output_path,const int naturalscale);
  void writeSEtofile(const wxString output_fname, const wxString output_path);
  double &operator[](unsigned long i);
  void AddName(const wxString &name); 
  void writetobinaryfile(const wxString output_fname, const wxString output_path,const int naturalscale);
  double &SE(unsigned long i);
  
  int count_arrays();
  int count_probesets();
  wxArrayString GetArrayNames();

 private:
  double *expressionvals;
  double *se_expressionvals;
  int n_probesets;
  int n_arrays;

  wxArrayString ProbesetNames;
  wxArrayString ArrayNames;

  wxString ArrayTypeName;

};





#endif
