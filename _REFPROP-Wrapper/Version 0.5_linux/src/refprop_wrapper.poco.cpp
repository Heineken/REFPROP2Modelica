/*
	wrapper code for a static library containing functions from the dynamic library refprop.dll
	to be used from Modelica
	
	This file is released under the Modelica License 2.
	
	Coded in 2010 by
	Henning Francke
	francke@gfz-potsdam.de
	
	Helmholtz Centre Potsdam
	GFZ German Research Centre for Geosciences
	Telegrafenberg, D-14473 Potsdam
	
	Modified for portability to Linux and
	introduction of POCO library in 2012 by
	Jorrit Wronski (jowr@mek.dtu.dk)
	DTU Mechanical Engineering

	needs
	refprop_lib.h     - header for librefprop.so, based on examples
	refprop_wrapper.h - header for static REFPROP_wrapper.a, also needed by Dymola
	
	Use the provided makefile to install the library file from source.
*/

//#define DEBUGMODE 1

#include <stdio.h>
#if defined(WIN32) || defined(_WIN32)
#  include <windows.h>
//#  include "REFPROP_dll.h"
#else // assuming Linux system
#  include <stdlib.h>
#  include <string.h>
//#  include <dlfcn.h>  // dlopen etc
#  include <ctype.h> // tolower etc
#endif
#include <refprop_lib.h>
//# error "Could not determine system."
#include "refprop_wrapper.h"

// get the POCO classes
#include "Poco/SharedLibrary.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/Environment.h"
#include "Poco/StringTokenizer.h"
#include "Poco/String.h"
#include "Poco/Exception.h"


// Some constants...
const long filepathlength=1024;
const long errormessagelength=255+filepathlength;
const long lengthofreference=3;
const long refpropcharlength=255;
const long ncmax=20;		// Note: ncmax is the max number of components

Poco::SharedLibrary *RefpropdllInstance = NULL;
char loadedfluids[refpropcharlength];
char loadedpath[filepathlength];

//  Define the functions either by their pointers or type.
WMOLdll_POINTER WMOL = NULL;
TPFL2dll_POINTER TPFL2 = NULL;
TPFLSHdll_POINTER TPFLSH = NULL;
PHFL1dll_POINTER PHFL1 = NULL;
PHFLSHdll_POINTER PHFLSH = NULL;
PDFL1dll_POINTER PDFL1 = NULL;
PDFLSHdll_POINTER PDFLSH = NULL;
PSFLSHdll_POINTER PSFLSH = NULL;
PQFLSHdll_POINTER PQFLSH = NULL;
THFLSHdll_POINTER THFLSH = NULL;
TDFLSHdll_POINTER TDFLSH = NULL;
TSFLSHdll_POINTER TSFLSH = NULL;
TQFLSHdll_POINTER TQFLSH = NULL;
DHFL1dll_POINTER DHFL1 = NULL;
DHFL2dll_POINTER DHFL2 = NULL;
DHFLSHdll_POINTER DHFLSH = NULL;
HSFLSHdll_POINTER HSFLSH = NULL;
DSFL1dll_POINTER DSFL1 = NULL;
DSFL2dll_POINTER DSFL2 = NULL;
DSFLSHdll_POINTER DSFLSH = NULL;
TRNPRPdll_POINTER TRNPRP = NULL;
SATTdll_POINTER SATT = NULL;
SATPdll_POINTER SATP = NULL;
SATDdll_POINTER SATD = NULL;


//str_replace    (fluidnames,          "|",       replace,          nX)
char *str_replace(char *str,  char *search, char *replace, long *count, int DEBUGMODE) {

  *count = 0;
  char ret[filepathlength];

  std::string fluids(str);
  std::string dlimit(search);
  std::string substi(replace);
  std::string result;

  // if (DEBUGMODE) printf("fluidnames: %s\n",fluids.c_str());
  // if (DEBUGMODE) printf("delimiter:  %s\n",dlimit.c_str());
  // if (DEBUGMODE) printf("replace:    %s\n",substi.c_str());
  // if (DEBUGMODE) printf("nX:         %li\n\n",*count);

  Poco::StringTokenizer tokens(fluids.c_str(), dlimit.c_str(), Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);
  result = std::string(Poco::cat(substi, tokens.begin(), tokens.end()));

  strcpy(ret,result.c_str());
  //strcat(ret,'\0');

  *count = tokens.count()-1;

  if (DEBUGMODE) printf("fluidnames: %s\n",fluids.c_str());
  if (DEBUGMODE) printf("delimiter:  %s\n",dlimit.c_str());
  if (DEBUGMODE) printf("replace:    %s\n",substi.c_str());
  if (DEBUGMODE) printf("replaced:   %s\n",result.c_str());
  if (DEBUGMODE) printf("nX:         %li\n\n",*count);

return ret;

}


int init_REFPROP(char* fluidnames, char* REFPROP_PATH_CHAR, long* nX, char* herr, char* errormsg, int DEBUGMODE){
// Sets up the interface to the REFPROP.DLL
// is called by props_REFPROP and satprops_REFPROP
//	char DLL_PATH[filepathlength], FLD_PATH[filepathlength];

	long ierr=0;
// 	DEBUGMODE = 1;

	if (strlen(REFPROP_PATH_CHAR)>filepathlength){
		sprintf(errormsg,"REFPROP_PATH_CHAR too long (%i > %li)\n",strlen(REFPROP_PATH_CHAR),filepathlength);
		return 0;
	}
	// Define temporary objects for checks.
	char REF_PATH_CHAR[filepathlength];
	Poco::Path REF_PATH(true);
	char FLUIDS_CHAR[filepathlength] = "fluids";
	Poco::Path FLD_PATH(true);
	char LIBRARY_CHAR[filepathlength];
	Poco::Path LIB_PATH(true);
	//Poco::File theFile;

	// Parse the string and append a path separator if necessary.
	REF_PATH.parse(REFPROP_PATH_CHAR, Poco::Path::PATH_NATIVE);
	if (!REF_PATH.isDirectory()) REF_PATH.append(REF_PATH.separator());
	// Overwrite the provided path
	strcpy(REF_PATH_CHAR,REF_PATH.toString().c_str());

	// Check the path if running in debugmode
	if (DEBUGMODE) {
	  Poco::File refFile(REF_PATH);
	  if ( !refFile.isDirectory() || !refFile.canRead() ){
	    printf ("REF_PATH is not a readable directory: %s \n", REF_PATH.toString().c_str());
	    sprintf (errormsg,"REF_PATH is not a readable directory: %s \n", REF_PATH.toString().c_str());
	    return 0;
	  } else {
	    printf ("REF_PATH is a readable directory: %s \n", REF_PATH.toString().c_str());
	  }
	}

	// The fluid files are in the Refprop directory, append "fluids".
	FLD_PATH.parse(REF_PATH_CHAR);
	FLD_PATH.pushDirectory(FLUIDS_CHAR);
	
// 	//std::string path(REF_PATH_CHAR); // This is not fail-safe, the OS might look somewhere else, e.g. /usr/lib ...
// 	Poco::Path SRC_PATH;
// 	SRC_PATH.parse(Poco::Environment::get("PATH"));
	
// 	if (DEBUGMODE) {
// 		// Determine which library file should be loaded
//  		bool is_linux = ( 0 == Poco::icompare(Poco::Environment::osName(), "linux") );
//  		if (is_linux){
//  			strcpy(LIBRARY_CHAR,"librefprop.so");
// 			SRC_PATH.pushDirectory("/usr/local/lib");
//  		} else {
//  			strcpy(LIBRARY_CHAR,"refprop.dll");
// 			SRC_PATH.pushDirectory(REF_PATH.toString());
//  		}


		//bool found_lib = Poco::Path::find(SRC_PATH.toString(), LIBRARY_CHAR, LIB_PATH);
// 		if (found_lib) {
// 			printf ("Found library %s in path %s \n", LIBRARY_CHAR, path.c_str());
// 		} else {
// 			printf ("Cannot find library %s in path %s \n", LIBRARY_CHAR, path.c_str());
// 		}
// 	} else {
// 		LIB_PATH.parse(REF_PATH_CHAR);
// 	}

	//LIB_PATH.parse(LIBRARY_CHAR);

// 	if (DEBUGMODE) {
// 	  printf ("REF_PATH as string: %s \n", REF_PATH.toString().c_str());
// 	  printf ("FLD_PATH as string: %s \n", FLD_PATH.toString().c_str());
// 	  printf ("LIB_PATH as string: %s \n", LIB_PATH.toString().c_str());
// 	  printf ("Running OS family : %s \n\n", Poco::Environment::osName().c_str());
// 	}

	Poco::Path SRC_PATH; // This is not fail-safe, the OS might look somewhere else, e.g. /usr/lib ...

	if (RefpropdllInstance==NULL) { // we need to load the library 
	  if (DEBUGMODE) printf ("RefpropdllInstance loaded: %s \n", "false");
	  
	  // Check the OS and assign the right names for the library 
	  bool is_linux = ( 0 == Poco::icompare(Poco::Environment::osName(), "linux") );
	  if (is_linux){
	    strcpy(LIBRARY_CHAR,"librefprop.so");
	    SRC_PATH.parse("/usr/local/lib");
	  } else {
	    strcpy(LIBRARY_CHAR,"refprop.dll");
	    SRC_PATH = REF_PATH;
	  }   
	  
	  // search the library at the given path 
	  bool found_lib = Poco::Path::find(SRC_PATH.toString(), LIBRARY_CHAR, LIB_PATH);
	  if (found_lib) {
	    if (DEBUGMODE) printf ("Found library %s in path %s \n", LIBRARY_CHAR, SRC_PATH.toString().c_str());
	  } else {
	    if (DEBUGMODE) printf ("Cannot find library %s in path %s \n", LIBRARY_CHAR, SRC_PATH.toString().c_str());
			  sprintf ("Cannot find library %s in path %s \n", LIBRARY_CHAR, SRC_PATH.toString().c_str());
	    return 0;
	  }
	  
	  // check if the file is correct and executable
	  if (DEBUGMODE) {
	    Poco::File libFile(LIB_PATH);
	    if ( !libFile.isFile() || !libFile.canRead() ){
	      printf ("LIB_PATH is not a readable file: %s \n", LIB_PATH.toString().c_str());
	      sprintf (errormsg,"LIB_PATH is not a readable file: %s \n", LIB_PATH.toString().c_str());
	      return 0;
	    } else {
	      printf ("LIB_PATH exists and is readable: %s \n", LIB_PATH.toString().c_str());
	    }
	  }
	  
	  // load a new library instance
	  RefpropdllInstance = new Poco::SharedLibrary(LIB_PATH.toString());
	  if (DEBUGMODE) printf ("RefpropdllInstance loaded: %s \n\n", "true");
	  
	} else { // library was already loaded 
	  if (DEBUGMODE) printf ("RefpropdllInstance loaded: %s \n\n", "true");
	}
	
	// Now the library is loaded and we can start checkicng the fluid path
	if (DEBUGMODE) {
	  Poco::File fldFile(FLD_PATH);
	  if ( !fldFile.isDirectory() || !fldFile.canRead() ){
	    printf ("FLD_PATH is not a readable directory: %s \n", FLD_PATH.toString().c_str());
	    sprintf (errormsg,"FLD_PATH is not a readable directory: %s \n", FLD_PATH.toString().c_str());
	    return 0;
	  } else {
	      printf ("FLD_PATH is a readable directory: %s \n", FLD_PATH.toString().c_str());
	  }
	} 
	char FLD_PATH_CHAR[filepathlength];
	strcpy(FLD_PATH_CHAR,FLD_PATH.toString().c_str());
	  
	  
	if (DEBUGMODE) {
	  printf ("%s\n"," ");
	  printf ("REF_PATH as string: %s \n", REF_PATH.toString().c_str());
	  printf ("FLD_PATH as string: %s \n", FLD_PATH.toString().c_str());
	  printf ("LIB_PATH as string: %s \n", LIB_PATH.toString().c_str());
	  printf ("Running OS family : %s \n", Poco::Environment::osName().c_str());
	  printf ("%s\n"," ");
	}


	// Check for new fluids and if the library has to be loaded again.
	bool isFluid = (strcmp(fluidnames,loadedfluids)==0);
	bool isPath  = (strcmp(REF_PATH_CHAR,loadedpath)==0);
	if (DEBUGMODE) printf ("Comparison of fluids : %i   \n", isFluid );
	if (DEBUGMODE) printf ("Comparison of path   : %i \n\n", isPath  );
	//if (DEBUGMODE) printf ("Checking setup       : %s and %s \n\n", REF_PATH_CHAR,loadedpath  );

	char *hf;
	
// // // // 	if (isFluid && isPath) {
// // // // 	  //sprintf(errormsg,"Library is already loaded: %s \n",LIB_PATH.toString().c_str());
// // // // 	  if (DEBUGMODE) printf ("No setup needed, fluids (%s) and path (%s) did not change.\n\n", fluidnames,REF_PATH_CHAR);
// // // // // // // // 	  return 0;
// // // // 	} else {
	  // we need to call setup 
	  
	  char hrf[lengthofreference+1],hfmix[filepathlength+1+7];
// // // // 	  char *hf;

	//parse fluid composition string and insert absolute paths
	  char replace[filepathlength+6];
	  strcpy(replace,".FLD|");
	//if (DEBUGMODE) printf("REPLACE: %s\n",replace);
	  strncat(replace, FLD_PATH_CHAR,filepathlength-strlen(replace));

	int hf_len = strlen(fluidnames)+ncmax*(strlen(replace)-1)+4;
	hf =  (char*) calloc(hf_len, sizeof(char));

	strncpy(hf,FLD_PATH_CHAR,hf_len);
	
	char replaced[filepathlength];
	
	strcpy(replaced,str_replace(fluidnames, "|", replace, nX, DEBUGMODE));
	
	//str_replace returns the number of delimiters -> nX, but components are one more ...
	if (++*nX>ncmax){ //...that's why nX is incremented
		sprintf(errormsg,"Too many components (More than %li)\n",ncmax);
		return 0;
	}

	
	strncat(hf,replaced,hf_len-strlen(hf)); 
	
	strncat(hf,".FLD",hf_len-strlen(hf));
	
	if (DEBUGMODE) printf("Fluid composition string: \"%s\"\n",hf);

	strncpy(hfmix,FLD_PATH_CHAR,filepathlength+1);//add absolute path
	strncat(hfmix,"hmx.bnc",filepathlength+1+7-strlen(hfmix));
	strcpy(hrf,"DEF");


	SETUPdll_POINTER SETUP = (SETUPdll_POINTER) RefpropdllInstance->getSymbol(SETUPdll_NAME);
	if (DEBUGMODE) printf("Running SETUP...\n");
	SETUP(*nX, hf, hfmix, hrf, ierr, herr);
	
	strcpy(loadedfluids,fluidnames);
	strcpy(loadedpath,REF_PATH_CHAR);
	if (DEBUGMODE) printf("SETUP run complete (Error no: %li)\n",ierr); 
	  
	if (DEBUGMODE) printf("Error code processing...\n");
	switch(ierr){
		case 101:
			//strcpy(errormsg,"error in opening file");
//			if (DEBUGMODE) printf("Error 101\n");
			sprintf(errormsg,"error in opening file %s",hf);
			break;
		case 102:
//			if (DEBUGMODE) printf("Error 102\n");
			strcpy(errormsg,"error in file or premature end of file");
			break;
		case -103:
//			if (DEBUGMODE) printf("Error -103\n");
			strcpy(errormsg,"unknown model encountered in file");
			break;
		case 104:
//			if (DEBUGMODE) printf("Error 104\n");
			strcpy(errormsg,"error in setup of model");
			break;
		case 105:
//			if (DEBUGMODE) printf("Error 105\n");
			strcpy(errormsg,"specified model not found");
			break;
		case 111:
//			if (DEBUGMODE) printf("Error 111\n");
			strcpy(errormsg,"error in opening mixture file");
			break;
		case 112:
//			if (DEBUGMODE) printf("Error 112\n");
			strcpy(errormsg,"mixture file of wrong type");
			break;
		case 114:
//			if (DEBUGMODE) printf("Error 114\n");
			strcpy(errormsg,"nc<>nc from setmod");
			break;
		case 0:
			break;
		default:
//			if (DEBUGMODE) printf("Unknown error\n");
			strcpy(errormsg,"Unknown error");
			//strcpy(errormsg,"Setup was successful!");
			strncpy(errormsg,herr,errormessagelength);
			break;
	}
// // // // 	free(hf);
	return ierr;


// 	if (DEBUGMODE) {
// 		theFile = Poco::File(LIB_PATH);
// 		if ( !theFile.isFile() || !theFile.canExecute() ){
// 			sprintf (errormsg,"LIB_PATH is not an executable file: %s \n", LIB_PATH.toString().c_str());
// 			return 0;
// 		}
// 	}
// 	char LIB_PATH_CHAR[filepathlength];
// 	strcpy(LIB_PATH_CHAR,LIB_PATH.toString().c_str());
// 
// 	if (DEBUGMODE) {
// 	theFile = Poco::File(FLD_PATH);
// 	if ( !theFile.isDirectory() || !theFile.canRead() ){
// 		sprintf (errormsg,"FLD_PATH is not a readable directory: %s \n", FLD_PATH.toString().c_str());
// 		return 0;
// 	}
// 	}
// 	char FLD_PATH_CHAR[filepathlength];
// 	strcpy(FLD_PATH_CHAR,FLD_PATH.toString().c_str());

////  First we load the library with the POCO foundation
////  classes and then define all the needed functions
////  by their names and a cast to the correct type.
//	if (DEBUGMODE) printf ("RefpropdllInstance loaded path: %s \n", RefpropdllInstance.getPath().c_str());
//	if (DEBUGMODE) printf ("New path for loading the library: %s \n", LIB_PATH.toString().c_str());
//	if (DEBUGMODE) printf ("Comparison: %i \n", LIB_PATH.toString().compare(RefpropdllInstance.getPath()) );
//	if ( LIB_PATH.toString().compare(RefpropdllInstance.getPath())!=0  ) {
//		RefpropdllInstance.unload();
//		if (DEBUGMODE) printf ("RefpropdllInstance unloaded: %s \n", "true");
//		RefpropdllInstance.load(LIB_PATH_CHAR);
//	}
//
//	if (!RefpropdllInstance.isLoaded()){
//	  	sprintf(errormsg,"ERROR in opening library at \"%s\"",LIB_PATH_CHAR);
//	   	return 100;
//	}


//     char hrf[lengthofreference+1],hfmix[filepathlength+1+7];
// 	char *hf;
// 
// 	//parse fluid composition string and insert absolute paths
// 	char replace[filepathlength+6];
// 	strcpy(replace,".FLD|");
// 	//if (DEBUGMODE) printf("REPLACE: %s\n",replace);
// 	strncat(replace, FLD_PATH_CHAR,filepathlength-strlen(replace));
// 
// 	int hf_len = strlen(fluidnames)+ncmax*(strlen(replace)-1)+4;
// 	hf =  (char*) calloc(hf_len, sizeof(char));
// 
// 	strncpy(hf,FLD_PATH_CHAR,hf_len);
// 	strncat(hf,str_replace(fluidnames, "|", replace, nX),hf_len-strlen(hf)); //str_replace returns the number of delimiters -> nX, but components are one more ...
// 	if (++*nX>ncmax){ //...that's why nX is incremented
// 		sprintf(errormsg,"Too many components (More than %i)\n",ncmax);
// 		return 0;
// 	}
// 	strncat(hf,".FLD",hf_len-strlen(hf));
// 	if (DEBUGMODE) printf("Fluid composition string: \"%s\"\n",hf);
// 
// 	strncpy(hfmix,FLD_PATH_CHAR,filepathlength+1);//add absolute path
// 	strncat(hfmix,"hmx.bnc",filepathlength+1+7-strlen(hfmix));
// 	strcpy(hrf,"DEF");
// 
// 
// //	SETUPdll_TYPE   * SETUPdll   = (SETUPdll_TYPE *  ) RefpropdllInstance.getSymbol(SETUPdll_NAME);
// 	if (DEBUGMODE) printf("Running SETUPdll...\n");
// 	SETUP(*nX, hf, hfmix, hrf, ierr, herr);
// 	strcpy(loadedfluids,fluidnames);
// 	if (DEBUGMODE) printf("SETUPdll run complete (Error no: %i)\n",ierr);
// 
// //	WMOLdll = (WMOLdll_POINTER) RefpropdllInstance.getSymbol(WMOLdll_NAME);
// //	TPFL2dll = (TPFL2dll_POINTER) RefpropdllInstance.getSymbol(TPFL2dll_NAME);
// //	TPFLSHdll = (TPFLSHdll_POINTER) RefpropdllInstance.getSymbol(TPFLSHdll_NAME);
// //	PHFL1dll = (PHFL1dll_POINTER) RefpropdllInstance.getSymbol(PHFL1dll_NAME);
// //	PHFLSHdll = (PHFLSHdll_POINTER) RefpropdllInstance.getSymbol(PHFLSHdll_NAME);
// //	PDFL1dll = (PDFL1dll_POINTER) RefpropdllInstance.getSymbol(PDFL1dll_NAME);
// //	PDFLSHdll = (PDFLSHdll_POINTER) RefpropdllInstance.getSymbol(PDFLSHdll_NAME);
// //	PSFLSHdll = (PSFLSHdll_POINTER) RefpropdllInstance.getSymbol(PSFLSHdll_NAME);
// //	PQFLSHdll = (PQFLSHdll_POINTER) RefpropdllInstance.getSymbol(PQFLSHdll_NAME);
// //	THFLSHdll = (THFLSHdll_POINTER) RefpropdllInstance.getSymbol(THFLSHdll_NAME);
// //	TDFLSHdll = (TDFLSHdll_POINTER) RefpropdllInstance.getSymbol(TDFLSHdll_NAME);
// //	TSFLSHdll = (TSFLSHdll_POINTER) RefpropdllInstance.getSymbol(TSFLSHdll_NAME);
// //	TQFLSHdll = (TQFLSHdll_POINTER) RefpropdllInstance.getSymbol(TQFLSHdll_NAME);
// //	DHFL1dll = (DHFL1dll_POINTER) RefpropdllInstance.getSymbol(DHFL1dll_NAME);
// //	DHFL2dll = (DHFL2dll_POINTER) RefpropdllInstance.getSymbol(DHFL2dll_NAME);
// //	DHFLSHdll = (DHFLSHdll_POINTER) RefpropdllInstance.getSymbol(DHFLSHdll_NAME);
// //	HSFLSHdll = (HSFLSHdll_POINTER) RefpropdllInstance.getSymbol(HSFLSHdll_NAME);
// //	DSFL1dll = (DSFL1dll_POINTER) RefpropdllInstance.getSymbol(DSFL1dll_NAME);
// //	DSFL2dll = (DSFL2dll_POINTER) RefpropdllInstance.getSymbol(DSFL2dll_NAME);
// //	DSFLSHdll = (DSFLSHdll_POINTER) RefpropdllInstance.getSymbol(DSFLSHdll_NAME);
// //	TRNPRPdll = (TRNPRPdll_POINTER) RefpropdllInstance.getSymbol(TRNPRPdll_NAME);
// //	SATTdll = (SATTdll_POINTER) RefpropdllInstance.getSymbol(SATTdll_NAME);
// //	SATPdll = (SATPdll_POINTER) RefpropdllInstance.getSymbol(SATPdll_NAME);
// //	SATDdll = (SATDdll_POINTER) RefpropdllInstance.getSymbol(SATDdll_NAME);


// //	if (DEBUGMODE) printf("Error code processing...\n");
// 	switch(ierr){
// 		case 101:
// 			//strcpy(errormsg,"error in opening file");
// //			if (DEBUGMODE) printf("Error 101\n");
// 			sprintf(errormsg,"error in opening file %s",hf);
// 			break;
// 		case 102:
// //			if (DEBUGMODE) printf("Error 102\n");
// 			strcpy(errormsg,"error in file or premature end of file");
// 			break;
// 		case -103:
// //			if (DEBUGMODE) printf("Error -103\n");
// 			strcpy(errormsg,"unknown model encountered in file");
// 			break;
//  		case 104:
// //			if (DEBUGMODE) printf("Error 104\n");
// 			strcpy(errormsg,"error in setup of model");
// 			break;
// 		case 105:
// //			if (DEBUGMODE) printf("Error 105\n");
// 			strcpy(errormsg,"specified model not found");
// 			break;
// 		case 111:
// //			if (DEBUGMODE) printf("Error 111\n");
// 			strcpy(errormsg,"error in opening mixture file");
// 			break;
// 		case 112:
// //			if (DEBUGMODE) printf("Error 112\n");
// 			strcpy(errormsg,"mixture file of wrong type");
// 			break;
// 		case 114:
// //			if (DEBUGMODE) printf("Error 114\n");
// 			strcpy(errormsg,"nc<>nc from setmod");
// 			break;
// 		case 0:
// 			break;
// 		default:
// //			if (DEBUGMODE) printf("Unknown error\n");
// 			strcpy(errormsg,"Unknown error");
// 			//strcpy(errormsg,"Setup was successful!");
// 			strncpy(errormsg,herr,errormessagelength);
// 			break;
// 	}
// 	free(hf);
// 	return ierr;
}


double props_REFPROP(char* what, char* statevars_in, char* fluidnames, double *props, double statevar1, double statevar2, double* x, int phase, char* REFPROP_PATH, char* errormsg, int DEBUGMODE){
/*Calculates thermodynamic properties of a pure substance/mixture, returns both single value and array containing all calculated values (because the are calculated anyway)
INPUT:
	what: character specifying return value (p,T,h,s,d,wm,q,e,w) - Explanation of variables at the end of this function
	statevars: string of any combination of two variables out of p,T,h,s,d
	fluidnames: string containing names of substances in mixtured separated by |, substance names are identical to those of *.fld-files in REFPROP program directory
	statevar1,statevar2: values of the two variables specified in statevars
	x: array containing the mass fractions of the components of the mixture
	REFPROP_PATH: string defining the path of the refprop.dll
OUTPUT
	return value: value of variable specified by the input variable what
	props: Array containing all calculated values (props[0] containing error number)
	errormsg: string containing error message
*/
	char statevars[3];
	double p, T, d, val, dl,dv,q,e,h,s,cv,cp,w,wm,wmliq,wmvap,eta,tcx;
	long nX,ierr=0; //zero means no error
    char herr[errormessagelength+1];
//    HINSTANCE RefpropdllInstance;// Then have windows load the library.
//    Poco::SharedLibrary RefpropdllInstance("");

//    DEBUGMODE = 1;

	if (DEBUGMODE) printf("\nStarting function props_REFPROP to calc %c...\n", what[0]);

	//initialize interface to REFPROP.dll
//	if(props[0]=(double)init_REFPROP(fluidnames, REFPROP_PATH, &nX, herr, &RefpropdllInstance, errormsg, DEBUGMODE)){
//		printf("Error no. %g initializing REFPROP: \"%s\"\n", props[0], errormsg);
//		return 0;
//	}
	if(props[0]=(double)init_REFPROP(fluidnames, REFPROP_PATH, &nX, herr, errormsg, DEBUGMODE)){
			printf("Error no. %g initializing REFPROP: \"%s\"\n", props[0], errormsg);
			return 0;
	}

	//CALCULATE MOLAR MASS
//	WMOL = (fp_WMOLdllTYPE) GetProcAddress(RefpropdllInstance,"WMOLdll");
//	WMOLdll_TYPE    * WMOL    = (WMOLdll_TYPE *   ) RefpropdllInstance.getSymbol(WMOLdll_NAME);
	WMOL = (WMOLdll_POINTER) RefpropdllInstance->getSymbol(WMOLdll_NAME);
	WMOL(x,wm);
//	sprintf(errormsg," %10.4f, %10.4f, %10.4f,",x[0],x[1],wm);
	wm /= 1000; //g/mol -> kg/mol

	//identify and assign passed state variables
	statevars[0] = tolower(statevars_in[0]);
	statevars[1] = tolower(statevars_in[1]);
	statevars[2] = '\0';
	if (statevars[0]!='\0'){
		if (statevars[0]==statevars[1]){
			props[0] = 3;
			sprintf(errormsg,"State variable 1 is the same as state variable 2 (%c)",statevars[0]);
			return 0;
		}
		for (int ii=0;ii<2;ii++){
			val = (ii==0?statevar1:statevar2);
			switch(statevars[ii]){
				case 'p':
					p = val/1000; //Pa->kPa
					break;
				case 't':
					T = val;
					break;
				case 's':
					s = val*wm; //J/(kg�K) -> kJ/(mol�K)
					break;
				case 'h':
					h = val*wm; //J/kg --> kJ/mol
					break;
				case 'd':
					d = val/wm/1000; //kg/m� -> mol/dm�
					break;
				case 'q': //vapor quality on a mass basis [mass vapor/total mass] (q=0 indicates saturated liquid, q=1 indicates saturated vapor)
					q = val;
					break;
				default:
					props[0] = 2;
					sprintf(errormsg,"Unknown state variable %i: %c",ii+1 ,statevars[ii]);/**/
					return 0;
			}
		}
	}

/*
//...If phase j is known, call TPRHO:
	long j=2; //phase definition(j=1: Liquid, j=2: Vapor)
	long tmp_int=0;
	TPRHOdll = (fp_TPRHOdllTYPE) GetProcAddress(RefpropdllInstance,"TPRHOdll");
	TPRHO(t,p,x,j,tmp_int,d,ierr,herr,errormessagelength);
*/
//...If phase is not known, call TPFLSH
	double xliq[ncmax],xvap[ncmax],f[ncmax];
	long kq=2;/*  additional input--only for TQFLSH and PQFLSH
      kq--flag specifying units for input quality
	  kq = 1 quality on MOLAR basis [moles vapor/total moles]
	  kq = 2 quality on MASS basis [mass vapor/total mass]*/
//	sprintf(errormsg,"Huhu! %s %d", statevars, ierr);
	if (ierr==0){
		if (strcmp(statevars,"pt")==0 || strcmp(statevars,"tp")==0){
//			strcat(errormsg,"Bin in TP!");
			if (phase==2){ //fluid state is known to be two phase
//				TPFL2dll = (fp_TPFL2dllTYPE) GetProcAddress(RefpropdllInstance,"TPFL2dll");
				TPFL2 = (TPFL2dll_POINTER) RefpropdllInstance->getSymbol(TPFL2dll_NAME);
				TPFL2(T,p,x,dl,dv,xliq,xvap,q,ierr,herr,errormessagelength);
			}else{
//				TPFLSHdll = (fp_TPFLSHdllTYPE) GetProcAddress(RefpropdllInstance,"TPFLSHdll");
				TPFLSH = (TPFLSHdll_POINTER) RefpropdllInstance->getSymbol(TPFLSHdll_NAME);
				TPFLSH(T,p,x,d,dl,dv,xliq,xvap,q,e,h,s,cv,cp,w,ierr,herr,errormessagelength);
			}
		}else if (strcmp(statevars,"ph")==0 || strcmp(statevars,"hp")==0){
//			if (phase==1){ //fluid state is known to be single phase
////				PHFL1dll = (fp_PHFL1dllTYPE) GetProcAddress(RefpropdllInstance,"PHFL1dll");
// 				PHFL1 = (PHFL1dll_POINTER) RefpropdllInstance->getSymbol(PHFL1dll_NAME);
//				PHFL1(p,h,x,liqvap,T,d,ierr,herr,errormessagelength);
////				if (liqvap==1) dl=d; else dv=d;
//			}else{
////				PHFLSHdll = (fp_PHFLSHdllTYPE) GetProcAddress(RefpropdllInstance,"PHFLSHdll");
				PHFLSH = (PHFLSHdll_POINTER) RefpropdllInstance->getSymbol(PHFLSHdll_NAME);
				PHFLSH(p,h,x,T,d,dl,dv,xliq,xvap,q,e,s,cv,cp,w,ierr,herr,errormessagelength);
//			}
		}else if (strcmp(statevars,"pd")==0 || strcmp(statevars,"dp")==0){
			if (phase==1){ //fluid state is known to be single phase
//				PDFL1dll = (fp_PDFL1dllTYPE) GetProcAddress(RefpropdllInstance,"PDFL1dll");
				PDFL1 = (PDFL1dll_POINTER) RefpropdllInstance->getSymbol(PDFL1dll_NAME);
				PDFL1(p,d,x,T,ierr,herr,errormessagelength);
			}else{
//				PDFLSHdll = (fp_PDFLSHdllTYPE) GetProcAddress(RefpropdllInstance,"PDFLSHdll");
				PDFLSH = (PDFLSHdll_POINTER) RefpropdllInstance->getSymbol(PDFLSHdll_NAME);
				PDFLSH(p,d,x,T,dl,dv,xliq,xvap,q,e,h,s,cv,cp,w,ierr,herr,errormessagelength);
			}
		}else if (strcmp(statevars,"ps")==0 || strcmp(statevars,"sp")==0){
/*			if (phase==1){ //fluid state is known to be single phase
				PSFL1dll = (fp_PSFL1dllTYPE) GetProcAddress(RefpropdllInstance,"PSFL1dll");
				PSFL1(p,s,x,kph,T,d,ierr,herr,errormessagelength);
				if (liqvap==1) dl=d; else dv=d;
			}else{*/
//				PSFLSHdll = (fp_PSFLSHdllTYPE) GetProcAddress(RefpropdllInstance,"PSFLSHdll");
				PSFLSH = (PSFLSHdll_POINTER) RefpropdllInstance->getSymbol(PSFLSHdll_NAME);
				PSFLSH(p,s,x,T,d,dl,dv,xliq,xvap,q,e,h,cv,cp,w,ierr,herr,errormessagelength);
//			}
		}else if (strcmp(statevars,"pq")==0 || strcmp(statevars,"qp")==0){
//			PQFLSHdll = (fp_PQFLSHdllTYPE) GetProcAddress(RefpropdllInstance,"PQFLSHdll");
			PQFLSH = (PQFLSHdll_POINTER) RefpropdllInstance->getSymbol(PQFLSHdll_NAME);
			PQFLSH(p,q,x,kq,T,d,dl,dv,xliq,xvap,e,h,s,cv,cp,w,ierr,herr,errormessagelength);
//			strcat(errormsg,"Bin in PQ!");
		}else if (strcmp(statevars,"th")==0 || strcmp(statevars,"ht")==0){
/*			if (phase==1){ //fluid state is known to be single phase
				THFL1dll = (fp_THFL1dllTYPE) GetProcAddress(RefpropdllInstance,"THFL1dll");
				THFL1(T,h,x,Dmin,Dmax,d,ierr,herr,errormessagelength);
			}else{*/
//				THFLSHdll = (fp_THFLSHdllTYPE) GetProcAddress(RefpropdllInstance,"THFLSHdll");
				THFLSH = (THFLSHdll_POINTER) RefpropdllInstance->getSymbol(THFLSHdll_NAME);
				long kr = 2;
/*       kr--phase flag: 1 = input state is liquid
		      2 = input state is vapor in equilibrium with liq
		      3 = input state is liquid in equilibrium with solid
		      4 = input state is vapor in equilibrium with solid */
				THFLSH (T,h,x,kr,p,d,dl,dv,xliq,xvap,q,e,s,cv,cp,w,ierr,herr,errormessagelength);
//			}
		}else if (strcmp(statevars,"td")==0 || strcmp(statevars,"dt")==0){
//			TDFLSHdll = (fp_TDFLSHdllTYPE) GetProcAddress(RefpropdllInstance,"TDFLSHdll");
			TDFLSH = (TDFLSHdll_POINTER) RefpropdllInstance->getSymbol(TDFLSHdll_NAME);
			TDFLSH(T,d,x,p,dl,dv,xliq,xvap,q,e,h,s,cv,cp,w,ierr,herr,errormessagelength);
		}else if (strcmp(statevars,"ts")==0 || strcmp(statevars,"st")==0){
//			TSFLSHdll = (fp_TSFLSHdllTYPE) GetProcAddress(RefpropdllInstance,"TSFLSHdll");
			TSFLSH = (TSFLSHdll_POINTER) RefpropdllInstance->getSymbol(TSFLSHdll_NAME);
			long kr = 2;
			TSFLSH(T,s,x,kr,p,d,dl,dv,xliq,xvap,q,e,h,cv,cp,w,ierr,herr,errormessagelength);
		}else if (strcmp(statevars,"tq")==0 || strcmp(statevars,"qt")==0){
//			TQFLSHdll = (fp_TQFLSHdllTYPE) GetProcAddress(RefpropdllInstance,"TQFLSHdll");
			TQFLSH = (TQFLSHdll_POINTER) RefpropdllInstance->getSymbol(TQFLSHdll_NAME);
			TQFLSH(T,q,x,kq,p,d,dl,dv,xliq,xvap,e,h,s,cv,cp,w,ierr,herr,errormessagelength);
		}else if (strcmp(statevars,"hd")==0 || strcmp(statevars,"dh")==0){
			switch(phase){ //fluid state is known to be single phase
				case 1:
//					DHFL1dll = (fp_DHFL1dllTYPE) GetProcAddress(RefpropdllInstance,"DHFL1dll");
					DHFL1 = (DHFL1dll_POINTER) RefpropdllInstance->getSymbol(DHFL1dll_NAME);
					DHFL1(d,h,x,T,ierr,herr,errormessagelength);
					break;
				case 2:
//					DHFL2dll = (fp_DHFL2dllTYPE) GetProcAddress(RefpropdllInstance,"DHFL2dll");
					DHFL2 = (DHFL2dll_POINTER) RefpropdllInstance->getSymbol(DHFL2dll_NAME);
					DHFL2(d,h,x,T,p,dl,dv,xliq,xvap,q,ierr,herr,errormessagelength);
					break;
				default:
//					DHFLSHdll = (fp_DHFLSHdllTYPE) GetProcAddress(RefpropdllInstance,"DHFLSHdll");
					DHFLSH = (DHFLSHdll_POINTER) RefpropdllInstance->getSymbol(DHFLSHdll_NAME);
					DHFLSH(d,h,x,T,p,dl,dv,xliq,xvap,q,e,s,cv,cp,w,ierr,herr,errormessagelength);
					break;
			}
		}else if (strcmp(statevars,"hs")==0 || strcmp(statevars,"sh")==0){
//			HSFLSHdll = (fp_HSFLSHdllTYPE) GetProcAddress(RefpropdllInstance,"HSFLSHdll");
			HSFLSH = (HSFLSHdll_POINTER) RefpropdllInstance->getSymbol(HSFLSHdll_NAME);
			HSFLSH(h,s,x,T,p,d,dl,dv,xliq,xvap,q,e,cv,cp,w,ierr,herr,errormessagelength);
		}else if (strcmp(statevars,"ds")==0 || strcmp(statevars,"sd")==0){
			switch(phase){ //fluid state is known to be single phase
				case 1:
//					DSFL1dll = (fp_DSFL1dllTYPE) GetProcAddress(RefpropdllInstance,"DSFL1dll");
					DSFL1 = (DSFL1dll_POINTER) RefpropdllInstance->getSymbol(DSFL1dll_NAME);
					DSFL1(d,s,x,T,ierr,herr,errormessagelength);
					break;
				case 2:
//					DSFL2dll = (fp_DSFL2dllTYPE) GetProcAddress(RefpropdllInstance,"DSFL2dll");
					DSFL2 = (DSFL2dll_POINTER) RefpropdllInstance->getSymbol(DSFL2dll_NAME);
					DSFL2(d,s,x,T,p,dl,dv,xliq,xvap,q,ierr,herr,errormessagelength);
					break;
				default:
//					DSFLSHdll = (fp_DSFLSHdllTYPE) GetProcAddress(RefpropdllInstance,"DSFLSHdll");
					DSFLSH = (DSFLSHdll_POINTER) RefpropdllInstance->getSymbol(DSFLSHdll_NAME);
					DSFLSH(d,s,x,T,p,dl,dv,xliq,xvap,q,e,h,cv,cp,w,ierr,herr,errormessagelength);
					break;
			}
		}else
			sprintf(errormsg,"Unknown combination of state variables! %s", statevars);
	}

	switch(tolower(what[0])){ 	//CHOOSE RETURN VARIABLE
		case 'v':	//dynamic viscosity uPa.s
		case 'l':	//thermal conductivity W/m.K
//			TRNPRPdll = (fp_TRNPRPdllTYPE) GetProcAddress(RefpropdllInstance,"TRNPRPdll");
			TRNPRP = (TRNPRPdll_POINTER) RefpropdllInstance->getSymbol(TRNPRPdll_NAME);
			TRNPRP(T,d,x,eta,tcx,ierr,herr,errormessagelength);
			break;
		}


	switch(ierr){
		case 1:
			sprintf(errormsg,"T=%f < Tmin",T);
			break;
		case 4:
			sprintf(errormsg,"P=%f < 0",p);
			break;
		case 5:
			sprintf(errormsg,"T=%f and p=%f out of range",T,p);
			break;
		case 8:
			strcpy(errormsg,"x out of range (component and/or sum < 0 or > 1)");
			break;
		case 9:
			sprintf(errormsg,"x or T=%f out of range",T);
			break;
		case 12:
			sprintf(errormsg,"x out of range and P=%f < 0",p);
			break;
		case 13:
			sprintf(errormsg,"x, T=%f and p=%f out of range",T,p);
			break;
		case 16:
			strcpy(errormsg,"TPFLSH error: p>melting pressure");
			break;
		case -31:
			sprintf(errormsg,"Temperature T=%f out of range for conductivity",T);
			break;
		case -32:
			sprintf(errormsg,"density d=%f out of range for conductivity",d);
			break;
		case -33:
			sprintf(errormsg,"Temperature T=%f and density d=%f out of range for conductivity",T,d);
			break;
		case -41:
			sprintf(errormsg,"Temperature T=%f out of range for viscosity",T);
			break;
		case -42:
			sprintf(errormsg,"density d=%f out of range for viscosity",d);
			break;
		case -43:
			sprintf(errormsg,"Temperature T=%f and density d=%f out of range for viscosity",T,d);
			break;
		case -51:
			sprintf(errormsg,"Temperature T=%f out of range for conductivity and viscosity",T);
			break;
		case -52:
			sprintf(errormsg,"density d=%f out of range for conductivity and viscosity",d);
			break;
		case -53:
			sprintf(errormsg,"Temperature T=%f and density d=%f out of range for conductivity and viscosity",T,d);
			break;
		case 39:
			sprintf(errormsg,"model not found for thermal conductivity");
			break;
		case 49:
			sprintf(errormsg,"model not found for viscosity");
			break;
		case 50:
			sprintf(errormsg,"ammonia/water mixture (no properties calculated)");
			break;
		case 51:
			sprintf(errormsg,"exactly at T=%f, rhoc for a pure fluid; k is infinite",T);
			break;
		case -58:
		case -59:
			sprintf(errormsg,"ECS model did not converge");
			break;
		case 211:
			sprintf(errormsg,"TPFLSH bubble point calculation did not converge: [SATTP error 1] iteration failed to converge");
			break;
		case 239:
			sprintf(errormsg,"THFLSH error: Input value of enthalpy (%f) is outside limits",h);
			break;
		case 248:
			sprintf(errormsg,"DSFLSH error: Iteration did not converge with d=%f and s=%f",d,s);
			break;
		case 249:
			sprintf(errormsg,"PHFLSH error: Input value of enthalpy (%f) is outside limits",h);
			break;
		case 271:
			sprintf(errormsg,"TQFLSH error: T=%f > Tcrit, T-q calculation not possible",T);
			break;
		case 291:
			sprintf(errormsg,"PQFLSH error: p=%f > pcrit, p-q calculation not possible",T);
			break;
		default:
			strncpy(errormsg,herr,errormessagelength);
			break;
	}


	//CONVERT TO SI-UNITS
	if (ierr==0){
		WMOL(xliq,wmliq);
		wmliq /= 1000; //g/mol -> kg/mol
		WMOL(xvap,wmvap);
		wmvap /= 1000; //g/mol -> kg/mol
		//printf("%d,%s\n%s\nP,T,D,H,CP            %10.4f,%10.4f,%10.4f,%10.4f,%10.4f\n",nX,hf,hfmix,p,t,d,h,wm);
		d *= wm*1000; //mol/dm� -> kg/m�
		dl *= wmliq*1000; //mol/dm� -> kg/m�
		dv *= wmvap*1000; //mol/dm� -> kg/m�
		e /= e/wm;	//kJ/mol -> J/kg
		h /= wm; 	//kJ/mol -> J/kg
		s /= wm;	//kJ/(mol�K) -> J/(kg�K)
		cv /= wm;
		cp /= wm;
		p *= 1000; //kPa->Pa
		if (nX>1 && abs(q)<990) q *= wmvap/wm; //molar bass -> mass basis
		eta/=1e6;	//uPa.s -> Pa.s
	}

	//ASSIGN VALUES TO RETURN ARRAY
	props[0] = ierr;//error code
	props[1] = p;//pressure in Pa
	props[2] = T;	//Temperature in K
	props[3] = wm;	//molecular weight
	props[4] = d;	//density
	props[5] = dl;	//density of liquid phase
	props[6] = dv;	//density of liquid phase
	props[7] = q;	//vapor quality on a mass basis [mass vapor/total mass] (q=0 indicates saturated liquid, q=1 indicates saturated vapor)
	props[8] = e;	//inner energy
	props[9] = h;	//specific enthalpy
	props[10] = s;//specific entropy
	props[11] = cv;
	props[12] = cp;
	props[13] = w; //speed of sound
	props[14] = wmliq;
	props[15] = wmvap;
	for (int ii=0;ii<nX;ii++){
		props[16+ii] = xliq[ii];
		props[16+nX+ii] = xvap[ii];
	}

	if (DEBUGMODE) printf("Returning %c\n",what[0]);

	switch(tolower(what[0])){ 	//CHOOSE RETURN VARIABLE
		case 'p': //molecular weight
			return p;
		case 't': //molecular weight
			return T;
		case 'm': //molecular weight
			return wm;
		case 'd': //density
			return d;
		case 'q': //quality
			return q;
		case 'e':	//inner energy
			return e;
		case 'h': //specific enthalpy
			return h;
		case 's':	//specific entropy
			return s;
		case 'w':	//speed of sound
			return w;
		case 'v':	//dynamic viscosity uPa.s
			return eta;
		case 'l':	//thermal conductivity W/m.K
			return tcx;
		default:
			return 1.0;
	}
}


//---------------------------------------------------------------------------


double satprops_REFPROP(char* what, char* statevar_in, char* fluidnames, double *props, double statevarval, double* x, char* REFPROP_PATH, char* errormsg, int DEBUGMODE){
/*Calculates thermodynamic saturation properties of a pure substance/mixture, returns both single value and array containing all calculated values (because the are calculated anyway)
INPUT:
	what: character specifying return value (p,T,h,s,d,wm,q,e,w) - Explanation of variables at the end of this function
	statevar: string of 1 variable out of p,T,h,s,d
	fluidnames: string containing names of substances in mixtured separated by |, substance names are identical to those of *.fld-files in REFPROP program directory
	statevarval: values of the variable specified in statevar
	x: array containing the mass fractions of the components of the mixture
	REFPROP_PATH: string defining the path of the refprop.dll
OUTPUT
	return value: value of variable specified by the input variable what
	props: Array containing all calculated values
	errormsg: string containing error message
*/
	double p, T, d, val, dl,dv,wm,wmliq,wmvap;
	long nX,ierr=0;
    char herr[errormessagelength+1];
//    HINSTANCE RefpropdllInstance;
//    void* RefpropdllInstance;
//    Poco::SharedLibrary RefpropdllInstance("");

//  DEBUGMODE = 1;

	if (DEBUGMODE)  printf("\nStarting function satprops_REFPROP...\n");

	//initialize interface to REFPROP.dll
	if(props[0]=(double)init_REFPROP(fluidnames, REFPROP_PATH, &nX, herr, errormsg, DEBUGMODE)){
		printf("Error no. %f initializing REFPROP: \"%s\"\n", props[0], errormsg);
		return 0;
	}

	if (DEBUGMODE)  printf("\nFunction init_REFPROP was called\n");

	//CALCULATE MOLAR MASS
//	WMOLdll = (fp_WMOLdllTYPE) GetProcAddress(RefpropdllInstance,"WMOLdll");
	WMOL = (WMOLdll_POINTER) RefpropdllInstance->getSymbol(WMOLdll_NAME);
	WMOL(x,wm);
//	sprintf(errormsg," %10.4f, %10.4f, %10.4f,",x[0],x[1],wm);
	if (DEBUGMODE)  printf("\nFunction WMOLdll was called\n");

	wm /= 1000; //g/mol -> kg/mol

	if (DEBUGMODE)  printf("wm converted.\n");

	if (DEBUGMODE)  printf("statevar is %s \n",statevar_in);
	//identify and assign passed state variables
	//	char tmpstr[1];
	//	strcpy(tmpstr,statevar[0]);
	//	statevar = toLowerCase(tmpstr);
	//statevar[0] = tolower(statevar[0]);
	char statevar[1];
	statevar[0] = tolower(statevar_in[0]);
	if (DEBUGMODE)  printf("\nstatevar lowercase.\n");
	if (statevar[0]!='\0'){
//	if (strcmp(statevar[0],"")){
//	if (statevar[0] != NULL){
		if (DEBUGMODE)  printf("\nentering statevar switch.\n");
			switch(statevar[0]){
				case 'p':
					p = statevarval/1000; //Pa->kPa
					break;
				case 't':
					T = statevarval;
					break;
				case 'd':
					d = statevarval/wm/1000; //kg/m� -> mol/dm�
					break;
/*				case 's':
					s = statevarval*wm; //J/(kg�K) -> kJ/(mol�K)
					break;
				case 'h':
					h = statevarval*wm; //J/kg --> kJ/mol
					break;
*/				default:
					props[0] = 2;
					sprintf(errormsg,"Unknown state variable: %f", statevarval);
					return 0;
			}
	}

	if (DEBUGMODE)  printf("\nstatevar checked.\n");

	double xliq[ncmax],xvap[ncmax],f[ncmax];

	long j=2,kr;
/* j--phase flag: 1 = input x is liquid composition (bubble point)
		    2 = input x is vapor composition (dew point)
		    3 = input x is liquid composition (freezing point)
		    4 = input x is vapor composition (sublimation point)
*/
	if (ierr==0) {
		if (~strcmp(statevar,"t")){
//			SATTdll = (fp_SATTdllTYPE) GetProcAddress(RefpropdllInstance,"SATTdll");
			SATT = (SATTdll_POINTER) RefpropdllInstance->getSymbol(SATTdll_NAME);
			SATT(T,x,j,p,dl,dv,xliq,xvap,ierr,herr,errormessagelength);
		}else if (~strcmp(statevar,"p")){
//			SATPdll = (fp_SATPdllTYPE) GetProcAddress(RefpropdllInstance,"SATPdll");
			SATP = (SATPdll_POINTER) RefpropdllInstance->getSymbol(SATPdll_NAME);
			SATP(p,x,j,T,dl,dv,xliq,xvap,ierr,herr,errormessagelength);
			switch(ierr){
				case 2:
					strcpy(errormsg,"P < Ptp");
					break;
				case 4:
					strcpy(errormsg,"P < 0");
					break;
			}
			//sprintf(errormsg,"p=%f, h=%f",p ,statevar2);
		}else if (~strcmp(statevar,"d")){
//			SATDdll = (fp_SATDdllTYPE) GetProcAddress(RefpropdllInstance,"SATDdll");
			SATD = (SATDdll_POINTER) RefpropdllInstance->getSymbol(SATDdll_NAME);
			SATD(d,x,j,kr,T,p,dl,dv,xliq,xvap,ierr,herr,errormessagelength);
			switch(ierr){
				case 2:
					strcpy(errormsg,"D > Dmax");
					break;
			}
		}
	}

	switch(ierr){
		case 0:
			strcpy(errormsg,"Saturation routine successful");
			break;
		case 1:
			sprintf(errormsg,"T=%f < Tmin",T);
			break;
		case 8:
			strcpy(errormsg,"x out of range");
			break;
		case 9:
			strcpy(errormsg,"T and x out of range");
			break;
		case 10:
			strcpy(errormsg,"D and x out of range");
			break;
		case 12:
			strcpy(errormsg,"P and x out of range");
			break;
		case 120:
			strcpy(errormsg,"CRITP did not converge");
			break;
		case 121:
			strcpy(errormsg,"T > Tcrit");
			break;
		case 122:
			strcpy(errormsg,"TPRHO-liquid did not converge (pure fluid)");
			break;
		case 123:
			strcpy(errormsg,"TPRHO-vapor did not converge (pure fluid)");
			break;
		case 124:
			strcpy(errormsg,"pure fluid iteration did not converge");
			break;
		case -125:
			strcpy(errormsg,"TPRHO did not converge for parent ph (mix)");
			break;
		case -126:
			strcpy(errormsg,"TPRHO did not converge for incipient (mix)");
			break;
		case -127:
			strcpy(errormsg,"composition iteration did not converge");
			break;
		case 128:
			strcpy(errormsg,"mixture iteration did not converge");
			break;
		case 140:
			strcpy(errormsg,"CRITP did not converge");
			break;
		case 141:
			strcpy(errormsg,"P > Pcrit");
			break;
		case 142:
			strcpy(errormsg,"TPRHO-liquid did not converge (pure fluid)");
			break;
		case 143:
			strcpy(errormsg,"TPRHO-vapor did not converge (pure fluid)");
			break;
		case 144:
			strcpy(errormsg,"pure fluid iteration did not converge");
			break;
		case -144:
			strcpy(errormsg,"Raoult's law (mixture initial guess) did not converge");
			break;
		case -145:
			strcpy(errormsg,"TPRHO did not converge for parent ph (mix)");
			break;
		case -146:
			strcpy(errormsg,"TPRHO did not converge for incipient (mix)");
			break;
		case -147:
			strcpy(errormsg,"composition iteration did not converge");
			break;
		case 148:
			strcpy(errormsg,"mixture iteration did not converge");
			break;
		case 160:
			strcpy(errormsg,"CRITP did not converge");
			break;
		case 161:
			strcpy(errormsg,"SATD did not converge");
			break;
		default:
			strncpy(errormsg,herr,errormessagelength);
			break;
	}

	/*SATHdll = (fp_SATHdllTYPE) GetProcAddress(RefpropdllInstance,"SATHdll");
	SATEdll = (fp_SATEdllTYPE) GetProcAddress(RefpropdllInstance,"SATEdll");
	SATSdll = (fp_SATSdllTYPE) GetProcAddress(RefpropdllInstance,"SATSdll");*/


	//CONVERT TO SI-UNITS
	if (ierr==0){
		WMOL(xliq,wmliq);
		wmliq /= 1000; //g/mol -> kg/mol
		WMOL(xvap,wmvap);
		wmvap /= 1000; //g/mol -> kg/mol
		//printf("%d,%s\n%s\nP,T,D,H,CP            %10.4f,%10.4f,%10.4f,%10.4f,%10.4f\n",nX,hf,hfmix,p,t,d,h,wm);
		d *= wm*1000; //mol/dm� -> kg/m�
		dl *= wmliq*1000; //mol/dm� -> kg/m�
		dv *= wmvap*1000; //mol/dm� -> kg/m�
/*		e /= e/wm;	//kJ/mol -> J/kg
		h /= wm; 	//kJ/mol -> J/kg
		s /= wm;	//kJ/(mol�K) -> J/(kg�K)
*/		p *= 1000; //kPa->Pa
	}


		//ASSIGN VALUES TO RETURN ARRAY
	props[0] = ierr;//error code
	props[1] = p;//pressure in kPa->Pa
	props[2] = T;	//Temperature in K
	props[3] = wm;	//molecular weight
	props[4] = d;	//density
	props[5] = dl;	//density of liquid phase
	props[6] = dv;	//density of liquid phase
	props[7] = 0;
	props[8] = 0;	//inner energy
	props[9] = 0;	//specific enthalpy
	props[10] = 0;//specific entropy
	props[11] = 0;
	props[12] = 0;
	props[13] = 0; //speed of sound
	props[14] = wmliq;
	props[15] = wmvap;
	for (int ii=0;ii<nX;ii++){
		props[16+ii] = xliq[ii];
		props[16+nX+ii] = xvap[ii];
	}


		if (DEBUGMODE) printf("Returning %c\n",what[0]);

		switch(tolower(what[0])){//CHOOSE RETURN VARIABLE
		case 'p': //molecular weight
			return p;
		case 't': //molecular weight
			return T;
		case 'm': //molecular weight
			return wm;
		case 'd': //density
			return d;
/*		case 'h': //specific enthalpy
			return h;
		case 's':	//specific entropy
			return s;*/
		default:
			props[0] = 2;
			sprintf(errormsg,"Cannot return variable %c in saturation calculation", what[0]);
			return 0;
	}
}
