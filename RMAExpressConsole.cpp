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

/*****************************************************
 **
 ** file: RMAExpressConsole.cpp
 **
 ** Copyright (C) 2003-2009    B. M. Bolstad
 **
 ** aim: A crossplatform GUI program for generating 
 **      RMA expression estimates.
 **      This file implements a console batch
 **      processor which takes a CDF file and
 **      a list of CEL files and RMA processes
 **      them to produce a flat file containing
 **      expression values
 **
 ** Created on June 24, 2005
 **
 ** History
 ** June 24, 2005 - Initial version
 ** August 23, 2005 - Fix so accepts fully pathed output name
 ** August 26, 2005 - Changed temporary file path location 
 **                   on windows it will default to the Windows TEMP location
 **                   or "c:\\"
 **                   on Linux/Defaults to "/tmp/"
 **                   Version 2 output settings file allows user to control
 **                   this more carefully.
 ** October 28, 2005 - add ability to output plots
 ** Oct 29-30 , 2005 - a bunch of futzing around to make the windows app work semi reasonably.
 **                    Make parse_output return 1 when somethings wrong rather than exit();
 ** May 1, 2006 - add ability to toggle binary file output.
 ** May 5, 2006 - bug fixes for parsing output file
 ** Sep 16, 2006 - fix compile problems on unicode builds of wxWidgets
 ** Mar 6-9, 2007 - add PLM summarize and output NUSE/RLE summary statistics
 ** Feb 7, 2008 - Add version 4 output format file
 **
 *****************************************************/



#include "wx/defs.h"


#if RMA_GUI_APP
//    #error "RMAExpressConsole can't be compiled in GUI mode."
#endif // RMA_GUI_APP

#include <stdio.h>

#include "wx/string.h"
#include "wx/file.h"
#include "wx/log.h"
#include "wx/app.h"
#include "wx/textfile.h"
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/bitmap.h>


#include "version_number.h"
#include "PMProbeBatch.h"

#include "DataGroup.h"
#include "ResidualsDataGroup.h"
#include "ResidualsImagesDrawing.h"

#include "QCStatsVisualize.h"

#include <wx/config.h>

wxConfig *g_mysettings;


static wxString AppName=_T("RMAExpress Console Application ");
static wxString AppAuthor=_T("Copyright (C) B. M. Bolstad 2004-2009\n");


static void parseinput(const wxString &inputfile, wxString& cdfName, wxArrayString& celfileNames){

  wxTextFile InputFile;
  wxString buffer;

  InputFile.Open(inputfile);


  cdfName = InputFile.GetFirstLine();
  
  
  while (!InputFile.Eof())
    {
      buffer = InputFile.GetNextLine();
      if (!buffer.empty()){
	celfileNames.Add(buffer);
      }
      
    }

  InputFile.Close();

  wxPrintf(_T("Input Settings\n"));
  wxPrintf(_T("CDF file : %s\n"),cdfName.c_str());
  
  wxPrintf(_T("Number of CEL files : %d\n"),celfileNames.GetCount());
  
  for (int i=0; i < celfileNames.GetCount(); i++)
    wxPrintf(_T("%s\n"),celfileNames[i].c_str());



  wxPrintf(_T("\n\n"));
}

static int parseoutput(const wxString &inputfile, long int *version, wxString& outputname, wxString& temppath,int *normalize, int *background, wxString& typeofresiduals, int *outputtype, int *plm_summarize, long int *bufferrows, long int *buffercols){
  
  wxTextFile InputFile;
  wxString buffer;
  
  InputFile.Open(inputfile);
  buffer = InputFile.GetFirstLine();
  buffer.ToLong(version);
  outputname = InputFile.GetNextLine();


  // version 3 and above has option for setting output type.
  if (*version >= 3){
    buffer = InputFile.GetNextLine(); 
    //wxPrintf(_T("Buffer is")+buffer,+"\n");
    if (buffer.CmpNoCase(_T("binary")) == 0){
      *outputtype= 1;
    } else if (buffer.CmpNoCase(_T("text")) ==0){
      *outputtype= 0;
    } else {
      wxPrintf(_T("ERROR: Output file type unrecognized. Should be one of: text or binary.\n"));
      return 1;
    }
  }
  //wxPrintf(_T("Output type is %d\n"),*outputtype);

  // version 2 and above have extra lines
  if (*version >= 2){
    temppath = InputFile.GetNextLine();
    typeofresiduals = InputFile.GetNextLine();
  } else {
    typeofresiduals = wxT("none");
  }

  // convert type of residuals to all lower case

  typeofresiduals = typeofresiduals.Lower();

  //check whether type of residuals is "residuals","pos.resids","neg.resids","sign.resids","all.resids"

  if (!((typeofresiduals.Cmp(wxT("residuals"))==0)
	| (typeofresiduals.Cmp(wxT("pos.resids"))==0)
	| (typeofresiduals.Cmp(wxT("neg.resids"))==0)
	| (typeofresiduals.Cmp(wxT("sign.resids"))==0)
	| (typeofresiduals.Cmp(wxT("all.resids"))==0)
	| (typeofresiduals.Cmp(wxT("none"))==0)))
    {
      wxPrintf(_T("ERROR: the residuals output should be one of: residuals , pos.resids, neg.resids, sign.resids, all.resids or none.\n"));
      return 1;
      
    } 
  

  if (*version > 3){
    buffer = InputFile.GetNextLine();
    buffer.ToLong(bufferrows);
    if (*bufferrows < 5000){
      wxPrintf(_T("WARNING: Input bufferrows %d is to low. Raising to 5000\n"),*bufferrows);
      *bufferrows = 5000;
    }
    buffer = InputFile.GetNextLine();
    buffer.ToLong(buffercols);
    if (*buffercols < 1){
      wxPrintf(_T("WARNING: Input buffercols %d is to low. Raising to 1\n"),*buffercols);
      *buffercols = 1;
    }
    if (*buffercols > 150){
      wxPrintf(_T("WARNING: Input buffercols %d is to high. Lowering to 150\n"),*buffercols);
      *buffercols = 150;
    }
  } else {
    /* These were the default settings before version 4 */
    *bufferrows = 25000;
    *buffercols = 30;
  }
  
  while (!InputFile.Eof())
    {
      buffer = InputFile.GetNextLine();
      if (!buffer.Cmp(_T("no_background"))){
	*background = 0;
      } else if (!buffer.Cmp(_T("no_normalization"))){
	*normalize = 0;
      } else if (!buffer.Cmp(_T("plm_summarize"))){
	*plm_summarize = 1;
      } else if (buffer.empty()){

      } else {
	wxPrintf(_T("WARNING: Output file string ") + buffer + _T(" not understood.\n"));
      }
    }

  InputFile.Close();

  /* print relevant information to the screen so we can see what settings were used */
  wxPrintf(_T("Output Settings\n"));
  wxPrintf(_T("Version of Settings File Used: %d\n"),*version);
  wxPrintf(_T("Output File: : %s\n"),outputname.c_str());
  if (*outputtype == 1){
    wxPrintf(_T("Output Format Type: binary\n"));
  } else {
    wxPrintf(_T("Output Format Type: text\n"));
  } 
 
  wxPrintf(_T("Temporary files stored in: %s\n"),temppath.c_str());


  wxPrintf(_T("Buffer Settings (rows): %d\n"),  *bufferrows);
  wxPrintf(_T("Buffer Settings (cols): %d\n"),  *buffercols);
  
  wxPrintf(_T("Residual Images: %s\n"),typeofresiduals.c_str());
  wxPrintf(_T("Preprocessing Options\n"));
  wxPrintf(_T("Background Correction: "));
  if (*background == 0){
    wxPrintf(_T("none\n"));
  } else {
    wxPrintf(_T("yes\n"));
  }
  wxPrintf(_T("Quantile Normalization: "));
  if (*normalize == 0){
    wxPrintf(_T("none\n"));
  } else {
    wxPrintf(_T("yes\n"));
  }
  wxPrintf(_T("Summarization Method: "));
  if (*plm_summarize == 0){
    wxPrintf(_T("Median Polish\n"));
  } else {
    wxPrintf(_T("PLM\n"));
  }

  wxPrintf(_T("\n\n"));
  
  return 0;
}



static void createImages(ResidualsDataGroup *resids,wxString &outputtype, wxString outputpath){

  int horizontalsize = resids->ncols()+ 40;
  int verticalsize = resids->nrows() + 40;
  int i;

  wxImage my_image(horizontalsize,verticalsize,false);
  wxArrayString ArrayNames = resids->GetArrayNames();     
  
  wxString currentname,savefilename;
  wxString extension = wxT("png");
  
  wxString imagetype = wxT("_resid.");

  wxFileName currentPath;

  if (outputtype.Cmp(_T("all.resids")) != 0){
    for (i=0; i < ArrayNames.GetCount();i++){ 

      if (outputtype.Cmp(_T("residuals")) ==0){
	imagetype = wxT("_resid.");
	drawPseudoChipImage(&my_image,ArrayNames[i],_T("residuals") ,resids); 
      } else if (outputtype.Cmp(_T("pos.resids")) ==0){
	imagetype = wxT("_resid_pos.");
	drawPseudoChipImage(&my_image,ArrayNames[i],_T("Positive") ,resids); 
      } else if (outputtype.Cmp(_T("sign.resids")) ==0){
	imagetype = wxT("_resid_sign.");
	drawPseudoChipImage(&my_image,ArrayNames[i],_T("Sign") ,resids); 
      } else if (outputtype.Cmp(_T("neg.resids")) ==0){
	imagetype = wxT("_resid_neg.");
	drawPseudoChipImage(&my_image,ArrayNames[i],_T("Negative") ,resids); 
      }
      
      currentname = ArrayNames[i];
      currentname.Replace(_T(".cel"),_T(""));
      savefilename = currentname;
      savefilename+= imagetype;
      savefilename+= extension;

      currentPath.Assign(outputpath,savefilename); 
      my_image.SaveFile(currentPath.GetFullPath(),wxBITMAP_TYPE_PNG);
      wxPrintf(_T("Drawing %d\n"),i);
    }
  } else {
     for (i=0; i < ArrayNames.GetCount();i++){ 
       imagetype = wxT("_resid.");
       drawPseudoChipImage(&my_image,ArrayNames[i],_T("residuals") ,resids); 
       currentname = ArrayNames[i];
       currentname.Replace(_T(".cel"),_T(""));
       savefilename = currentname;
       savefilename+= imagetype;
       savefilename+= extension;       
       currentPath.Assign(outputpath,savefilename); 
       my_image.SaveFile(currentPath.GetFullPath(),wxBITMAP_TYPE_PNG);
       
       imagetype = wxT("_resid_pos.");
       drawPseudoChipImage(&my_image,ArrayNames[i],_T("Positive") ,resids); 
       currentname = ArrayNames[i];
       currentname.Replace(_T(".cel"),_T(""));
       savefilename = currentname;
       savefilename+= imagetype;
       savefilename+= extension;    
       currentPath.Assign(outputpath,savefilename); 
       my_image.SaveFile(currentPath.GetFullPath(),wxBITMAP_TYPE_PNG); 
       

       imagetype = wxT("_resid_sign.");
       drawPseudoChipImage(&my_image,ArrayNames[i],_T("Sign") ,resids); 
       currentname = ArrayNames[i];
       currentname.Replace(_T(".cel"),_T(""));
       savefilename = currentname;
       savefilename+= imagetype;
       savefilename+= extension;       
       currentPath.Assign(outputpath,savefilename);   
       my_image.SaveFile(currentPath.GetFullPath(),wxBITMAP_TYPE_PNG); 
       
       imagetype = wxT("_resid_neg.");
       drawPseudoChipImage(&my_image,ArrayNames[i],_T("Negative") ,resids); 
       currentname = ArrayNames[i];
       currentname.Replace(_T(".cel"),_T(""));
       savefilename = currentname;
       savefilename+= imagetype;
       savefilename+= extension;         
       currentPath.Assign(outputpath,savefilename);   
       my_image.SaveFile(currentPath.GetFullPath(),wxBITMAP_TYPE_PNG); 
     }
  }



}





int not_main(int argc, char **argv)
{

  DataGroup *currentexperiment = (DataGroup *) NULL;
  expressionGroup *myexprs = (expressionGroup *)NULL;
  // myresids = (ResidualsDataGroup *)NULL;


  wxString cdfName,cdfPath;
  wxFileName cdfFileName;
  wxArrayString celfileNames,celfilePaths;
  wxFileName temp;

  long int OutputVersion=0;
  long int bufferrows=0;
  long int buffercols=0;

  int background=1,normalize=1;
  int plm_summarize = 0;
  wxString outputname,temppath;

  wxString typeofresiduals;

  wxFileName outputFileName;

  int i;
  ResidualsDataGroup *myresids;
  Preferences *myprefs;
  char wintemppath[512];

  int outputtype=0;
 
  // Application starting messages

  wxPrintf(AppName+ version_number + _T("\n"));
  wxPrintf(AppAuthor + _T("\n"));

  
  // Check that two setting files have been supplied

  if (argc != 3){
    wxPrintf(_T("Error: Incorrect number of arguments for ") + AppName+ _T("\n"));
    return 1;
  }
  wxPrintf(_T("Input Settings Read From: ")+ wxString(argv[1],wxConvUTF8)+_T("\n"));
  wxPrintf(_T("Output Settings Read From: ")+ wxString(argv[2],wxConvUTF8)+_T("\n\n"));
  
  // Parse input file to get names of CDF and CEL files

  if (wxFileExists(wxString(argv[1], wxConvUTF8))){
    parseinput(wxString(argv[1], wxConvUTF8),cdfName,celfileNames);
  } else {
    wxPrintf(_T("Error: Input Settings File %s does not seem to exist.\n"), argv[1]);
    return 1;
  }
 

  // Parse output settings file
  if (wxFileExists(wxString(argv[2], wxConvUTF8))){
    if (parseoutput(wxString(argv[2], wxConvUTF8),&OutputVersion,outputname,temppath,&normalize,&background,typeofresiduals,&outputtype,&plm_summarize, &bufferrows, &buffercols)){
      return 1;
    }
  } else {
    wxPrintf(_T("Error: Output Settings File %s does not seem to exist.\n"), argv[2]);
    return 1;
  }
  

  if (!wxFileExists(cdfName)){
    wxPrintf(_T("Error: ")+ temp.GetFullPath() +_T(" does not exist or can't be found.\n"));
    return 1;
  }


  if ((OutputVersion > 4) && (OutputVersion < 1)){
    wxPrintf(_T("Error: Output version is not among supported versions\n"));
    return 1;
  }
  
  cdfFileName = wxFileName(cdfName);
  outputFileName = wxFileName(outputname);
  
  for (i = 0; i < celfileNames.GetCount(); i++){
    temp = wxFileName(celfileNames[i]);
    celfileNames[i] = temp.GetFullName();
    celfilePaths.Add(temp.GetFullPath());
    if (!wxFileExists(temp.GetFullPath())){
      wxPrintf(_T("Error: ")+ temp.GetFullPath() +_T(" does not exist or can't be found.\n"));
      return 1;
    }
  }
  
  
  /** So can deal with outputting PNG or JPG images */
  wxImage::AddHandler( new wxPNGHandler );
  wxImage::AddHandler( new wxJPEGHandler );





  try{
    wxPrintf(_T("Reading Data\n"));  
#if _WIN32 
    if (OutputVersion == 1){

      if(GetTempPath(512,wintemppath)!=0){
	myprefs = new Preferences(bufferrows,buffercols,wxString(wintemppath));
      } else {
	myprefs = new Preferences(bufferrows,buffercols,wxString(_T("c:\\")));
      }
    } else {
      myprefs = new Preferences(bufferrows,buffercols,temppath);
    }
#else
    if (OutputVersion ==1){
      myprefs = new Preferences(bufferrows,buffercols,wxString(_T("/tmp/")));
    } else {
      myprefs = new  Preferences(bufferrows,buffercols,temppath);
    }


#endif

    currentexperiment = new DataGroup(NULL,cdfFileName.GetFullName(),cdfFileName.GetFullPath(),celfileNames,celfilePaths,myprefs); 
    wxPrintf(_T("Computing Expression values\n")); 
    
    currentexperiment->ReadOnlyMode(true);

    PMProbeBatch PMSet(*currentexperiment, myprefs);
    if (background){
      wxPrintf(_T("Background Correcting\n")); 
      PMSet.background_adjust();
    }
    if (normalize){
      wxPrintf(_T("Normalizing\n")); 
      PMSet.normalize(1);   // Low memory Overhead
    }

    if (!plm_summarize){
      wxPrintf(_T("Summarizing\n"));
      myexprs = PMSet.summarize(); 
    } else {
      wxPrintf(_T("Summarizing using PLM\n"));
      myexprs = PMSet.summarize_PLM(); 

      
      QCStatsVisualizeFrame *frame = new QCStatsVisualizeFrame(myexprs);

      wxPrintf(_T("Computing RLE\n"));
      frame->GenerateRLE();      
      frame->WriteRLEStats(outputFileName.GetPath(),wxString(wxT("RLE_Summary.txt")));


      wxPrintf(_T("Computing NUSE\n"));
      frame->GenerateNUSE();
      frame->WriteNUSEStats(outputFileName.GetPath(),wxString(wxT("NUSE_Summary.txt")));


    }

    if (typeofresiduals.Cmp(_T("none")) != 0){ 
      wxPrintf(_T("Writing Images in ")+outputFileName.GetPath() + _T("\n")); 
      myresids = new ResidualsDataGroup(&PMSet,currentexperiment, myprefs);
      

      createImages(myresids,typeofresiduals,outputFileName.GetPath());
      

      delete myresids;
    }



    if (outputtype == 1){
      wxPrintf(_T("Writing output to ")+ outputFileName.GetFullName()+ _T(" in ")+outputFileName.GetPath()+_T(" using the binary format.\n"));

      myexprs->writetobinaryfile(outputFileName.GetFullName(),outputFileName.GetPath(),0);
    } else {
       wxPrintf(_T("Writing output to ")+ outputFileName.GetFullName()+ _T(" in ")+outputFileName.GetPath()+_T(" using the text format.\n"));

      myexprs->writetofile(outputFileName.GetFullName(),outputFileName.GetPath(),0);                    //outputname,"./",0);
    }
    delete currentexperiment;

  }
  catch (wxString &Problem){
    wxPrintf(Problem);
    
  }

  return 0;
}





#if _WIN32 





#include <wchar.h>
#include <string.h>

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

  char **tmp;
  char *buffer = (char *)malloc(1024);

  int argv;
  LPWSTR *szArglist;


  DWORD dwBytesWritten; char temp[1];
  /* *char* msg1 = "This console was created in WinMain() \nand the messages below were generated by different handles.\n\"return\" to exit.\n\n";
   char* msg2 = "This line was writen by WriteFile() through operating system handle.\n";
   char* msg3 = "This line was writen by write() through run time handle.\n";
   char* msg4 = "This line was writen by fprintf()through stream.\n";
   char* msg5 = "This line was writen by printf()through standard output stream.\n"; */

   AllocConsole();
   HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);		// return an OS file handle
   /*WriteFile(handle, msg1, strlen(msg1), &dwBytesWritten, NULL);
     WriteFile(handle, msg2, strlen(msg2), &dwBytesWritten, NULL); */
   int hCrt = _open_osfhandle((long)handle,_O_TEXT);	// return a runtime file handle
   //write(hCrt, msg3, strlen(msg3));						// for files opened by _open()
   FILE * hf = _fdopen( hCrt, "w" );					// stream
   char buf[2];
   setvbuf( hf, buf, _IONBF, 1 );
   *stdout = *hf;
   //fprintf(hf, msg4);									// for files opened by  fopen */
   // printf(msg5);
   


   szArglist = CommandLineToArgvW(GetCommandLineW(), &argv);

   //wxPrintf("%d", argv);
   //  not_main(argv,szArglist);
   

   tmp = (char**)calloc(argv,sizeof(char *));

   for (int i=0; i < argv; i++){ 
     wcstombs(buffer,szArglist[i] , 1024 );
     tmp[i] = (char *)calloc(strlen(buffer)+1,sizeof(char));
     strcpy(tmp[i],buffer);
     //wxPrintf("%s\n",tmp[i]);
   }
   
   
   if (not_main(argv,tmp)){

     HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);			// return an OS file handle
     ReadFile(hIn, temp, 1, &dwBytesWritten, NULL);
   } 
   FreeConsole(); 
   return (0);
}



#endif

int main(int argc, char **argv){


  not_main(argc,argv);








}


