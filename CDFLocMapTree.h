#ifndef CDFLocMapTree_H
#define CDFLocMapTree_H

#include <wx/wx.h>

class LocMapItem
{
  friend class CDFLocMapNode;
  
 public:
  LocMapItem();
  LocMapItem(wxString name, int n_pm_probes,int n_mm_probes,  int *PM, int *MM);
  ~LocMapItem();
  wxString GetName() const;
  int GetSize();
  int GetMMSize();
  int GetPMSize();
  int *GetPMLocs();
  int *GetMMLocs();
 private:
  wxString ProbesetName;
  
  int n_pm_probes;
  int n_mm_probes;

  int *MMLoc;
  int *PMLoc;


};


class CDFLocMapNode
{
  friend class CDFLocMapTree;

 public:
  CDFLocMapNode();
  CDFLocMapNode(const LocMapItem *);
  ~CDFLocMapNode();
  long getSkew();
 private:
  LocMapItem *data;
  long skew;
  CDFLocMapNode *Left;
  CDFLocMapNode *Right;
};

class CDFLocMapTree
{
 public:
  CDFLocMapTree();
  ~CDFLocMapTree();
  void Insert(const LocMapItem *);
  LocMapItem *Find(const wxString &x);
  bool isEmpty();
 private:
  long InsertNode(CDFLocMapNode **ptr, const LocMapItem *value);
  LocMapItem *FindNode(CDFLocMapNode **ptr, const wxString &x);
  long avlLeftGrew(CDFLocMapNode **ptr);
  long avlRightGrew(CDFLocMapNode **ptr);
  void  avlRotLeft(CDFLocMapNode **ptr);
  void  avlRotRight(CDFLocMapNode **ptr);

  CDFLocMapNode *root;
  

};

#endif
