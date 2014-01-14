
#ifndef CPP_WRAPPED_H
#define CPP_WRAPPED_H

//#include "../src/refprop_library.h"

// Some constants for REFPROP... defined by macros for ease of use
#define refpropcharlength 255
#define refpropcharlong 10000
#define filepathlength 255
#define lengthofreference 3
#define errormessagelength 255
#define ncmax 20		// Note: ncmax is the max number of components
#define numparams 72
#define maxcoefs 50

/* Define some new functions to access low-level refprop routines.
 *
 * The new functions should be used to compare the speed of the wrapper to the speed
 * of the Fortran code and the Matlab implementation.
 */

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__WIN64__)
#	define __ISWINDOWS__
#	include <windows.h>
#	define _CRT_SECURE_NO_WARNINGS
#elif __APPLE__
#	define __ISAPPLE__
#elif __linux
#	define __ISLINUX__
#endif

#  	define CALLCONV __stdcall
# 	define SETUPdll SETUPdll
# 	define WMOLdll WMOLdll
#	define XMOLEdll XMOLEdll
# 	define PHFLSHdll PHFLSHdll
# 	define TDFLSHdll TDFLSHdll

# 	define RMIX2dll RMIX2dll
# 	define RDXHMXdll RDXHMXdll
# 	define PHIXdll PHIXdll
#   define PHI0dll PHI0dll
#   define PRESSdll PRESSdll
#   define ENTHALdll ENTHALdll
#   define CRITPdll CRITPdll
#   define MASSdll MASSdll


typedef void (CALLCONV *SETUPdll_POINTER)(long &,char*,char*,char*,long &,char*,long ,long ,long ,long );
typedef void (CALLCONV *WMOLdll_POINTER)(double *,double &);
typedef void (CALLCONV *XMOLEdll_POINTER)(double *,double *,double &);
typedef void (CALLCONV *PHFLSHdll_POINTER)(double &,double &,double *,double &,double &,double &,double &,double *,double *,double &,double &,double &,double &,double &,double &,long &,char*,long );
typedef void (CALLCONV *TDFLSHdll_POINTER)(double &,double &,double *,double &,double &,double &,double *,double *,double &,double &,double &,double &,double &,double &,double &,long &,char*,long );

typedef void (CALLCONV *RMIX2dll_POINTER)(double *,double &);
typedef void (CALLCONV *RDXHMXdll_POINTER)(long *,long *,long *,double *,double &,double &,long &,char*,long );
typedef void (CALLCONV *PHIXdll_POINTER)(long *,long *,double &,double &,double *,double &);
typedef void (CALLCONV *PHI0dll_POINTER)(long *,long *,double &,double &,double *,double &);

typedef void (CALLCONV *PRESSdll_POINTER)(double &,double &,double *,double &);
typedef void (CALLCONV *ENTHALdll_POINTER)(double &,double &,double *,double &);
typedef void (CALLCONV *CRITPdll_POINTER)(double *,double &,double &,double &,long &,char*,long );
typedef void (CALLCONV *XMASSdll_POINTER)(double *,double *,double &);

SETUPdll_POINTER SETUPdll;
WMOLdll_POINTER WMOLdll;
XMOLEdll_POINTER XMOLEdll;
PHFLSHdll_POINTER  PHFLSHdll;
TDFLSHdll_POINTER  TDFLSHdll;

RMIX2dll_POINTER RMIX2dll;
RDXHMXdll_POINTER RDXHMXdll;
PHIXdll_POINTER PHIXdll;
PHI0dll_POINTER PHI0dll;
PRESSdll_POINTER PRESSdll;
ENTHALdll_POINTER ENTHALdll;
CRITPdll_POINTER CRITPdll;
XMASSdll_POINTER XMASSdll;

#endif
