#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <sstream>
#include "../src/refprop_wrapper.h"
#include "../src/refprop_library.h"
#include <math.h>
#include <ctime>

// Some constants for REFPROP... defined by macros for ease of use
#define refpropcharlength 255
#define refpropcharlong 10000
#define filepathlength 255
#define lengthofreference 3
#define errormessagelength 255
#define ncmax 20		// Note: ncmax is the max number of components
#define numparams 72
#define maxcoefs 50


int main(int argc, char* argv[]){
	char errormsg[255];
	double* x;
	double *props;
	double *ders;
	double *trns;
	int DEBUG = 0;


	/* Define some new functions to access low-level refprop routines.
	 *
	 * The new functions should be used to compare the speed of the wrapper to the speed
	 * of the Fortran code and the Matlab implementation.
	 */

//#	if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__WIN64__)
//#  		define __ISWINDOWS__
//#  		include <windows.h>
//#  		define _CRT_SECURE_NO_WARNINGS
//#	elif __APPLE__
//#  		define __ISAPPLE__
//#	elif __linux
//#  		define __ISLINUX__
//#	endif

	int nX=2; // Number of components

#	if defined(__ISWINDOWS__)
		char thepathChar[255] = "c:\\Program Files (x86)\\Refprop";
		char theSepChar[2]    = "\\";
#	elif defined(__ISLINUX__)
		char thepathChar[255] = "/opt/refprop";
		char theSepChar[2]    = "/";
#	endif

	// Allocate space for objects
	x     = (double*) calloc(nX,sizeof(double));
	props = (double*) calloc(16+2*nX,sizeof(double));
	ders  = (double*) calloc(21,sizeof(double));
	trns  = (double*) calloc(3,sizeof(double));

	char fluidname[255] = "ammoniaL|water";
	x[0] = 0.2;
	x[1] = 1.0 - x[0];

	double p  = 50e5;
    double h  = 3e5;
	double dh = 25e5;

	int N     = 75; // steps in enthalpy
	int M     = 15; // repetitions

	double res  = 0.0;
	double h_in = 0.0;

	// Format the output properly
	int decimals = 4;
	int width    = 10;
	char buffer [50];


	std::ostringstream out;

	// Get the time, calculate and get the time again
	time_t tstart, tend;
	tstart = time(0);
	for (int i = 0; i < M; i++) {
		for (int count = 0; count < N; count++) {
			h_in = h + dh * count / (N - 1);
			res = props_REFPROP((char*)"u", (char*)"ph", fluidname, ders, trns, props, p, h_in, x, 0, thepathChar, errormsg, DEBUG);
			sprintf (buffer, "d = %8.4f [kg/m3]",props[4]);
			out << buffer << std::endl;
			sprintf (buffer, "T = %8.4f [K]",props[2]);
			out << buffer << std::endl << std::endl;
		}
	}
	tend = time(0);

	if (N*M<10) {
		printf("%s", out.str().c_str());
	} else {
		printf("\nThere were %d calls to the wrapper. \n", N*M);
	}
	int deltat = difftime(tend, tstart);
	printf("It took %d second(s) with an average time per call of %8.4f ms.\n",deltat,deltat*1e3/N/M);


	// Do the same for the unwrapped functions

//	char hf[refpropcharlength*ncmax], hrf[lengthofreference+1],
//	      herr[errormessagelength+1],hfmix[refpropcharlength+1];
//
//	double xliq[ncmax],xvap[ncmax],f[ncmax];
//	double wm,ttp,tnbp,tc,pc,dc,zc,acf,dip,rgas;
//	double t=100.0;
//	double dl,dv;
//	long j=1 ,lerr = 0, i=0, ierr=0;
//	double d=0.0,q=0.0,e=0.0,s=0.0,cv=0.0,cp=0.0,w=0.0,b=0.0,c=0.0,
//		  dpdrho,d2pdd2,dpdt,dhdt_d,dhdt_p,dhdp_t,dhdp_d,
//		  sigma,dhdd_t,dhdd_p,eta,tcx,pp,tt,hjt,h1,dd;
//
//	strcpy(hf,thepathChar);
//	strcat(hf,theSepChar);
//	strcat(hf,"ammoniaL.fld|");
//	strcat(hf,thepathChar);
//	strcat(hf,theSepChar);
//	strcat(hf,"water.fld");
//
//	strcpy(hfmix,thepathChar);
//	strcat(hfmix,theSepChar);
//	strcat(hfmix,"fluids");
//	strcat(hfmix,theSepChar);
//	strcat(hfmix,"hmx.bnc");
//
//	strcpy(hrf,"DEF");
//	strcpy(herr,"Ok");
//
//
////	SETUPdll_POINTER SETUPdll;
////	WMOLdll_POINTER WMOLdll;
////	PHFLSHdll_POINTER  PHFLSHdll;
//
//	//...Call SETUP to initialize the program
//	//long ,char*,char*,char*,long ,char*,long ,long ,long ,long
//	SETUPdll(i, hf, hfmix, hrf, ierr, herr);//,
//				//refpropcharlength*ncmax,refpropcharlength,
//				//lengthofreference,errormessagelength);
//
//	if (ierr != 0) printf("%s\n",herr);
//
//	WMOLdll(x,wm);
//	printf("WM, %10.4f\n",wm);
//
//	double hmol  = h*wm/1000.0;
//	double dhmol = dh*wm/1000.0;
//	double pkpa  = p/1000.0;
//
//	//printf("hmol, pkpa %10.4f, %10.4f\n",hmol,pkpa);
//
//	//PHFLSHdll (&pkpa,&hmol,x,&tt,&dd,&dl,&dv,xliq,xvap,&q,&e,&s,&cv,&cp,&w,&ierr,herr,errormessagelength);
//    //printf("T, D, Pkpa, hmol from PHFLSH   %10.4f,%10.4f,%10.4f,%10.4f\n",tt,dd,pkpa,hmol);
//
//
//
//
//	tstart = time(0);
//	for (int i = 0; i < M; i++) {
//		for (int count = 0; count < N; count++) {
//			h_in = hmol + dhmol * count / (N - 1);
//			PHFLSHdll(pkpa,h_in,x,tt,dd,dl,dv,xliq,xvap,q,e,s,cv,cp,w,ierr,herr,errormessagelength);
//			sprintf (buffer, "d = %8.4f [kg/m3]",dd);
//			out << buffer << std::endl;
//			sprintf (buffer, "T = %8.4f [K]",tt);
//			out << buffer << std::endl << std::endl;
//		}
//	}
//	tend = time(0);
//
//	if (N*M<10) {
//		printf("%s", out.str().c_str());
//	} else {
//		printf("\nThere were %d calls to REFPROP. \n", N*M);
//	}
//
//	deltat = difftime(tend, tstart);
//	printf("It took %d second(s) with an average time per call of %8.4f ms.\n",deltat,deltat*1e3/N/M);

	printf("%s\n"," ");
	return 0;
}
