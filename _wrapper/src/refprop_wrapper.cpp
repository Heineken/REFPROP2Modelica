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
#include "refprop_library.h"
#include "refprop_wrapper.h"

#include <stdlib.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <vector>

#if defined(__ISWINDOWS__)
#include <windows.h>
#define pathSep STR_VALUE(\\)
#define libName STR_VALUE(refprop.dll)
#define _CRT_SECURE_NO_WARNINGS
#elif defined(__ISLINUX__)
#include <dlfcn.h>
#define pathSep STR_VALUE(/)
#define libName STR_VALUE(librefprop.so)
#elif defined(__ISAPPLE__)
#include <dlfcn.h>
#define pathSep STR_VALUE(/)
#define libName STR_VALUE(librefprop.dylib)
#endif


// Some constants for REFPROP... defined by macros for ease of use
#define refpropcharlength 255
#define refpropcharlong 10000
#define filepathlength 255
#define lengthofreference 3
#define errormessagelength 255
#define ncmax 20		// Note: ncmax is the max number of components
#define numparams 72
#define maxcoefs 50

/*
 * Define pointer to library as well as the
 * strings we need to determine whether a
 * call to SETUPdll is needed.
 */
#if defined(__ISWINDOWS__)
HINSTANCE RefpropdllInstance= NULL;
#elif defined(__ISLINUX__)
void *RefpropdllInstance = NULL;
#elif defined(__ISAPPLE__)
void *RefpropdllInstance = NULL;
#else
void *RefpropdllInstance = NULL;
#endif
std::string loadedFluids;



/*
 * Here we define the fluid's properties. These values get updated after each
 * call to Refprop and are used for caching values. I decided to use the
 * Refprop units for internal data storage Be careful when converting
 * properties using the molecular weight. It is stored in g/mol.
 */
     long kq 				= 2; // all qualities are calculated on a mass basis
static const double noValue	= 0;

//static const std::string FLUIDS_PATH = "fluids";
//static const std::string LIN_LIBRARY = "librefprop.so";
//static const std::string WIN_LIBRARY = "refprop.dll";
//Poco::Path FLD_PATH(true);


//	static const double noFactor   = 1e+0;
//	static const double presFactor = 1e+3;
//	static const double viscFactor = 1e+6;
//	static const double molwFactor = 1e-3;
//  double energyFactor          = noValue;   // J/mol / g/mol * 1000g/kg = J/kg
//  double densityFactor[3]      = {noValue}; // mol/l * g/mol = g/l = kg/m3 for all phases
//  double fractionFactor[ncmax] = {noValue}; // xkgi  = xmi * mwi * mw  (array)
//  double qualityFactor         = noValue;   // qmass = qmole * mw_vap / mw

bool   debug;      		// set the debug flag
bool calcTrans;
//bool calcTwoPhaseNumDers;
//bool calcTwoPhasePsuedoAnalDers;
//bool dynstatesTPX;
int PartialDersInputChoice;

//long   lerr;  			// Error return mechanism
double dhelp = noValue;

// Properties for setSat functions
double dxmolsat[ncmax], dwmsat, dpsat, dtsat,ddlsat,ddvsat,
	dxmollsat[ncmax],dxmolvsat[ncmax],dplsat,dhlsat,dslsat,
	dcvlsat,dcplsat,dwlsat,dpvsat,dhvsat,dsvsat,dcvvsat,dcpvsat,
	dwvsat,dtlsat,dtvsat;

/*
 * Most of the fluid properties are stored here. There are arrays for
 * composition information as well as single values for the other
 * properties.
 * The flush function gets called when the state changes and the
 * previously calculated are not valid anymore.
 */
double 	dt, dp, de, dh, ds, dqmol, dd, dxmol[ncmax], ddl,
	ddv, dxmoll[ncmax], dxmolv[ncmax], dCv, dCp, dw, dwliq, dwvap,
	dhjt, dZ[ncmax], dA, dG, dxkappa, dbeta;

double *dxkg;

// viscosity and thermal conductivity
double deta, dtcx;

// partial derivatives
double ddddp_h, ddddh_p, ddddX_ph,
	ddddp_T, ddddT_p, ddddX_pT,
	ddhdp_T, ddhdT_p, ddhdX_pT;

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
	deta=noValue;
	dtcx=noValue;

	ddddX_ph=noValue;
	ddddp_h=noValue;
	ddddh_p=noValue;

	ddddp_T=noValue;
	ddddT_p=noValue;
	ddddX_pT=noValue;
	ddhdp_T=noValue;
	ddhdT_p=noValue;
	ddhdX_pT=noValue;

	if (debug) printf ("Finished flushing normal fluid properties.\n");
	return 0;
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

char hfmix[] = "HMX.BNC";
char hrf[] = "DEF";





/*
 * Define functions as pointers and initialise them to NULL
 * Declare the functions for direct access
 */
RPVersion_POINTER RPVersion;
SETPATHdll_POINTER SETPATHdll;
ABFL1dll_POINTER ABFL1dll;
ABFL2dll_POINTER ABFL2dll;
ACTVYdll_POINTER ACTVYdll;
AGdll_POINTER AGdll;
CCRITdll_POINTER CCRITdll;
CP0dll_POINTER CP0dll;
CRITPdll_POINTER CRITPdll;
CSATKdll_POINTER CSATKdll;
CV2PKdll_POINTER CV2PKdll;
CVCPKdll_POINTER CVCPKdll;
CVCPdll_POINTER CVCPdll;
DBDTdll_POINTER DBDTdll;
DBFL1dll_POINTER DBFL1dll;
DBFL2dll_POINTER DBFL2dll;
DDDPdll_POINTER DDDPdll;
DDDTdll_POINTER DDDTdll;
DEFLSHdll_POINTER DEFLSHdll;
DHD1dll_POINTER DHD1dll;
DHFLSHdll_POINTER DHFLSHdll;
DHFL1dll_POINTER DHFL1dll;
DHFL2dll_POINTER DHFL2dll;
DIELECdll_POINTER DIELECdll;
DOTFILLdll_POINTER DOTFILLdll;
DPDD2dll_POINTER DPDD2dll;
DPDDKdll_POINTER DPDDKdll;
DPDDdll_POINTER DPDDdll;
DPDTKdll_POINTER DPDTKdll;
DPDTdll_POINTER DPDTdll;
DPTSATKdll_POINTER DPTSATKdll;
DSFLSHdll_POINTER DSFLSHdll;
DSFL1dll_POINTER DSFL1dll;
DSFL2dll_POINTER DSFL2dll;
ENTHALdll_POINTER ENTHALdll;
ENTROdll_POINTER ENTROdll;
ESFLSHdll_POINTER ESFLSHdll;
FGCTYdll_POINTER FGCTYdll;
FPVdll_POINTER FPVdll;
GERG04dll_POINTER GERG04dll;
GETFIJdll_POINTER GETFIJdll;
GETKTVdll_POINTER GETKTVdll;
GIBBSdll_POINTER GIBBSdll;
HSFLSHdll_POINTER HSFLSHdll;
INFOdll_POINTER INFOdll;
LIMITKdll_POINTER LIMITKdll;
LIMITSdll_POINTER LIMITSdll;
LIMITXdll_POINTER LIMITXdll;
MELTPdll_POINTER MELTPdll;
MELTTdll_POINTER MELTTdll;
MLTH2Odll_POINTER MLTH2Odll;
NAMEdll_POINTER NAMEdll;
PDFL1dll_POINTER PDFL1dll;
PDFLSHdll_POINTER PDFLSHdll;
PEFLSHdll_POINTER PEFLSHdll;
PHFL1dll_POINTER PHFL1dll;
PHFLSHdll_POINTER PHFLSHdll;
PQFLSHdll_POINTER PQFLSHdll;
PREOSdll_POINTER PREOSdll;
PRESSdll_POINTER PRESSdll;
PSFL1dll_POINTER PSFL1dll;
PSFLSHdll_POINTER PSFLSHdll;
PUREFLDdll_POINTER PUREFLDdll;
QMASSdll_POINTER QMASSdll;
QMOLEdll_POINTER QMOLEdll;
RESIDUALdll_POINTER RESIDUALdll;
SATDdll_POINTER SATDdll;
SATEdll_POINTER SATEdll;
SATHdll_POINTER SATHdll;
SATPdll_POINTER SATPdll;
SATSdll_POINTER SATSdll;
SATTdll_POINTER SATTdll;
SETAGAdll_POINTER SETAGAdll;
SETKTVdll_POINTER SETKTVdll;
SETMIXdll_POINTER SETMIXdll;
SETMODdll_POINTER SETMODdll;
SETREFdll_POINTER SETREFdll;
SETUPdll_POINTER SETUPdll;
//  SPECGRdll_POINTER SPECGRdll; // not found in library
SUBLPdll_POINTER SUBLPdll;
SUBLTdll_POINTER SUBLTdll;
SURFTdll_POINTER SURFTdll;
SURTENdll_POINTER SURTENdll;
TDFLSHdll_POINTER TDFLSHdll;
TEFLSHdll_POINTER TEFLSHdll;
THERM0dll_POINTER THERM0dll;
THERM2dll_POINTER THERM2dll;
THERM3dll_POINTER THERM3dll;
THERMdll_POINTER THERMdll;
THFLSHdll_POINTER THFLSHdll;
TPFLSHdll_POINTER TPFLSHdll;
TPFL2dll_POINTER TPFL2dll;
TPRHOdll_POINTER TPRHOdll;
TQFLSHdll_POINTER TQFLSHdll;
TRNPRPdll_POINTER TRNPRPdll;
TSFLSHdll_POINTER TSFLSHdll;
VIRBdll_POINTER VIRBdll;
VIRCdll_POINTER VIRCdll;
WMOLdll_POINTER WMOLdll;
XMASSdll_POINTER XMASSdll;
XMOLEdll_POINTER XMOLEdll;

RMIX2dll_POINTER RMIX2dll;
RDXHMXdll_POINTER RDXHMXdll;
PHIXdll_POINTER PHIXdll;
PHI0dll_POINTER PHI0dll;


/*
 * Helper function to split strings the
 * Python way. Taken from CoolProp.
 */
std::vector<std::string> strsplit(std::string s, char del) {
	int iL = 0, iR = 0, N;
	N = s.size();
	std::vector<std::string> v;
	// Find the first instance of the delimiter
	iR = s.find_first_of(del);
	// Delimiter not found, return the same string again
	if (iR < 0) {
		v.push_back(s);
		return v;
	}
	while (iR != N-1) {
		v.push_back(s.substr(iL,iR-iL));
		iL = iR;
		iR = s.find_first_of(del,iR+1);
		// Move the iL to the right to avoid the delimiter
		iL += 1;
		if (iR == (int)std::string::npos) {
			v.push_back(s.substr(iL,N-iL));
			break;
		}
	}
	return v;
}

// http://stackoverflow.com/questions/11635/case-insensitive-string-comparison-in-c
bool strCompare(const std::string& str1, const std::string& str2) {
    if (str1.size() != str2.size()) {
        return false;
    }
    for (std::string::const_iterator c1 = str1.begin(), c2 = str2.begin(); c1 != str1.end(); ++c1, ++c2) {
        if (tolower(*c1) != tolower(*c2)) {
            return false;
        }
    }
    return true;
}




/*
 * Just a helper function control the output of
 * an array. Used to debug the handling of the
 * composition arrays.
 */
std::string printX(double arr[], long nc) {
  std::string ret = std::string("(");
  int stop = nc-1;
  char buffer [10];

  for(int i = 0; i <= (stop); i++) {
	sprintf(buffer, "%1.4f", arr[i]); // four decimals
	ret.append(buffer);
  }
  ret.append(")");
  return ret;
}

/*
 * Resolves Refprop error numbers and returns meaningful error message
 * INPUT:
 * 	in1:  character specifying input
 * 	lerr: error as a long value
 * OUTPUT
 * 	errormsg:    string containing error message
 * 	std::string: the same as errormsg
 */
std::string resolve_error(std::string in1, long lerr, char* errormsg) {
	switch(lerr){
		case 0:
			strcpy(errormsg,"Calculation successful");
			break;
		case 1:
			sprintf(errormsg,"T=%f < Tmin",dt);
			break;
		case 2:
			if (!in1.compare("p")){
				sprintf(errormsg,"P=%f < Ptp",dp);
			} else if (!in1.compare("d")){
				sprintf(errormsg,"D=%f > Dmax",dd);
			} else {
				strcpy(errormsg,"unknown error");
			}
			break;
		case 4:
			if (!in1.compare("p")){
				sprintf(errormsg,"P=%f < 0",dp);
			} else {
				strcpy(errormsg,"unknown error");
			}
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
		case 10:
			strcpy(errormsg,"D and x out of range");
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
			strcpy(errormsg,"unknown error");
			break;
		case -59:
			sprintf(errormsg,"ECS model did not converge");
			break;
		case 101:
			sprintf(errormsg,"error in opening fluid file");
			break;
		case 102:
			strcpy(errormsg,"error in file or premature end of file");
			break;
		case -103:
			strcpy(errormsg,"unknown model encountered in file");
			break;
		case 104:
			strcpy(errormsg,"error in setup of model");
			break;
		case 105:
			strcpy(errormsg,"specified model not found");
			break;
		case 111:
			strcpy(errormsg,"error in opening mixture file");
			break;
		case 112:
			strcpy(errormsg,"mixture file of wrong type");
			break;
		case 114:
			strcpy(errormsg,"nc<>nc from setmod");
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
			strcpy(errormsg,"unknown error");
			break;
		}

	if (debug) {
		printf("Error code: %ld \n",lerr);
		printf("Yields message: %s \n", errormsg);
	}
	return std::string(errormsg);
}

/*
 * Load the library and handle all the
 * platform specific stuff.
 */
double loadLibrary(std::string sPath) {
	if (RefpropdllInstance == NULL) { // Refprop is not loaded
#if defined(__ISWINDOWS__)
#if   defined(UNICODE)
		sPath.append((LPCWSTR)pathSep);
		sPath.append((LPCWSTR)libName);
		RefpropdllInstance = LoadLibraryW(sPath.c_str());
#else
		sPath.append((LPCSTR)pathSep);
		sPath.append((LPCSTR)libName);
		RefpropdllInstance = LoadLibrary(sPath.c_str()); // this works in cpp tester, but not in Modelica..
#endif
#elif defined(__ISLINUX__)
		RefpropdllInstance = dlopen(libName, RTLD_LAZY);
#elif defined(__ISAPPLE__)
		RefpropdllInstance = dlopen (libName, RTLD_LAZY);
#else
		throw NotImplementedError("We should not reach this point.");
		RefpropdllInstance = NULL;
#endif

		if (RefpropdllInstance == NULL) { // Still NULL after loading
#if		defined(__ISWINDOWS__)
			printf("Library problem: ");
#elif 	defined(__ISLINUX__)
			fputs(dlerror(), stderr);
#elif 	defined(__ISAPPLE__)
			fputs(dlerror(), stderr);
#else
			throw NotImplementedError("You should not be here.");
#endif
			printf("Could not load %s\n\n", libName);
			return FAIL;
		}

		if (debug) printf ("Library was loaded successfully.\n");
		return OK;
	} else { // Refprop was already loaded
		if (debug) printf ("Library was already loaded, doing nothing.\n");
		return OK;
	}
	return FAIL;
}

/*
 * Multiplatform pointer handling
 * Not very neat, but works.
 */
void *getFunctionPointer(char * name) {
#if defined(__ISWINDOWS__)
	return (void *) GetProcAddress(RefpropdllInstance,name);
#elif defined(__ISLINUX__)
	return dlsym(RefpropdllInstance, name);
#else
	throw NotImplementedError("This function should not be called.");
	return NULL;
#endif
}

/*
 * Moved pointer handling to a function, helps to maintain
 * an overview and structures OS dependent parts
 */
double setFunctionPointers() {
//	if (RefpropdllInstance != NULL) {
//		printf("You only need to define the pointers once, ");
//		printf("please revise your code.\n");
//		return FAIL;
//	} else { // set the pointers
//		if (loadLibrary() != OK) {
//			printf("Refprop library %s cannot be loaded, make sure you ",libName);
//			printf("installed it properly.\n");
//			return FAIL;
//		}

	if (RefpropdllInstance == NULL) {
		printf("Refprop library %s is not loaded, make sure you ",libName);
		printf("referenced and installed it properly.\n");
		return FAIL;
	} else { // set the pointers
		// set the pointers, platform independent
		RPVersion = (RPVersion_POINTER) getFunctionPointer((char *) RPVersion_NAME);
		ABFL1dll = (ABFL1dll_POINTER) getFunctionPointer((char *) ABFL1dll_NAME);
		ABFL2dll = (ABFL2dll_POINTER) getFunctionPointer((char *) ABFL2dll_NAME);
		ACTVYdll = (ACTVYdll_POINTER) getFunctionPointer((char *) ACTVYdll_NAME);
		AGdll = (AGdll_POINTER) getFunctionPointer((char *) AGdll_NAME);
		CCRITdll = (CCRITdll_POINTER) getFunctionPointer((char *) CCRITdll_NAME);
		CP0dll = (CP0dll_POINTER) getFunctionPointer((char *) CP0dll_NAME);
		CRITPdll = (CRITPdll_POINTER) getFunctionPointer((char *) CRITPdll_NAME);
		CSATKdll = (CSATKdll_POINTER) getFunctionPointer((char *) CSATKdll_NAME);
		CV2PKdll = (CV2PKdll_POINTER) getFunctionPointer((char *) CV2PKdll_NAME);
		CVCPKdll = (CVCPKdll_POINTER) getFunctionPointer((char *) CVCPKdll_NAME);
		CVCPdll = (CVCPdll_POINTER) getFunctionPointer((char *) CVCPdll_NAME);
		DBDTdll = (DBDTdll_POINTER) getFunctionPointer((char *) DBDTdll_NAME);
		DBFL1dll = (DBFL1dll_POINTER) getFunctionPointer((char *) DBFL1dll_NAME);
		DBFL2dll = (DBFL2dll_POINTER) getFunctionPointer((char *) DBFL2dll_NAME);
		DDDPdll = (DDDPdll_POINTER) getFunctionPointer((char *) DDDPdll_NAME);
		DDDTdll = (DDDTdll_POINTER) getFunctionPointer((char *) DDDTdll_NAME);
		DEFLSHdll = (DEFLSHdll_POINTER) getFunctionPointer((char *) DEFLSHdll_NAME);
		DHD1dll = (DHD1dll_POINTER) getFunctionPointer((char *) DHD1dll_NAME);
		DHFLSHdll = (DHFLSHdll_POINTER) getFunctionPointer((char *) DHFLSHdll_NAME);
		DIELECdll = (DIELECdll_POINTER) getFunctionPointer((char *) DIELECdll_NAME);
		DOTFILLdll = (DOTFILLdll_POINTER) getFunctionPointer((char *) DOTFILLdll_NAME);
		DPDD2dll = (DPDD2dll_POINTER) getFunctionPointer((char *) DPDD2dll_NAME);
		DPDDKdll = (DPDDKdll_POINTER) getFunctionPointer((char *) DPDDKdll_NAME);
		DPDDdll = (DPDDdll_POINTER) getFunctionPointer((char *) DPDDdll_NAME);
		DPDTKdll = (DPDTKdll_POINTER) getFunctionPointer((char *) DPDTKdll_NAME);
		DPDTdll = (DPDTdll_POINTER) getFunctionPointer((char *) DPDTdll_NAME);
		DPTSATKdll = (DPTSATKdll_POINTER) getFunctionPointer((char *) DPTSATKdll_NAME);
		DSFLSHdll = (DSFLSHdll_POINTER) getFunctionPointer((char *) DSFLSHdll_NAME);
		ENTHALdll = (ENTHALdll_POINTER) getFunctionPointer((char *) ENTHALdll_NAME);
		ENTROdll = (ENTROdll_POINTER) getFunctionPointer((char *) ENTROdll_NAME);
		ESFLSHdll = (ESFLSHdll_POINTER) getFunctionPointer((char *) ESFLSHdll_NAME);
		FGCTYdll = (FGCTYdll_POINTER) getFunctionPointer((char *) FGCTYdll_NAME);
		FPVdll = (FPVdll_POINTER) getFunctionPointer((char *) FPVdll_NAME);
		GERG04dll = (GERG04dll_POINTER) getFunctionPointer((char *) GERG04dll_NAME);
		GETFIJdll = (GETFIJdll_POINTER) getFunctionPointer((char *) GETFIJdll_NAME);
		GETKTVdll = (GETKTVdll_POINTER) getFunctionPointer((char *) GETKTVdll_NAME);
		GIBBSdll = (GIBBSdll_POINTER) getFunctionPointer((char *) GIBBSdll_NAME);
		HSFLSHdll = (HSFLSHdll_POINTER) getFunctionPointer((char *) HSFLSHdll_NAME);
		INFOdll = (INFOdll_POINTER) getFunctionPointer((char *) INFOdll_NAME);
		LIMITKdll = (LIMITKdll_POINTER) getFunctionPointer((char *) LIMITKdll_NAME);
		LIMITSdll = (LIMITSdll_POINTER) getFunctionPointer((char *) LIMITSdll_NAME);
		LIMITXdll = (LIMITXdll_POINTER) getFunctionPointer((char *) LIMITXdll_NAME);
		MELTPdll = (MELTPdll_POINTER) getFunctionPointer((char *) MELTPdll_NAME);
		MELTTdll = (MELTTdll_POINTER) getFunctionPointer((char *) MELTTdll_NAME);
		MLTH2Odll = (MLTH2Odll_POINTER) getFunctionPointer((char *) MLTH2Odll_NAME);
		NAMEdll = (NAMEdll_POINTER) getFunctionPointer((char *) NAMEdll_NAME);
		PDFL1dll = (PDFL1dll_POINTER) getFunctionPointer((char *) PDFL1dll_NAME);
		PDFLSHdll = (PDFLSHdll_POINTER) getFunctionPointer((char *) PDFLSHdll_NAME);
		PEFLSHdll = (PEFLSHdll_POINTER) getFunctionPointer((char *) PEFLSHdll_NAME);
		PHFL1dll = (PHFL1dll_POINTER) getFunctionPointer((char *) PHFL1dll_NAME);
		PHFLSHdll = (PHFLSHdll_POINTER) getFunctionPointer((char *) PHFLSHdll_NAME);
		PQFLSHdll = (PQFLSHdll_POINTER) getFunctionPointer((char *) PQFLSHdll_NAME);
		PREOSdll = (PREOSdll_POINTER) getFunctionPointer((char *) PREOSdll_NAME);
		PRESSdll = (PRESSdll_POINTER) getFunctionPointer((char *) PRESSdll_NAME);
		PSFL1dll = (PSFL1dll_POINTER) getFunctionPointer((char *) PSFL1dll_NAME);
		PSFLSHdll = (PSFLSHdll_POINTER) getFunctionPointer((char *) PSFLSHdll_NAME);
		PUREFLDdll = (PUREFLDdll_POINTER) getFunctionPointer((char *) PUREFLDdll_NAME);
		RESIDUALdll = (RESIDUALdll_POINTER) getFunctionPointer((char *) RESIDUALdll_NAME);
		QMASSdll = (QMASSdll_POINTER) getFunctionPointer((char *) QMASSdll_NAME);
		QMOLEdll = (QMOLEdll_POINTER) getFunctionPointer((char *) QMOLEdll_NAME);
		SATDdll = (SATDdll_POINTER) getFunctionPointer((char *) SATDdll_NAME);
		SATEdll = (SATEdll_POINTER) getFunctionPointer((char *) SATEdll_NAME);
		SATHdll = (SATHdll_POINTER) getFunctionPointer((char *) SATHdll_NAME);
		SATPdll = (SATPdll_POINTER) getFunctionPointer((char *) SATPdll_NAME);
		SATSdll = (SATSdll_POINTER) getFunctionPointer((char *) SATSdll_NAME);
		SATTdll = (SATTdll_POINTER) getFunctionPointer((char *) SATTdll_NAME);
		SETAGAdll = (SETAGAdll_POINTER) getFunctionPointer((char *) SETAGAdll_NAME);
		SETKTVdll = (SETKTVdll_POINTER) getFunctionPointer((char *) SETKTVdll_NAME);
		SETMIXdll = (SETMIXdll_POINTER) getFunctionPointer((char *) SETMIXdll_NAME);
		SETMODdll = (SETMODdll_POINTER) getFunctionPointer((char *) SETMODdll_NAME);
		SETREFdll = (SETREFdll_POINTER) getFunctionPointer((char *) SETREFdll_NAME);
		SETUPdll = (SETUPdll_POINTER) getFunctionPointer((char *) SETUPdll_NAME);
//			SPECGRdll = (SPECGRdll_POINTER) getFunctionPointer((char *)SPECGRdll_NAME); // not in library
		SUBLPdll = (SUBLPdll_POINTER) getFunctionPointer((char *) SUBLPdll_NAME);
		SUBLTdll = (SUBLTdll_POINTER) getFunctionPointer((char *) SUBLTdll_NAME);
		SURFTdll = (SURFTdll_POINTER) getFunctionPointer((char *) SURFTdll_NAME);
		SURTENdll = (SURTENdll_POINTER) getFunctionPointer((char *) SURTENdll_NAME);
		TDFLSHdll = (TDFLSHdll_POINTER) getFunctionPointer((char *) TDFLSHdll_NAME);
		TEFLSHdll = (TEFLSHdll_POINTER) getFunctionPointer((char *) TEFLSHdll_NAME);
		THERM0dll = (THERM0dll_POINTER) getFunctionPointer((char *) THERM0dll_NAME);
		THERM2dll = (THERM2dll_POINTER) getFunctionPointer((char *) THERM2dll_NAME);
		THERM3dll = (THERM3dll_POINTER) getFunctionPointer((char *) THERM3dll_NAME);
		THERMdll = (THERMdll_POINTER) getFunctionPointer((char *) THERMdll_NAME);
		THFLSHdll = (THFLSHdll_POINTER) getFunctionPointer((char *) THFLSHdll_NAME);
		TPFLSHdll = (TPFLSHdll_POINTER) getFunctionPointer((char *) TPFLSHdll_NAME);
		TPRHOdll = (TPRHOdll_POINTER) getFunctionPointer((char *) TPRHOdll_NAME);
		TQFLSHdll = (TQFLSHdll_POINTER) getFunctionPointer((char *) TQFLSHdll_NAME);
		TRNPRPdll = (TRNPRPdll_POINTER) getFunctionPointer((char *) TRNPRPdll_NAME);
		TSFLSHdll = (TSFLSHdll_POINTER) getFunctionPointer((char *) TSFLSHdll_NAME);
		VIRBdll = (VIRBdll_POINTER) getFunctionPointer((char *) VIRBdll_NAME);
		VIRCdll = (VIRCdll_POINTER) getFunctionPointer((char *) VIRCdll_NAME);
		WMOLdll = (WMOLdll_POINTER) getFunctionPointer((char *) WMOLdll_NAME);
		XMASSdll = (XMASSdll_POINTER) getFunctionPointer((char *) XMASSdll_NAME);
		XMOLEdll = (XMOLEdll_POINTER) getFunctionPointer((char *) XMOLEdll_NAME);

		RMIX2dll = (RMIX2dll_POINTER) getFunctionPointer((char *) RMIX2dll_NAME);
		RDXHMXdll = (RDXHMXdll_POINTER) getFunctionPointer((char *) RDXHMXdll_NAME);
		PHIXdll = (PHIXdll_POINTER) getFunctionPointer((char *) PHIXdll_NAME);
		PHI0dll = (PHI0dll_POINTER) getFunctionPointer((char *) PHI0dll_NAME);

		if (debug) printf ("Function pointers set to macro values.\n");
		return OK;
	}
	return FAIL;
}

/*
 * Make sure the library is loaded
 * properly and set pointers.
 */
double initRefprop(std::string sPath) {

	//std::string rPath = std::string(sPath);

	if (RefpropdllInstance == NULL) {
		if (debug) printf ("Library not loaded, trying to do so.\n");
		if (loadLibrary(sPath) != OK) {
			printf("Refprop library %s cannot be loaded, make sure you ",libName);
			printf("installed it properly.\n");
			return FAIL;
		}
		if (setFunctionPointers() != OK) {
			printf("There was an error setting the REFPROP function pointers, ");
			printf("check types and names in header file.\n");
			return FAIL;
		}
		// Set the desired equation of state, consult the Refprop
		// documentation for more details.
		// 0 : use default values
		// 2 : force Peng-Robinson
		long eosSwitch = 0;
		PREOSdll(eosSwitch);
		return OK;
	} else {
		if (debug) printf ("Library loaded, not doing anything.\n");
		return OK;
	}
	return FAIL;
}

/*
 * Construct the fluid string with the full path
 * from the simple names, needed for Linux version.
 * Call setupdll if necessary to define the fluid
 * or mixture for subsequent calls to flash routines.
 */
double setFluids(std::string sPath, std::string sFluids, char* error){
	// sPath:   "/opt/refprop" or "C:\Program Files\Refprop"
	// sFluids: "pentane|butane" or "air"

	//std::string rPath = std::string(sPath);

	if (initRefprop(sPath) != OK){
		std::cerr << "ERROR: library not loaded.\n";
		std::terminate();
	}

	long ierr=999;
	char hf[refpropcharlength*ncmax];

	std::string RefString;
	std::string fdPath = std::string(sPath);
	fdPath.append((char *)pathSep);
	fdPath.append("fluids");
	fdPath.append((char *)pathSep);

	if (loadedFluids.compare(sFluids)) { // The fluid is not already loaded
		std::vector<std::string> components_split = strsplit(sFluids,'|');// Split into components
		RefString.clear(); // Flush out fluid string

		// Build new fluid string
		for (unsigned int j=0;j<components_split.size();j++) {
			if (j == 0) {
				RefString = fdPath + components_split[j]+".fld";
			}
			else {
				RefString += "|" + fdPath + components_split[j]+".fld";
			}
			lnc = j+1;
		}

		// Check for pseudo pure components
		if (       !sFluids.compare("Air")
				|| !sFluids.compare("R507A")
				|| !sFluids.compare("R404A")
				|| !sFluids.compare("R410A")
				|| !sFluids.compare("R407C")
				|| !sFluids.compare("SES36") ) {
			lnc=1;
			RefString = fdPath + std::string(sFluids)+std::string(".ppf");
			dxmol[0]=1.0;     //Pseudo-Pure fluid
		}

		if (lnc>ncmax){
			sprintf(error,"Too many components (More than %i)\n",ncmax);
			return FAIL;
		}

		// Prepare strings and call SETUP to initialise the program
		strcpy(hf,RefString.c_str());
		char* hfm = (char*) calloc(refpropcharlength+8, sizeof(char));
		strcpy(hfm,fdPath.c_str());
		strcat(hfm,hfmix);

		//...Call SETUPdll to set the fluids
		if (debug) {
			printf ("Running SETUP...\n");
			printf ("No. of components: %li \n", lnc);
			printf ("Fluid files: %s \n", RefString.c_str());
			printf ("Mixture file: %s \n", hfmix);
		}

		SETUPdll(lnc, hf, hfm, hrf, ierr, error,
				refpropcharlength*ncmax,refpropcharlength,
				lengthofreference,errormessagelength);
		free (hfm);

		if (ierr != 0) {
			printf("REFPROP setup gives this error during SETUP:\n %s\n",error);
			printf("Further information:\n %s\n",resolve_error("",ierr,error).c_str());
			return FAIL;
		} else { //Copy the name of the loaded fluid
			loadedFluids = std::string(sFluids);
			int flush = flushConstants();
			if (debug) printf("Loading a new fluid, flushing constants: %i.\n",flush);
			return OK;
		}
	} // Fluid was already loaded
	if (debug) printf("Fluid was already loaded.\n");
	return OK;
}


bool isInput(std::string in1, std::string in2, std::string def){
	// if the first equals the first
	if ( strCompare(in1, def.substr(0,1)) ) {
		if ( strCompare(in2, def.substr(1,1)) ) return true;
	} else if ( strCompare(in2, def.substr(0,1)) ) {
		if ( strCompare(in1, def.substr(1,1)) ) return true;
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
	return dwm/1000;  // g/mol to kg/mol
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
		if (dwvap==noValue) WMOLdll(dxmolv,dwvap);
		return dqmol*dwvap/dwm; // TODO can you show a case where this function is called more than once?
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
	if (dwliq==noValue) WMOLdll(dxmoll,dwliq);
	return dwliq/1000.;
}

double getWMV_modelica(){
	if (dwvap==noValue) WMOLdll(dxmolv,dwvap);
	return dwvap/1000.;
}

//double* getXL_modelica(){
//	double dxlkg[ncmax];
//	XMASSdll(dxmoll,dxlkg,dwliq);
//	return dxlkg;
//}
//
//double* getXV_modelica(){
//	double dxvkg[ncmax];
//	XMASSdll(dxmolv,dxvkg,dwvap);
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
/*
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
*/
/*
double get_dhdt_d_modelica() { //dH/dT at constant density [J/(mol-K)] / g/mol * 1000g/kg = J/kg.K
	return noValue;//ddhdt_d/dwm*1000;
}
double get_dhdt_p_modelica() { //dH/dT at constant pressure [J/(mol-K)]
	return noValue;//ddhdt_p/dwm*1000;
}
double get_dhdd_t_modelica() { //dH/drho at constant temperature [(J/mol)/(mol/L)] * mol/g * 1000g/kg / g/mol = (J/kg) / (kg/m3)
	return noValue;//ddhdd_t /dwm*1000. / dwm;
}
double get_dhdd_p_modelica() { //dH/drho at constant pressure [(J/mol)/(mol/L)]
	return noValue;//ddhdd_p /dwm*1000. / dwm;
}
double get_dhdp_t_modelica() { //dH/dP at constant temperature [J/(mol-kPa)] /dwm*1000. / (1000Pa/kPa) = J/kg.Pa
	return noValue;//ddhdp_t / dwm;
}
double get_dhdp_d_modelica() { //dH/dP at constant density [J/(mol-kPa)]
	return noValue;//ddhdp_d / dwm;
}
*/

// Derivatives
// Derivative of density with respect to enthalpy at constant pressure
double get_dddX_ph_modelica(){
//	double dxmoltmp[ncmax],dwm1;
//	dxmoltmp[0]=1;
//	dxmoltmp[1]=0;
//	WMOLdll(dxmoltmp,dwm1); // TODO this must be molecular weight of species 1 instead of vap... Take a look at how ddddX_pt is computed!!!
	return ddddX_ph;//*dwm*dwm/dwm1; // [mol/l * mol/mol1] * g/mol * g/mol * mol1/g1 * 1e-3kg/g / 1e-3 m3/L =  kg/m3 * g/gi
}
double get_dddh_p_modelica(){
	return ddddh_p*dwm*dwm/1000.; // [mol/l * mol/J] * g/mol * g/mol * 1e-3kg/g * 1e-3kg/g / 1e-3 m3/L =  kg/m3 * kg/J
}
// Derivative of density with respect to pressure at constant enthalpy
double get_dddp_h_modelica(){
	return ddddp_h*dwm/1000; // [mol/l * 1/kPa) * g/mol * 1e-3kg/g  / 1e-3 m3/L  / 1e-3 kPa/Pa = kg/(m3.Pa)
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

	if ( strCompare(out, "p") ) {
		if (dp!=noValue) return getP_modelica();
	} else if ( strCompare(out, "t") ) {
		if (dt!=noValue) return getT_modelica();
	} else if ( strCompare(out, "m") ) {
		if (dwm!=noValue) return getWM_modelica();
	} else if ( strCompare(out, "d") ) {
		if (dd!=noValue) return getD_modelica();
	} else if ( strCompare(out, "e") ) {
		if (de!=noValue) return getE_modelica();
	} else if ( strCompare(out, "h") ) {
		if (dh!=noValue) return getH_modelica();
	} else if ( strCompare(out, "s") ) {
		if (ds!=noValue) return getS_modelica();
	} else if ( strCompare(out, "w") ) {
		if (dw!=noValue) return getW_modelica();
	} else if ( strCompare(out, "v") ) {
		if (deta!=noValue) return getETA_modelica();
	} else if ( strCompare(out, "l") ) {
		if (dtcx!=noValue) return getTCX_modelica();
	}
	return noValue;
}



int updateDers(double *ders, long lerr){
	//ASSIGN VALUES TO RETURN ARRAY
/*  ders[0]  = lerr;//error code
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
	ders[19] = get_dddh_p_modelica();	// dD/dh at constant pressure [kg/m3 * kg/J]
	ders[20] = get_dddp_h_modelica();	// dD/dp at constant enthalpy [kg/(m3.Pa)]
*/
	// TODO - I have reduced the variables thrown back.. Why us jtc, a, g derivatives here?
	ders[0] = lerr;//error code
	ders[1] = getXKAPPA_modelica();	// isothermal compressibility (= -1/V dV/dP = 1/rho dD/dP) [1/Pa]
	ders[2] = getBETA_modelica();		// volume expansivity (= 1/V dV/dT = -1/rho dD/dT) [1/K]
	ders[3] = get_dddX_ph_modelica();	// dD/dX at constant p,h
	ders[4] = get_dddh_p_modelica();	// dD/dh at constant pressure [kg/m3 * kg/J]
	ders[5] = get_dddp_h_modelica();	// dD/dp at constant enthalpy [kg/(m3.Pa)]

	ders[6] =  ddddp_T*dwm/1000;        // (mol/L * 1/kPa) * g/mol / 1000 g/kg * 1000L/m3 / 1000 Pa/kPa
	ders[7] =  ddddT_p*dwm;			    // (mol/L * 1/K) * g/mol / 1000 g/kg * 1000L/m3
	ders[8] =  ddddX_pT;// Done in function *dwm*dwm/dw1;  // [mol/l * mol/molvap] * g/mol * g/mol * moli/gi * 1e-3kg/g / 1e-3 m3/L =  kg/m3 * g/gi
	ders[9] =  ddhdp_T/dwm;   	        // (J/mol * 1/kPa) / g/mol * 1000 g/kg / 1000 Pa/kPa
	ders[10] = ddhdT_p/dwm*1000;		// (J/mol * 1/K) / g/mol * 1000 g/kg
	ders[11] = ddhdX_pT;// Done in function /dwm*1000*dwm/dw1;				// (J/mol * mol/molvap) / g/mol * 1000 g/kg * g/mol * molvap/gvap = J/kg * g/gvap

	return 0;
}

int ders_REFPROP(double *ders, char* errormsg, int DEBUGMODE){
	debug = false;
	if (DEBUGMODE) debug = true;
	long lerr = 0;

	double spare3,spare4,spare5,spare6,spare7,spare8,spare9,spare10,spare2[ncmax],spare1[ncmax];
	double spare11,spare12,spare13,spare14,spare15,spare16,spare17,spare19,spare18,spare20,spare21;

	if ((dd!=noValue)&&(dt!=noValue)) {
		// THERM2dll(dt, dd, dxmol, spare5, spare6, spare9, spare10, dCv, dCp, dw,dZ, dhjt, dA, dG, dxkappa, dbeta, ddpdd, dd2pdd2, ddpdt, ddddt,ddddp, dd2pdt2, dd2pdtd, spare3, spare4);
		// we want to get props depending on quality below..

		if (PartialDersInputChoice==4) { // use TPX numeric derivs

			if (dqmol < 0. || dqmol > 1.) { // single-phase region
				if (debug) printf ("Using single-phase derivatives.\n");
				if (debug) printf("Calling THERM3 with T=%f and rho=%f.\n",dt,dd);
				THERM3dll (dt,dd,dxmol,dxkappa,dbeta,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12);

				// analytical derivatives of density:
				ddddT_p = -dbeta*dd;
				ddddp_T = dxkappa*dd;

				// analytical derivatives of enthalpy:
				ddhdT_p = dCp;
				ddhdp_T = (1-dt*dbeta)/dd;


				// numerical derivatives wrt. independent mass fraction,
				// NOTE THAT IT IS DIFFICULT TO CHANGE FROM MOLE TO MASS BASIS, WHEN PERFORMING A NUMERICAL DERIVATIVE wrt composition. I DID NOT SUCCEED
				double dxmolnew[ncmax], dxkgnew[ncmax], dwmnew;
				double dnew,hnew;
				//XMASSdll(dxmol,dxkgnew,dwmnew);
				dxkgnew[0] = dxkg[0]+1e-7;
				dxkgnew[1] = dxkg[1]-1e-7;
				XMOLEdll(dxkgnew,dxmolnew,dwmnew);
				TPFLSHdll(dt,dp,dxmolnew,dnew,spare3,spare4,spare1,spare2,spare5,spare6,hnew,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
				ddddX_pT = (dnew*dwmnew-dd*dwm)/1e-7; // this is on mass basis, due to the above mass change
				ddhdX_pT = (hnew/dwmnew*1000-dh/dwm*1000)/1e-7; // this is on mass basis, due to the above mass change


//			    double n;
//				double href[ncmax],dpdz_tv[ncmax],dhdz_tv[ncmax];
//			    href[0] = 28945.714499977374; // ammoniaL
//			    href[1] = 45957.1914944204584; // water
//
//			    double delx=1e-7,delx2,R,dxmoln,dxmolp,t0,d0,tau,delta,phi01,phi10,phig10,pn,hn,pp,hp;
//				long im1=-1, i0=0, ip1=1;
//				int i;
//
//				for (n = 0; n < lnc; n++)  {
//				//double n=1;// derivative wrt component n
//
//			 	for (i = 0; i < lnc; i++)  {
//			 		dxmoln[i] = dxmol[i];
//			 		dxmolp[i] = dxmol[i];
//			    }
//			 	dxmoln[n-1] = dxmoln[n-1] - delx;
//			    dxmolp[n-1] = dxmolp[n-1] + delx;
//
//				// negative increment
//				 RMIX2dll(dxmol, R); // R must be constant for some reason?
//				 RDXHMXdll(&im1,&i0,&i0,dxmoln,t0,d0,lerr,errormsg,errormessagelength);
//				 tau = t0 / dt;
//				 delta =dd / d0;
//				 PHIXdll(&i0,&ip1, tau, delta, dxmoln, phi01);
//				 PHIXdll(&ip1,&i0, tau, delta, dxmoln, phi10);
//			     PHI0dll(&ip1, &i0, dt, dd, dxmoln, phig10);
//				 pn = dd * R * dt * (1 + phi01);
//				 //hrn = R * t * (1 + phi10 + phi01);
//			     hn = R * dt * (1 + phig10 + phi10 + phi01)  + dxmoln[0]*href[0] + dxmoln[1]*href[1];;
//
//			    // positive increment
//			     //RMIX2dll(dxmol, R); // R must be constant for some reason?
//				 RDXHMXdll(&im1,&i0,&i0,dxmolp,t0,d0,lerr,errormsg,errormessagelength);
//				 tau = t0 / dt;
//				 delta =dd / d0;
//				 PHIXdll(&i0,&ip1, tau, delta, dxmolp, phi01);
//				 PHIXdll(&ip1,&i0, tau, delta, dxmolp, phi10);
//			     PHI0dll(&ip1, &i0, dt, dd, dxmolp, phig10);
//				 pp = dd * R * dt * (1 + phi01);
//			     //hrp = R * t * (1 + phi10 + phi01);
//			     hp = R * dt * (1 + phig10 + phi10 + phi01)  + dxmolp[0]*href[0] + dxmolp[1]*href[1];;
//
//			    // derivatives..
//			     dpdz_tv[n-1] = (pp - pn) / delx2;
//			     //dhrdxi1 = (hrp - hrn) / delx2;
//			     dhdz_tv[n-1] = (hp - hn) / delx2;
//				}



			} else {  // two-phase region, get derivative of density

				// numerical derivative wrt. Temperature:
				double dnew,hnew,dtnew;
				dtnew=dt+1e-4;
				TPFLSHdll(dtnew,dp,dxmol,dnew,spare3,spare4,spare1,spare2,spare5,spare6,hnew,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
				ddddT_p = (dnew-dd)/1e-4;
				ddhdT_p = (hnew-dh)/1e-4;

				// numerical derivative wrt. Pressure:
				double dpnew=dp+1e-3; // this is a change in 1 Pa
				TPFLSHdll(dt,dpnew,dxmol,dnew,spare3,spare4,spare1,spare2,spare5,spare6,hnew,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
				ddddp_T = (dnew-dd)/1e-3; // /1;
				ddhdp_T = (hnew-dh)/1e-3; // /1;

				// numerical derivative for independent mass fraction
				/* NOTE THAT IT IS DIFFICULT TO CHANGE FROM MOLE TO MASS BASIS, WHEN PERFORMING A NUMERICAL DERIVATIVE wrt composition. I DID NOT SUCCEED */
				double dxmolnew[ncmax], dxkgnew[ncmax], dwmnew;
				//XMASSdll(dxmol,dxkgnew,dwmnew);
				dxkgnew[0] = dxkg[0]+1e-7;
				dxkgnew[1] = dxkg[1]-1e-7;
				XMOLEdll(dxkgnew,dxmolnew,dwmnew);
				TPFLSHdll(dt,dp,dxmolnew,dnew,spare3,spare4,spare1,spare2,spare5,spare6,hnew,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
				ddddX_pT = (dnew*dwmnew-dd*dwm)/1e-7; // this is on mass basis, due to the above mass change
				ddhdX_pT = (hnew/dwmnew*1000-dh/dwm*1000)/1e-7; // this is on mass basis, due to the above mass change

			}

		} else {

			if (dqmol < 0. || dqmol > 1.) { // single-phase region
				if (debug) printf ("Using single-phase derivatives.\n");

				if (debug) printf("Calling THERM3 with T=%f and rho=%f.\n",dt,dd);
				THERM3dll (dt,dd,dxmol,dxkappa,dbeta,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12);

				// analytical derivatives
				ddddp_h = (-dt*dbeta*dbeta + dbeta + dxkappa*dd*dCp)/dCp;
				ddddh_p = -dbeta*dd/dCp;


				// numerical derivative for ddddzi

				double dpdT_zv,dpdv_Tz,dhdT_vz,dhdv_Tz;
				dpdT_zv = dbeta/dxkappa;
				dpdv_Tz= -dd/dxkappa;
				dhdT_vz = dCv+dbeta/(dd*dxkappa);
				dhdv_Tz = (dt*dbeta/dxkappa - 1/dxkappa);

/*
				double href[ncmax],dpdz_tv[ncmax],dhdz_tv[ncmax],dvdz_ph[ncmax],ddddX_ph_tmp[ncmax];
				//href[0] = 28945.714499977374; // ammoniaL
			    //href[1] = 45957.1914944204584; // water

				href[0] = 8295.6883966232; //methane

				href[1] = 14873.1879732343; //ethane

			    double delx=1e-7,delx2,R,dxmoln[ncmax],dxmolp[ncmax],t0,d0,tau,delta,phi01,phi10,phig10,pn,hn,pp,hp;
				long im1=-1, i0=0, ip1=1;
				delx2=delx*2;

				for (int n = 1; n < lnc+1; n++)  {
				//double n=1;// derivative wrt component n

			 	for (int i = 0; i < lnc; i++)  {
			 		dxmoln[i] = dxmol[i];
			 		dxmolp[i] = dxmol[i];
			    }
			 	dxmoln[n-1] = dxmoln[n-1] - delx;
			    dxmolp[n-1] = dxmolp[n-1] + delx;

				// negative increment
				 RMIX2dll(dxmol, R); // R must be constant for some reason?
				 RDXHMXdll(&im1,&i0,&i0,dxmoln,t0,d0,lerr,errormsg,errormessagelength);
				 tau = t0 / dt;
				 delta =dd / d0;
				 PHIXdll(&i0,&ip1, tau, delta, dxmoln, phi01);
				 PHIXdll(&ip1,&i0, tau, delta, dxmoln, phi10);
			     PHI0dll(&ip1, &i0, dt, dd, dxmoln, phig10);
				 pn = dd * R * dt * (1 + phi01);
				 //hrn = R * t * (1 + phi10 + phi01);
			     hn = R * dt * (1 + phig10 + phi10 + phi01)  + dxmoln[0]*href[0] + dxmoln[1]*href[1];;

			    // positive increment
			     //RMIX2dll(dxmol, R); // R must be constant for some reason?
				 RDXHMXdll(&im1,&i0,&i0,dxmolp,t0,d0,lerr,errormsg,errormessagelength);
				 tau = t0 / dt;
				 delta =dd / d0;
				 PHIXdll(&i0,&ip1, tau, delta, dxmolp, phi01);
				 PHIXdll(&ip1,&i0, tau, delta, dxmolp, phi10);
			     PHI0dll(&ip1, &i0, dt, dd, dxmolp, phig10);
				 pp = dd * R * dt * (1 + phi01);
			     //hrp = R * t * (1 + phi10 + phi01);
			     hp = R * dt * (1 + phig10 + phi10 + phi01)  + dxmolp[0]*href[0] + dxmolp[1]*href[1];;

			    // numerical  derivatives..
			     dpdz_tv[n-1] = (pp - pn) / delx2;
			     dhdz_tv[n-1] = (hp - hn) / delx2;

			    // expression from jacobian matrix transformation
				dvdz_ph[n-1] = (dpdT_zv*dhdz_tv[n-1] - dhdT_vz*dpdz_tv[n-1]) / (dpdT_zv*dhdv_Tz - dhdT_vz*dpdv_Tz);
				ddddX_ph_tmp[n-1] = -dd*dd*dvdz_ph[n-1]; // this is still in refprop units, so X is moli/moltot..

				}

				//printf("dhdz_tv[0] = %f\n",dhdz_tv[0]);
				//printf("dhdz_tv[1] = %f\n",dhdz_tv[1]);

				double dxmolnew[ncmax],dwmnew[ncmax];
				dxmolnew[0] =1;
				dxmolnew[1] =0;
				WMOLdll(dxmolnew,dwmnew[0]);
				dxmolnew[0] =0;
				dxmolnew[1] =1;
				WMOLdll(dxmolnew,dwmnew[1]);
//				ddddX_ph_tmp[0] = ddddX_ph_tmp[0]*dxmol[0]/dxkg[0]*dwm;
//				ddddX_ph_tmp[1] = ddddX_ph_tmp[1]*dxmol[1]/dxkg[1]*dwm;
				//ddddX_ph_tmp[0] = ddddX_ph_tmp[0]*1/(dwmnew[0]/dwm)*dwm;
				//ddddX_ph_tmp[1] = ddddX_ph_tmp[1]*1/(dwmnew[1]/dwm)*dwm;
				//ddddX_ph_tmp[0] = ddddX_ph_tmp[0]*dwm;
				//ddddX_ph_tmp[1] = ddddX_ph_tmp[1]*dwm;

				double dxdz1 = dwmnew[0]/dwm - dxmol[0]*dwmnew[0]*dwmnew[0]/(dwm*dwm);
				double dxdz2 = dwmnew[1]/dwm - dxmol[1]*dwmnew[1]*dwmnew[1]/(dwm*dwm);

				//double ddmassddmol = dd*(dwmnew[0]*)

				//ddddX_ph_tmp[0] = ddddX_ph_tmp[0]*(dwmnew[0]*dxmol[1]*dwmnew[1]/(dwm*dwm))*dwm;
				//ddddX_ph_tmp[1] = ddddX_ph_tmp[1]*(dwmnew[0]*dxmol[0]*dwmnew[1]/(dwm*dwm))*dwm;

				ddddX_ph = (ddddX_ph_tmp[0] - ddddX_ph_tmp[1])/(dxdz1+dxdz2)*dwm;

				// this does not work...
				*/

				double dpdz_td,dhdz_td;
				//double dxmolnew[ncmax],dwmnew;
				double pnew,hnew;
				double dvdz_ph;

				// numerical derivative for independent mass fraction
				/* NOTE THAT IT IS DIFFICULT TO CHANGE FROM MOLE TO MASS BASIS, WHEN PERFORMING A NUMERICAL DERIVATIVE wrt composition. I DID NOT SUCCEED */
				double dxmolnew[ncmax], dxkgnew[ncmax], dwmnew;
				//XMASSdll(dxmol,dxkgnew,dwmnew);
				dxkgnew[0] = dxkg[0]+1e-5;
				dxkgnew[1] = dxkg[1]-1e-5;
				XMOLEdll(dxkgnew,dxmolnew,dwmnew);
				double din;
				din=dd*dwm/dwmnew;
				PRESSdll (dt,din,dxmolnew,pnew);
				ENTHALdll (dt,din,dxmolnew,hnew);

				dpdz_td = (pnew-dp)/1e-5;
				dhdz_td = (hnew/dwmnew-dh/dwm)*dwm/1e-5;

				// expression from jacobian matrix transformation
				dvdz_ph = -(dpdT_zv*dhdz_td - dhdT_vz*dpdz_td) / (dpdT_zv*dhdv_Tz - dhdT_vz*dpdv_Tz);
				ddddX_ph = -dd*dd*dvdz_ph;
				ddddX_ph = ddddX_ph*dwm;  // this is already per X due to the above change in mass fraction and conversion..


			} else { // two-phase region, get derivative of density

				// These are not computed in two-phase!
				dxkappa=noValue;
				dbeta=noValue;

				if (PartialDersInputChoice==3) { // Using  phX  Analytical twophase derivatives
					if (debug) printf ("Using Analytical twophase derivatives\n");

		//			// TODO the below analytical derivs are wrong since saturated liquid and vapor concentrations change along evaporation..


					/* j--phase flag: 1 = input x is liquid composition (bubble point)
								2 = input x is vapor composition (dew point)
								3 = input x is liquid composition (freezing point)
								4 = input x is vapor composition (sublimation point)
					*/

					//compute saturated vapor state at const p
					long kph2 = 2;
					double dt_v,ddv_p,dCv_v,dCp_v,dwm_v,dxkappa_v,dbeta_v,h_v,s_v;
					SATPdll(dp,dxmol,kph2,dt_v,spare3,ddv_p,spare2,spare1,lerr,errormsg,errormessagelength);
					ENTHALdll(dt_v,ddv_p,dxmol,h_v);
					//compute saturated liquid state at const p
					long kph1 = 1;
					double dt_l,ddl_p,dCv_l,dCp_l,dwm_l,dxkappa_l,dbeta_l,h_l,s_l;
					SATPdll(dp,dxmol,kph1,dt_l,ddl_p,spare3,spare2,spare1,lerr,errormsg,errormessagelength);
					ENTHALdll(dt_l,ddl_p,dxmol,h_l);
					// compute partials
					double dvdh_p,dTdp_clasius, dhdpL,dhdpV,dvdpL,dvdpV,dxdp_h,dvdp_h;
					// compute drhodh_p
					dvdh_p = (1/ddv_p - 1/ddl_p)/(h_v-h_l);
					ddddh_p = -dd*dd*dvdh_p;
					// get other sat props at const T,p,x
					THERM2dll(dt, ddv, dxmolv, spare3, spare4, h_v, s_v, dCv_v, dCp_v, dwm_v,spare2, spare10, spare11, spare12, dxkappa_v, dbeta_v, spare13, spare14, spare15, spare16,spare17, spare18, spare19, spare20, spare21);
					THERM2dll(dt, ddl, dxmoll, spare3, spare4, h_l, s_l, dCv_l, dCp_l, dwm_l,spare2, spare10, spare11, spare12, dxkappa_l, dbeta_l, spare13, spare14, spare15, spare16,spare17, spare18, spare19, spare20, spare21);
					//ddv_v = ddv;
					//ddl_l = ddl;
					// compute drhodp_h
					//dTdp_clasius =  (1/ddv - 1/ddl)/(s_v-s_l);
					dTdp_clasius =  dt*(1/ddv - 1/ddl)/(h_v-h_l);
					dhdpL = 1/ddl*(1-dbeta_l*dt_l)+dCp_l*dTdp_clasius;
					dhdpV = 1/ddv*(1-dbeta_v*dt_v)+dCp_v*dTdp_clasius;
					dvdpL = dbeta_l*1/ddl*dTdp_clasius-dxkappa_l*1/ddl;
					dvdpV = dbeta_v*1/ddv*dTdp_clasius-dxkappa_v*1/ddv;
					dxdp_h = (dhdpL + dqmol * (dhdpV-dhdpL))/(h_l-h_v);
					dvdp_h = dvdpL+dxdp_h*(1/ddv-1/ddl)+dqmol*(dvdpV-dvdpL);
					ddddp_h = -dd*dd*dvdp_h;

					// analytical derivative for ddddzi
					double dhdz_tp, dvdz_tp;
					double dxkgv[ncmax],dwmv,dxkgl[ncmax],dwml;
					XMASSdll(dxmolv,dxkgv,dwmv);
					XMASSdll(dxmoll,dxkgl,dwml);
					dhdz_tp = (h_v/dwmv*1000-h_l/dwml*1000)/(dxkgv[0]-dxkgl[0]);
				    dvdz_tp = (1/(ddv*dwmv)-1/(ddl*dwml))/(dxkgv[0]-dxkgl[0]);
				    dvdh_p = dvdh_p/dwm*dwm/1000;
				    ddddX_ph = -dd*dwm*dd*dwm*(-dvdh_p*dhdz_tp+dvdz_tp);

					//dhdz_tp = (h_v-h_l)/(dxmolv[0]-dxmoll[0]);
				    //dvdz_tp = (1/ddv_v-1/ddl_l)/(dxmolv[0]-dxmoll[0]);
				    //ddddX_ph = -dd*dd*(-dvdh_p*dhdz_tp+dvdz_tp);


//					// numerical derivative for ddddzi, that ensures xmol is between 0 and 1
//					double HA,HB,smooth,DX,dxmolnew[ncmax],dlow,dhigh;
//					HA = 0.925*(h_v-h_l)+h_l;
//					HB = 0.975*(h_v-h_l)+h_l;
//
//					/*
//					// using PHFLSH to get numerical derivative ddddzi
//					DX=0.005;
//					if (dh < HA) {
//						dxmolnew[0]=dxmol[0]+DX;  // Be carefull when comparing.. keeping X2 const is not the same as keeping z2 const ??? or..
//						dxmolnew[1]=dxmol[1]-DX; // This is wrong as you may show that x2 should be eheld constant for an ideal gas mix, but refprop needs sum(x)=1
//						PHFLSHdll(dp,dh,dxmolnew,spare14,dlow,spare3,spare4,spare1,spare2,spare5,spare6,spare13,spare7,spare8,spare11,lerr,errormsg,errormessagelength);
//						ddddX_ph = (dlow-dd)/DX;
//					} else if (dh > HB) {
//						DX=-DX;
//						dxmolnew[0]=dxmol[0]+DX;  // Be carefull when comparing.. keeping X2 const is not the same as keeping z2 const ??? or..
//						dxmolnew[1]=dxmol[1]-DX; // This is wrong as you may show that x2 should be eheld constant for an ideal gas mix, but refprop needs sum(x)=1
//						PHFLSHdll(dp,dh,dxmolnew,spare14,dhigh,spare3,spare4,spare1,spare2,spare5,spare6,spare13,spare7,spare8,spare11,lerr,errormsg,errormessagelength);
//						ddddX_ph = (dhigh-dd)/DX;
//					} else {
//						smooth=(dh-HA)/(HB-HA);
//						dxmolnew[0]=dxmol[0]+DX;  // Be carefull when comparing.. keeping X2 const is not the same as keeping z2 const ??? or..
//						dxmolnew[1]=dxmol[1]-DX; // This is wrong as you may show that x2 should be eheld constant for an ideal gas mix
//						PHFLSHdll(dp,dh,dxmolnew,spare14,dlow,spare3,spare4,spare1,spare2,spare5,spare6,spare13,spare7,spare8,spare11,lerr,errormsg,errormessagelength);
//						DX = -DX;
//						dxmolnew[0]=dxmol[0]+DX;  // Be carefull when comparing.. keeping X2 const is not the same as keeping z2 const ??? or..
//						dxmolnew[1]=dxmol[1]-DX; // This is wrong as you may show that x2 should be eheld constant for an ideal gas mix
//						PHFLSHdll(dp,dh,dxmolnew,spare14,dhigh,spare3,spare4,spare1,spare2,spare5,spare6,spare13,spare7,spare8,spare11,lerr,errormsg,errormessagelength);
//						ddddX_ph = (dlow-dd)/(-DX)*(1-smooth) + smooth*(dhigh-dd)/(DX);
//					}
//					 */
//
//					// Using TPFLSH to get numerical derivative  ddddzi
//					double dhdT_pX, dddT_pX,hlow,hhigh,dddX_pT,dhdX_pT;
//
//					dhdT_pX = (h_v - h_l)/(dt_v-dt_l);
//					//dvdT_p = (1/ddv_v - 1/ddl_l)/(dt_v-dt_l);
//					dddT_pX = -dd*dd*(1/ddv_v - 1/ddl_l)/(dt_v-dt_l);
//
//					DX=0.0025;
//					if (dh < HA) {
//						//DX = DX;
//						dxmolnew[0]=dxmol[0]+DX;  // Be carefull when comparing.. keeping X2 const is not the same as keeping z2 const ??? or..
//						dxmolnew[1]=dxmol[1]-DX; // This is wrong as you may show that x2 should be eheld constant for an ideal gas mix, but refprop needs sum(x)=1
//						TPFLSHdll(dt,dp,dxmolnew,dlow,spare3,spare4,spare1,spare2,spare5,spare6,hlow,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
//						dddX_pT = (dlow-dd)/(DX);
//						dhdX_pT = (hlow-dh)/(DX);
//					} else if (dh > HB) {
//						DX = -DX;
//						dxmolnew[0]=dxmol[0]+DX;  // Be carefull when comparing.. keeping X2 const is not the same as keeping z2 const ??? or..
//						dxmolnew[1]=dxmol[1]-DX; // This is wrong as you may show that x2 should be eheld constant for an ideal gas mix
//						TPFLSHdll(dt,dp,dxmolnew,dhigh,spare3,spare4,spare1,spare2,spare5,spare6,hhigh,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
//						dddX_pT = (dhigh-dd)/(DX);
//						dhdX_pT = (hhigh-dh)/(DX);
//					} else { // something smooth
//						smooth=(dh-HA)/(HB-HA);
//						//DX = DX;
//						dxmolnew[0]=dxmol[0]+DX;  // Be carefull when comparing.. keeping X2 const is not the same as keeping z2 const ??? or..
//						dxmolnew[1]=dxmol[1]-DX; // This is wrong as you may show that x2 should be eheld constant for an ideal gas mix
//						TPFLSHdll(dt,dp,dxmolnew,dlow,spare3,spare4,spare1,spare2,spare5,spare6,hlow,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
//						DX = -DX;
//						dxmolnew[0]=dxmol[0]+DX;  // Be carefull when comparing.. keeping X2 const is not the same as keeping z2 const ??? or..
//						dxmolnew[1]=dxmol[1]-DX; // This is wrong as you may show that x2 should be eheld constant for an ideal gas mix
//						TPFLSHdll(dt,dp,dxmolnew,dhigh,spare3,spare4,spare1,spare2,spare5,spare6,hhigh,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
//						dddX_pT = (dlow-dd)/(-DX)*(1-smooth) + smooth*(dhigh-dd)/(DX);
//						dhdX_pT = (hlow-dh)/(-DX)*(1-smooth) + smooth*(hhigh-dh)/(DX);
//					}
//					ddddX_ph = -(dddT_pX*dhdX_pT-dhdT_pX*dddX_pT)/dhdT_pX;


				} else if (PartialDersInputChoice==2) { // Using phX Numerical twophase derivatives
					if (debug) printf ("Using Numerical two phase derivatives\n");

				// in the following HA and HB are points in two-phase, where the numerical derivative changes "the sign of change", e.g dT into -dT smoothly (linearly)

					//using PHFLSH to get numerical derivative ddddzi
					double dnew,dpnew;
					dpnew = dp +1e-3; // change in 1 Pa
					PHFLSHdll(dpnew,dh,dxmol,spare14,dnew,spare3,spare4,spare1,spare2,spare5,spare6,spare13,spare7,spare8,spare11,lerr,errormsg,errormessagelength);
					ddddp_h = (dnew-dd)/1e-3;

					double dhnew;
					dhnew = dh +(1*dwm/1000); // change in 1 J/kg*K
					PHFLSHdll(dp,dhnew,dxmol,spare14,dnew,spare3,spare4,spare1,spare2,spare5,spare6,spare13,spare7,spare8,spare11,lerr,errormsg,errormessagelength);
					ddddh_p = (dnew-dd)/(1*dwm/1000);

					double dxmolnew[ncmax], dxkgnew[ncmax], dwmnew;
//					XMASSdll(dxmol,dxkg,dwmnew);
					dxkgnew[0] = dxkg[0]+1e-5;
					dxkgnew[1] = dxkg[1]-1e-5;
					XMOLEdll(dxkgnew,dxmolnew,dwmnew);
					PHFLSHdll(dp,dh,dxmolnew,spare14,dnew,spare3,spare4,spare1,spare2,spare5,spare6,spare13,spare7,spare8,spare11,lerr,errormsg,errormessagelength);
					ddddX_ph = (dnew*dwm-dd*dwm)/1e-5; // mass basis

/*
					double HA,HB,dxmolnew[ncmax],Tnew,pnew,dlow,dhigh,hlow,hhigh,h_l,h_v;
					//double ddq,ddl_l,ddv_v,ddx[ncmax],ddy[ncmax];

					ENTHALdll (dt,ddl,dxmoll,h_l);
					ENTHALdll (dt,ddv,dxmolv,h_v);
					HA = 0.025*(h_v-h_l)+h_l;
					HB = 0.075*(h_v-h_l)+h_l;

					double DT,DP,DX,smooth,dddT_pX,dhdT_pX,dddP_TX,dhdP_TX,dddX_pT,dhdX_pT;

					DT=0.5;
					DP=-25;
					DX=0.002;

					if (dh < HA) {
					// DT
						//DT=DT;
						Tnew= dt+DT;
							//this function does not work?
							//(t,p,z,Dl,Dv,x,y,q,ierr,herr)
							//TPFL2dll (Tnew,dp,dxmol,ddl_l,ddv_v,ddx,ddy,ddq,lerr,errormsg,errormessagelength);
							//dlow = 1/(ddq/ddv_v + (1-ddq)/ddl_l);
							//ENTHALdll (Tnew,ddl_l,ddx,h_l);
							//ENTHALdll (Tnew,ddv_v,ddy,h_v);
							//hlow = ddq*h_v + (1-ddq)*h_l;
						if (debug) printf("Calling TPFLSHdll with T=%f and P=%f. and xmol[0]=%f\n",Tnew,dp,dxmol[0]);
						TPFLSHdll(Tnew,dp,dxmol,dlow,spare3,spare4,spare1,spare2,spare5,spare6,hlow,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
						dddT_pX = (dlow-dd)/(DT);
						dhdT_pX = (hlow-dh)/(DT);
					// DP
						//DP=DP;
						pnew = dp+DP;
							//TPFL2dll (dt,pnew,dxmol,ddl_l,ddv_v,ddx,ddy,ddq,lerr,errormsg,errormessagelength);
							//dlow = 1/(ddq/ddv_v + (1-ddq)/ddl_l);
							//ENTHALdll (dt,ddl_l,ddx,h_l);
							//ENTHALdll (dt,ddv_v,ddy,h_v);
							//hlow = ddq*h_v + (1-ddq)*h_l;
						if (debug) printf("Calling TPFLSHdll with T=%f and P=%f. and xmol[0]=%f\n",dt,pnew,dxmol[0]);
						TPFLSHdll(dt,pnew,dxmol,dlow,spare3,spare4,spare1,spare2,spare5,spare6,hlow,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
						dddP_TX = (dlow-dd)/(DP);
						dhdP_TX = (hlow-dh)/(DP);
					// DX
						//DX = DX;
						dxmolnew[0]=dxmol[0]+DX;  // Be carefull when comparing.. keeping X2 const is not the same as keeping z2 const ??? or..
						dxmolnew[1]=dxmol[1]-DX; // This is wrong as you may show that x2 should be eheld constant for an ideal gas mix, but refprop needs sum(x)=1
							//TPFL2dll (dt,dp,dxmolnew,ddl_l,ddv_v,ddx,ddy,ddq,lerr,errormsg,errormessagelength);
							//dlow = 1/(ddq/ddv_v + (1-ddq)/ddl_l);
							//ENTHALdll (dt,ddl_l,ddx,h_l);
							//ENTHALdll (dt,ddv_v,ddy,h_v);
							//hlow = ddq*h_v + (1-ddq)*h_l;
						if (debug) printf("Calling TPFLSHdll with T=%f and P=%f. and xmol[0]=%f\n",dt,dp,dxmolnew[0]);
						TPFLSHdll(dt,dp,dxmolnew,dlow,spare3,spare4,spare1,spare2,spare5,spare6,hlow,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
						double dwmnew;
						WMOLdll(dxmolnew,dwmnew);
						dddX_pT = (dlow/dwmnew-dd/dwm)*dwm/(DX);
						dhdX_pT = (hlow/dwmnew-dh/dwm)*dwm/(DX);
					} else if (dh > HB) {
					// DT
						DT = -DT;
						Tnew= dt+DT;
							//dhigh = 1/(ddq/ddv_v + (1-ddq)/ddl_l);
							//ENTHALdll (Tnew,ddl_l,ddx,h_l);
							//ENTHALdll (Tnew,ddv_v,ddy,h_v);
							//hhigh = ddq*h_v + (1-ddq)*h_l;
						if (debug) printf("Calling TPFLSHdll with T=%f and P=%f. and xmol[0]=%f\n",Tnew,dp,dxmol[0]);
						TPFLSHdll(Tnew,dp,dxmol,dhigh,spare3,spare4,spare1,spare2,spare5,spare6,hhigh,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
						dddT_pX = (dhigh-dd)/(DT);
						dhdT_pX = (hhigh-dh)/(DT);
					// DP
						DP = -DP;
						pnew=dp+DP;
							//TPFL2dll (dt,pnew,dxmol,ddl_l,ddv_v,ddx,ddy,ddq,lerr,errormsg,errormessagelength);
							//dhigh = 1/(ddq/ddv_v + (1-ddq)/ddl_l);
							//ENTHALdll (dt,ddl_l,ddx,h_l);
							//ENTHALdll (dt,ddv_v,ddy,h_v);
							//hhigh = ddq*h_v + (1-ddq)*h_l;
						if (debug) printf("Calling TPFLSHdll with T=%f and P=%f. and xmol[0]=%f\n",dt,pnew,dxmol[0]);
						TPFLSHdll(dt,pnew,dxmol,dhigh,spare3,spare4,spare1,spare2,spare5,spare6,hhigh,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
						dddP_TX = (dhigh-dd)/(DP);
						dhdP_TX = (hhigh-dh)/(DP);
					// DX
						DX = -DX;
						dxmolnew[0]=dxmol[0]+DX;  // Be carefull when comparing.. keeping X2 const is not the same as keeping z2 const ??? or..
						dxmolnew[1]=dxmol[1]-DX; // This is wrong as you may show that x2 should be eheld constant for an ideal gas mix
							//TPFL2dll (dt,dp,dxmolnew,ddl_l,ddv_v,ddx,ddy,ddq,lerr,errormsg,errormessagelength);
							//dhigh = 1/(ddq/ddv_v + (1-ddq)/ddl_l);
							//ENTHALdll (dt,ddl_l,ddx,h_l);
							//ENTHALdll (dt,ddv_v,ddy,h_v);
							//hhigh = ddq*h_v + (1-ddq)*h_l;
						if (debug) printf("Calling TPFLSHdll with T=%f and P=%f. and xmol[0]=%f\n",dt,dp,dxmolnew[0]);
						TPFLSHdll(dt,dp,dxmolnew,dhigh,spare3,spare4,spare1,spare2,spare5,spare6,hhigh,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
						double dwmnew;
						WMOLdll(dxmolnew,dwmnew);
						dddX_pT = (dhigh/dwmnew-dd/dwm)*dwm/(DX);
						dhdX_pT = (hhigh/dwmnew-dh/dwm)*dwm/(DX);
					} else { // something smooth
						smooth=(dh-HA)/(HB-HA);
					// DT
						//DT = DT;
						Tnew= dt+DT;
							//TPFL2dll (Tnew,dp,dxmol,ddl_l,ddv_v,ddx,ddy,ddq,lerr,errormsg,errormessagelength);
							//dlow = 1/(ddq/ddv_v + (1-ddq)/ddl_l);
							//ENTHALdll (Tnew,ddl_l,ddx,h_l);
							//ENTHALdll (Tnew,ddv_v,ddy,h_v);
							//hlow = ddq*h_v + (1-ddq)*h_l;
						if (debug) printf("Calling TPFLSHdll with T=%f and P=%f. and xmol[0]=%f\n",Tnew,dp,dxmol[0]);
						TPFLSHdll(Tnew,dp,dxmol,dlow,spare3,spare4,spare1,spare2,spare5,spare6,hlow,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
						DT=-DT;
						Tnew= dt+DT;
							//TPFL2dll (Tnew,dp,dxmol,ddl_l,ddv_v,ddx,ddy,ddq,lerr,errormsg,errormessagelength);
							//dhigh = 1/(ddq/ddv_v + (1-ddq)/ddl_l);
							//ENTHALdll (Tnew,ddl_l,ddx,h_l);
							//ENTHALdll (Tnew,ddv_v,ddy,h_v);
							//hhigh = ddq*h_v + (1-ddq)*h_l;
						if (debug) printf("Calling TPFLSHdll with T=%f and P=%f. and xmol[0]=%f\n",Tnew,dp,dxmol[0]);
						TPFLSHdll(Tnew,dp,dxmol,dhigh,spare3,spare4,spare1,spare2,spare5,spare6,hhigh,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
						dddT_pX = (dlow-dd)/(-DT)*(1-smooth) + smooth*(dhigh-dd)/(DT);
						dhdT_pX = (hlow-dh)/(-DT)*(1-smooth) + smooth*(hhigh-dh)/(DT);
					// DP
						//DP = DP;
						pnew=dp+DP;
							//TPFL2dll (dt,pnew,dxmol,ddl_l,ddv_v,ddx,ddy,ddq,lerr,errormsg,errormessagelength);
							//dlow = 1/(ddq/ddv_v + (1-ddq)/ddl_l);
							//ENTHALdll (dt,ddl_l,ddx,h_l);
							//ENTHALdll (dt,ddv_v,ddy,h_v);
							//hlow = ddq*h_v + (1-ddq)*h_l;
						if (debug) printf("Calling TPFLSHdll with T=%f and P=%f. and xmol[0]=%f\n",dt,pnew,dxmol[0]);
						TPFLSHdll(dt,pnew,dxmol,dlow,spare3,spare4,spare1,spare2,spare5,spare6,hlow,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
						DP = -DP;
						pnew=dp+DP;
							//TPFL2dll (dt,pnew,dxmol,ddl_l,ddv_v,ddx,ddy,ddq,lerr,errormsg,errormessagelength);
							//dhigh = 1/(ddq/ddv_v + (1-ddq)/ddl_l);
							//ENTHALdll (dt,ddl_l,ddx,h_l);
							//ENTHALdll (dt,ddv_v,ddy,h_v);
							//hhigh = ddq*h_v + (1-ddq)*h_l;
						if (debug) printf("Calling TPFLSHdll with T=%f and P=%f. and xmol[0]=%f\n",dt,pnew,dxmol[0]);
						TPFLSHdll(dt,pnew,dxmol,dhigh,spare3,spare4,spare1,spare2,spare5,spare6,hhigh,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
						dddP_TX = (dlow-dd)/(-DP)*(1-smooth) + smooth*(dhigh-dd)/(DP);
						dhdP_TX = (hlow-dh)/(-DP)*(1-smooth) + smooth*(hhigh-dh)/(DP);
					// DX
						//DX = DX;
						dxmolnew[0]=dxmol[0]+DX;  // Be carefull when comparing.. keeping X2 const is not the same as keeping z2 const ??? or..
						dxmolnew[1]=dxmol[1]-DX; // This is wrong as you may show that x2 should be eheld constant for an ideal gas mix
							//TPFL2dll (dt,dp,dxmolnew,ddl_l,ddv_v,ddx,ddy,ddq,lerr,errormsg,errormessagelength);
							//dlow = 1/(ddq/ddv_v + (1-ddq)/ddl_l);
							//ENTHALdll (dt,ddl_l,ddx,h_l);
							//ENTHALdll (dt,ddv_v,ddy,h_v);
							//hlow = ddq*h_v + (1-ddq)*h_l;
						if (debug) printf("Calling TPFLSHdll with T=%f and P=%f. and xmol[0]=%f\n",dt,dp,dxmolnew[0]);
						TPFLSHdll(dt,dp,dxmolnew,dlow,spare3,spare4,spare1,spare2,spare5,spare6,hlow,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
						DX = -DX;
						dxmolnew[0]=dxmol[0]+DX;  // Be carefull when comparing.. keeping X2 const is not the same as keeping z2 const ??? or..
						dxmolnew[1]=dxmol[1]-DX; // This is wrong as you may show that x2 should be eheld constant for an ideal gas mix
							//TPFL2dll (dt,dp,dxmolnew,ddl_l,ddv_v,ddx,ddy,ddq,lerr,errormsg,errormessagelength);
							//dhigh = 1/(ddq/ddv_v + (1-ddq)/ddl_l);
							//ENTHALdll (dt,ddl_l,ddx,h_l);
							//ENTHALdll (dt,ddv_v,ddy,h_v);
							//hhigh = ddq*h_v + (1-ddq)*h_l;
						if (debug) printf("Calling TPFLSHdll with T=%f and P=%f. and xmol[0]=%f\n",dt,dp,dxmolnew[0]);
						TPFLSHdll(dt,dp,dxmolnew,dhigh,spare3,spare4,spare1,spare2,spare5,spare6,hhigh,spare7,spare8,spare11,spare12,lerr,errormsg,errormessagelength);
						dddX_pT = (dlow-dd)/(-DX)*(1-smooth) + smooth*(dhigh-dd)/(DX);
						dhdX_pT = (hlow-dh)/(-DX)*(1-smooth) + smooth*(hhigh-dh)/(DX);
					}

					ddddh_p = dddT_pX/dhdT_pX;
					ddddp_h = (dddP_TX*dhdT_pX-dhdP_TX*dddT_pX)/dhdT_pX;
					ddddX_ph = -(dddT_pX*dhdX_pT-dhdT_pX*dddX_pT)/dhdT_pX;
					ddddX_ph = ddddX_ph*dwm;
*/

				}


			}

		}

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
		TRNPRPdll(dt,dd,dxmol,deta,dtcx,lerr,errormsg,errormessagelength);
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

	double dxlkg[ncmax], dxvkg[ncmax];
	XMASSdll(dxmoll,dxlkg,dwliq);
	XMASSdll(dxmolv,dxvkg,dwvap);

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

	for (int dim=0; dim<lnc; dim++){
		//if (debug) printf("Processing %i:%f, %f \n",dim,dxlkg[dim],dxvkg[dim]);
		props[16+dim] = dxlkg[dim];
		props[16+lnc+dim] = dxvkg[dim];
	}
	return 0;
}


double props_REFPROP(char* what, char* statevars_in, char* fluidnames, double *ders, double *trns, double *props, double statevar1, double statevar2, double* x, int phase, char* REFPROP_PATH, char* errormsg, int DEBUGMODE, int calcTransport, int partialDersInputChoice){
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
	if (calcTransport) calcTrans = true;
	//if (calcTwoPhaseNumericalDerivatives) calcTwoPhaseNumDers = true;
	//if (calcTwoPhasePsuedoAnalyticalDerivatives) calcTwoPhasePsuedoAnalDers = true;
	//if (statesTPX) dynstatesTPX = true;

	if (partialDersInputChoice==1) PartialDersInputChoice=1;	    // no partial derivatives is computed
	else if (partialDersInputChoice==2) PartialDersInputChoice=2;   // Numerical derivatives of density wrt. pressure, enthalpy and mass fraction is computed
	else if (partialDersInputChoice==3) PartialDersInputChoice=3;   // Pseudo analytical derivatives of density wrt. pressure, enthalpy and mass fraction is computed (not exact, but faster)
	else if (partialDersInputChoice==4) PartialDersInputChoice=4;	// Numerical derivatives of density and enthalpy wrt. pressure, temperature and mass fraction is computed
	else {
		printf("Unexpected partialDersInputChoice %i\n", partialDersInputChoice);
		return -FAIL;
	}

//	if (partialDersInputChoice==1) {
//		calcTwoPhaseNumDers = true;
//		calcTwoPhasePsuedoAnalDers = false;
//		dynstatesTPX = false;
//	} else if (partialDersInputChoice==2) {
//		calcTwoPhaseNumDers = false;
//		calcTwoPhasePsuedoAnalDers = true;
//		dynstatesTPX = false;
//	} else if (partialDersInputChoice==3) {
//		calcTwoPhaseNumDers = true;
//		calcTwoPhasePsuedoAnalDers = false;
//		dynstatesTPX = true;
//	} else {
//		calcTwoPhaseNumDers = false;
//		calcTwoPhasePsuedoAnalDers = false;
//		dynstatesTPX = false;
//	}

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

	if (debug) printf("partial derivative choice = %i\n", partialDersInputChoice);


	if (setFluids(rPath,fluids,errormsg) != OK) {
		printf("Error initialising REFPROP: \"%s\"\n", errormsg);
		return -FAIL;
	}


	/*
	 * Here should be the state checking to avoid unnecessary calculations. when
	 * working with mixtures, the constants also have to be flushed if the
	 * composition changes.
	 */
	// Convert mass-based composition to mole fractions and set molecular weight.
	//double *dxkg;
	//dxkg = (double*) calloc(ncmax,sizeof(double));
	double dxmoltmp[ncmax];
	double dwmtmp;
	dxkg = x;
	XMOLEdll(dxkg,dxmoltmp,dwmtmp);
	// dwm = dwm / 1000; // from g/mol to kg/mol

	double val1 = 0;
	double val2 = 0;
	double var1 = 0;
	double var2 = 0;
	double var = 0;
	double val = 0;

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
		if ( strCompare(tmpVar, "p") ) {
			var = dp;
			val = getP_refprop(tmpValue);
		} else if ( strCompare(tmpVar, "T") ) {
			var = dt;
			val = getT_refprop(tmpValue);
		} else if ( strCompare(tmpVar, "s") ) {
			var = ds;
			val = getS_refprop(tmpValue);
		} else if ( strCompare(tmpVar, "h") ) {
			var = dh;
			val = getH_refprop(tmpValue);
		} else if ( strCompare(tmpVar, "d") ) {
			var = dd;
			val = getD_refprop(tmpValue);
		//} else if ( strCompare(tmpVar, "q") ) {
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



	if (lnc>1) flushConstants();
	else flushProperties();
	if (debug) printf("Loading a new state, flushed state. \n");


/* //TODO Is this really nescesary? I think the possibility of dymola calling with same state twice is almost none (it will call for other states to compute, in between)..

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
			updateProps(props, lerr);	// TODO Why only props? Are ders,trns, and props not already known?
			return result;
		}
	}
*/

	/*
	 * If we get to this point, the requested value was not part of an earlier
	 * calculation and we have to proceed to determine it via the Refprop library.
	 */
	// 	( strCompare(Poco::Environment::osName(), "linux") )

	// Set variables to input values
	if ( strCompare(in1, in2) ) {
		sprintf(errormsg,"State variable 1 is the same as state variable 2 (%s)\n",in1.c_str());
		return FAIL;
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
		if ( strCompare(tmpVar, "p") ) {
			dp = getP_refprop(tmpValue);
		} else if ( strCompare(tmpVar, "T") ) {
			dt = getT_refprop(tmpValue);
		} else if ( strCompare(tmpVar, "s") ) {
			ds = getS_refprop(tmpValue);
		} else if ( strCompare(tmpVar, "h") ) {
			dh = getH_refprop(tmpValue);
		} else if ( strCompare(tmpVar, "d") ) {
			dd = getD_refprop(tmpValue);
		} else if ( strCompare(tmpVar, "q") ) {
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
//				TPFL2dll(dt,dp,dxmol,ddl,ddv,dxmoll,dxmolv,dqmol,lerr,errormsg,errormessagelength);
//			}else{
				if (debug) printf("Calling TPFLSH with %f and %f.\n",dt,dp);
				TPFLSHdll(dt,dp,dxmol,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,dh,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
				//if (debug) printf("Getting dd: %f\n",dd);
//			}
		}else if (isInput(in1,in2,std::string("ph"))){
//			if (phase==1){ //fluid state is known to be single phase
//				PHFL1(p,h,x,liqvap,T,d,ierr,herr,errormessagelength);
////				if (liqvap==1) dl=d; else dv=d;
//			}else{
				if (debug) printf("Calling PHFLSH with %f and %f.\n",dp,dh);
				PHFLSHdll(dp,dh,dxmol,dt,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
//			}
		}else if (isInput(in1,in2,std::string("pd"))){
			if (phase==1){ //fluid state is known to be single phase
				PDFL1dll(dp,dd,dxmol,dt,lerr,errormsg,errormessagelength);
			}else{
				if (debug) printf("Calling PDFLSH with %f and %f.\n",dp,dd);
				PDFLSHdll(dp,dd,dxmol,dt,ddl,ddv,dxmoll,dxmolv,dqmol,de,dh,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
			}
		}else if (isInput(in1,in2,std::string("sp"))){
/*			if (phase==1){ //fluid state is known to be single phase
				PSFL1(p,s,dxmol,kph,T,d,lerr,errormsg,errormessagelength);
				if (liqvap==1) dl=d; else dv=d;
			}else{*/
				if (debug) printf("Calling PSFLSH with %f and %f.\n",dp,ds);
				PSFLSHdll(dp,ds,dxmol,dt,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,dh,dCv,dCp,dw,lerr,errormsg,errormessagelength);
//			}
		}else if (isInput(in1,in2,std::string("pq"))){
			if (debug) printf("Calling PQFLSH with %f and %f.\n",dp,dqkg);
			PQFLSHdll(dp,dqkg,dxmol,kq,dt,dd,ddl,ddv,dxmoll,dxmolv,de,dh,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
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
				THFLSHdll(dt,dh,dxmol,kr,dp,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
//			}
		}else if (isInput(in1,in2,std::string("td"))){
			if (debug) printf("Calling TDFLSH with %f and %f.\n",dt,dd);
			TDFLSHdll(dt,dd,dxmol,dp,ddl,ddv,dxmoll,dxmolv,dqmol,de,dh,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
		}else if (isInput(in1,in2,std::string("ts"))){
			long kr = 2;
			if (debug) printf("Calling TSFLSH with %f and %f.\n",dt,ds);
			TSFLSHdll(dt,ds,dxmol,kr,dp,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,dh,dCv,dCp,dw,lerr,errormsg,errormessagelength);
		}else if (isInput(in1,in2,std::string("tq"))){
			if (debug) printf("Calling TQFLSH with %f and %f.\n",dt,dqkg);
			TQFLSHdll(dt,dqkg,dxmol,kq,dp,dd,ddl,ddv,dxmoll,dxmolv,de,dh,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
		}else if (isInput(in1,in2,std::string("dh"))){
			switch(phase){ //fluid state is known to be single phase
				case 1:
					DHFL1dll(dd,dh,dxmol,dt,lerr,errormsg,errormessagelength);
					break;
				case 2:
					DHFL2dll(dd,dh,dxmol,dt,dp,ddl,ddv,dxmoll,dxmolv,dqmol,lerr,errormsg,errormessagelength);
					break;
				default:
					if (debug) printf("Calling DHFLSH with %f and %f.\n",dd,dh);
					DHFLSHdll(dd,dh,dxmol,dt,dp,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
					break;
			}
		}else if (isInput(in1,in2,std::string("hs"))){
			HSFLSHdll(dh,ds,dxmol,dt,dp,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,dCv,dCp,dw,lerr,errormsg,errormessagelength);
		}else if (isInput(in1,in2,std::string("ds"))){
			switch(phase){ //fluid state is known to be single phase
				case 1:
					DSFL1dll(dd,ds,dxmol,dt,lerr,errormsg,errormessagelength);
					break;
				case 2:
					DSFL2dll(dd,ds,dxmol,dt,dp,ddl,ddv,dxmoll,dxmolv,dqmol,lerr,errormsg,errormessagelength);
					break;
				default:
					if (debug) printf("Calling DSFLSH with %f and %f.\n",dd,ds);
					DSFLSHdll(dd,ds,dxmol,dt,dp,ddl,ddv,dxmoll,dxmolv,dqmol,de,dh,dCv,dCp,dw,lerr,errormsg,errormessagelength);
					break;
			}
		}else
			sprintf(errormsg,"Unknown combination of state variables! %s and %s", in1.c_str(), in2.c_str());
	}


//	switch(tolower(what[0])){ 	//CHOOSE RETURN VARIABLE
//		case 'v':	//dynamic viscosity uPa.s
//		case 'l':	//thermal conductivity W/m.K
//			TRNPRPdll(dt,dd,dxmol,deta,dtcx,lerr,errormsg,errormessagelength);
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


	if (PartialDersInputChoice!=1) {
		int outVal = ders_REFPROP(ders,errormsg,debug);
		if ( 0 != outVal || ders[0] != 0 ) printf("Error in derivative function, returned %i\n",outVal);
	} else { // compute beta and kappa anyway in single-phase region
		if (dqmol < 0. || dqmol > 1.) {
			if (debug) printf("Calling THERM3 with T=%f and rho=%f.\n",dt,dd);
			double spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12;
			THERM3dll (dt,dd,dxmol,dxkappa,dbeta,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12);
		} else {
			dxkappa=noValue; //TODO why is this not flushed?
			dbeta=noValue;
		}
		updateDers(ders, lerr);
	}

	if (calcTrans) {
		int outVal = trns_REFPROP(trns,errormsg,debug);
		if ( 0 != outVal || trns[0] != 0 ) printf("Error in transport property function, returned %i\n",outVal);
	}
	if ( strCompare(out, "p") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getP_modelica());
		return getP_modelica();
	} else if ( strCompare(out, "t") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getT_modelica());
		return getT_modelica();
	} else if ( strCompare(out, "m") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getWML_modelica());
		return getWM_modelica();
	} else if ( strCompare(out, "d") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getD_modelica());
		return getD_modelica();
	} else if ( strCompare(out, "q") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getQ_modelica());
		return getQ_modelica();
	} else if ( strCompare(out, "e") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getE_modelica());
		return getE_modelica();
	} else if ( strCompare(out, "h") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getH_modelica());
		return getH_modelica();
	} else if ( strCompare(out, "s") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getS_modelica());
		return getS_modelica();
	} else if ( strCompare(out, "w") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getW_modelica());
		return getW_modelica();
	} else if ( strCompare(out, "v") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getETA_modelica());
		return getETA_modelica();
	} else if ( strCompare(out, "l") ) {
		if (debug) printf("Returning %s = %f\n",out.c_str(),getTCX_modelica());
		return getTCX_modelica();
	} else {
		return -1.0;
	}

}


//---------------------------------------------------------------------------


double satprops_REFPROP(char* what, char* statevar_in, char* fluidnames, double *satprops, double statevarval, double Tsurft, double* x, char* REFPROP_PATH, char* errormsg, int DEBUGMODE, int calcTransport){
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

	if (setFluids(rPath,fluids,errormsg) != OK) {
		printf("Error initialising REFPROP: \"%s\"\n", errormsg);
		return -FAIL;
	}


	/*
	 * Here should be the state checking to avoid unnecessary calculations. when
	 * working with mixtures, the constants also have to be flushed if the
	 * composition changes.
	 */
	bool knownState = false; // dummies to force recalculation
	bool valueExists = false;

// TODO This is out-commented for props_refprop function
// TODO - why do we want to flush properties for the setState functions here when setsat is called?
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
//	double* dxkg;
	dxkg = x;
	XMOLEdll(dxkg,dxmolsat,dwmsat);
	// dwm = dwm / 1000; // from g/mol to kg/mol   //keep refprop units..

	// loop through possible inputs
	if ( strCompare(in1, "p") ) {
		dpsat = getP_refprop(statevarval);
	} else if ( strCompare(in1, "t") ) {
		dtsat = getT_refprop(statevarval);
	}
	/* else if ( strCompare(in1, "d") ) {
		dd = getD_refprop(statevarval);
	} */
	 else {
		lerr = 2;
		sprintf(errormsg,"Unknown state variable: %s\n", in1.c_str());
		return lerr;
	}
	if (debug)  printf("\nstatevar %s checked\n",in1.c_str());

	long j;
/* j--phase flag: 1 = input x is liquid composition (bubble point)
		    2 = input x is vapor composition (dew point)
		    3 = input x is liquid composition (freezing point)
		    4 = input x is vapor composition (sublimation point)
*/
	//long kph = -1;
/* kph--flag specifying desired root for multi-valued inputs
           has meaning only for water at temperatures close to its triple point
          -1 = return middle root (between 0 and 4 C)
           1 = return highest temperature root (above 4 C)
           3 = return lowest temperature root (along freezing line) */

	double spare1,spare2,spare3,spare4,spare10[ncmax];

	if (lerr==0) {
		if ( strCompare(in1, "t") ) {
			j=1;
			SATTdll(dtsat,dxmolsat,j,dplsat,ddlsat,spare1,dxmollsat,spare10,lerr,errormsg,errormessagelength);
			THERMdll (dtsat,ddlsat,dxmollsat,spare1,spare2,dhlsat,dslsat,dcvlsat,dcplsat,spare3,spare4);
			j=2;
			SATTdll(dtsat,dxmolsat,j,dpvsat,spare1,ddvsat,spare10,dxmolvsat,lerr,errormsg,errormessagelength);
			THERMdll (dtsat,ddvsat,dxmolvsat,spare1,spare2,dhvsat,dsvsat,dcvvsat,dcpvsat,spare3,spare4);
			dtlsat=dtsat;
			dtvsat=dtsat;
			//THERM (t,		rho,		x,		p,		e,			h,		s,		cv,		cp,	 	w,		hjt)
		} else if ( strCompare(in1, "p") ) {
			j=1;
			SATPdll(dpsat,dxmolsat,j,dtlsat,ddlsat,spare1,dxmollsat,spare10,lerr,errormsg,errormessagelength);
			THERMdll (dtlsat,ddlsat,dxmollsat,dplsat,spare2,dhlsat,dslsat,dcvlsat,dcplsat,spare3,spare4);
			j=2;
			SATPdll(dpsat,dxmolsat,j,dtvsat,spare1,ddvsat,spare10,dxmolvsat,lerr,errormsg,errormessagelength);
			THERMdll (dtvsat,ddvsat,dxmolvsat,dpvsat,spare2,dhvsat,dsvsat,dcvvsat,dcpvsat,spare3,spare4);
			//dplsat=dpsat;
			//dpvsat=dpsat;
			switch(lerr){
				case 2:
					strcpy(errormsg,"P < Ptp");
					break;
				case 4:
					strcpy(errormsg,"P < 0");
					break;
			}
			//sprintf(errormsg,"p=%f, h=%f",p ,statevar2);
		}
		/*
		 else if ( strCompare(in1, "d") ) {
			SATDdll(dd,dxmol,j,kph,dt,dp,ddl,ddv,dxmoll,dxmolv,lerr,errormsg,errormessagelength);
			switch(lerr){
				case 2:
					strcpy(errormsg,"D > Dmax");
					break;
			}
		}
		*/
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




	double dsigma;

 	if (calcTransport) {
 		if ( strCompare(in1, "t") ) {
 			SURTENdll (dtsat,ddlsat,ddvsat,dxmollsat,dxmolvsat,dsigma,lerr,errormsg,errormessagelength);
 		} else if ( strCompare(in1, "p") ) {
 			SURFTdll (Tsurft,ddlsat,dxmollsat,dsigma,lerr,errormsg,errormessagelength);
 		}
	} else {
		dsigma =0;
	}


	double dxlkg[ncmax], dxvkg[ncmax];
	XMASSdll(dxmollsat,dxlkg,dwlsat);
	XMASSdll(dxmolvsat,dxvkg,dwvsat);

	//ASSIGN VALUES TO RETURN ARRAY
	satprops[0] = lerr;//error code
	satprops[1] = dtlsat;				//Temperature in K
	satprops[2] = dtvsat;				//Temperature in K
	satprops[3] = dplsat*1000;			//pressure in kPa->Pa
	satprops[4] = dpvsat*1000;			//pressure in kPa->Pa
	satprops[5] = ddlsat*dwlsat;		//density of liquid phase mol/L ->g/L (kg/m3)
	satprops[6] = ddvsat*dwvsat;		//density of vapor phase mol/L ->g/L (kg/m3)
	satprops[7] = dhlsat/dwlsat*1000;	//enthalpy of liquid J/mol -> J/g e-3 -> J/kg
	satprops[8] = dhvsat/dwvsat*1000;	//enthalpy of vapor J/mol -> J/g e-3 -> J/kg
	satprops[9] = dslsat/dwlsat*1000;	//entropy of liquid J/molK -> J/gK e-3 -> J/kgK
	satprops[10] = dsvsat/dwvsat*1000;	//entropy of vapor J/molK -> J/gK e-3 -> J/kgK
	satprops[11] = dsigma; 				//surface tension

//	satprops[12] = dwlsat/1000;			//molecular weight g/mol -> kg/mol
//	satprops[13] = dwvsat/1000;			//molecular weight g/mol -> kg/mol
	for (int ii=0;ii<lnc;ii++){
		satprops[12+ii] = dxkg[ii];
//		satprops[14+lnc+ii] = dxlkg[ii];
//		satprops[14+lnc+lnc+ii] = dxvkg[ii];
	}

	if (debug) printf("Returning %s\n",out.c_str());

	if ( strCompare(out, "p") ) {
		return dpsat*1000;
	} else if ( strCompare(out, "t") ) {
		return dtsat;
	}
	/* else if ( strCompare(out, "m") ) {
		return getWM_modelica();
	} else if ( strCompare(out, "d") ) {
		return getD_modelica();
	}
	*/
	 else {
		satprops[0] = 2.0;
		sprintf(errormsg,"Cannot return variable %s in saturation calculation", out.c_str());
		return -1.0;
	}
}
