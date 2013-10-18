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

/*********************************************************
 ** 
 ** file: CDFLocMapTree.cpp
 **
 ** Copyright (C) 2003-2005 B. M. Bolstad
 **
 ** aim: a tree structure to hold mappings between probesets
 **      and locations on the array.
 **
 ** Created on: Apr 17, 2003
 **
 ** History
 ** Apr 17, 2003 - Initial version
 ** Apr 24, 2003 - add a destructor for CDFLocMapNode
 ** Apr 30, 2003 - change the tree from ordinary binary tree 
 **                into an AVL tree.
 ** Jul 21, 2003 - Access MMLocs
 ** Sep 12, 2003 - an isEmpty function for trees.
 ** Mar 22, 2005 - Change insertion to just pass a pointer rather
 **                than make a copy (removes a long standing memory leak)
 ** Mar 29, 2005 - adapt structure to hold Probesets which have
 **                differing numbers of PM and MM probes
 **
 *********************************************************/

#include <wx/wx.h>
#include "CDFLocMapTree.h"
//#include <iostream.h>

wxString  LocMapItem::GetName() const{
  return ProbesetName;

}


int  LocMapItem::GetMMSize(){
  return n_mm_probes;
}

int  LocMapItem::GetPMSize(){
  return n_pm_probes;
}

int *LocMapItem::GetPMLocs(){
  return PMLoc;
}

int *LocMapItem::GetMMLocs(){
  return MMLoc;
}



CDFLocMapTree::CDFLocMapTree() {

  root = 0;

}

CDFLocMapTree::~CDFLocMapTree(){

  delete root;

}


bool CDFLocMapTree::isEmpty(){

  if (root == 0){
    return true;
  }
  return false;

}



 void  CDFLocMapTree::avlRotLeft(CDFLocMapNode **ptr)
{
  CDFLocMapNode *tmp = *ptr;
  
  *ptr = (*ptr)->Right;
  tmp->Right = (*ptr)->Left;
  (*ptr)->Left = tmp;
}




void  CDFLocMapTree::avlRotRight(CDFLocMapNode **ptr)
{
  CDFLocMapNode *tmp = *ptr;
  
  *ptr = (*ptr)->Left;
  tmp->Left = (*ptr)->Right;
  (*ptr)->Right = tmp;
}




long CDFLocMapTree::avlLeftGrew(CDFLocMapNode **ptr){

  if ((*ptr)->skew == -1){
    if ((*ptr)->Left->skew == -1){
      (*ptr)->skew = (*ptr)->Left->skew = 0;
      avlRotRight(ptr);
    } else {
      if ((*ptr)->Left->Right->skew == -1){
	(*ptr)->skew = 1;
	(*ptr)->Left->skew =0;
      } else if ( (*ptr)->Left->Right->skew == 1){
	(*ptr)->skew = 0;
	(*ptr)->Left->skew = -1;
      } else {
	(*ptr)->skew = 0;
	(*ptr)->Left->skew =0;
      }
      (*ptr)->Left->Right->skew = 0;
      avlRotLeft(& (*ptr)->Left);
      avlRotRight(ptr);
    }
    return 0;
  } else if ((*ptr)->skew == 1){
    (*ptr)->skew = 0;
    return 0;
  } else {
    (*ptr)->skew = -1;
    return 1;
  }
}

long CDFLocMapTree::avlRightGrew(CDFLocMapNode **ptr){


  if ((*ptr)->skew == -1){
    (*ptr) ->skew = 0;
    return 0;
  } else if ((*ptr)->skew == 1){
    if ((*ptr)->Right->skew == 1) {        
      (*ptr)->skew = (*ptr)->Right->skew = 0;
      avlRotLeft(ptr);
    } else {

      if ((*ptr)->Right->Left->skew == 1){
	(*ptr)->skew = -1;
	(*ptr)->Right->skew = 0;
      } else if ((*ptr)->Right->Left->skew == -1){
	(*ptr)->skew = 0;
	(*ptr)->Right->skew = 1;
      } else {
	(*ptr)->skew = 0;
	(*ptr)->Right->skew = 0;
      }
      (*ptr)->Right->Left->skew = 0;
      avlRotRight(& (*ptr)->Right);
      avlRotLeft(ptr);
    }
    
    return 0;
  } else {
    (*ptr)->skew = 1;
    return 1;
  }
}


/************************************************
 **
 ** long CDFLocMapTree::InsertNode(CDFLocMapNode **ptr, const LocMapItem &value)
 **
 **
 **
 ** return value of 1 when balance needed, 0 when
 ** no such balance needed. 
 **
 ************************************************/

long CDFLocMapTree::InsertNode(CDFLocMapNode **ptr, const LocMapItem *value){
  
  wxString CurrentName;
  wxString CompareName;
  
  long tmp;

  if (*ptr == 0){
    *ptr = new CDFLocMapNode(value);
    return 1;
  } else {
    CurrentName = ((*ptr)->data)->GetName();
    CompareName = (value->GetName());
    if (CurrentName.Cmp(CompareName)   < 0){
      tmp = InsertNode(&((*ptr)->Left),value);
      if (tmp){
	return avlLeftGrew(ptr);
      }
      return tmp;
    } else {
      tmp = InsertNode( &( ( *ptr )->Right ), value );
      if (tmp){
	return avlRightGrew(ptr);
      }
      return tmp;
    }

  }

}



void CDFLocMapTree::Insert(const LocMapItem *value){
  InsertNode(&root,value);
}



CDFLocMapNode::CDFLocMapNode(const LocMapItem *dataitem){

  data = (LocMapItem *)dataitem;
  skew = 0;
  Left = 0;
  Right = 0;
}


CDFLocMapNode::~CDFLocMapNode(){
  if (Left != 0){
    delete Left;
  }
  if (Right !=0){
    delete Right;
  }
  delete data;
}

long CDFLocMapNode::getSkew(){
  return skew;
}






LocMapItem::LocMapItem(){

  MMLoc =0;
  PMLoc = 0;
  


}



LocMapItem::LocMapItem(wxString name, int n_pm_probes,int n_mm_probes,  int *PM, int *MM){

  this->ProbesetName = name;
  this->n_pm_probes = n_pm_probes;
  this->n_mm_probes = n_mm_probes;
  this->MMLoc = MM;
  this->PMLoc = PM;

}






LocMapItem::~LocMapItem(){
  
  if (MMLoc != 0){
    delete [] MMLoc;
  }
  if (PMLoc != 0){
    delete [] PMLoc;
  }

  
}



LocMapItem *CDFLocMapTree::Find(const wxString &x){

  return FindNode(&root,x);

}


LocMapItem *CDFLocMapTree::FindNode(CDFLocMapNode **ptr, const wxString &x){

  LocMapItem *result;
  wxString CurrentName;

  //wxPrintf("Looking for "+ x + "\n");

  if (*ptr == 0){
    // Not found in tree should give error
    result = NULL;
  } else {
    CurrentName = ((*ptr)->data)->GetName();
    //cout << "Looking for " << x << " at " <<CurrentName << endl;
    if (CurrentName.Cmp(x)   < 0){
      result = FindNode(&((*ptr)->Left),x);
    } else if (CurrentName.Cmp(x)   >  0){
      result = FindNode( &( ( *ptr )->Right ), x);
    } else {
      result = ((*ptr)->data);
    }
    
  } 
  return result;


}



