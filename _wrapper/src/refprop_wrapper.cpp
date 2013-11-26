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
static const double noValue	= -1e+10;

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
//long   lerr;  			// Error return mechanism
double dhelp = noValue;


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

double ddddp_h, ddddh_p;

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

	ddddp_h=noValue;
	ddddh_p=noValue;

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
double loadLibrary() {
	if (RefpropdllInstance == NULL) { // Refprop is not loaded
#if defined(__ISWINDOWS__)
#if   defined(UNICODE)
		//RefpropdllInstance = LoadLibrary((LPCWSTR)libName);
		RefpropdllInstance = LoadLibraryW((LPCWSTR)libName);
#else
		RefpropdllInstance = LoadLibrary((LPCSTR)libName);
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
		if (debug) printf ("Function pointers set to macro values.\n");
		return OK;
	}
	return FAIL;
}

/*
 * Make sure the library is loaded
 * properly and set pointers.
 */
double initRefprop() {
	if (RefpropdllInstance == NULL) {
		if (debug) printf ("Library not loaded, trying to do so.\n");
		if (loadLibrary() != OK) {
			printf("Refprop library %s cannot be loaded, make sure you ",libName);
			printf("installed it properly.\n");
			return FAIL;
		}
		if (setFunctionPointers() != OK) {
			printf("There was an error setting the REFPROP function pointers, ");
			printf("check types and names in header file.\n");
			return FAIL;
		}
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

	if (initRefprop() != OK){
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
		
		//printf ("%s \n",RefString.c_str());
		
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
double get_dddh_p_modelica(){
	return ddddh_p*dwm*dwm/1000.; // (mol/l * mol/J) * g/mol * g/mol * 1kg/1000g = kg/m3 * kg/J
}
// Derivative of density with respect to pressure at constant enthalpy
double get_dddp_h_modelica(){
	return ddddp_h*dwm/1000.; // mol/(l.kPa) * g/mol * 1kPa/1000Pa = kg/(m3.Pa)
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
//	THERM2dll (dt,dd,dxmol,spare5,spare6,spare8,spare9,dCv,dCp,dw,dZ,dhjt,dA,dG,dxkappa,dbeta,ddpdd,dd2pdd2,ddpdt,ddddt,ddddp,dd2pdt2,dd2pdtd,spare3,spare4);
////	deltaP = 1.;
////	pLow   = dp - 0.5*deltaP;
////	pHigh  = dp + 0.5*deltaP;
////	rhoLow = 0;
////	rhoHigh = 0;
////	//PHFLSHdll(dp,hLow,dxmol,dt,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
////	if (debug) printf("Calling PHFLSH with %f and %f.\n",pLow,dh);
////	PHFLSHdll(pLow,dh,dxmol,spare3,rhoLow,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
////	if (debug) printf("Calling PHFLSH with %f and %f.\n",pHigh,dh);
////	PHFLSHdll(pHigh,dh,dxmol,spare3,rhoHigh,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
////	if (debug) printf("Setting ddddp from %f and %f.\n",rhoHigh,rhoLow);
////	ddddp = (rhoHigh-rhoLow) / (pHigh-pLow);
//
//	// get derivative of density with respect to enthalpy numerically
//	deltaH = 20.;
//	hLow   = dh - 0.5*deltaH;
//	hHigh  = dh + 0.5*deltaH;
//	rhoLow = 0;
//	rhoHigh = 0;
//	//PHFLSHdll(dp,hLow,dxmol,dt,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
//	if (debug) printf("Calling PHFLSH with %f and %f.\n",dp,hLow);
//	PHFLSHdll(dp,hLow,dxmol,spare3,rhoLow,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
//	if (debug) printf("Calling PHFLSH with %f and %f.\n",dp,hHigh);
//	PHFLSHdll(dp,hHigh,dxmol,spare3,rhoHigh,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
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
////	THERM2dll (dt,dd,dxmol,spare5,spare6,spare8,spare9,dCv,dCp,dw,dZ,dhjt,dA,dG,dxkappa,dbeta,ddpdd,dd2pdd2,ddpdt,ddddt,ddddp,dd2pdt2,dd2pdtd,spare3,spare4);
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
	ders[19] = get_dddh_p_modelica();	// dD/dh at constant pressure [kg/m3 * kg/J]
	ders[20] = get_dddp_h_modelica();	// dD/dp at constant enthalpy [kg/(m3.Pa)]
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
		THERM2dll (dt,dd,dxmol,spare5,spare6,spare9,spare10,dCv,dCp,dw,dZ,dhjt,dA,dG,dxkappa,dbeta,ddpdd,dd2pdd2,ddpdt,ddddt,ddddp,dd2pdt2,dd2pdtd,spare3,spare4);

		// get derivatives of enthalpy
		if (debug) printf("Calling DHD1 with T=%f and rho=%f.\n",dt,dd);
		DHD1dll(dt,dd,dxmol,ddhdt_d,ddhdt_p,ddhdd_t,ddhdd_p,ddhdp_t,ddhdp_d);

		/*
		 * With the above values, the cyclic relation can be used to
		 * determine all necessary values.
		 *
		 * -1 = ddddp_ana * 1/(props.state.dhdp_rho) * props.state.dhdrho_p;
		 * ddddh_ana * props.state.dhdrho_p= 1;
		 *
		 * Below is a numerical approximation due to problems in the
		 * two-phase region.
		 */

		if (dqmol < 0. || dqmol > 1.) { // single-phase region
			if (debug) printf ("Using single-phase derivatives.\n");
			ddddp_h = -1. * ddhdp_d / ddhdd_p;
			ddddh_p = 1./ddhdd_p;
		} else { // two-phase region, get derivative of density with respect to enthalpy numerically
			

			// TODO
			ddddp_h = -1;
			ddddh_p = -1;
			/*
			if (debug) printf ("Using two-phase derivatives.\n");
			deltaP = 0.00005; // 0.05 Pascal difference
			pLow   = dp - 0.5*deltaP;
			pHigh  = dp + 0.5*deltaP;
			rhoLow = 0;
			rhoHigh = 0;
			//PHFLSHdll(dp,hLow,dxmol,dt,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
			if (debug) printf("Calling PHFLSH with %f and %f.\n",pLow,dh);
			PHFLSHdll(pLow,dh,dxmol,spare3,rhoLow,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
			if (debug) printf("Calling PHFLSH with %f and %f.\n",pHigh,dh);
			PHFLSHdll(pHigh,dh,dxmol,spare3,rhoHigh,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
			if (debug) printf("Setting dddp_h_num from %f and %f.\n",rhoHigh,rhoLow);
			ddddp_h = (rhoHigh-rhoLow) / (pHigh-pLow);

			// get derivative of density with respect to enthalpy numerically
			deltaH = 0.05; // 0.05 Joule total difference
			hLow   = dh - 0.5*deltaH;
			hHigh  = dh + 0.5*deltaH;
			rhoLow = 0;
			rhoHigh = 0;
			//PHFLSHdll(dp,hLow,dxmol,dt,dd,ddl,ddv,dxmoll,dxmolv,dqmol,de,ds,dCv,dCp,dw,lerr,errormsg,errormessagelength);
			if (debug) printf("Calling PHFLSH with p=%f and h=%f for derivative.\n",dp,hLow);
			PHFLSHdll(dp,hLow,dxmol,spare3,rhoLow,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
			if (debug) printf("Calling PHFLSH with p=%f and h=%f for derivative.\n",dp,hHigh);
			PHFLSHdll(dp,hHigh,dxmol,spare3,rhoHigh,spare5,spare6,spare7,spare8,spare9,spare10,spare11,spare12,spare13,spare14,lerr,errormsg,errormessagelength);
			if (debug) printf("Setting dddh_p_num from %f and %f.\n",rhoHigh,rhoLow);
			ddddh_p = (rhoHigh-rhoLow) / (hHigh-hLow);
			*/
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

	XMASSdll(dxmoll,dxlkg,dwliq);
	XMASSdll(dxmolv,dxvkg,dwvap);

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
	double* dxkg;
	double dxmoltmp[ncmax];
	double dwmtmp;
	//dxkg = (double*) calloc(ncmax,sizeof(double));
	//dxmoltmp = (double*) calloc(ncmax,sizeof(double));
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


	// TODO
	/*
	int outVal = ders_REFPROP(ders,errormsg,debug);
	if ( 0 != outVal || ders[0] != 0 ) printf("Error in derivative function, returned %i\n",outVal);

	outVal = trns_REFPROP(trns,errormsg,debug);
	if ( 0 != outVal || trns[0] != 0 ) printf("Error in transport property function, returned %i\n",outVal);
	*/

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
	if (debug) printf("\nStarting function props_REFPROP to calculate %s.\n", out.c_str());

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
	XMOLEdll(dxkg,dxmol,dwm);
	// dwm = dwm / 1000; // from g/mol to kg/mol

	// loop through possible inputs
	if ( strCompare(in1, "p") ) {
		dp = getP_refprop(statevarval);
	} else if ( strCompare(in1, "t") ) {
		dt = getT_refprop(statevarval);
	} else if ( strCompare(in1, "d") ) {
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
		if ( strCompare(in1, "t") ) {
			SATTdll(dt,dxmol,j,dp,ddl,ddv,dxmoll,dxmolv,lerr,errormsg,errormessagelength);
		} else if ( strCompare(in1, "p") ) {
			SATPdll(dp,dxmol,j,dt,ddl,ddv,dxmoll,dxmolv,lerr,errormsg,errormessagelength);
			switch(lerr){
				case 2:
					strcpy(errormsg,"P < Ptp");
					break;
				case 4:
					strcpy(errormsg,"P < 0");
					break;
			}
			//sprintf(errormsg,"p=%f, h=%f",p ,statevar2);
		} else if ( strCompare(in1, "d") ) {
			SATDdll(dd,dxmol,j,kph,dt,dp,ddl,ddv,dxmoll,dxmolv,lerr,errormsg,errormessagelength);
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

	XMASSdll(dxmoll,dxlkg,dwliq);
	XMASSdll(dxmolv,dxvkg,dwvap);

	for (int ii=0;ii<lnc;ii++){
		props[16+ii] = dxlkg[ii];
		props[16+lnc+ii] = dxvkg[ii];
	}

	if (debug) printf("Returning %s\n",out.c_str());

	if ( strCompare(out, "p") ) {
		return getP_modelica();
	} else if ( strCompare(out, "t") ) {
		return getT_modelica();
	} else if ( strCompare(out, "m") ) {
		return getWM_modelica();
	} else if ( strCompare(out, "d") ) {
		return getD_modelica();
	} else {
		props[0] = 2.0;
		sprintf(errormsg,"Cannot return variable %s in saturation calculation", out.c_str());
		return -1.0;
	}
}
