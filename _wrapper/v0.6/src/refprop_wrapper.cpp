/*
	This is a wrapper file for the Refprop library. It does not include
	any functionality besides providing the connection to Refprop.

	The current version was developed by Jorrit Wronski (jowr@mek.dtu.dk)
	based on Henning Francke's (francke@gfz-potsdam.de) wrapper. A little
	inspiration also came from Ian Bell's (ian.h.bell@gmail.com) wrapper
	used in CoolProp - http://coolprop.sourceforge.net/
	
	Compatible to the Modelica interface developed by Henning Francke
	(francke@gfz-potsdam.de) and his first wrapper class that you can
	find in the folder with version 0.5 for Windows systems.
*/

/*
 * Here are the included files
 */

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif

#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include <map>
#include <utility>
#include <ctype.h> // tolower, toupper, etc

#include <refprop_lib.h>
//#include <refprop_names.h>
//#include <refprop_constants.h>
//#include <refprop_types_cpp.h>
//#include <refprop_types.h>

#include "refprop_wrapper.h"

// get the POCO classes
#include "Poco/SharedLibrary.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/Environment.h"
#include "Poco/String.h"
#include "Poco/NumberFormatter.h"
#include "Poco/StringTokenizer.h"
#include "Poco/Exception.h"


//  Define the functions either by their pointers or type.
WMOLdll_POINTER WMOLlib = NULL;
TPFL2dll_POINTER TPFL2lib = NULL;
TPFLSHdll_POINTER TPFLSHlib = NULL;
PHFL1dll_POINTER PHFL1lib = NULL;
PHFLSHdll_POINTER PHFLSHlib = NULL;
PDFL1dll_POINTER PDFL1lib = NULL;
PDFLSHdll_POINTER PDFLSHlib = NULL;
PSFLSHdll_POINTER PSFLSHlib = NULL;
PQFLSHdll_POINTER PQFLSHlib = NULL;
THFLSHdll_POINTER THFLSHlib = NULL;
TDFLSHdll_POINTER TDFLSHlib = NULL;
TSFLSHdll_POINTER TSFLSHlib = NULL;
TQFLSHdll_POINTER TQFLSHlib = NULL;
DHFL1dll_POINTER DHFL1lib = NULL;
DHFL2dll_POINTER DHFL2lib = NULL;
DHFLSHdll_POINTER DHFLSHlib = NULL;
HSFLSHdll_POINTER HSFLSHlib = NULL;
DSFL1dll_POINTER DSFL1lib = NULL;
DSFL2dll_POINTER DSFL2lib = NULL;
DSFLSHdll_POINTER DSFLSHlib = NULL;
TRNPRPdll_POINTER TRNPRPlib = NULL;
SATTdll_POINTER SATTlib = NULL;
SATPdll_POINTER SATPlib = NULL;
SATDdll_POINTER SATDlib = NULL;
SETUPdll_POINTER SETUPlib = NULL;
XMASSdll_POINTER XMASSlib = NULL;
XMOLEdll_POINTER XMOLElib = NULL;

THERM2dll_POINTER THERM2lib = NULL;
DHD1dll_POINTER DHD1lib = NULL;


/*
 * Just a helper function control the output of
 * an array. Used to debug the handling of the
 * composition arrays.
 */
std::string printX(double arr[], long nc) {
  std::string ret = "(";
  int stop = nc-1;

  for(int i = 0; i < (stop); i++) {
	ret = ret + Poco::NumberFormatter::format(arr[i], 4) + ", "; // four decimals
  } 

  ret = ret + Poco::NumberFormatter::format(arr[stop], 4) + ")"; // four decimals
  return ret;
}


/*
 * Define pointer to library as well as the
 * strings we need to determine whether a
 * call to SETUPlib is needed.
 */
Poco::SharedLibrary *RefpropdllInstance = NULL;
std::string loadedFluids;


/*
 * Here we define the fluid's properties. These values get updated after each
 * call to Refprop and are used for caching values. I decided to use the
 * Refprop units for internal data storage Be careful when converting
 * properties using the molecular weight. It is stored in g/mol.
 */
     long kq 				= 2; // all qualities are calculated on a mass basis
static const double noValue	= -1e+10;

static const std::string FLUIDS_PATH = "fluids";
static const std::string LIN_LIBRARY = "librefprop.so";
static const std::string WIN_LIBRARY = "refprop.dll";
Poco::Path FLD_PATH(true);


//	static const double noFactor   = 1e+0;
//	static const double presFactor = 1e+3;
//	static const double viscFactor = 1e+6;
//	static const double molwFactor = 1e-3;
//  double energyFactor          = noValue;   // J/mol / g/mol * 1000g/kg = J/kg
//  double densityFactor[3]      = {noValue}; // mol/l * g/mol = g/l = kg/m3 for all phases
//  double fractionFactor[ncmax] = {noValue}; // xkgi  = xmi * mwi * mw  (array)
//  double qualityFactor         = noValue;   // qmass = qmole * mw_vap / mw

bool   debug;      		// set the debug flag
//long   lerr;  			// Error return mechanism
double dhelp = noValue;
//double delta = 1e-20; 	// tolerance for evaluating differences


/*
 * Properties for saturation states. "dew" refers to the dew point and
 * "bub" describes the bubble point.
 */
double  dtdew, dpdew, ddldew, ddvdew, dtbub, dpbub, ddlbub, ddvbub;
int flushSaturation() {
	dtdew=noValue;
	dpdew=noValue;
	ddldew=noValue;
	ddvdew=noValue;
	dtbub=noValue;
	dpbub=noValue;
	ddlbub=noValue;
	ddvbub=noValue;
	if (debug) printf ("Finished flushing saturation properties.\n");
	return 0;
}


/*
 * Most of the fluid properties are stored here. There are arrays for
 * composition information as well as single values for the other
 * properties.
 * The flush function gets called when the state changes and the
 * previously calculated are not valid anymore. A change of state also leads
 * to changed saturation conditions.
 */
double 	dt, dp, de, dh, ds, dqmol, dd, dxmol[ncmax], ddl,
	ddv, dxmoll[ncmax], dxmolv[ncmax], dCv, dCp, dw, dwliq, dwvap,
	dhjt, dZ[ncmax], dA, dG, dxkappa, dbeta, ddpdd, dd2pdd2, ddpdt, ddddt,
	ddddp, dd2pdt2, dd2pdtd, ddhdt,	df, deta, dtcx, dstn;

double ddhdt_d, ddhdt_p, ddhdd_t, ddhdd_p, ddhdp_t, ddhdp_d;

double ddddp_h_num, ddddh_p_num; // get numerical derivatives

int flushProperties(){
	dt=noValue;
	dp=noValue;
	de=noValue;
	dh=noValue;
	ds=noValue;
	dqmol=noValue;
	dd=noValue;
	dxmol[0]=noValue;
	ddl=noValue;
	ddv=noValue;
	dxmoll[0]=noValue;
	dxmolv[0]=noValue;
	dCv=noValue;
	dCp=noValue;
	dw=noValue;
	dwliq=noValue;
	dwvap=noValue;
	dhjt=noValue;
	dZ[0]=noValue;
	dA=noValue;
	dG=noValue;
	dxkappa=noValue;
	dbeta=noValue;
	ddpdd=noValue;
	dd2pdd2=noValue;
	ddpdt=noValue;
	ddddt=noValue;
	ddddp=noValue;
	dd2pdt2=noValue;
	dd2pdtd=noValue;
	ddhdt=noValue;
	df=noValue;
	deta=noValue;
	dtcx=noValue;
	dstn=noValue;

	ddhdt_d=noValue;
	ddhdt_p=noValue;
	ddhdd_t=noValue;
	ddhdd_p=noValue;
	ddhdp_t=noValue;
	ddhdp_d=noValue;

	ddddp_h_num=noValue;
	ddddh_p_num=noValue;

	if (debug) printf ("Finished flushing normal fluid properties.\n");
	return flushSaturation();
}


/*
 * Properties that are constants for pure fluids. Hence, the flushing
 * function gets called when a new fluid is loaded. In case of a mixture,
 * a change of composition also triggers a flushing since this also
 * changes those that also exist for mixtures.
 * A change of composition or fluid always leads to a flushing of the
 * other properties.
 */
long lnc; // number of components
double 	dwm, dttp, dtnbp, dtc, dpc, ddc, dZc, dacf, ddip, drgas;
int flushConstants() {
	//lnc		= -1;
	dwm 	= noValue;
	dttp 	= noValue;
	dtnbp 	= noValue;
	dtc		= noValue;
	dpc		= noValue;
	ddc		= noValue;
	dZc		= noValue;
	dacf	= noValue;
	ddip	= noValue;
	drgas	= noValue;
	if (debug) printf ("Finished flushing fluid/mixture \"constants\".\n");
	return flushProperties();
}


/*
 * Make sure that the library is loaded and that all the functions have
 * pointers. Perform detailed checks if the debug flag is set. This is
 * could be set from inside Modelica to find out why there are problems
 * with loading the Refprop library. It is assumed that the library
 * file lies in the same directory as the fluid folder.
 */
int setPaths(std::string pathToRefprop, Poco::Path* LP, char* error) {

	if (debug) printf ("\nSetting paths\n");

	if (pathToRefprop.length()>filepathlength){
		sprintf(error,"Path too long (%i > %i)\n",pathToRefprop.length(),filepathlength);
		return -1;
	}

	// Parse the string and append a path separator if necessary.
	Poco::Path REF_PATH;
	REF_PATH.parse(pathToRefprop, Poco::Path::PATH_NATIVE);
	if (!REF_PATH.isDirectory()) REF_PATH.append(REF_PATH.separator());
	// Check the path if running in debug mode
	if (debug) {
	  Poco::File refFile(REF_PATH);
	  if ( !refFile.isDirectory() || !refFile.canRead() ){
		printf ("The provided library path is not a readable directory: %s \n", REF_PATH.toString().c_str());
		sprintf (error,"The provided library path is not a readable directory: %s \n", REF_PATH.toString().c_str());
		return -1;
	  } else {
		printf ("The provided library path is a readable directory: %s \n", REF_PATH.toString().c_str());
	  }
	}

	//	Poco::Path FLD_PATH = (*FP); // get the object
	// The fluid files are in the Refprop directory, append "fluids".
	FLD_PATH.parse(REF_PATH.toString());
	FLD_PATH.pushDirectory(FLUIDS_PATH);
	// Check the path if running in debug mode
	if (debug) {
	  Poco::File fluidFile(FLD_PATH);
	  if ( !fluidFile.isDirectory() || !fluidFile.canRead() ){
		printf ("The provided fluid path is not a readable directory: %s \n", FLD_PATH.toString().c_str());
		sprintf (error,"The provided fluid path is not a readable directory: %s \n", FLD_PATH.toString().c_str());
		return -1;
	  } else {
		printf ("The provided fluid path is a readable directory: %s \n", FLD_PATH.toString().c_str());
	  }
	}

	Poco::Path SRC_PATH; // We might want to define the search path differently
	bool found_lib;

	// Check the OS and assign the right names for the library
	bool is_linux = ( 0 == Poco::icompare(Poco::Environment::osName(), "linux") );
	if (is_linux){
		//SRC_PATH.parse("/usr/local/lib");
		SRC_PATH = REF_PATH;
		found_lib = Poco::Path::find(SRC_PATH.toString(), LIN_LIBRARY, (*LP));
	} else {
		SRC_PATH = REF_PATH;
		found_lib = Poco::Path::find(SRC_PATH.toString(), WIN_LIBRARY, (*LP));
	}

	if (found_lib) {
		if (debug) printf ("Found library at %s \n", LP->toString().c_str());
	} else {
		if (debug) printf ("Cannot find library in path %s \n", SRC_PATH.toString().c_str());
		sprintf (error,"Cannot find library in path %s \n", SRC_PATH.toString().c_str());
		return -1;
	}

	// Check the file if running in debug mode
	if (debug) {
	  Poco::File libFile(LP);
	  if ( !libFile.canRead() ){
		printf ("The provided library is not a readable file: %s \n", LP->toString().c_str());
		sprintf (error,"The provided library is not a readable file: %s \n", LP->toString().c_str());
		return -1;
	  } else {
		printf ("The provided library is a readable file: %s \n", LP->toString().c_str());
	  }
	}

	return 0;
}



int loadLibrary(std::string pathToRefprop, char* error) {

	if (debug) printf ("\nLoading library.\n");

	if (RefpropdllInstance==NULL) {

		long lerr = 0;

		if (debug) printf ("Library is not loaded, trying to do so.\n");
		Poco::Path LIB_PATH(true);

		// use the function to set the global path to fluids and
		// get the path to the library to load
		lerr = setPaths(pathToRefprop, &LIB_PATH, error);
		if (lerr!=0) {
			printf ("\nThere was an error setting the paths. This hint \n");
			printf ("might help: %s \n", error);
			return lerr;
		}

		// load a new library instance
		if (debug) printf ("Loaded library at: %s \n",LIB_PATH.toString().c_str());

		RefpropdllInstance = new Poco::SharedLibrary(LIB_PATH.toString());
		if (RefpropdllInstance==NULL) {
		  printf("Could not load Refprop library, but no error message was provided.\n");
		  return -1;
		}

		// Get pointers to functions from library.
		DHFLSHlib 	= (DHFLSHdll_POINTER) 	RefpropdllInstance->getSymbol(DHFLSHdll_NAME);
		DSFLSHlib 	= (DSFLSHdll_POINTER) 	RefpropdllInstance->getSymbol(DSFLSHdll_NAME);
		HSFLSHlib 	= (HSFLSHdll_POINTER)	RefpropdllInstance->getSymbol(HSFLSHdll_NAME);
		PDFL1lib 	= (PDFL1dll_POINTER)	RefpropdllInstance->getSymbol(PDFL1dll_NAME);
		PDFLSHlib 	= (PDFLSHdll_POINTER) 	RefpropdllInstance->getSymbol(PDFLSHdll_NAME);
		PHFL1lib 	= (PHFL1dll_POINTER) 	RefpropdllInstance->getSymbol(PHFL1dll_NAME);
		PHFLSHlib 	= (PHFLSHdll_POINTER) 	RefpropdllInstance->getSymbol(PHFLSHdll_NAME);
		PQFLSHlib 	= (PQFLSHdll_POINTER) 	RefpropdllInstance->getSymbol(PQFLSHdll_NAME);
		PSFLSHlib 	= (PSFLSHdll_POINTER) 	RefpropdllInstance->getSymbol(PSFLSHdll_NAME);
		SATDlib 	= (SATDdll_POINTER) 	RefpropdllInstance->getSymbol(SATDdll_NAME);
		SATPlib 	= (SATPdll_POINTER) 	RefpropdllInstance->getSymbol(SATPdll_NAME);
		SATTlib 	= (SATTdll_POINTER) 	RefpropdllInstance->getSymbol(SATTdll_NAME);
		TDFLSHlib 	= (TDFLSHdll_POINTER) 	RefpropdllInstance->getSymbol(TDFLSHdll_NAME);
		THFLSHlib 	= (THFLSHdll_POINTER) 	RefpropdllInstance->getSymbol(THFLSHdll_NAME);
		TPFLSHlib 	= (TPFLSHdll_POINTER) 	RefpropdllInstance->getSymbol(TPFLSHdll_NAME);
		TQFLSHlib 	= (TQFLSHdll_POINTER) 	RefpropdllInstance->getSymbol(TQFLSHdll_NAME);
		TRNPRPlib 	= (TRNPRPdll_POINTER) 	RefpropdllInstance->getSymbol(TRNPRPdll_NAME);
		TSFLSHlib 	= (TSFLSHdll_POINTER) 	RefpropdllInstance->getSymbol(TSFLSHdll_NAME);
		WMOLlib 	= (WMOLdll_POINTER) 	RefpropdllInstance->getSymbol(WMOLdll_NAME);
		XMASSlib 	= (XMASSdll_POINTER) 	RefpropdllInstance->getSymbol(XMASSdll_NAME);
		XMOLElib 	= (XMOLEdll_POINTER) 	RefpropdllInstance->getSymbol(XMOLEdll_NAME);
		SETUPlib 	= (SETUPdll_POINTER) 	RefpropdllInstance->getSymbol(SETUPdll_NAME);
		//
		THERM2lib 	= (THERM2dll_POINTER) 	RefpropdllInstance->getSymbol(THERM2dll_NAME);
		DHD1lib     = (DHD1dll_POINTER) 	RefpropdllInstance->getSymbol(DHD1dll_NAME);

		if (debug) printf ("Library instance successfully loaded.\n");
	} else { // library was already loaded
	  if (debug) printf ("Library instance already, not doing anything.\n");
	}
	return 0;
}


/*
 * Processes the provided strings and constructs a fluid definition string
 * that can be digested by the Refprop library.
 */
int setFluid(std::string fluid, char* error){

	if (debug) printf ("\nSetting fluids\n");

	char* hf;
	char* hfmix;
	char* hrf;
	std::string RefString;
	long lerr;

	// If the name of the fluid doesn't match that of the currently loaded fluid
	if (debug) printf("Loaded fluids: %s \n",loadedFluids.c_str());
	if (debug) printf("New fluids:    %s \n",fluid.c_str());
	if (loadedFluids.compare(fluid)) { // There is a mismath
		if (debug) printf("Loading a new fluid.\n");
		if (fluid.find("|") != std::string::npos) { // check if contains mixture separator
			// Created string to insert into parsed fluids
			std::string replace = std::string(".FLD|") + FLD_PATH.toString();
			// Prepare final string for values and get tokens
			RefString = FLD_PATH.toString();
			Poco::StringTokenizer tokens(std::string(fluid), "|", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);
			RefString += Poco::cat(replace, tokens.begin(), tokens.end());
			RefString += std::string(".FLD");
			lnc = tokens.count();
			if (lnc>ncmax){
				sprintf(error,"Too many components (More than %i)\n",ncmax);
				return -1;
			}
		}
		else if (!fluid.compare("Air") || !fluid.compare("R507A") || !fluid.compare("R404A") || !fluid.compare("R410A") || !fluid.compare("R407C") || !fluid.compare("SES36"))
		{
			lnc=1;
			RefString = FLD_PATH.toString() + fluid + std::string(".PPF");
			dxmol[0]=1.0;     //Pseudo-Pure fluid
		}
		else
		{
			lnc=1;
			RefString = FLD_PATH.toString() + fluid + std::string(".FLD");
			dxmol[0]=1.0;     //Pure fluid
		}

		if (debug) printf("RefString: %s \n",RefString.c_str());

		hf    =  (char*) calloc(refpropcharlong, sizeof(char));
		hfmix =  (char*) calloc(refpropcharlength+8, sizeof(char));
		hrf   =  (char*) calloc(refpropcharlength, sizeof(char));

		strcpy(hf,RefString.c_str());
		strcpy(hfmix,FLD_PATH.toString().c_str());
		strcat(hfmix,"HMX.BNC");
		strcpy(hrf,"DEF");
		strcpy(error,"Ok");

		//...Call SETUPlib to set the fluids
		if (debug) {
			printf("Running SETUP...\n");
			printf ("No. of components: %li \n", lnc);
			printf ("Fluid files: %s \n", RefString.c_str());
			printf ("Mixture file: %s \n", hfmix);
		}

		lerr=999;
		SETUPlib(lnc, hf, hfmix, hrf, lerr, error);
		if (lerr != 0) {
			printf("REFPROP setup gives this error during SETUP: %s\n",error);
			return lerr;
		}
		free(hf);
		free(hfmix);
		free(hrf);

		//Copy the name of the loaded refrigerant back into the temporary holder
		loadedFluids = std::string(fluid);

		int flush = flushConstants();
		if (debug) printf("Loading a new fluid, flushing constants: %i.\n",flush);
	} else {
		if (debug) printf("Fluid was already loaded.\n");
	}
	return 0;
}


int init_REFPROP(std::string fluidnames, std::string REFPROP_PATH_CHAR, char* errormsg){
// Sets up the interface to the REFPROP.DLL
// is called by props_REFPROP and satprops_REFPROP
//	char DLL_PATH[filepathlength], FLD_PATH[filepathlength];

	long lerr = 0;
	//ierr=0;
// 	debug = true;

	if (debug) printf ("\nInitialising library\n");

	lerr = loadLibrary(REFPROP_PATH_CHAR, errormsg);
	if (lerr!=0) {
		printf ("There was an error loading the library. This hint \n");
		printf ("might help: %s \n", errormsg);
	}
	  
//	if (debug) {
//	  printf ("%s\n"," ");
//	  printf ("Running OS family : %s \n", Poco::Environment::osName().c_str());
//	  printf ("Loaded library    : %li \n", lerr);
//	  printf ("Fluids are located: %s \n", FLD_PATH.toString().c_str());
//	  //printf ("%s\n"," ");
//	}
	
	lerr = setFluid(fluidnames, errormsg);
	if (lerr!=0) {
		printf ("There was an error loading the fluids. This hint \n");
		printf ("might help: %s \n", errormsg);
	}

	if (debug) printf("Error code processing...\n");
	switch(lerr){
		case 101:
			//strcpy(errormsg,"error in opening file");
//			if (DEBUGMODE) printf("Error 101\n");
			sprintf(errormsg,"error in opening fluid file");
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
			sprintf(errormsg,"There was an unknown error(%li): %s",lerr,errormsg);
			// strcpy(errormsg,"Unknown error");
			//strcpy(errormsg,"Setup was successful!");
			// strncpy(errormsg,herr,errormessagelength);
			break;
	}
	return lerr;
}

bool isInput(std::string in1, std::string in2, std::string def){
	// if the first equals the first
	if ( 0 == Poco::icompare(in1, def.substr(0,1)) ) {
		if ( 0 == Poco::icompare(in2, def.substr(1,1)) ) return true;
	} else if ( 0 == Poco::icompare(in2, def.substr(0,1)) ) {
		if ( 0 == Poco::icompare(in1, def.substr(1,1)) ) return true;
	}
	return false;
}

double getT_refprop(double t) {
	// t--temperature [K]
	return t*1.;
}

double getP_refprop(double p) {
	// p--pressure [kPa]
	return p/1000.0;
}

double getD_refprop(double d) {
	// d--bulk molar density [mol/L]
	return d/dwm;  // kg/m3 = g/l / g/mol  = mol/l
}

double getE_refprop(double e) {
	// e--internal energy [J/mol]
	return e*dwm/1000.; // J/kg * g/mol * kg/(1000g) = J/mol
}

double getH_refprop(double h) {
	// h--enthalpy [J/mol]
	return h*dwm/1000.; // J/kg * g/mol * kg/(1000g) = J/mol
}

double getS_refprop(double s) {
	// s--entropy [[J/mol-K]
	return s*dwm/1000.; // J/(kg.K) * g/mol * kg/(1000g) = J/(mol.K)
}

/*
 * Convert to SI units
 */
double getT_modelica() {
	// t--temperature [K]
	return dt;
}

double getP_modelica() {
	// p--pressure [kPa]
	return dp*1000.0;
}

double getD_modelica() {
	// d--bulk molar density [mol/L]
	return dd*dwm;  // mol/l * g/mol = g/l = kg/m3
}

double getE_modelica() {
	// e--internal energy [J/mol]
	return de/dwm * 1000.0; // J/mol / g/mol * 1000g/kg = J/kg
}

double getH_modelica() {
	// h--enthalpy [J/mol]
	return dh/dwm * 1000.0; // J/mol / g/mol * 1000g/kg = J/kg
}

double getS_modelica() {
	// s--entropy [[J/mol-K]
	return ds/dwm * 1000.0; // J/(mol.K) / g/mol * 1000g/kg = J/(kg.K)
}

double getWM_modelica(){
	//molecular weight
	return dwm/1000;
}

double getDL_modelica(){
	//density of liquid phase
	return ddl*dwliq;  // mol/l * g/mol = g/l = kg/m3
}

double getDV_modelica(){
	//density of gaseous phase
	return ddv*dwvap;  // mol/l * g/mol = g/l = kg/m3
}

double getQ_modelica(){
	if (lnc>1 && abs(dqmol)<990) { // maintain special values
		if (dwvap==noValue) WMOLlib(dxmolv,dwvap);
		return dqmol*dwvap/dwm;
	} else {
		return dqmol;
	}
}

double getCV_modelica(){
	return dCv/dwm * 1000.0;
}

double getCP_modelica(){
	return dCp/dwm * 1000.0;
}

double getW_modelica(){
	//speed of sound
	return dw;
}

double getWML_modelica(){
	if (dwliq==noValue) WMOLlib(dxmoll,dwliq);
	return dwliq/1000.;
}

double getWMV_modelica(){
	if (dwvap==noValue) WMOLlib(dxmolv,dwvap);
	return dwvap/1000.;
}

//double* getXL_modelica(){
//	double dxlkg[ncmax];
//	XMASSlib(dxmoll,dxlkg,dwliq);
//	return dxlkg;
//}
//
//double* getXV_modelica(){
//	double dxvkg[ncmax];
//	XMASSlib(dxmolv,dxvkg,dwvap);
//	return dxvkg;
//}

double getETA_modelica(){
	return deta/1e6;
}

double getTCX_modelica(){
	return dtcx;
}

double getHJT_modelica() {	// isenthalpic Joule-Thompson coefficient [K/kPa]/1000Pa*kPa = K/Pa
	return dhjt/1000;
}
//double* getZ_modelica() {	// compressibility factor (= PV/RT) [dimensionless]
//	return dZ;
//}
double getA_modelica() {	// Helmholtz energy [J/mol]
	return dA/dwm * 1000.0; // J/mol / g/mol * 1000g/kg = J/kg
}
double getG_modelica() {	// Gibbs free energy [J/mol]
	return dG/dwm * 1000.0; // J/mol / g/mol * 1000g/kg = J/kg
}
double getXKAPPA_modelica() {	// isothermal compressibility (= -1/V dV/dP = 1/rho dD/dP) [1/kPa] /1000Pa*kPa = 1/Pa
	return dxkappa / 1000. ;
}
double getBETA_modelica() {	// volume expansivity (= 1/V dV/dT = -1/rho dD/dT) [1/K]
	return dbeta;
}
double getDPDD_modelica() {	// derivative dP/drho [kPa-L/mol] * 1000Pa/kPa * mol/g = Pa.m3 / kg
	return ddpdd * 1000. / dwm;
}
double getD2PDD2_modelica() {	// derivative d^2P/drho^2 [kPa-L^2/mol^2] * 1000Pa/kPa * mol/g * mol/g = Pa m6 / kg2
	return dd2pdd2 * 1000. / dwm / dwm;
}
double getDPDT_modelica() {	// derivative dP/dT [kPa/K] * 1000Pa/kPa = Pa/K
	return ddpdt * 1000.;
}
double getDDDT_modelica() {	// derivative drho/dT [mol/(L-K)] * g/mol = kg/m3 / K
	return ddddt * dwm;
}
double getDDDP_modelica() {	// derivative drho/dP [mol/(L-kPa)]
	return ddddp*dwm/1000.; // mol/(l.kPa) * g/mol * 1kPa/1000Pa = kg/(m3.Pa)
}
double getD2PDT2_modelica() {	// derivative d2P/dT2 [kPa/K^2] * 1000Pa/kPa = Pa/K2
	return dd2pdt2 * 1000.;
}
double getD2PDTD_modelica() {	// derivative d2P/dTd(rho) [J/mol-K] / g/mol * 1000g/kg = J/kg.K
	return  dd2pdtd/dwm*1000.;
}


double get_dhdt_d_modelica() { //dH/dT at constant density [J/(mol-K)] / g/mol * 1000g/kg = J/kg.K
	return ddhdt_d/dwm*1000;
}
double get_dhdt_p_modelica() { //dH/dT at constant pressure [J/(mol-K)]
	return ddhdt_p/dwm*1000;
}
double get_dhdd_t_modelica() { //dH/drho at constant temperature [(J/mol)/(mol/L)] * mol/g * 1000g/kg / g/mol = (J/kg) / (kg/m3)
	return ddhdd_t /dwm*1000. / dwm;
}
double get_dhdd_p_modelica() { //dH/drho at constant pressure [(J/mol)/(mol/L)]
	return ddhdd_p /dwm*1000. / dwm;
}
double get_dhdp_t_modelica() { //dH/dP at constant temperature [J/(mol-kPa)] /dwm*1000. / (1000Pa/kPa) = J/kg.Pa
	return ddhdp_t / dwm;
}
double get_dhdp_d_modelica() { //dH/dP at constant density [J/(mol-kPa)]
	return ddhdp_d / dwm;
}

// Numerical derivatives
// Derivative of density with respect to enthalpy at constant pressure
double get_dddh_p_num_modelica(){
	return ddddh_p_num*dwm*dwm/1000.; // (mol/l * mol/J) * g/mol * g/mol * 1kg/1000g = kg/m3 * kg/J
}
// Derivative of density with respect to pressure at constant enthalpy
double get_dddp_h_num_modelica(){
	return ddddp_h_num*dwm/1000.; // mol/(l.kPa) * g/mol * 1kPa/1000Pa = kg/(m3.Pa)
}


/*
 * Checks if the provided variables match the currently active
 * state. Needs the current values of the state properties as
 * well as the new ones. Additionally, the mole based composition
 * has to be provided with the number of components.
 * Changing the amount of components or the fluids itself should
 * be covered by the flushing routines above.
 */
bool isState(double var1, double val1, double var2, double val2, double xmol[], long nc){
	if (debug) printf ("\nChecking state.\n");
	if (var1!=val1) return false;
	if (var2!=val2) return false;
	//if (lnc!=nc) return false;
	// If we have a mixture, we need to check the composition.
	if (nc>1) {
		for ( int i = 0; i < nc; i++ ) {
			if (dxmol[i]!=xmol[i]) return false;
		}
	}
	if (debug) printf ("Going to return \"true\".\n");
	return true;
}


double getValue(std::string out) {

	if (debug) printf("\nChecking for %s \n",out.c_str());

	if ( 0 == Poco::icompare(out, "p") ) {
		if (dp!=noValue) return getP_modelica();
	} else if ( 0 == Poco::icompare(out, "t") ) {
		if (dt!=noValue) return getT_modelica();
	} else if ( 0 == Poco::icompare(out, "m") ) {
		if (dwm!=noValue) return getWM_modelica();
	} else if ( 0 == Poco::icompare(out, "d") ) {
		if (dd!=noValue) return getD_modelica();
	} else if ( 0 == Poco::icompare(out, "e") ) {
		if (de!=noValue) return getE_modelica();
	} else if ( 0 == Poco::icompare(out, "h") ) {
		if (dh!=noValue) return getH_modelica();
	} else if ( 0 == Poco::icompare(out, "s") ) {
		if (ds!=noValue) return getS_modelica();
	} else if ( 0 == Poco::icompare(out, "w") ) {
		if (dw!=noValue) return getW_modelica();
	} else if ( 0 == Poco::icompare(out, "v") ) {
		if (deta!=noValue) return getETA_modelica();
	} else if ( 0 == Poco::icompare(out, "l") ) {
		if (dtcx!=noValue) return getTCX_modelica();
	}
	return noValue;
}

///*
// * Improvised derivative computing. These functions are called
// * after properties were calculated. Hence, we have density and
// * pressure available. REFPROP is formulated with explicit d and
// * T it should not take too much extra time.
// */
//double spare3,spare4,spare5,spare6,spare7[ncmax],spare8[ncmax],spare9,spare10,spare11,spare12,spare13,spare14;
//double deltaH,hLow,hHigh,deltaP,pLow,pHigh,rhoLow,rhoHigh;
//int setExtra(bool debug, long lerr, char* errormsg){
//	double rho,T;
//	rho = getValue("d");
//	T = getValue("T");
//	if ((rho!=noValue)&&(T!=noValue)) { // call explicit function
//	// get derivative of density with respect to pressure from Refprop library
//	if (debug) printf("Calling THERM2 with %f and %f.\n",dt,dd);
//	THERM2lib (dt,dd,dxmol,spare5,spare6,spare8,spare9,dCv,dCp,dw,dZ,dhjt,dA,dG,dxkappa,dbeta,ddpdd,dd2pdd2,ddpdt,ddddt,ddddp,dd2pdt2,dd2pdtd,spare3,spare4);
////	deltaP = 1.;
////	pLow   = dp - 0.5*deltaP;
////	pHigh  = dp + 0.5*deltaP;
////	rhoLow = 0;
////	rhoHigh = 0;
////	//PHFLSHlib(dp,hLow,dxmol,dt,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
////	if (debug) printf("Calling PHFLSH with %f and %f.\n",pLow,dh);
////	PHFLSHlib(pLow,dh,dxmol,spare3,rhoLow,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
////	if (debug) printf("Calling PHFLSH with %f and %f.\n",pHigh,dh);
////	PHFLSHlib(pHigh,dh,dxmol,spare3,rhoHigh,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
////	if (debug) printf("Setting ddddp from %f and %f.\n",rhoHigh,rhoLow);
////	ddddp = (rhoHigh-rhoLow) / (pHigh-pLow);
//
//	// get derivative of density with respect to enthalpy numerically
//	deltaH = 20.;
//	hLow   = dh - 0.5*deltaH;
//	hHigh  = dh + 0.5*deltaH;
//	rhoLow = 0;
//	rhoHigh = 0;
//	//PHFLSHlib(dp,hLow,dxmol,dt,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
//	if (debug) printf("Calling PHFLSH with %f and %f.\n",dp,hLow);
//	PHFLSHlib(dp,hLow,dxmol,spare3,rhoLow,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
//	if (debug) printf("Calling PHFLSH with %f and %f.\n",dp,hHigh);
//	PHFLSHlib(dp,hHigh,dxmol,spare3,rhoHigh,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
//	if (debug) printf("Setting dddhp from %f and %f.\n",rhoHigh,rhoLow);
//	ddddh = (rhoHigh-rhoLow) / (hHigh-hLow);
//	} else { // We have a problem!
//		printf("Derivative calculation called at the wrong time: rho=%f and T=%f\n",rho,T);
//	}
//	return 0;
//}

//int updateExtra(double *der, long lerr){
////	c  inputs:
////	c        t--temperature [K]
////	c      rho--molar density [mol/L]
////	c        x--composition [array of mol frac]
////	c  outputs:
////	c        p--pressure [kPa]
////	c        e--internal energy [J/mol]
////	c        h--enthalpy [J/mol]
////	c        s--entropy [J/mol-K]
////	c       Cv--isochoric heat capacity [J/mol-K]
////	c       Cp--isobaric heat capacity [J/mol-K]
////	c        w--speed of sound [m/s]
////	c        Z
////	c      hjt
////	c        A
////	c        G
////	c   xkappa
////	c     beta
////	c   dPdrho
////	c   d2PdD2
////	c      dPT
////	c   drhodT
////	c   drhodP
////	c    d2PT2
////	c   d2PdTD
////	c   sparei--2 space holders for possible future properties
//
//
////    subroutine DHD1(t,rho,x,dhdt_d,dhdt_p,dhdd_t,dhdd_p,dhdp_t,dhdp_d)
////c
////c  compute partial derivatives of enthalpy w.r.t. t, p, or rho at constant
////c  t, p, or rho as a function of temperature, density, and composition
////c
////c  inputs:
////c        t--temperature [K]
////c      rho--molar density [mol/L]
////c        x--composition [array of mol frac]
////c  outputs:
////c   get_dhdt_d_modelica();		--dH/dT at constant density [J/(mol-K)]
////c   get_dhdt_p_modelica();		--dH/dT at constant pressure [J/(mol-K)]
////c   get_dhdd_t_modelica();		--dH/drho at constant temperature [(J/mol)/(mol/L)]
////c   get_dhdd_p_modelica();		--dH/drho at constant pressure [(J/mol)/(mol/L)]
////c   get_dhdp_t_modelica();		--dH/dP at constant temperature [J/(mol-kPa)]
////c   get_dhdp_d_modelica();		--dH/dP at constant density [J/(mol-kPa)]
//
////	double 	dt, dp, de, dh, ds, dqmol, dd, dxmol[ncmax], ddl,
////		ddv, dxmoll[ncmax], dxmolv[ncmax], dCv, dCp, dw, dwliq, dwvap,
////


//
//
//
//
//	getDDDH
//
//
//
//
////	THERM2lib (dt,dd,dxmol,spare5,spare6,spare8,spare9,dCv,dCp,dw,dZ,dhjt,dA,dG,dxkappa,dbeta,ddpdd,dd2pdd2,ddpdt,ddddt,ddddp,dd2pdt2,dd2pdtd,spare3,spare4);
//
//	//ASSIGN VALUES TO RETURN ARRAY
//	der[0] = lerr;//error code
//	der[1] = getP_modelica();   //pressure in Pa
//	der[2] = getT_modelica();   //Temperature in K
//	der[3] = getWM_modelica();  //molecular weight
//	der[4] = getD_modelica();   //density
//	der[5] = getDL_modelica();  //density of liquid phase
//	der[6] = getDV_modelica();  //density of liquid phase
//	der[7] = getQ_modelica();   //vapor quality on a mass basis [mass vapor/total mass] (q=0 indicates saturated liquid, q=1 indicates saturated vapor)
//	der[8] = getE_modelica();   //internal energy
//	der[9] = getH_modelica();   //specific enthalpy
//	der[10] = getS_modelica();  //specific entropy
//	der[11] = getCV_modelica(); // heat capacity
//	der[12] = getCP_modelica(); // heat capacity
//	der[13] = getW_modelica();  //speed of sound
//	der[14] = getDDDH_modelica(); //ddhp
//	der[15] = getDDDP_modelica(); //ddph
//	der[16] = getWML_modelica();
//	der[17] = getWMV_modelica();
//
//	double dxlkg[ncmax], dxvkg[ncmax];
//
//
//	return 0;
//}



int updateDers(double *ders, long lerr){
	//ASSIGN VALUES TO RETURN ARRAY
	ders[0]  = lerr;//error code
	ders[1]  = getHJT_modelica(); 		// isenthalpic Joule-Thompson coefficient [K/Pa]
	ders[2]  = getA_modelica();			// Helmholtz energy [J/kg]
	ders[3]  = getG_modelica();			// Gibbs free energy [J/kg]
	ders[4]  = getXKAPPA_modelica();	// isothermal compressibility (= -1/V dV/dP = 1/rho dD/dP) [1/Pa]
	ders[5]  = getBETA_modelica();		// volume expansivity (= 1/V dV/dT = -1/rho dD/dT) [1/K]
	ders[6]  = getDPDD_modelica();		// derivative dP/drho [Pa-m3/kg]
	ders[7]  = getD2PDD2_modelica();	// derivative d^2P/drho^2 [Pa-m6/kg2]
	ders[8]  = getDPDT_modelica();		// derivative dP/dT [Pa/K]
	ders[9]  = getDDDT_modelica();		// derivative drho/dT [kg/(m3-K)]
	ders[10] = getDDDP_modelica();		// derivative drho/dP [kg/(m3-kPa)]
	ders[11] = getD2PDT2_modelica();	// derivative d2P/dT2 [Pa/K2]
	ders[12] = getD2PDTD_modelica();	// derivative d2P/dTd(rho) [J/kg-K]
	ders[13] = get_dhdt_d_modelica();	// dH/dT at constant density [J/(kg-K)]
	ders[14] = get_dhdt_p_modelica();	// dH/dT at constant pressure [J/(kg-K)]
	ders[15] = get_dhdd_t_modelica();	// dH/drho at constant temperature [(J/kg) / (kg/m3)]
	ders[16] = get_dhdd_p_modelica();	// dH/drho at constant pressure [(J/kg) / (kg/m3)]
	ders[17] = get_dhdp_t_modelica();	// dH/dP at constant temperature [J/(kg-Pa)]
	ders[18] = get_dhdp_d_modelica();	// dH/dP at constant density [J/(kg-Pa)]
	ders[19] = get_dddh_p_num_modelica();	// dD/dh at constant pressure [kg/m3 * kg/J]
	ders[20] = get_dddp_h_num_modelica();	// dD/dp at constant enthalpy [kg/(m3.Pa)]
	return 0;
}

int ders_REFPROP(double *ders, char* errormsg, int DEBUGMODE){
	debug = false;
	if (DEBUGMODE) debug = true;
	long lerr = 0;

	double spare3,spare4,spare5,spare6,spare7[ncmax],spare8[ncmax],spare9,spare10,spare11,spare12,spare13,spare14;
	double deltaH,hLow,hHigh,deltaP,pLow,pHigh,rhoLow,rhoHigh;

	if ((dd!=noValue)&&(dt!=noValue)) {
		// call explicit functions in d and T
		// get derivatives from Refprop library
		if (debug) printf("Calling THERM2 with T=%f and rho=%f.\n",dt,dd);
		THERM2lib (dt,dd,dxmol,spare5,spare6,spare9,spare10,dCv,dCp,dw,dZ,dhjt,dA,dG,dxkappa,dbeta,ddpdd,dd2pdd2,ddpdt,ddddt,ddddp,dd2pdt2,dd2pdtd,spare3,spare4);

		// get derivatives of enthalpy
		if (debug) printf("Calling DHD1 with T=%f and rho=%f.\n",dt,dd);
		DHD1lib(dt,dd,dxmol,ddhdt_d,ddhdt_p,ddhdd_t,ddhdd_p,ddhdp_t,ddhdp_d);

		// get derivative of density with respect to enthalpy numerically
		deltaP = 0.005; // 5 Pascal difference
		pLow   = dp - 0.5*deltaP;
		pHigh  = dp + 0.5*deltaP;
		rhoLow = 0;
		rhoHigh = 0;
		//PHFLSHlib(dp,hLow,dxmol,dt,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
		if (debug) printf("Calling PHFLSH with %f and %f.\n",pLow,dh);
		PHFLSHlib(pLow,dh,dxmol,spare3,rhoLow,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
		if (debug) printf("Calling PHFLSH with %f and %f.\n",pHigh,dh);
		PHFLSHlib(pHigh,dh,dxmol,spare3,rhoHigh,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
		if (debug) printf("Setting dddp_h_num from %f and %f.\n",rhoHigh,rhoLow);
		ddddp_h_num = (rhoHigh-rhoLow) / (pHigh-pLow);

		// get derivative of density with respect to enthalpy numerically
		deltaH = 5.; // 5 Joule total difference
		hLow   = dh - 0.5*deltaH;
		hHigh  = dh + 0.5*deltaH;
		rhoLow = 0;
		rhoHigh = 0;
		//PHFLSHlib(dp,hLow,dxmol,dt,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
		if (debug) printf("Calling PHFLSH with p=%f and h=%f for derivative.\n",dp,hLow);
		PHFLSHlib(dp,hLow,dxmol,spare3,rhoLow,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
		if (debug) printf("Calling PHFLSH with p=%f and h=%f for derivative.\n",dp,hHigh);
		PHFLSHlib(dp,hHigh,dxmol,spare3,rhoHigh,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
		if (debug) printf("Setting dddh_p_num from %f and %f.\n",rhoHigh,rhoLow);
		ddddh_p_num = (rhoHigh-rhoLow) / (hHigh-hLow);

	} else { // We have a problem!
		printf("Derivatives and transport properties calculations called at the wrong time: rho=%f and T=%f\n",dd,dt);
	}

	switch(lerr){
		case 4:
			sprintf(errormsg,"P=%f < 0",dp);
			break;
		case 8:
			sprintf(errormsg,"x out of range (component and/or sum < 0 or > 1):%s",printX(dxmol,lnc).c_str());
			break;
		case 12:
			sprintf(errormsg,"x=%s out of range and P=%f < 0",printX(dxmol,lnc).c_str(),dp);
			break;
		case 249:
			sprintf(errormsg,"PHFLSH error: Input value of enthalpy (%f) is outside limits",dh);
			break;
		default:
			break;
	}
	return updateDers(ders, lerr);
}

int updateTrns(double *trns, long lerr){
	//ASSIGN VALUES TO RETURN ARRAY
	trns[0] = lerr;//error code
	trns[1] = getETA_modelica(); // dynamic viscosity in Pa.s
	trns[2] = getTCX_modelica(); // thermal conductivity in W/m.K
	return 0;
}

int trns_REFPROP(double *trns, char* errormsg, int DEBUGMODE){
	debug = false;
	if (DEBUGMODE) debug = true;
	long lerr = 0;

	if ((dd!=noValue)&&(dt!=noValue)) {
		// call explicit functions in d and T
		// compute transport properties
		if (debug) printf("Getting transport properties from T=%f and rho=%f.\n",dt,dd);
		TRNPRPlib(dt,dd,dxmol,deta,dtcx,lerr,errormsg,errormessagelength);
		if (debug) printf("Thermal conductivity is lambda=%f W/m.K.\n",dtcx);
		if (debug) printf("Dynamic viscosity is eta=%fµPa.s.\n",deta);

	} else { // We have a problem!
		printf("Derivatives and transport properties calculations called at the wrong time: rho=%f and T=%f\n",dd,dt);
	}

	switch(lerr){
		case -31:
			sprintf(errormsg,"Temperature T=%f out of range for conductivity",dt);
			break;
		case -32:
			sprintf(errormsg,"density d=%f out of range for conductivity",dd);
			break;
		case -33:
			sprintf(errormsg,"Temperature T=%f and density d=%f out of range for conductivity",dt,dd);
			break;
		case -41:
			sprintf(errormsg,"Temperature T=%f out of range for viscosity",dt);
			break;
		case -42:
			sprintf(errormsg,"density d=%f out of range for viscosity",dd);
			break;
		case -43:
			sprintf(errormsg,"Temperature T=%f and density d=%f out of range for viscosity",dt,dd);
			break;
		case -51:
			sprintf(errormsg,"Temperature T=%f out of range for conductivity and viscosity",dt);
			break;
		case -52:
			sprintf(errormsg,"density d=%f out of range for conductivity and viscosity",dd);
			break;
		case -53:
			sprintf(errormsg,"Temperature T=%f and density d=%f out of range for conductivity and viscosity",dt,dd);
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
			sprintf(errormsg,"exactly at T=%f, rhoc for a pure fluid; k is infinite",dt);
			break;
		case -58:
		case -59:
			sprintf(errormsg,"ECS model did not converge");
			break;
		default:
			break;
	}
	return updateTrns(trns, lerr);
}


int updateProps(double *props, long lerr){
	//ASSIGN VALUES TO RETURN ARRAY
	props[0] = lerr;//error code
	props[1] = getP_modelica();		//pressure in Pa
	props[2] = getT_modelica();		//Temperature in K
	props[3] = getWM_modelica();	//molecular weight
	props[4] = getD_modelica();		//density
	props[5] = getDL_modelica();	//density of liquid phase
	props[6] = getDV_modelica();	//density of liquid phase
	props[7] = getQ_modelica();		//vapor quality on a mass basis [mass vapor/total mass] (q=0 indicates saturated liquid, q=1 indicates saturated vapor)
	props[8] = getE_modelica();		//inner energy
	props[9] = getH_modelica();		//specific enthalpy
	props[10] = getS_modelica();	//specific entropy
	props[11] = getCV_modelica();
	props[12] = getCP_modelica();
	props[13] = getW_modelica(); 	//speed of sound
	props[14] = getWML_modelica();
	props[15] = getWMV_modelica();

	double dxlkg[ncmax], dxvkg[ncmax];

	XMASSlib(dxmoll,dxlkg,dwliq);
	XMASSlib(dxmolv,dxvkg,dwvap);

	for (int dim=0; dim<lnc; dim++){
		//if (debug) printf("Processing %i:%f, %f \n",dim,dxlkg[dim],dxvkg[dim]);
		props[16+dim] = dxlkg[dim];
		props[16+lnc+dim] = dxvkg[dim];
	}
	return 0;
}

double props_REFPROP(char* what, char* statevars_in, char* fluidnames, double *ders, double *trns, double *props, double statevar1, double statevar2, double* x, int phase, char* REFPROP_PATH, char* errormsg, int DEBUGMODE){
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
	//char statevars[3];
	//double p, T, d, val, dl,dv,q,e,h,s,cv,cp,w,wm,wmliq,wmvap,eta,tcx;
	//long nX,ierr=0; //zero means no error
    //char herr[errormessagelength+1];
//    HINSTANCE RefpropdllInstance;// Then have windows load the library.
//    Poco::SharedLibrary RefpropdllInstance("");

	long lerr = 0;



//    DEBUGMODE = 1;
	if (DEBUGMODE) debug = true;
	std::string out 	= std::string(what).substr(0,1);
	std::string in1 	= std::string(statevars_in).substr(0,1);
	std::string in2 	= std::string(statevars_in).substr(1,1);
	std::string fluids 	= std::string(fluidnames);
	std::string rPath 	= std::string(REFPROP_PATH);




	/*
	 * Call method to initialise the library and check for new fluids.
	 * Afterwards, the fluids have been processed and the constants might
	 * have been flushed.
	 */
	if (debug) printf("\nStarting function props_REFPROP to calculate %s.\n", out.c_str());

	lerr = init_REFPROP(fluids, rPath, errormsg);

	if (lerr!=0) {
		printf("Error no. %li initialising REFPROP: \"%s\"\n", lerr, errormsg);
		return lerr;
	}


	/*
	 * Here should be the state checking to avoid unnecessary calculations. when
	 * working with mixtures, the constants also have to be flushed if the
	 * composition changes.
	 */
	// Convert mass-based composition to mole fractions and set molecular weight.
	double* dxkg;
	double dxmoltmp[ncmax];
	double dwmtmp;
	//dxkg = (double*) calloc(ncmax,sizeof(double));
	//dxmoltmp = (double*) calloc(ncmax,sizeof(double));
	dxkg = x;
	XMOLElib(dxkg,dxmoltmp,dwmtmp);
	// dwm = dwm / 1000; // from g/mol to kg/mol

	double val1,val2,var1,var2,var,val;
	double tmpValue;
	std::string tmpVar;
	for (int ii=1;ii<3;ii++){

		if (ii==1) {
			tmpVar = in1;
			tmpValue = statevar1;
		} else if (ii==2) {
			tmpVar = in2;
			tmpValue = statevar2;
		}

		// loop through possible inputs
		if ( 0 == Poco::icompare(tmpVar, "p") ) {
			var = dp;
			val = getP_refprop(tmpValue);
		} else if ( 0 == Poco::icompare(tmpVar, "T") ) {
			var = dt;
			val = getT_refprop(tmpValue);
		} else if ( 0 == Poco::icompare(tmpVar, "s") ) {
			var = ds;
			val = getS_refprop(tmpValue);
		} else if ( 0 == Poco::icompare(tmpVar, "h") ) {
			var = dh;
			val = getH_refprop(tmpValue);
		} else if ( 0 == Poco::icompare(tmpVar, "d") ) {
			var = dd;
			val = getD_refprop(tmpValue);
		//} else if ( 0 == Poco::icompare(tmpVar, "q") ) {
		//	var = dqkg = tmpValue;
		} else {
			//lerr = 2;
			//sprintf(errormsg,"Unknown state variable %i: %s",ii ,tmpVar.c_str());
			//return lerr;
		}

		if (ii==1) {
			val1 = val;
			var1 = var;
		} else if (ii==2) {
			val2 = val;
			var2 = var;
		}

		//if (debug) printf("Checked input variable: %s\n",tmpVar.c_str());
	}

	bool knownState = isState(var1,val1,var2,val2,dxmoltmp,lnc); // dummies to force recalculation
	double result = getValue(out);
	bool valueExists = (result!=noValue);

	if (!knownState) {
		if (lnc>1) flushConstants();
		else flushProperties();
		if (debug) printf("Loading a new state, flushed state. \n");
	} else { // We have calculated it before.
		if (valueExists) {
			if (debug) printf("Working with old state, returning value for %s: %f.\n",out.c_str(),result);
			updateProps(props, lerr);
			return result;
		}
	}


	/*
	 * If we get to this point, the requested value was not part of an earlier
	 * calculation and we have to proceed to determine it via the Refprop library.
	 */
	// 	( 0 == Poco::icompare(Poco::Environment::osName(), "linux") )

	// Set variables to input values
	if ( 0 == Poco::icompare(in1, in2) ) {
		sprintf(errormsg,"State variable 1 is the same as state variable 2 (%s)\n",in1.c_str());
		return -1;
	}

	memcpy(dxmol, dxmoltmp, sizeof(dxmoltmp)) ;
	//dxmol = dxmoltmp;
	dwm = dwmtmp;

	double dqkg;
	for (int ii=1;ii<3;ii++){

		if (ii==1) {
			tmpVar = in1;
			tmpValue = statevar1;
		} else if (ii==2) {
			tmpVar = in2;
			tmpValue = statevar2;
		}

		// loop through possible inputs
		if ( 0 == Poco::icompare(tmpVar, "p") ) {
			dp = getP_refprop(tmpValue);
		} else if ( 0 == Poco::icompare(tmpVar, "T") ) {
			dt = getT_refprop(tmpValue);
		} else if ( 0 == Poco::icompare(tmpVar, "s") ) {
			ds = getS_refprop(tmpValue);
		} else if ( 0 == Poco::icompare(tmpVar, "h") ) {
			dh = getH_refprop(tmpValue);
		} else if ( 0 == Poco::icompare(tmpVar, "d") ) {
			dd = getD_refprop(tmpValue);
		} else if ( 0 == Poco::icompare(tmpVar, "q") ) {
			dqkg = tmpValue;
		} else {
			lerr = 2;
			sprintf(errormsg,"Unknown state variable %i: %s",ii ,tmpVar.c_str());
			return lerr;
		}

		if (debug) printf("Checked input variable: %s\n",tmpVar.c_str());
	}

	if (lerr==0){
		if (isInput(in1,in2,std::string("tp"))){
//			if (phase==2){ //fluid state is known to be two phase
//				TPFL2lib(dt,dp,dxmol,ddl,ddv,dxmoll,dxmolv,dqmol,lerr,errormsg,errormessagelength);
//			}else{
				if (debug) printf("Calling TPFLSH with %f and %f.\n",dt,dp);
				TPFLSHlib(dt,dp,dxmol,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,dh,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
				//if (debug) printf("Getting dd: %f\n",dd);
//			}
		}else if (isInput(in1,in2,std::string("ph"))){
//			if (phase==1){ //fluid state is known to be single phase
//				PHFL1(p,h,x,liqvap,T,d,ierr,herr,errormessagelength);
////				if (liqvap==1) dl=d; else dv=d;
//			}else{
				if (debug) printf("Calling PHFLSH with %f and %f.\n",dp,dh);
				PHFLSHlib(dp,dh,dxmol,dt,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
//			}
		}else if (isInput(in1,in2,std::string("pd"))){
			if (phase==1){ //fluid state is known to be single phase
				PDFL1lib(dp,dd,dxmol,dt,lerr,errormsg,errormessagelength);
			}else{
				if (debug) printf("Calling PDFLSH with %f and %f.\n",dp,dd);
				PDFLSHlib(dp,dd,dxmol,dt,ddl,ddv,dxmoll,dxmolv,dqmol,de,dh,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
			}
		}else if (isInput(in1,in2,std::string("sp"))){
/*			if (phase==1){ //fluid state is known to be single phase
				PSFL1(p,s,dxmol,kph,T,d,lerr,errormsg,errormessagelength);
				if (liqvap==1) dl=d; else dv=d;
			}else{*/
				if (debug) printf("Calling PSFLSH with %f and %f.\n",dp,ds);
				PSFLSHlib(dp,ds,dxmol,dt,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,dh,dCv,dCp,dw,lerr,errormsg,errormessagelength);
//			}
		}else if (isInput(in1,in2,std::string("pq"))){
			if (debug) printf("Calling PQFLSH with %f and %f.\n",dp,dqkg);
			PQFLSHlib(dp,dqkg,dxmol,kq,dt,dd,ddl,ddv,dxmoll,dxmolv,de,dh,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
//			strcat(errormsg,"Bin in PQ!");
		}else if (isInput(in1,in2,std::string("th"))){
/*			if (phase==1){ //fluid state is known to be single phase
				THFL1(T,h,dxmol,Dmin,Dmax,d,lerr,errormsg,errormessagelength);
			}else{*/
				long kr = 2;
/*       kr--phase flag: 1 = input state is liquid
		      2 = input state is vapor in equilibrium with liq
		      3 = input state is liquid in equilibrium with solid
		      4 = input state is vapor in equilibrium with solid */
				if (debug) printf("Calling THFLSH with %f and %f.\n",dt,dh);
				THFLSHlib(dt,dh,dxmol,kr,dp,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
//			}
		}else if (isInput(in1,in2,std::string("td"))){
			if (debug) printf("Calling TDFLSH with %f and %f.\n",dt,dd);
			TDFLSHlib(dt,dd,dxmol,dp,ddl,ddv,dxmoll,dxmolv,dqmol,de,dh,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
		}else if (isInput(in1,in2,std::string("ts"))){
			long kr = 2;
			if (debug) printf("Calling TSFLSH with %f and %f.\n",dt,ds);
			TSFLSHlib(dt,ds,dxmol,kr,dp,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,dh,dCv,dCp,dw,lerr,errormsg,errormessagelength);
		}else if (isInput(in1,in2,std::string("tq"))){
			if (debug) printf("Calling TQFLSH with %f and %f.\n",dt,dqkg);
			TQFLSHlib(dt,dqkg,dxmol,kq,dp,dd,ddl,ddv,dxmoll,dxmolv,de,dh,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
		}else if (isInput(in1,in2,std::string("dh"))){
			switch(phase){ //fluid state is known to be single phase
				case 1:
					DHFL1lib(dd,dh,dxmol,dt,lerr,errormsg,errormessagelength);
					break;
				case 2:
					DHFL2lib(dd,dh,dxmol,dt,dp,ddl,ddv,dxmoll,dxmolv,dqmol,lerr,errormsg,errormessagelength);
					break;
				default:
					if (debug) printf("Calling DHFLSH with %f and %f.\n",dd,dh);
					DHFLSHlib(dd,dh,dxmol,dt,dp,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
					break;
			}
		}else if (isInput(in1,in2,std::string("hs"))){
			HSFLSHlib(dh,ds,dxmol,dt,dp,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,dCv,dCp,dw,lerr,errormsg,errormessagelength);
		}else if (isInput(in1,in2,std::string("ds"))){
			switch(phase){ //fluid state is known to be single phase
				case 1:
					DSFL1lib(dd,ds,dxmol,dt,lerr,errormsg,errormessagelength);
					break;
				case 2:
					DSFL2lib(dd,ds,dxmol,dt,dp,ddl,ddv,dxmoll,dxmolv,dqmol,lerr,errormsg,errormessagelength);
					break;
				default:
					if (debug) printf("Calling DSFLSH with %f and %f.\n",dd,ds);
					DSFLSHlib(dd,ds,dxmol,dt,dp,ddl,ddv,dxmoll,dxmolv,dqmol,de,dh,dCv,dCp,dw,lerr,errormsg,errormessagelength);
					break;
			}
		}else
			sprintf(errormsg,"Unknown combination of state variables! %s and %s", in1.c_str(), in2.c_str());
	}


//	switch(tolower(what[0])){ 	//CHOOSE RETURN VARIABLE
//		case 'v':	//dynamic viscosity uPa.s
//		case 'l':	//thermal conductivity W/m.K
//			TRNPRPlib(dt,dd,dxmol,deta,dtcx,lerr,errormsg,errormessagelength);
//			break;
//		}


	switch(lerr){
		case 1:
			sprintf(errormsg,"T=%f < Tmin",dt);
			break;
		case 4:
			sprintf(errormsg,"P=%f < 0",dp);
			break;
		case 5:
			sprintf(errormsg,"T=%f and p=%f out of range",dt,dp);
			break;
		case 8:
			sprintf(errormsg,"x out of range (component and/or sum < 0 or > 1):%s",printX(dxmol,lnc).c_str());
			break;
		case 9:
			sprintf(errormsg,"x=%s or T=%f out of range",printX(dxmol,lnc).c_str(),dt);
			break;
		case 12:
			sprintf(errormsg,"x=%s out of range and P=%f < 0",printX(dxmol,lnc).c_str(),dp);
			break;
		case 13:
			sprintf(errormsg,"x=%s, T=%f and p=%f out of range",printX(dxmol,lnc).c_str(),dt,dp);
			break;
		case 16:
			strcpy(errormsg,"TPFLSH error: p>melting pressure");
			break;
		case -31:
			sprintf(errormsg,"Temperature T=%f out of range for conductivity",dt);
			break;
		case -32:
			sprintf(errormsg,"density d=%f out of range for conductivity",dd);
			break;
		case -33:
			sprintf(errormsg,"Temperature T=%f and density d=%f out of range for conductivity",dt,dd);
			break;
		case -41:
			sprintf(errormsg,"Temperature T=%f out of range for viscosity",dt);
			break;
		case -42:
			sprintf(errormsg,"density d=%f out of range for viscosity",dd);
			break;
		case -43:
			sprintf(errormsg,"Temperature T=%f and density d=%f out of range for viscosity",dt,dd);
			break;
		case -51:
			sprintf(errormsg,"Temperature T=%f out of range for conductivity and viscosity",dt);
			break;
		case -52:
			sprintf(errormsg,"density d=%f out of range for conductivity and viscosity",dd);
			break;
		case -53:
			sprintf(errormsg,"Temperature T=%f and density d=%f out of range for conductivity and viscosity",dt,dd);
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
			sprintf(errormsg,"exactly at T=%f, rhoc for a pure fluid; k is infinite",dt);
			break;
		case -58:
		case -59:
			sprintf(errormsg,"ECS model did not converge");
			break;
		case 211:
			sprintf(errormsg,"TPFLSH bubble point calculation did not converge: [SATTP error 1] iteration failed to converge");
			break;
		case 239:
			sprintf(errormsg,"THFLSH error: Input value of enthalpy (%f) is outside limits",dh);
			break;
		case 248:
			sprintf(errormsg,"DSFLSH error: Iteration did not converge with d=%f and s=%f",dd,ds);
			break;
		case 249:
			sprintf(errormsg,"PHFLSH error: Input value of enthalpy (%f) is outside limits",dh);
			break;
		case 271:
			sprintf(errormsg,"TQFLSH error: T=%f > Tcrit, T-q calculation not possible",dt);
			break;
		case 291:
			sprintf(errormsg,"PQFLSH error: p=%f > pcrit, p-q calculation not possible",dt);
			break;
		default:
			//strncpy(errormsg,errormsg,errormessagelength);
			break;
	}


	updateProps(props, lerr);

	int outVal = ders_REFPROP(ders,errormsg,debug);
	if ( 0 != outVal || ders[0] != 0 ) printf("Error in derivative function, returned %i\n",outVal);

	outVal = trns_REFPROP(trns,errormsg,debug);
	if ( 0 != outVal || trns[0] != 0 ) printf("Error in transport property function, returned %i\n",outVal);


	if ( 0 == Poco::icompare(out, "p") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getP_modelica());
		return getP_modelica();
	} else if ( 0 == Poco::icompare(out, "t") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getT_modelica());
		return getT_modelica();
	} else if ( 0 == Poco::icompare(out, "m") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getWML_modelica());
		return getWM_modelica();
	} else if ( 0 == Poco::icompare(out, "d") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getD_modelica());
		return getD_modelica();
	} else if ( 0 == Poco::icompare(out, "q") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getQ_modelica());
		return getQ_modelica();
	} else if ( 0 == Poco::icompare(out, "e") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getE_modelica());
		return getE_modelica();
	} else if ( 0 == Poco::icompare(out, "h") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getH_modelica());
		return getH_modelica();
	} else if ( 0 == Poco::icompare(out, "s") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getS_modelica());
		return getS_modelica();
	} else if ( 0 == Poco::icompare(out, "w") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getW_modelica());
		return getW_modelica();
	} else if ( 0 == Poco::icompare(out, "v") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getETA_modelica());
		return getETA_modelica();
	} else if ( 0 == Poco::icompare(out, "l") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getTCX_modelica());
		return getTCX_modelica();
	} else {
		return -1.0;
	}

}


//---------------------------------------------------------------------------


double satprops_REFPROP(char* what, char* statevar_in, char* fluidnames, double *ders, double *trns, double *props, double statevarval, double* x, char* REFPROP_PATH, char* errormsg, int DEBUGMODE){
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


	long lerr = 0;


//    DEBUGMODE = 1;
	if (DEBUGMODE) debug = true;
	std::string out 	= std::string(what).substr(0,1);
	std::string in1 	= std::string(statevar_in).substr(0,1);
	std::string fluids 	= std::string(fluidnames);
	std::string rPath 	= std::string(REFPROP_PATH);


	/*
	 * Call method to initialise the library and check for new fluids.
	 * Afterwards, the fluids have been processed and the constants might
	 * have been flushed.
	 */
	if (debug) printf("\nStarting function satprops_REFPROP to calculate %s.\n", out.c_str());

	lerr = init_REFPROP(fluids, rPath, errormsg);

	if (lerr!=0) {
		printf("Error no. %li initialising REFPROP: \"%s\"\n", lerr, errormsg);
		return lerr;
	}


	/*
	 * Here should be the state checking to avoid unnecessary calculations. when
	 * working with mixtures, the constants also have to be flushed if the
	 * composition changes.
	 */
	bool knownState = false; // dummies to force recalculation
	bool valueExists = false;

	if (!knownState) {
		if (lnc>1) flushConstants();
		else flushProperties();
		if (debug) printf("Loading a new state, flushed state. \n");
	} else { // We have calculated it before.
		if (valueExists) {
			if (debug) printf("Working with old state, returning value for %i: %f.\n",what[0],-1.0);
			return -1.0;
		}
	}


	/*
	 * If we get to this point, the requested value was not part of an earlier
	 * calculation and we have to proceed to determine it via the Refprop library.
	 */

	// Convert mass-based composition to mole fractions and set molecular weight.
	double* dxkg;
	dxkg = x;
	XMOLElib(dxkg,dxmol,dwm);
	// dwm = dwm / 1000; // from g/mol to kg/mol

	// loop through possible inputs
	if ( 0 == Poco::icompare(in1, "p") ) {
		dp = getP_refprop(statevarval);
	} else if ( 0 == Poco::icompare(in1, "t") ) {
		dt = getT_refprop(statevarval);
	} else if ( 0 == Poco::icompare(in1, "d") ) {
		dd = getD_refprop(statevarval);
	} else {
		lerr = 2;
		sprintf(errormsg,"Unknown state variable: %s\n", in1.c_str());
		return lerr;
	}
	if (debug)  printf("\nstatevar %s checked\n",in1.c_str());

	long j=2;
/* j--phase flag: 1 = input x is liquid composition (bubble point)
		    2 = input x is vapor composition (dew point)
		    3 = input x is liquid composition (freezing point)
		    4 = input x is vapor composition (sublimation point)
*/
	long kph = -1;
/* kph--flag specifying desired root for multi-valued inputs
           has meaning only for water at temperatures close to its triple point
          -1 = return middle root (between 0 and 4 C)
           1 = return highest temperature root (above 4 C)
           3 = return lowest temperature root (along freezing line) */
	if (lerr==0) {
		if ( 0 == Poco::icompare(in1, "t") ) {
			SATTlib(dt,dxmol,j,dp,ddl,ddv,dxmoll,dxmolv,lerr,errormsg,errormessagelength);
		} else if ( 0 == Poco::icompare(in1, "p") ) {
			SATPlib(dp,dxmol,j,dt,ddl,ddv,dxmoll,dxmolv,lerr,errormsg,errormessagelength);
			switch(lerr){
				case 2:
					strcpy(errormsg,"P < Ptp");
					break;
				case 4:
					strcpy(errormsg,"P < 0");
					break;
			}
			//sprintf(errormsg,"p=%f, h=%f",p ,statevar2);
		} else if ( 0 == Poco::icompare(in1, "d") ) {
			SATDlib(dd,dxmol,j,kph,dt,dp,ddl,ddv,dxmoll,dxmolv,lerr,errormsg,errormessagelength);
			switch(lerr){
				case 2:
					strcpy(errormsg,"D > Dmax");
					break;
			}
		}
	}

	switch(lerr){
		case 0:
			strcpy(errormsg,"Saturation routine successful");
			break;
		case 1:
			sprintf(errormsg,"T=%f < Tmin",dt);
			break;
		case 8:
			sprintf(errormsg,"x out of range, %s",printX(dxmol,lnc).c_str());
			break;
		case 9:
			sprintf(errormsg,"T=%f and x=%s out of range",dt,printX(dxmol,lnc).c_str());
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
			//strncpy(errormsg,herr,errormessagelength);
			break;
	}

	//ASSIGN VALUES TO RETURN ARRAY
	props[0] = lerr;//error code
	props[1] = getP_modelica();		//pressure in kPa->Pa
	props[2] = getT_modelica();		//Temperature in K
	props[3] = getWM_modelica();	//molecular weight
	props[4] = getD_modelica();		//density
	props[5] = getDL_modelica();	//density of liquid phase
	props[6] = getDV_modelica();	//density of liquid phase
	props[7] = 0;
	props[8] = 0;	//inner energy
	props[9] = 0;	//specific enthalpy
	props[10] = 0;	//specific entropy
	props[11] = 0;
	props[12] = 0;
	props[13] = 0; //speed of sound
	props[14] = getWML_modelica();
	props[15] = getWMV_modelica();

	double dxlkg[ncmax], dxvkg[ncmax];

	XMASSlib(dxmoll,dxlkg,dwliq);
	XMASSlib(dxmolv,dxvkg,dwvap);

	for (int ii=0;ii<lnc;ii++){
		props[16+ii] = dxlkg[ii];
		props[16+lnc+ii] = dxvkg[ii];
	}

	if (debug) printf("Returning %s\n",out.c_str());

	if ( 0 == Poco::icompare(out, "p") ) {
		return getP_modelica();
	} else if ( 0 == Poco::icompare(out, "t") ) {
		return getT_modelica();
	} else if ( 0 == Poco::icompare(out, "m") ) {
		return getWM_modelica();
	} else if ( 0 == Poco::icompare(out, "d") ) {
		return getD_modelica();
	} else {
		props[0] = 2.0;
		sprintf(errormsg,"Cannot return variable %s in saturation calculation", out.c_str());
		return -1.0;
	}
}
