/******************************************************************
 **
 ** file: PGF_CLF_to_RME.cpp
 ** 
 ** Aim: implement parsing of PGF/CLF format files and changing them
 **      into something that can be read into RMAExpress
 **
 ** Copyright (C) 2008    B. M. Bolstad
 **
 **
 ** History
 ** Feb 8, 2008 - add code for selecting a specific set of probesets using
 **               a PS file
 **
 ** Mar 17, 2008 - introduce code for selecting meta probesets (ie gene-level)
 **                using MPS files
 **
 ** Mar 19, 2008 - change how output CDFRME is named for cases where ps and mps files are used
 **                also add pgfclf to name for standard procedure
 **                now probeset types "main", "normgene->intron" and "normgene->exon:main"
 **                are included in the standard output
 **
 ** May 18, 2008 - Fix number of probesets placed in CDFRME file using PGF/CLF conversion no PS or MPS file
 ** May 20, 2009 - fix MPS conversion issues
 ** Oct 22, 2013 - remove strict checking of clf/pgf version number
 ** 
 ******************************************************************/


#include <wx/string.h>
#include <wx/dynarray.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/datstrm.h>

#ifdef RMA_GUI_APP
#include <wx/progdlg.h>
#endif

#include <map>
#include <vector>
#include <utility>

using namespace std;


#include "Parsing/read_clf.h"
#include "Parsing/read_pgf.h"
#include "Parsing/read_ps.h"
#include "Parsing/read_mps.h"

#include "version_number.h"




static int xy2i(int xloc, int yloc, int sidelength){

  return yloc*sidelength + xloc;

}



void Convert_PGF_CLF_to_RME(const wxString &pgf_fname,
			    const wxString &clf_fname,
			    const wxString &output_path){
			    
  pgf_file *pgf;
  clf_file *clf;

  wxString pgf_libset;
  wxString clf_libset;
  wxString pgf_libver;
  wxString clf_libver;
  
  wxArrayInt CurrentIDs;

  wxArrayString ProbesetTypes;

  wxArrayString ArrayTypes;
 
  int numberOfTypes;
  probeset_type_list *pgf_probe_types;
  
  int ProbeSetCount =0;

  const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(pgf_fname.c_str());
  const char *pgf_cname = (const char*) tmp_buf;
  const wxWX2MBbuf tmp_buf2 = wxConvCurrent->cWX2MB(clf_fname.c_str());
  const char *clf_cname = (const char*) tmp_buf2;

  int x,y;

  size_t i,j;

  //wxPrintf(_T("%s %s %s\n"),pgf_fname.c_str(), clf_fname.c_str(), (char *)pgf_cname);
    
#ifdef RMA_GUI_APP
  wxProgressDialog PGF_CLF_Progress(_T("Building CDFRME"),_T("Reading CLF"),4,NULL,wxPD_AUTO_HIDE | wxPD_SMOOTH);
#endif
  clf = read_clf((char *)clf_cname);
#ifdef RMA_GUI_APP
  PGF_CLF_Progress.Update(1,_T("Reading PGF"));
#endif
  pgf = read_pgf((char *)pgf_cname);

  
  pgf_libver = pgf_get_libsetversion(pgf);
  clf_libver = clf_get_libsetversion(clf);
  
  pgf_libset = pgf_get_libsetname(pgf);
  clf_libset = clf_get_libsetname(clf);
    
  
  if (pgf_libset.Cmp(clf_libset) != 0){   // || pgf_libver.Cmp(clf_libver) !=0){
    dealloc_pgf_file(pgf);
    dealloc_clf_file(clf);
    
    wxString error = wxString(_T("PGF file and CLF file do not match based upon lib_set_name and lib_set_version.\n"),wxConvUTF8)
      +  _T("PGF file: ") + pgf_libset + _T(" ") + pgf_libver + _T("\n")
      + _T("CLF file: ") + clf_libset + _T(" ") + clf_libver + _T("\n");

    throw error;

  }

  pgf_probe_types = pgf_count_probeset_types(pgf, &numberOfTypes);

  for (i = 0; i < numberOfTypes; i++){
    if (wxString(pgf_probe_types[i].type,wxConvUTF8).Cmp(wxString(_T("main"))) == 0){
      break;
    }
  }
#ifdef RMA_GUI_APP
  PGF_CLF_Progress.Update(3,_T("Writing CDFRME"));
#endif
    
  ArrayTypes = pgf_get_arraytypes(pgf);

  ProbesetTypes.Alloc(10);
  ProbesetTypes.Insert(wxString("main",wxConvUTF8),0);
  ProbesetTypes.Insert(wxString("normgene->intron",wxConvUTF8),1);
  ProbesetTypes.Insert(wxString("normgene->exon:main",wxConvUTF8),2);
  ProbesetTypes.SetCount(3); 
  for (i =0;  i < numberOfTypes; i++){
    if (wxString(pgf_probe_types[i].type,wxConvUTF8).Cmp(wxString(_T("main"))) == 0){
      ProbeSetCount+=pgf_probe_types[i].count;
    } else if (wxString(pgf_probe_types[i].type,wxConvUTF8).Cmp(wxString(_T("normgene->intron"))) == 0){
      ProbeSetCount+=pgf_probe_types[i].count;
    } else if (wxString(pgf_probe_types[i].type,wxConvUTF8).Cmp(wxString(_T("normgene->exon:main"))) == 0){
      ProbeSetCount+=pgf_probe_types[i].count;
    } 
  }
  free(pgf_probe_types);
  //  wxPrintf(_T("%s\n\n"),ProbesetTypes[0].c_str());
  wxFileName currentPath(output_path, pgf_libset + _T(".") + pgf_libver + _T("_pgfclf") + _T(".") _T("CDFRME"),wxPATH_NATIVE);
  wxString currentName =currentPath.GetFullPath();
  
  wxFileOutputStream output(currentName);
  wxDataOutputStream store(output);
  store.WriteString(wxString(_T("RMECDF")));
  store.Write32(3);
  store.Write32(ArrayTypes.GetCount());
  for (j = 0; j < ArrayTypes.GetCount(); j++){
    store.WriteString(ArrayTypes[j]);
  }
  store.Write32(4);   // Number of Strings 
  store.WriteString(wxString(_T("Created using RMADataConv ")) + version_number);

  /* work out the current system date */

  wxDateTime now = wxDateTime::Now();
  store.WriteString(wxString(_T("Creation Date: ")) + now.FormatDate() + wxString(_T(" ")) + now.FormatTime());
  store.WriteString(wxString(_T("PGF file: ")) + pgf_fname);
  store.WriteString(wxString(_T("CLF file: ")) + clf_fname);
  

  store.Write32(clf_get_rows(clf));
  store.Write32(clf_get_cols(clf));
  store.Write32(ProbeSetCount);
 
  if (pgf_set_cur_to_first(pgf, ProbesetTypes)){
    pgf_get_cur_PM_probe_ids(pgf,CurrentIDs); 
    store.WriteString(wxString::Format(wxT("%d"), (int)pgf_get_cur_probeset_id(pgf)));
    store.Write32(CurrentIDs.GetCount());

    for (i = 0; i < CurrentIDs.GetCount(); i++){
      clf_get_x_y(clf, CurrentIDs[i], &x, &y);
      store.Write32(xy2i(x,y,clf_get_cols(clf)));
    } 
    store.Write32(0);
  }
  
  while (pgf_set_cur_to_next(pgf, ProbesetTypes)){
    pgf_get_cur_PM_probe_ids(pgf,CurrentIDs); 
    store.WriteString(wxString::Format(wxT("%d"), (int)pgf_get_cur_probeset_id(pgf)));
    store.Write32(CurrentIDs.GetCount());
    // wxPrintf(_T("%d %d\n"),CurrentIDs.GetCount(),CurrentIDs[0]);
    for (i = 0; i < CurrentIDs.GetCount(); i++){
      clf_get_x_y(clf, CurrentIDs[i], &x, &y);
      store.Write32(xy2i(x,y,clf_get_cols(clf)));
    }
    store.Write32(0);
  }

  dealloc_pgf_file(pgf);
  dealloc_clf_file(clf);
  
}


/* This is testing code */

/*
  int main(){
  
  wxString pgfname = wxString("/tmp/HuEx-1_0-st-v2.r2.pgf",wxConvUTF8);// HuGene-1_0-st-v1.r3.pgf",wxConvUTF8);
  wxString clfname = wxString("/tmp/HuEx-1_0-st-v2.r2.clf",wxConvUTF8);//HuGene-1_0-st-v1.r3.clf",wxConvUTF8);

  
  try{
    Convert_PGF_CLF_to_RME(pgfname, clfname);
    }
    catch (wxString Problem){
    wxPrintf(Problem);
    }

    }
*/



void Convert_PGF_CLF_to_RME_with_PS(const wxString &pgf_fname,
				    const wxString &clf_fname,
				    const wxString &ps_fname,
				    const wxString &output_path){
			    
  pgf_file *pgf;
  clf_file *clf;
  ps_file *ps;

  wxString pgf_libset;
  wxString clf_libset;
  wxString pgf_libver;
  wxString clf_libver;
  wxString ps_libset;
  wxString ps_libver;
  
  wxArrayInt CurrentIDs;

  wxArrayString ProbesetTypes;

  wxArrayString ArrayTypes;
 
  int numberOfTypes;
  probeset_type_list *pgf_probe_types;
  

  const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(pgf_fname.c_str());
  const char *pgf_cname = (const char*) tmp_buf;
  const wxWX2MBbuf tmp_buf2 = wxConvCurrent->cWX2MB(clf_fname.c_str());
  const char *clf_cname = (const char*) tmp_buf2;
  const wxWX2MBbuf tmp_buf3 = wxConvCurrent->cWX2MB(ps_fname.c_str());
  const char *ps_cname = (const char*) tmp_buf3;
  

  int x,y;

  size_t i,j;

  //wxPrintf(_T("%s %s %s\n"),pgf_fname.c_str(), clf_fname.c_str(), (char *)pgf_cname);
#ifdef RMA_GUI_APP
  wxProgressDialog PGF_CLF_Progress(_T("Building CDFRME"),_T("Reading CLF"),4,NULL,wxPD_AUTO_HIDE);
#endif 
  clf = read_clf((char *)clf_cname);
#ifdef RMA_GUI_APP
  PGF_CLF_Progress.Update(1,_T("Reading PGF"));
#endif
  pgf = read_pgf((char *)pgf_cname);
#ifdef RMA_GUI_APP
  PGF_CLF_Progress.Update(2,_T("Reading PS"));
#endif
  ps = read_ps((char *)ps_cname);
  
  pgf_libver = pgf_get_libsetversion(pgf);
  clf_libver = clf_get_libsetversion(clf);
  ps_libver = ps_get_libsetversion(ps);
  
  pgf_libset = pgf_get_libsetname(pgf);
  clf_libset = clf_get_libsetname(clf);
  ps_libset = ps_get_libsetname(ps);
  
  if (pgf_libset.Cmp(clf_libset) != 0 || pgf_libver.Cmp(clf_libver) !=0 || pgf_libset.Cmp(ps_libset) != 0 || pgf_libver.Cmp(ps_libver) !=0 ){
    dealloc_pgf_file(pgf);
    dealloc_clf_file(clf);
    dealloc_ps_file(ps);
    
    wxString error = wxString(_T("PGF file, CLF file and PS file do not match based upon lib_set_name and lib_set_version.\n"),wxConvUTF8)
      +  _T("PGF file: ") + pgf_libset + _T(" ") + pgf_libver + _T("\n")
      + _T("CLF file: ") + clf_libset + _T(" ") + clf_libver + _T("\n")
      + _T("PS file: ") + ps_libset + _T(" ") + ps_libver + _T("\n");

    throw error;

  }

  pgf_probe_types = pgf_count_probeset_types(pgf, &numberOfTypes);

  // for (i = 0; i < numberOfTypes; i++){
  //  if (wxString(pgf_probe_types[i].type,wxConvUTF8).Cmp(wxString(_T("main"))) == 0){
  //    break;
  //  }
  //}

  sort_probesets(ps);


  ArrayTypes = pgf_get_arraytypes(pgf);

  ProbesetTypes.Alloc(numberOfTypes);
  //  ProbesetTypes.Insert(wxString("main",wxConvUTF8),0);
  //ProbesetTypes.SetCount(1);
  for (i = 0; i < numberOfTypes; i++){
    ProbesetTypes.Insert(wxString(pgf_probe_types[i].type,wxConvUTF8),i);
  }

  for (i =0;  i < numberOfTypes; i++){
    free(pgf_probe_types[i].type);
  }
  free(pgf_probe_types);
#ifdef RMA_GUI_APP
  PGF_CLF_Progress.Update(3,_T("Writing CDFRME"));
#endif


  //  wxPrintf(_T("%s\n\n"),ProbesetTypes[0].c_str());
  wxFileName currentPath(output_path, pgf_libset + _T(".") + pgf_libver + _T("_ps_") + wxFileName(ps_fname).GetName() + _T(".CDFRME"),wxPATH_NATIVE);
  wxString currentName =currentPath.GetFullPath();
  
  wxFileOutputStream output(currentName);
  wxDataOutputStream store(output);
  store.WriteString(wxString(_T("RMECDF")));
  store.Write32(3);
  store.Write32(ArrayTypes.GetCount());
  for (j = 0; j < ArrayTypes.GetCount(); j++){
    store.WriteString(ArrayTypes[j]);
  }
  store.Write32(5);   // Number of Strings 
  store.WriteString(wxString(_T("Created using RMADataConv ")) + version_number);

  /* work out the current system date */

  wxDateTime now = wxDateTime::Now();
  store.WriteString(wxString(_T("Creation Date: ")) + now.FormatDate() + wxString(_T(" ")) + now.FormatTime());
  store.WriteString(wxString(_T("PGF file: ")) + pgf_fname);
  store.WriteString(wxString(_T("CLF file: ")) + clf_fname);
  store.WriteString(wxString(_T("PS file: ")) + ps_fname);

  store.Write32(clf_get_rows(clf));
  store.Write32(clf_get_cols(clf));
  store.Write32(count_unique_probesets(ps));


  //bool find_probesets(ps_file *my_ps,int my_id);
 
  if (pgf_set_cur_to_first(pgf, ProbesetTypes)){

    if (find_probesets(ps,pgf_get_cur_probeset_id(pgf))){
      pgf_get_cur_PM_probe_ids(pgf,CurrentIDs); 
      store.WriteString(wxString::Format(wxT("%d"), (int)pgf_get_cur_probeset_id(pgf)));
      store.Write32(CurrentIDs.GetCount());
      
      for (i = 0; i < CurrentIDs.GetCount(); i++){
	clf_get_x_y(clf, CurrentIDs[i], &x, &y);
	store.Write32(xy2i(x,y,clf_get_cols(clf)));
      } 
      store.Write32(0);
    }
  }
  
  while (pgf_set_cur_to_next(pgf, ProbesetTypes)){
    if (find_probesets(ps,pgf_get_cur_probeset_id(pgf))){
      pgf_get_cur_PM_probe_ids(pgf,CurrentIDs); 
      store.WriteString(wxString::Format(wxT("%d"), (int)pgf_get_cur_probeset_id(pgf)));
      store.Write32(CurrentIDs.GetCount());
      // wxPrintf(_T("%d %d\n"),CurrentIDs.GetCount(),CurrentIDs[0]);
      for (i = 0; i < CurrentIDs.GetCount(); i++){
	clf_get_x_y(clf, CurrentIDs[i], &x, &y);
	store.Write32(xy2i(x,y,clf_get_cols(clf)));
      }
      store.Write32(0);
    }
  }

  dealloc_pgf_file(pgf);
  dealloc_clf_file(clf);
  dealloc_ps_file(ps);
}






void Convert_PGF_CLF_to_RME_with_MPS(const wxString &pgf_fname,
				    const wxString &clf_fname,
				    const wxString &mps_fname,
				    const wxString &output_path){
			    
  pgf_file *pgf;
  clf_file *clf;
  mps_file *mps;

  wxString pgf_libset;
  wxString clf_libset;
  wxString pgf_libver;
  wxString clf_libver;
  wxString mps_libset;
  wxString mps_libver;
  
  wxArrayInt CurrentIDs;

  wxArrayString ProbesetTypes;

  wxArrayString ArrayTypes;
 
  int numberOfTypes;
  probeset_type_list *pgf_probe_types;
  

  const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(pgf_fname.c_str());
  const char *pgf_cname = (const char*) tmp_buf;
  const wxWX2MBbuf tmp_buf2 = wxConvCurrent->cWX2MB(clf_fname.c_str());
  const char *clf_cname = (const char*) tmp_buf2;
  const wxWX2MBbuf tmp_buf3 = wxConvCurrent->cWX2MB(mps_fname.c_str());
  const char *mps_cname = (const char*) tmp_buf3;
  

  int x,y;

  size_t i,j,k;

  //wxPrintf(_T("%s %s %s\n"),pgf_fname.c_str(), clf_fname.c_str(), (char *)pgf_cname);
#ifdef RMA_GUI_APP
  wxProgressDialog PGF_CLF_Progress(_T("Building CDFRME"),_T("Reading CLF"),4,NULL,wxPD_AUTO_HIDE);
#endif    
  clf = read_clf((char *)clf_cname);
#ifdef RMA_GUI_APP
  PGF_CLF_Progress.Update(1,_T("Reading PGF"));
#endif
  pgf = read_pgf((char *)pgf_cname);
#ifdef RMA_GUI_APP
  PGF_CLF_Progress.Update(2,_T("Reading MPS"));
#endif
  mps = read_mps((char *)mps_cname);
  
  pgf_libver = pgf_get_libsetversion(pgf);
  clf_libver = clf_get_libsetversion(clf);
  mps_libver = mps_get_libsetversion(mps);
  
  pgf_libset = pgf_get_libsetname(pgf);
  clf_libset = clf_get_libsetname(clf);
  mps_libset = mps_get_libsetname(mps);
  
  if (pgf_libset.Cmp(clf_libset) != 0 || pgf_libver.Cmp(clf_libver) !=0 || pgf_libset.Cmp(mps_libset) != 0 || pgf_libver.Cmp(mps_libver) !=0 ){
    dealloc_pgf_file(pgf);
    dealloc_clf_file(clf);
    dealloc_mps_file(mps);
    
    wxString error = wxString(_T("PGF file, CLF file and MPS file do not match based upon lib_set_name and lib_set_version.\n"),wxConvUTF8)
      +  _T("PGF file: ") + pgf_libset + _T(" ") + pgf_libver + _T("\n")
      + _T("CLF file: ") + clf_libset + _T(" ") + clf_libver + _T("\n")
      + _T("MPS file: ") + mps_libset + _T(" ") + mps_libver + _T("\n");

    throw error;

  }

  pgf_probe_types = pgf_count_probeset_types(pgf, &numberOfTypes);

  // for (i = 0; i < numberOfTypes; i++){
  //  if (wxString(pgf_probe_types[i].type,wxConvUTF8).Cmp(wxString(_T("main"))) == 0){
  //    break;
  //  }
  //}

  //sort_probesets(ps);


  ArrayTypes = pgf_get_arraytypes(pgf);

  ProbesetTypes.Alloc(numberOfTypes);
  ProbesetTypes.Insert(wxString("main",wxConvUTF8),0);
  ProbesetTypes.Insert(wxString("normgene->intron",wxConvUTF8),1);
  ProbesetTypes.Insert(wxString("normgene->exon:main",wxConvUTF8),2);
  ProbesetTypes.Insert(wxString("main:normgene->exon",wxConvUTF8),3);
  ProbesetTypes.SetCount(4);
  //for (i = 0; i < numberOfTypes; i++){
  //  ProbesetTypes.Insert(wxString(pgf_probe_types[i].type,wxConvUTF8),i);
  //} 
  for (i =0;  i < numberOfTypes; i++){
    free(pgf_probe_types[i].type);
  }
  free(pgf_probe_types);

  std::map <int, wxArrayInt> pgf_map;
    
  if (pgf_set_cur_to_first(pgf, ProbesetTypes)){
    pgf_get_cur_PM_probe_ids(pgf,CurrentIDs); 
    pgf_map.insert(pair<int, wxArrayInt> ((int)pgf_get_cur_probeset_id(pgf), CurrentIDs));
  }
  while (pgf_set_cur_to_next(pgf, ProbesetTypes)){
    pgf_get_cur_PM_probe_ids(pgf,CurrentIDs); 
    pgf_map.insert(pair<int, wxArrayInt> ((int)pgf_get_cur_probeset_id(pgf), CurrentIDs));
  }
 

#ifdef RMA_GUI_APP
  PGF_CLF_Progress.Update(3,_T("Writing CDFRME"));
#endif


  //wxPrintf(_T("%s\n\n"),ProbesetTypes[0].c_str());
  wxFileName currentPath(output_path, pgf_libset + _T(".") + pgf_libver + _T("_mps_") + wxFileName(mps_fname).GetName() + _T(".CDFRME"),wxPATH_NATIVE);
  wxString currentName =currentPath.GetFullPath();
  
  wxFileOutputStream output(currentName);
  wxDataOutputStream store(output);
  store.WriteString(wxString(_T("RMECDF")));
  store.Write32(3);
  store.Write32(ArrayTypes.GetCount());
  for (j = 0; j < ArrayTypes.GetCount(); j++){
    store.WriteString(ArrayTypes[j]);
  }
  store.Write32(5);   // Number of Strings 
  store.WriteString(wxString(_T("Created using RMADataConv ")) + version_number);

  /* work out the current system date */

  wxDateTime now = wxDateTime::Now();
  store.WriteString(wxString(_T("Creation Date: ")) + now.FormatDate() + wxString(_T(" ")) + now.FormatTime());
  store.WriteString(wxString(_T("PGF file: ")) + pgf_fname);
  store.WriteString(wxString(_T("CLF file: ")) + clf_fname);
  store.WriteString(wxString(_T("MPS file: ")) + mps_fname);

  store.Write32(clf_get_rows(clf));
  store.Write32(clf_get_cols(clf));
  store.Write32(mps_get_number_probesets(mps));

  
  map<int,wxArrayInt>::iterator it;

  vector<int> currentProbesets;

  int curProbeset_id;
  int curNumberProbes;
  for (k = 0; k < mps_get_number_probesets(mps); k++){
    curProbeset_id = mps_get_probeset_id(mps,k);
    curNumberProbes = mps_get_probe_count(mps,k);
    currentProbesets = mps_get_probeset_list(mps,k);
    // wxPrintf(_T("%d %d %d\n"),curProbeset_id,curNumberProbes,currentProbesets.size());
    store.WriteString(wxString::Format(wxT("%d"), (int)curProbeset_id));

    // check that curNumberofProbes can actually be found
    int curCount=0;
    for (j = 0; j < currentProbesets.size(); j++){
      it=pgf_map.find(currentProbesets[j]);
      curCount+=it->second.GetCount();
    }
    if (curCount == curNumberProbes){
      store.Write32(curNumberProbes);
    } else {
      wxPrintf(_T("Warning MPS %d should have %d probes, but found only %d\n"),(int)curProbeset_id,curNumberProbes,curCount);
      store.Write32(curCount);
    }
    for (j = 0; j < currentProbesets.size(); j++){
      it=pgf_map.find(currentProbesets[j]);
      //wxPrintf(_T("%d %d Size: %d\n"),curProbeset_id,currentProbesets[j],it->second.GetCount());
      for (i = 0; i < it->second.GetCount(); i++){
	clf_get_x_y(clf, it->second.Item(i), &x, &y);
	store.Write32(xy2i(x,y,clf_get_cols(clf)));
      } 
    }
    store.Write32(0); /* No MM type things */
  }

  dealloc_pgf_file(pgf);
  dealloc_clf_file(clf);
  dealloc_mps_file(mps);

}








