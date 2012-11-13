/*
	wrapper file for refprop.
	
	Based on earlier work by Henning Francke (francke@gfz-potsdam.de)
	and the CoolProp wrapper by Ian Bell (ian.h.bell@gmail.com).
	
	"Copied and pasted" by Jorrit Wronski (jowr@mek.dtu.dk)
	
*/

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif

// // #include "REFPROP.h"
// //#include "refprop_lib.h"
// #include "refprop_constants.h"
// #include "refprop_names.h"
// #include "refprop_cfunctions.h"
#define refpropcharlong 10000
// Some constants for REFPROP... defined by macros for ease of use 
#define refpropcharlength 255
#define filepathlength 255
#define lengthofreference 3
#define errormessagelength 255
#define ncmax 20		// Note: ncmax is the max number of components
#define numparams 72 
#define maxcoefs 50

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <ctype.h> // tolower etc

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
//
//
//
ABFL1dll_POINTER ABFL1lib = NULL;
ABFL2dll_POINTER ABFL2lib = NULL;
ACTVYdll_POINTER ACTVYlib = NULL;
AGdll_POINTER AGlib = NULL;
CCRITdll_POINTER CCRITlib = NULL;
CP0dll_POINTER CP0lib = NULL;
CRITPdll_POINTER CRITPlib = NULL;
CSATKdll_POINTER CSATKlib = NULL;
CV2PKdll_POINTER CV2PKlib = NULL;
CVCPKdll_POINTER CVCPKlib = NULL;
CVCPdll_POINTER CVCPlib = NULL; 
DBDTdll_POINTER DBDTlib = NULL;
DBFL1dll_POINTER DBFL1lib = NULL;
DBFL2dll_POINTER DBFL2lib = NULL;
DDDPdll_POINTER DDDPlib = NULL;
DDDTdll_POINTER DDDTlib = NULL;
DEFLSHdll_POINTER DEFLSHlib = NULL;
DHD1dll_POINTER DHD1lib = NULL;
DHFLSHdll_POINTER DHFLSHlib = NULL;
DIELECdll_POINTER DIELEClib = NULL;
DOTFILLdll_POINTER DOTFILLlib = NULL;
DPDD2dll_POINTER DPDD2lib = NULL;
DPDDKdll_POINTER DPDDKlib = NULL;
DPDDdll_POINTER DPDDlib = NULL;
DPDTKdll_POINTER DPDTKlib = NULL;
DPDTdll_POINTER DPDTlib = NULL;
DPTSATKdll_POINTER DPTSATKlib = NULL;
DSFLSHdll_POINTER DSFLSHlib = NULL;
ENTHALdll_POINTER ENTHALlib = NULL; //**
ENTROdll_POINTER ENTROlib = NULL;
ESFLSHdll_POINTER ESFLSHlib = NULL;
FGCTYdll_POINTER FGCTYlib = NULL;
FPVdll_POINTER FPVlib = NULL;
GERG04dll_POINTER GERG04lib = NULL;
GETFIJdll_POINTER GETFIJlib = NULL;
GETKTVdll_POINTER GETKTVlib = NULL;
GIBBSdll_POINTER GIBBSlib = NULL;
HSFLSHdll_POINTER HSFLSHlib = NULL;
INFOdll_POINTER INFOlib = NULL;
LIMITKdll_POINTER LIMITKlib = NULL;
LIMITSdll_POINTER LIMITSlib = NULL;
LIMITXdll_POINTER LIMITXlib = NULL;
MELTPdll_POINTER MELTPlib = NULL;
MELTTdll_POINTER MELTTlib = NULL;
MLTH2Odll_POINTER MLTH2Olib = NULL;
NAMEdll_POINTER NAMElib = NULL;
PDFL1dll_POINTER PDFL1lib = NULL;
PDFLSHdll_POINTER PDFLSHlib = NULL;
PEFLSHdll_POINTER PEFLSHlib = NULL;
PHFL1dll_POINTER PHFL1lib = NULL;
PHFLSHdll_POINTER PHFLSHlib = NULL;
PQFLSHdll_POINTER PQFLSHlib = NULL;
PREOSdll_POINTER PREOSlib = NULL;
PRESSdll_POINTER PRESSlib = NULL;
PSFL1dll_POINTER PSFL1lib = NULL;
PSFLSHdll_POINTER PSFLSHlib = NULL;
PUREFLDdll_POINTER PUREFLDlib = NULL;
QMASSdll_POINTER QMASSlib = NULL;
QMOLEdll_POINTER QMOLElib = NULL;
SATDdll_POINTER SATDlib = NULL;
SATEdll_POINTER SATElib = NULL;
SATHdll_POINTER SATHlib = NULL;
SATPdll_POINTER SATPlib = NULL;
SATSdll_POINTER SATSlib = NULL;
SATTdll_POINTER SATTlib = NULL;
SETAGAdll_POINTER SETAGAlib = NULL;
SETKTVdll_POINTER SETKTVlib = NULL;
SETMIXdll_POINTER SETMIXlib = NULL;
SETMODdll_POINTER SETMODlib = NULL;
SETREFdll_POINTER SETREFlib = NULL;
SETUPdll_POINTER SETUPlib = NULL;
//SPECGRdll_POINTER SPECGRlib = NULL;
SUBLPdll_POINTER SUBLPlib = NULL;
SUBLTdll_POINTER SUBLTlib = NULL;
SURFTdll_POINTER SURFTlib = NULL;
SURTENdll_POINTER SURTENlib = NULL;
TDFLSHdll_POINTER TDFLSHlib = NULL;
TEFLSHdll_POINTER TEFLSHlib = NULL;
THERM0dll_POINTER THERM0lib = NULL;
THERM2dll_POINTER THERM2lib = NULL;
THERM3dll_POINTER THERM3lib = NULL;
THERMdll_POINTER THERMlib = NULL;
THFLSHdll_POINTER THFLSHlib = NULL;
TPFLSHdll_POINTER TPFLSHlib = NULL;
TPRHOdll_POINTER TPRHOlib = NULL;
TQFLSHdll_POINTER TQFLSHlib = NULL;
TRNPRPdll_POINTER TRNPRPlib = NULL;
TSFLSHdll_POINTER TSFLSHlib = NULL;
VIRBdll_POINTER VIRBlib = NULL;
VIRCdll_POINTER VIRClib = NULL;
WMOLdll_POINTER WMOLlib = NULL;
XMASSdll_POINTER XMASSlib = NULL;
XMOLEdll_POINTER XMOLElib = NULL;

// double REFPROP(char Output,char Name1, double Prop1, char Name2, double Prop2, double* xkg, char * Ref, char * Path, char * herr, int DEBUGMODE);

// Load the library
char LoadedREFPROPRef[2550];
// HINSTANCE RefpropdllInstance=NULL;
Poco::SharedLibrary *RefpropdllInstance = NULL;

// global variables for array
double T,p,d,dl,dv,dl_,dv_,q,e,h,s,cv,cp,w,MW,hl,hv,sl,sv,ul,
		uv,pl,pv,hjt,eta,tcx,Q,Tcrit,pcrit,dcrit,rho,sigma;
double x[ncmax],xliq[ncmax],xvap[ncmax];
long i,ierr;

int init_REFPROP(char* fluidnames, char* REFPROP_PATH_CHAR, long* nX, char* herr, char* errormsg, int DEBUGMODE){
// Sets up the interface to the REFPROP.DLL
// is called by props_REFPROP and satprops_REFPROP
//	char DLL_PATH[filepathlength], FLD_PATH[filepathlength];
return 0;
}

int getProps(double *props) {  
	double MWliq,MWvap;
  
	//CONVERT TO SI-UNITS
	if (ierr==0){
		WMOLlib(xliq,MWliq);
		WMOLlib(xvap,MWvap);
	}

	//ASSIGN VALUES TO RETURN ARRAY
	props[0] = ierr;  	//error code
	props[1] = p*1000;	//kPa -> Pa
	props[2] = T;		//Temperature in K
	props[3] = MW/1000;	//molecular weight, g/mol -> kg/mol
	props[4] = d*MW;	//density, mol/l -> kg/m3
	props[5] = dl*MWliq;	//density of liquid phase, mol/l -> kg/m3
	props[6] = dv*MWvap;	//density of vapour phase, mol/l -> kg/m3
	props[7] = q*MWvap/MW;	//vapor quality on a mass basis [mass vapor/total mass] (q=0 indicates saturated liquid, q=1 indicates saturated vapor)
	props[8] = e/MW;	//inner energy, J/mol -> J/kg
	props[9] = h/MW;	//specific enthalpy, J/mol -> J/kg
	props[10] = s/MW;	//specific entropy, J/molK -> J/kgK
	props[11] = cv/MW;	// heat capacity
	props[12] = cp/MW;	// heat capacity
	props[13] = w; 		//speed of sound
	props[14] = MWliq/1000;
	props[15] = MWvap/1000;
	for (int ii=0;ii<i;ii++){
		props[16+ii] = xliq[ii];
		props[16+i+ii] = xvap[ii];
	}
	return 0;  
}

double props_REFPROP(char* what, char* statevars_in, char* fluidnames, double *props, double statevar1, double statevar2, double* xkg, int phase, char* REFPROP_PATH, char* errormsg, int DEBUGMODE)
{
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
	double ret = REFPROP(toupper(what[0]),toupper(statevars_in[0]),statevar1,toupper(statevars_in[1]),statevar2,xkg,fluidnames,REFPROP_PATH,errormsg,DEBUGMODE);
	int ex = getProps(props);
return ret;
}


double satprops_REFPROP(char* what, char* statevar, char* fluidnames, double *props, double statevarval, double* xkg, char* REFPROP_PATH, char* errormsg, int DEBUGMODE){
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
	//double p, T, d, val, dl,dv,wm,wmliq,wmvap;
    // This is a bogus function call
	double ret = REFPROP(toupper(what[0]),toupper(statevar[0]),statevarval,'Q',0.5,xkg,fluidnames,REFPROP_PATH,errormsg,DEBUGMODE);
	int ex = getProps(props);
return ret;
}



double REFPROP(char Output,char Name1, double Prop1, char Name2, double Prop2, double* xkg, char * Ref, char* RefpropPath, char* herr, int DEBUGMODE)
{  
	int j;
	i=0;
	ierr=0;
	//char hf[refpropcharlength*ncmax], hrf[lengthofreference+1],
	char hf[refpropcharlong], hrf[lengthofreference+1];
	// herr[errormessagelength+1]
	char hfmix[refpropcharlength+1];
	char path[refpropcharlength+1];
	
	p=0;
	// double x[ncmax],xliq[ncmax],xvap[ncmax];
	x[0] = 0.0;
	
	char RefString[255];
		
        
	if (strlen(RefpropPath)>filepathlength){
		sprintf(herr,"Path too long (%i > %i)\n",strlen(RefpropPath),filepathlength);
		return 0;
	}
	// Define temporary objects for checks.
	Poco::Path REF_PATH(true);
	
	char FLUIDS_CHAR[filepathlength+1] = "fluids";
	Poco::Path FLD_PATH(true);
	char LIBRARY_CHAR[filepathlength+1];
	Poco::Path LIB_PATH(true);

	// Parse the string and append a path separator if necessary.
	REF_PATH.parse(RefpropPath, Poco::Path::PATH_NATIVE);
	if (!REF_PATH.isDirectory()) REF_PATH.append(REF_PATH.separator());
	// Overwrite the provided path
	strcpy(path,REF_PATH.toString().c_str());

	// Check the path if running in debugmode
	if (DEBUGMODE) {
	  Poco::File refFile(REF_PATH);
	  if ( !refFile.isDirectory() || !refFile.canRead() ){
	    printf ("\nREF_PATH is not a readable directory: %s \n", REF_PATH.toString().c_str());
	    sprintf (herr,"\nREF_PATH is not a readable directory: %s \n", REF_PATH.toString().c_str());
	    return 0;
	  } else {
	    printf ("\nREF_PATH is a readable directory: %s \n", REF_PATH.toString().c_str());
	  }
	}

	// The fluid files are in the Refprop directory, append "fluids".
	FLD_PATH.parse(path);
	FLD_PATH.pushDirectory(FLUIDS_CHAR);
	
	// First create a pointer to an instance of the library
	// Then have windows load the library.
	
	Poco::Path SRC_PATH; // This is not fail-safe, the OS might look somewhere else, e.g. /usr/lib ...
	// If REFPROP is not loaded, try to load it
	if (RefpropdllInstance==NULL)
	{
		if (DEBUGMODE) printf ("RefpropdllInstance loaded: %s \n", "false");
		// Check the OS and assign the right names for the library 
		bool is_linux = ( 0 == Poco::icompare(Poco::Environment::osName(), "linux") );
		if (is_linux){
		  strcpy(LIBRARY_CHAR,"librefprop.so");
		  //SRC_PATH.parse("/usr/local/lib");
		} else {
		  strcpy(LIBRARY_CHAR,"refprop.dll");
		  //SRC_PATH = REF_PATH;
		} 
		SRC_PATH = REF_PATH;
		
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
		    sprintf (herr,"LIB_PATH is not a readable file: %s \n", LIB_PATH.toString().c_str());
		    return 0;
		  } else {
		    printf ("LIB_PATH exists and is readable: %s \n", LIB_PATH.toString().c_str());
		  }
		}
		
		// load a new library instance
		RefpropdllInstance = new Poco::SharedLibrary(LIB_PATH.toString());
		if (RefpropdllInstance==NULL)
		{
		  printf("Could not load REFPROP. \n");
		  return -1;
		}
		
		//DHFL1 = (DHFL1dll_POINTER) RefpropdllInstance->getSymbol(DHFL1dll_NAME);
		//DHFL1(d,h,x,T,ierr,herr,errormessagelength);
		
		//ABFL1dll_POINTER {aka void (*)(double&, double&, double*, long int&, double&, double&, double&, double&, double&, double&, long int&, char*, long int)}’ to ‘
		//ABFL1dll_TYPE    {aka void    (double&, double&, double*, long int&, double&, double&, double&, double&, double&, double&, long int&, char*, long int)}’ 

		// Then get pointers into the dll to the actual functions.
		ABFL1lib = (ABFL1dll_POINTER) RefpropdllInstance->getSymbol(ABFL1dll_NAME);
		ABFL2lib = (ABFL2dll_POINTER) RefpropdllInstance->getSymbol(ABFL2dll_NAME);
		ACTVYlib = (ACTVYdll_POINTER) RefpropdllInstance->getSymbol(ACTVYdll_NAME);
		AGlib = (AGdll_POINTER) RefpropdllInstance->getSymbol(AGdll_NAME);
		CCRITlib = (CCRITdll_POINTER) RefpropdllInstance->getSymbol(CCRITdll_NAME);
		CP0lib = (CP0dll_POINTER) RefpropdllInstance->getSymbol(CP0dll_NAME);
		CRITPlib = (CRITPdll_POINTER) RefpropdllInstance->getSymbol(CRITPdll_NAME);
		CSATKlib = (CSATKdll_POINTER) RefpropdllInstance->getSymbol(CSATKdll_NAME);
		CV2PKlib = (CV2PKdll_POINTER) RefpropdllInstance->getSymbol(CV2PKdll_NAME);
		CVCPKlib = (CVCPKdll_POINTER) RefpropdllInstance->getSymbol(CVCPKdll_NAME);
		CVCPlib = (CVCPdll_POINTER) RefpropdllInstance->getSymbol(CVCPdll_NAME); 
		DBDTlib = (DBDTdll_POINTER) RefpropdllInstance->getSymbol(DBDTdll_NAME);
		DBFL1lib = (DBFL1dll_POINTER) RefpropdllInstance->getSymbol(DBFL1dll_NAME);
		DBFL2lib = (DBFL2dll_POINTER) RefpropdllInstance->getSymbol(DBFL2dll_NAME);
		DDDPlib = (DDDPdll_POINTER) RefpropdllInstance->getSymbol(DDDPdll_NAME);
		DDDTlib = (DDDTdll_POINTER) RefpropdllInstance->getSymbol(DDDTdll_NAME);
		DEFLSHlib = (DEFLSHdll_POINTER) RefpropdllInstance->getSymbol(DEFLSHdll_NAME);
		DHD1lib = (DHD1dll_POINTER) RefpropdllInstance->getSymbol(DHD1dll_NAME);
		DHFLSHlib = (DHFLSHdll_POINTER) RefpropdllInstance->getSymbol(DHFLSHdll_NAME);
		DIELEClib = (DIELECdll_POINTER) RefpropdllInstance->getSymbol(DIELECdll_NAME);
		DOTFILLlib = (DOTFILLdll_POINTER) RefpropdllInstance->getSymbol(DOTFILLdll_NAME);
		DPDD2lib = (DPDD2dll_POINTER) RefpropdllInstance->getSymbol(DPDD2dll_NAME);
		DPDDKlib = (DPDDKdll_POINTER) RefpropdllInstance->getSymbol(DPDDKdll_NAME);
		DPDDlib = (DPDDdll_POINTER) RefpropdllInstance->getSymbol(DPDDdll_NAME);
		DPDTKlib = (DPDTKdll_POINTER) RefpropdllInstance->getSymbol(DPDTKdll_NAME);
		DPDTlib = (DPDTdll_POINTER) RefpropdllInstance->getSymbol(DPDTdll_NAME);
		DPTSATKlib = (DPTSATKdll_POINTER) RefpropdllInstance->getSymbol(DPTSATKdll_NAME);
		DSFLSHlib = (DSFLSHdll_POINTER) RefpropdllInstance->getSymbol(DSFLSHdll_NAME);
		ENTHALlib = (ENTHALdll_POINTER) RefpropdllInstance->getSymbol(ENTHALdll_NAME); //**
		ENTROlib = (ENTROdll_POINTER) RefpropdllInstance->getSymbol(ENTROdll_NAME);
		ESFLSHlib = (ESFLSHdll_POINTER) RefpropdllInstance->getSymbol(ESFLSHdll_NAME);
		FGCTYlib = (FGCTYdll_POINTER) RefpropdllInstance->getSymbol(FGCTYdll_NAME);
		FPVlib = (FPVdll_POINTER) RefpropdllInstance->getSymbol(FPVdll_NAME);
		GERG04lib = (GERG04dll_POINTER) RefpropdllInstance->getSymbol(GERG04dll_NAME);
		GETFIJlib = (GETFIJdll_POINTER) RefpropdllInstance->getSymbol(GETFIJdll_NAME);
		GETKTVlib = (GETKTVdll_POINTER) RefpropdllInstance->getSymbol(GETKTVdll_NAME);
		GIBBSlib = (GIBBSdll_POINTER) RefpropdllInstance->getSymbol(GIBBSdll_NAME);
		HSFLSHlib = (HSFLSHdll_POINTER) RefpropdllInstance->getSymbol(HSFLSHdll_NAME);
		INFOlib = (INFOdll_POINTER) RefpropdllInstance->getSymbol(INFOdll_NAME);
		LIMITKlib = (LIMITKdll_POINTER) RefpropdllInstance->getSymbol(LIMITKdll_NAME);
		LIMITSlib = (LIMITSdll_POINTER) RefpropdllInstance->getSymbol(LIMITSdll_NAME);
		LIMITXlib = (LIMITXdll_POINTER) RefpropdllInstance->getSymbol(LIMITXdll_NAME);
		MELTPlib = (MELTPdll_POINTER) RefpropdllInstance->getSymbol(MELTPdll_NAME);
		MELTTlib = (MELTTdll_POINTER) RefpropdllInstance->getSymbol(MELTTdll_NAME);
		MLTH2Olib = (MLTH2Odll_POINTER) RefpropdllInstance->getSymbol(MLTH2Odll_NAME);
		NAMElib = (NAMEdll_POINTER) RefpropdllInstance->getSymbol(NAMEdll_NAME);
		PDFL1lib = (PDFL1dll_POINTER) RefpropdllInstance->getSymbol(PDFL1dll_NAME);
		PDFLSHlib = (PDFLSHdll_POINTER) RefpropdllInstance->getSymbol(PDFLSHdll_NAME);
		PEFLSHlib = (PEFLSHdll_POINTER) RefpropdllInstance->getSymbol(PEFLSHdll_NAME);
		PHFL1lib = (PHFL1dll_POINTER) RefpropdllInstance->getSymbol(PHFL1dll_NAME);
		PHFLSHlib = (PHFLSHdll_POINTER) RefpropdllInstance->getSymbol(PHFLSHdll_NAME);
		PQFLSHlib = (PQFLSHdll_POINTER) RefpropdllInstance->getSymbol(PQFLSHdll_NAME);
		PREOSlib = (PREOSdll_POINTER) RefpropdllInstance->getSymbol(PREOSdll_NAME);
		PRESSlib = (PRESSdll_POINTER) RefpropdllInstance->getSymbol(PRESSdll_NAME);
		PSFL1lib = (PSFL1dll_POINTER) RefpropdllInstance->getSymbol(PSFL1dll_NAME);
		PSFLSHlib = (PSFLSHdll_POINTER) RefpropdllInstance->getSymbol(PSFLSHdll_NAME);
		PUREFLDlib = (PUREFLDdll_POINTER) RefpropdllInstance->getSymbol(PUREFLDdll_NAME);
		QMASSlib = (QMASSdll_POINTER) RefpropdllInstance->getSymbol(QMASSdll_NAME);
		QMOLElib = (QMOLEdll_POINTER) RefpropdllInstance->getSymbol(QMOLEdll_NAME);
		SATDlib = (SATDdll_POINTER) RefpropdllInstance->getSymbol(SATDdll_NAME);
		SATElib = (SATEdll_POINTER) RefpropdllInstance->getSymbol(SATEdll_NAME);
		SATHlib = (SATHdll_POINTER) RefpropdllInstance->getSymbol(SATHdll_NAME);
		SATPlib = (SATPdll_POINTER) RefpropdllInstance->getSymbol(SATPdll_NAME);
		SATSlib = (SATSdll_POINTER) RefpropdllInstance->getSymbol(SATSdll_NAME);
		SATTlib = (SATTdll_POINTER) RefpropdllInstance->getSymbol(SATTdll_NAME);
		SETAGAlib = (SETAGAdll_POINTER) RefpropdllInstance->getSymbol(SETAGAdll_NAME);
		SETKTVlib = (SETKTVdll_POINTER) RefpropdllInstance->getSymbol(SETKTVdll_NAME);
		SETMIXlib = (SETMIXdll_POINTER) RefpropdllInstance->getSymbol(SETMIXdll_NAME);
		SETMODlib = (SETMODdll_POINTER) RefpropdllInstance->getSymbol(SETMODdll_NAME);
		SETREFlib = (SETREFdll_POINTER) RefpropdllInstance->getSymbol(SETREFdll_NAME);
		SETUPlib = (SETUPdll_POINTER) RefpropdllInstance->getSymbol(SETUPdll_NAME);
// 		SPECGRlib = (SPECGRdll_POINTER) RefpropdllInstance->getSymbol(SPECGRdll_NAME);
		SUBLPlib = (SUBLPdll_POINTER) RefpropdllInstance->getSymbol(SUBLPdll_NAME);
		SUBLTlib = (SUBLTdll_POINTER) RefpropdllInstance->getSymbol(SUBLTdll_NAME);
		SURFTlib = (SURFTdll_POINTER) RefpropdllInstance->getSymbol(SURFTdll_NAME);
		SURTENlib = (SURTENdll_POINTER) RefpropdllInstance->getSymbol(SURTENdll_NAME);
		TDFLSHlib = (TDFLSHdll_POINTER) RefpropdllInstance->getSymbol(TDFLSHdll_NAME);
		TEFLSHlib = (TEFLSHdll_POINTER) RefpropdllInstance->getSymbol(TEFLSHdll_NAME);
		THERM0lib = (THERM0dll_POINTER) RefpropdllInstance->getSymbol(THERM0dll_NAME);
		THERM2lib = (THERM2dll_POINTER) RefpropdllInstance->getSymbol(THERM2dll_NAME);
		THERM3lib = (THERM3dll_POINTER) RefpropdllInstance->getSymbol(THERM3dll_NAME);
		THERMlib = (THERMdll_POINTER) RefpropdllInstance->getSymbol(THERMdll_NAME);
		THFLSHlib = (THFLSHdll_POINTER) RefpropdllInstance->getSymbol(THFLSHdll_NAME);
		TPFLSHlib = (TPFLSHdll_POINTER) RefpropdllInstance->getSymbol(TPFLSHdll_NAME);
		TPRHOlib = (TPRHOdll_POINTER) RefpropdllInstance->getSymbol(TPRHOdll_NAME);
		TQFLSHlib = (TQFLSHdll_POINTER) RefpropdllInstance->getSymbol(TQFLSHdll_NAME);
		TRNPRPlib = (TRNPRPdll_POINTER) RefpropdllInstance->getSymbol(TRNPRPdll_NAME);
		TSFLSHlib = (TSFLSHdll_POINTER) RefpropdllInstance->getSymbol(TSFLSHdll_NAME);
		VIRBlib = (VIRBdll_POINTER) RefpropdllInstance->getSymbol(VIRBdll_NAME);
		VIRClib = (VIRCdll_POINTER) RefpropdllInstance->getSymbol(VIRCdll_NAME);
		WMOLlib = (WMOLdll_POINTER) RefpropdllInstance->getSymbol(WMOLdll_NAME);
		XMASSlib = (XMASSdll_POINTER) RefpropdllInstance->getSymbol(XMASSdll_NAME);
		XMOLElib = (XMOLEdll_POINTER) RefpropdllInstance->getSymbol(XMOLEdll_NAME);
		// 
		if (DEBUGMODE) printf ("RefpropdllInstance loaded: %s \n", "true");
	} else { // library was already loaded 
	  if (DEBUGMODE) printf ("RefpropdllInstance loaded: %s \n", "true");
	}
		
	
	// Now the library is loaded and we can start checkicng the fluid path
	if (DEBUGMODE) {
	  Poco::File fldFile(FLD_PATH);
	  if ( !fldFile.isDirectory() || !fldFile.canRead() ){
	    printf ("FLD_PATH is not a readable directory: %s \n", FLD_PATH.toString().c_str());
	    sprintf (herr,"FLD_PATH is not a readable directory: %s \n", FLD_PATH.toString().c_str());
	    return 0;
	  } else {
	      printf ("FLD_PATH is a readable directory: %s \n", FLD_PATH.toString().c_str());
	  }
	} 
	char FLD_PATH_CHAR[filepathlength+1];
	strcpy(FLD_PATH_CHAR,FLD_PATH.toString().c_str());
	
	
	// If the fluid name starts with the string "REFPROP-", chop off the "REFPROP-"
	if (!strncmp(Ref,"REFPROP-",8))
	{
	char *REFPROPRef=NULL,*RefCopy=NULL;
	double prop;
		
	// Allocate space for refrigerant name
		RefCopy=(char *)malloc(strlen(Ref)+1);
	// Make a backup copy
		strcpy(RefCopy,Ref);
	// Chop off the "REFPROP-"
		REFPROPRef = strtok(RefCopy,"-");
		REFPROPRef = strtok(NULL,"-");
	// Run with the stripped Refrigerant name
		prop=REFPROP(Output,Name1,Prop1,Name2,Prop2,xkg,REFPROPRef,path,herr,DEBUGMODE);
	// Free allocated memory
		free(RefCopy);
	// Return the new value
		return prop;
	}
	
	// find the delimiter
	char *pointer;
	pointer = strchr(Ref, '|');
	
	if (!strncmp(Ref,"MIX",3))
	{
		// Sample is "REFPROP-MIX:R32[0.697615]&R125[0.302385]" -  this is R410A
		char *REFPROPRef=NULL,*RefCopy=NULL,*Refs[20],*Refrigerant;
		double molefraction;

		// Allocate space for refrigerant name
		RefCopy=(char *)malloc(strlen(Ref)+1);
		// Make a backup copy
		strcpy(RefCopy,Ref);
		// Chop off the "MIX"
		REFPROPRef = strtok(RefCopy,":");
		i=1;
		while (REFPROPRef!=NULL)
		{
			Refs[i-1]=strtok(NULL,"&");
			if (Refs[i-1]==NULL)
			{
				i--;
				break;
			}
			else
				i++;
		}
		// Check maximum number of components
		if (i>ncmax){
			sprintf(herr,"Too many components (More than %i)\n",ncmax);
			return 1;
		}
		//Flush out RefString
		sprintf(RefString,"");
		for (j=0;j<i;j++)
		{	
			//Get component and its mole fraction
			Refrigerant=strtok(Refs[j],"[]");
			//printf("current refrigerant: %s ",Refrigerant);
			molefraction=strtod(strtok(NULL,"[]"),NULL);
			//printf("mole fraction: %f \n",molefraction);
			x[j]=molefraction;
			if (j==0) {
				sprintf(RefString,"%s%s%s.FLD",RefString,FLD_PATH_CHAR,Refs[j]);
			} else {
				sprintf(RefString,"%s|%s%s.FLD",RefString,FLD_PATH_CHAR,Refs[j]);
			}
		}
		// Free allocated memory
		free(RefCopy);
	} 
	else if (pointer != NULL)
	{
		// Sample is "R32|R125" -  this is R410A
		char *REFPROPRef=NULL,*RefCopy=NULL,*Refs[20],*Refrigerant;
		//double molefraction;
		
		// Allocate space for refrigerant name
		RefCopy=(char *)malloc(strlen(Ref)+1);
		// Make a backup copy
		strcpy(RefCopy,Ref);
		// Chop off the "MIX"
		REFPROPRef = strtok(RefCopy,"|");
		Refs[0] = REFPROPRef;
		i=2;
		while (REFPROPRef!=NULL)
		{
			Refs[i-1]=strtok(NULL,"|");
			if (Refs[i-1]==NULL)
			{
				i--;
				break;
			}
			else
				i++;
		}
		// Check maximum number of components
		if (i>ncmax){
			sprintf(herr,"Too many components (More than %i)\n",ncmax);
			return 1;
		}
		if (DEBUGMODE) printf("Mass-based mixture with %li components \n",i);
		//Flush out RefString
		sprintf(RefString,"");
		for (j=0;j<i;j++)
		{	
// 			//Get component and its mole fraction
// 			Refrigerant=strtok(Refs[j],"[]");
// 			//printf("current refrigerant: %s ",Refrigerant);
// 			molefraction=strtod(strtok(NULL,"[]"),NULL);
// 			//printf("mole fraction: %f \n",molefraction);
// 			x[j]=molefraction;
			if (j==0) {
				sprintf(RefString,"%s%s%s.FLD",RefString,FLD_PATH_CHAR,Refs[j]);
			} else {
				sprintf(RefString,"%s|%s%s.FLD",RefString,FLD_PATH_CHAR,Refs[j]);
			}
		}
		// Free allocated memory
		free(RefCopy);
	}	
	else if (!strcmp(Ref,"Air") || !strcmp(Ref,"R507A") || !strcmp(Ref,"R404A") || !strcmp(Ref,"R410A") || !strcmp(Ref,"R407C") || !strcmp(Ref,"SES36"))
	{
		i=1;
		strcpy(RefString,"");
		strcat(RefString,Ref);
		strcat(RefString,".ppf");
		x[0]=1.0;     //Pseudo-Pure fluid
	}
	else if (!strcmp(Ref,"R507A"))
	{
		i=2;
		strcpy(RefString,"R23.fld|R116.fld");
		x[0]=0.62675;
		x[1]=0.37325;
	}
	else if (!strcmp(Ref,"R410A"))
	{
		i=2;
		strcpy(RefString,"R32.fld|R125.fld");
		x[0]=0.697615;
		x[1]=0.302385;
	}
	else if (!strcmp(Ref,"R404A"))
	{
		i=3;
		strcpy(RefString,"R125.fld|R134a.fld|R143a.fld");
		x[0]=0.35782;
		x[1]=0.038264;
		x[2]=0.60392;
	}
    else if (!strcmp(Ref,"Air"))
	{
		i=3;
		strcpy(RefString,"Nitrogen.fld|Oxygen.fld|Argon.fld");
		x[0]=0.7812;
		x[1]=0.2096;
		x[2]=0.0092;
	}
	else
	{
		i=1;
		strcpy(RefString,"");
		strcat(RefString,FLD_PATH_CHAR);
		strcat(RefString,Ref);
		strcat(RefString,".FLD");
		x[0]=1.0;     //Pure fluid
	}
	
// 	// Add the path to the string
// 	FLD_PATH_CHAR
// 
// 	i=1;
// 	while (true)
// 	{
// 		Refs[i-1]=strtok(NULL,"&");
// 		if (Refs[i-1]==NULL)
// 		{
// 			i--;
// 			break;
// 		}
// 		else
// 			i++;
// 	}

	if (DEBUGMODE) printf("RefString: %s \n",RefString);
	strcpy(hf,RefString);
	strcpy(hfmix,FLD_PATH_CHAR);
	strcat(hfmix,"HMX.BNC");
	strcpy(hrf,"DEF");
	strcpy(herr,"Ok");
	
	// If the name of the refrigerant doesn't match 
	// that of the currently loaded refrigerant
	if (strcmp(LoadedREFPROPRef,Ref))
	{
		ierr=999;
		//...Call SETUP to initialize the program 
// 		SETUPlib(&i, hf, hfmix, hrf, &ierr, herr,
// 			refpropcharlength*ncmax,refpropcharlength,
// 			lengthofreference,errormessagelength);		
		if (DEBUGMODE) {
			printf("Running SETUP...\n");
			printf ("No. of components: %li \n", i);
			printf ("Fluid files: %s \n", RefString);
			printf ("Mixture file: %s \n", hfmix);
		}
		SETUPlib(i, hf, hfmix, hrf, ierr, herr);
		if (ierr != 0) printf("REFPROP setup gives this error during SETUP: %s\n",herr);
		//Copy the name of the loaded refrigerant back into the temporary holder
		strcpy(LoadedREFPROPRef,Ref);
	}

	// Get the molar mass of the fluid
	// WMOLlib(x,MW);
	
	if (x[0] != 0.0) { // there was input
		// Use another function to handle the mass based input
		XMASSlib(x,xkg,MW);
		//MW = MW / 1000;
		if (DEBUGMODE) printf("Mole-based input: x:%f - xkg:%f \n",x[0],xkg[0]);
	} else {
		XMOLElib(xkg,x,MW);
		//MW = MW / 1000;
		if (DEBUGMODE) printf("Mass-based input, x:%f - xkg:%f \n",x[0],xkg[0]);
	}
	
	//
	if (Output=='B')
	{
		// Critical temperature
		CRITPlib(x,Tcrit,pcrit,dcrit,ierr,herr,255);
		return Tcrit;
	}
	else if (Output=='M')
	{
		// mole mass
		return MW;
	}
	else if (Output=='E')
	{
		// Critical pressure
		CRITPlib(x,Tcrit,pcrit,dcrit,ierr,herr,255);
		return pcrit;
	}
	else if (Output =='N')
	{
		// Critical density
		CRITPlib(x,Tcrit,pcrit,dcrit,ierr,herr,255);
		return dcrit*MW;
		
	}
	else if (Output == 't')
	{
		// Minimum temperature
		double tmin,tmax,Dmax,pmax;
		LIMITSlib("EOS",x,tmin,tmax,Dmax,pmax,255);
		return tmin;
	}
	else if (Output =='w')
	{
		double wmm,Ttriple,tnbpt,tc,pc,Dc,Zc,acf,dip,Rgas;
		// Accentric factor
		if (i>1)
		{
			fprintf(stderr,"Error: Accentric factor only defined for pure fluids\n");
			return 1;
		}
		INFOlib(i,wmm,Ttriple,tnbpt,tc,pc,Dc,Zc,acf,dip,Rgas);
		return acf;
	}
	else if (Output =='o')
	{
		double wmm,Ttriple,tnbpt,tc,pc,Dc,Zc,acf,dip,Rgas;
		// Dipole moment
		if (i>1)
		{
			fprintf(stderr,"Error: Dipole moment only defined for pure fluids\n");
			return 1;
		}
		INFOlib(i,wmm,Ttriple,tnbpt,tc,pc,Dc,Zc,acf,dip,Rgas);
		return dip;
	}
	else if (Output=='R')
	{
		long icomp;
		double wmm,Ttriple,tnbpt,tc,pc,Dc,Zc,acf,dip,Rgas;
		// Triple point temperature
		icomp=1;
		if (i>1)
		{
			fprintf(stderr,"Error: Triple point temperature only defined for pure fluids\n");
			return 200;
		}
		INFOlib(icomp,wmm,Ttriple,tnbpt,tc,pc,Dc,Zc,acf,dip,Rgas);
		return Ttriple;
	}
	else if (Output=='I')
	{
		if (Name1=='T'){
			SURFTlib(Prop1,dl,x,sigma,i,herr,errormessagelength);
			return sigma/1000;
		}
		else{
			std::cout<< "If surface tension is the output, temperature must be the first input" << std::endl;
			return 1;
		}
	}
	
	else if ((Name1=='T' && Name2=='P') || (Name2=='T' && Name1=='P'))
	{
		// T in K, P in Pa
		if (Name1 == 'T'){
			T = Prop1; p = Prop2/1000;
		}
		else{
			p = Prop1/1000; T = Prop2;
		}
		

		// Use flash routine to find properties
		TPFLSHlib(T,p,x,d,dl,dv,xliq,xvap,q,e,h,s,cv,cp,w,ierr,herr,errormessagelength);
		if (Output=='H') return h/MW;
		else if (Output=='D') return d*MW;
		else if (Output=='S') return s/MW;
		else if (Output=='U') return e/MW;
		else if (Output=='C') return cp/MW;
		else if (Output=='O') return cv/MW;
		else if (Output=='P') return p*1000;
		else if (Output=='A') return w;
		else if (Output=='V') 
		{
			TRNPRPlib(T,d,x,eta,tcx,ierr,herr,errormessagelength);
			return eta/1.0e6; //uPa-s to Pa-s
		} 
		else if (Output=='L')
		{
			TRNPRPlib(T,d,x,eta,tcx,ierr,herr,errormessagelength);
			return tcx/1000.0; //W/m-K to kW/m-K
		}
		else
			return 1;
	}
	else if ((Name1=='T' && Name2=='D') || (Name2=='T' && Name1=='D'))
	{
		// T in K, D in kg/m^3
		if (Name1 == 'T'){
			T = Prop1; rho = Prop2/MW;
		}
		else{
			rho = Prop1/MW; T = Prop2;
		}
		
		// This is the explicit formulation of the EOS
		TDFLSHlib(T,rho,x,p,dl,dv,xliq,xvap,q,e,h,s,cv,cp,w,ierr,herr,errormessagelength);

		if (Output=='P')
		{
			return p*1000;
		}
		if (Output=='H')
		{
			return h/MW;
		}
		else if (Output=='A')
		{
			return w;
		}
		else if (Output=='S')
		{
			return s/MW;
		}
		else if (Output=='U')
		{
			return (h-p/rho)/MW;
		}
		else if (Output=='C')
		{
			return cp/MW;
		}
		else if (Output=='O')
		{
			return cv/MW;
		}
		else if (Output=='V') 
		{
			TRNPRPlib(T,rho,x,eta,tcx,ierr,herr,errormessagelength);
			return eta/1.0e6; //uPa-s to Pa-s
		} 
		else if (Output=='L')
		{
			TRNPRPlib(T,rho,x,eta,tcx,ierr,herr,errormessagelength);
			return tcx/1000.0; //W/m-K to kW/m-K
		}
		else if (Output=='D')
		{
			return rho*MW;
		}
		else
			return 1;
	}
	else if ((Name1=='T' && Name2=='Q') || (Name2=='T' && Name1=='Q'))
	{

		if (Name1 == 'T'){
			T = Prop1; Q = Prop2;
		}
		else{
			Q = Prop1; T = Prop2;
		}
		
		// Saturation Density
		i=1;
		SATTlib(T,x,i,pl,dl,dv_,xliq,xvap,ierr,herr,errormessagelength);
		i=2;
		SATTlib(T,x,i,pv,dl_,dv,xliq,xvap,ierr,herr,errormessagelength);
		if (Output=='D') 
		{
			return 1/(Q/dv+(1-Q)/dl)*MW;
		}
		else if (Output=='P') 
		{
			return (pv*Q+pl*(1-Q))*1000;
		}
		else if (Output=='A')
		{
			rho=1/(Q/dv+(1-Q)/dl);
			THERMlib(T,rho,x,p,e,h,s,cv,cp,w,hjt);
			return w;
		}
		else if (Output=='H') 
		{
			ENTHALlib(T,dl,xliq,hl);
			ENTHALlib(T,dv,xvap,hv);
			return (hv*Q+hl*(1-Q))/MW*1000; // J/kg to kJ/kg
		}
		else if (Output=='S') 
		{
			ENTROlib(T,dl,xliq,sl);
			ENTROlib(T,dv,xvap,sv);
			return (sv*Q+sl*(1-Q))/MW*1000; // J/kg-K to kJ/kg-K
		}
		else if (Output=='U') 
		{
			ENTHALlib(T,dl,xliq,hl);
			ENTHALlib(T,dv,xvap,hv);
			p=pv*Q+pl*(1-Q);
			ul=hl-p/dl;
			uv=hv-p/dv;
			return (uv*Q+ul*(1-Q))/MW*1000; // J/kg to kJ/kg
		}
		else if (Output=='C') 
		{
			d=1/(Q/dv+(1-Q)/dl);
			CVCPlib(T,d,x,cv,cp);
			return cp/MW*1000; // J/kg-K to kJ/kg-K
		}
		else if (Output=='O') 
		{
			d=1/(Q/dv+(1-Q)/dl);
			CVCPlib(T,d,x,cv,cp);
			return cv/MW*1000; // J/kg-K to kJ/kg-K
		}
		else if (Output=='V') 
		{
			d=1/(Q/dv+(1-Q)/dl);
			TRNPRPlib(T,d,x,eta,tcx,ierr,herr,errormessagelength);
			return eta/1.0e6; //uPa-s to Pa-s
		}
		else if (Output=='L') 
		{
			d=1/(Q/dv+(1-Q)/dl);
			TRNPRPlib(T,d,x,eta,tcx,ierr,herr,errormessagelength);
			return tcx/1000.0; //W/m-K to kW/m-K
		}
		else
			return 1;
	}
	else if ((Name1=='P' && Name2=='Q') || (Name2=='P' && Name1=='Q'))
	{

		if (Name1 == 'P'){
			p = Prop1/1000; Q = Prop2;
		}
		else{
			Q = Prop1; p = Prop2/1000;
		}

		// Saturation Density
		SATPlib(p,x,i,T,dl,dv,xliq,xvap,ierr,herr,errormessagelength);
		if (Output=='T')
		{
			return T;
		}
		else if (Output=='D') 
		{
			return 1/(Q/dv+(1-Q)/dl)*MW;
		}
		else if (Output=='P') 
		{
			PRESSlib(T,dl,xliq,pl);
			PRESSlib(T,dv,xvap,pv);
			return (pv*Q+pl*(1-Q))*1000;
		}
		else if (Output=='H') 
		{
			ENTHALlib(T,dl,xliq,hl);
			ENTHALlib(T,dv,xvap,hv);
			return (hv*Q+hl*(1-Q))/MW*1000; // J/kg to kJ/kg
		}
		else if (Output=='S') 
		{
			ENTROlib(T,dl,xliq,sl);
			ENTROlib(T,dv,xvap,sv);
			return (sv*Q+sl*(1-Q))/MW*1000; // J/kg-K to kJ/kg-K
		}
		else if (Output=='U') 
		{
			ENTHALlib(T,dl,xliq,hl);
			ENTHALlib(T,dv,xvap,hv);
			ul=hl-p/dl;
			uv=hv-p/dv;
			return (uv*Q+ul*(1-Q))/MW*1000; // J/kg to kJ/kg
		}
		else if (Output=='C') 
		{
			d=1/(Q/dv+(1-Q)/dl);
			CVCPlib(T,d,x,cv,cp);
			return cp/MW*1000; // J/kg-K to kJ/kg-K
		}
		else if (Output=='O') 
		{
			d=1/(Q/dv+(1-Q)/dl);
			CVCPlib(T,d,x,cv,cp);
			return cv/MW*1000; // J/kg-K to kJ/kg-K
		}
		else if (Output=='A')
		{
			rho=1/(Q/dv+(1-Q)/dl);
			THERMlib(T,rho,x,p,e,h,s,cv,cp,w,hjt);
			return w;
		}
		else if (Output=='V') 
		{
			d=1/(Q/dv+(1-Q)/dl);
			TRNPRPlib(T,d,x,eta,tcx,ierr,herr,errormessagelength);
			return eta/1.0e6; //uPa-s to Pa-s
		}
		else if (Output=='L') 
		{
			d=1/(Q/dv+(1-Q)/dl);
			TRNPRPlib(T,d,x,eta,tcx,ierr,herr,errormessagelength);
			return tcx; //W/m-K to kW/m-K
		}
		else
			return 1;
	}
	else if ((Name1=='P' && Name2=='H') || (Name2=='P' && Name1=='H'))
	{
		// p in Pa, h in J/kg
		if (Name1 == 'P'){
			p = Prop1/1000; h = Prop2*MW/1000;
		}
		else{
			h = Prop1*MW/1000; p = Prop2/1000;
		}
		
		// Use flash routine to find properties
		PHFLSHlib(p,h,x,T,d,dl,dv,xliq,xvap,q,e,s,cv,cp,w,ierr,herr,errormessagelength);
		if (Output=='H') return h/MW;
		else if (Output=='T') return T;
		else if (Output=='D') return d*MW;
		else if (Output=='S') return s/MW;
		else if (Output=='U') return e/MW;
		else if (Output=='C') return cp/MW;
		else if (Output=='O') return cv/MW;
		else if (Output=='P') return p*1000;
		else if (Output=='A') return w;
		else if (Output=='V') 
		{
			TRNPRPlib(T,d,x,eta,tcx,ierr,herr,errormessagelength);
			return eta/1.0e6; //uPa-s to Pa-s
		} 
		else if (Output=='L')
		{
			TRNPRPlib(T,d,x,eta,tcx,ierr,herr,errormessagelength);
			return tcx/1000.0; //W/m-K to kW/m-K
		}
		else
			return 1;
	}
	else
		return 1;
}
// #endif
