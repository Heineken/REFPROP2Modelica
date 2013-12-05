// EX_C2.c written by:
// Ian Bell
// Herrick Labs
// Purdue University
// ibell@purdue.edu
// August 31, 2010

// heavily based on example file EX_C1.cpp by

// Chris Muzny
// N.I.S.T.
// Chemical Science and Technology Laboratory
// Physical and Chemical Properties of Fluids Division
// (303) 497-5549 
// chris.muzny@nist.gov

//This program demonstrates explicitly linking the subroutines available in
// refprop.dll.  In order to link this code refprop1.h 
// must be available in the current directory.  When executing refprop.dll must be in the dll
// search path (current directory and $PATH).

#if defined(__ISWINDOWS__)
#include <windows.h>
#elif defined(__ISLINUX__)
#include <dlfcn.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "refprop_c.h"


// Some constants...
#define refpropcharlength 255
#define filepathlength 255
#define lengthofreference 3
#define errormessagelength 255
#define ncmax 20		// Note: ncmax is the max number of components
#define numparams 72 
#define maxcoefs 50

int main(int argc, char* argv[])
{
    int count;
	clock_t myt;
	
	double dh;
	int    N=500;
	double* T_save;
	double* h_in;

	char *FLD_PATH;  

	double x[ncmax],xliq[ncmax],xvap[ncmax],f[ncmax],xmol[ncmax];

    long i,ierr=0;
    char hf[refpropcharlength*ncmax], hrf[lengthofreference+1],
      herr[errormessagelength+1],hfmix[refpropcharlength+1];

	double wm,ttp,tnbp,tc,pc,dc,zc,acf,dip,rgas;
	long info_index=1;
	double t=100.0;
	double p,dl,dv;
	long j=1;
	  double d=0.0,q=0.0,e=0.0,h=0.0,s=0.0,cv=0.0,cp=0.0,w=0.0,b=0.0,c=0.0,
		  dpdrho,d2pdd2,dpdt,dhdt_d,dhdt_p,dhdp_t,dhdp_d,
		  sigma,dhdd_t,dhdd_p,eta,tcx,pp,tt,hjt,h1,dd;
	  long tmp_int=0;
	  long kr=1;

	//xkg = (double*) calloc(ncmax,sizeof(double));

	//double xkg[ncmax];
	//double wmix;
	double hmol,pkpa;


#if defined(__ISWINDOWS__)
	RefpropdllInstance = LoadLibrary("C:\\Program Files (x86)\\REFPROP\\refprop.dll");
	PHFLSHdll = (fp_PHFLSHdllTYPE) GetProcAddress(RefpropdllInstance,"PHFLSHdll");
	SETUPdll  = (fp_SETUPdllTYPE)  GetProcAddress(RefpropdllInstance,"SETUPdll");
	XMOLEdll  = (fp_XMOLEdllTYPE)  GetProcAddress(RefpropdllInstance,"XMOLEdll");
#elif defined(__ISLINUX__)
	RefpropdllInstance = dlopen("librefprop.so", RTLD_LAZY);
	PHFLSHdll = (fp_PHFLSHdllTYPE) dlsym(RefpropdllInstance, "PHFLSHdll");
	SETUPdll  = (fp_SETUPdllTYPE)  dlsym(RefpropdllInstance, "SETUPdll");
	XMOLEdll  = (fp_XMOLEdllTYPE)  dlsym(RefpropdllInstance, "XMOLEdll");
#endif



// Now use the functions.

// Refprop variables that need to be defined
//
// nc = Number of components in the mixture
// x[NumberOfComponentsInMixtures] = Mole fraction of each component
// ierr =  An integer flag defining an error
// hf[] = a character array defining the fluids in a mixture
// hrf[] = a character array denoting the reference state
// herr[] = a character array for storing a string - Error message
// hfmix[] a character array defining the path to the mixture file
 
	
	//Exlicitely set the fluid file PATH
#	if defined(__ISWINDOWS__)
		FLD_PATH = "C:\\Program Files (x86)\\REFPROP\\fluids";
		char theSepChar[2]    = "\\";
#	elif defined(__ISLINUX__)
		FLD_PATH = "/opt/refprop/fluids";
		char theSepChar[2]    = "/";
#	endif
//	  strcpy(hf,FLD_PATH);
//	  strcpy(hfmix,FLD_PATH);

	  //...initialize the program and set the pure fluid component name
      //i=1;
      //strcpy(hf,"nitrogen.fld");
      //strcpy(hfmix,"hmx.bnc");
      //strcpy(hrf,"DEF");
      //strcpy(herr,"Ok");

//...For a mixture, use the following setup instead of the lines above.
	// Use "|" as the file name delimiter for mixtures
	i=2;
	strcpy(hf,FLD_PATH);
	strcat(hf,theSepChar);
	strcat(hf,"ammoniaL.fld|");
	strcat(hf,FLD_PATH);
	strcat(hf,theSepChar);
	strcat(hf,"water.fld");

	strcpy(hfmix,FLD_PATH);
	strcat(hfmix,theSepChar);
	strcat(hfmix,"HMX.BNC");

	strcpy(hrf,"DEF");
	strcpy(herr,"Ok");

	x[0]=.5;     //Air composition
	x[1]=.5;
	//x[2]=.2096;

	  //...Call SETUP to initialize the program
	SETUPdll(&i, hf, hfmix, hrf, &ierr, herr,
			refpropcharlength*ncmax,refpropcharlength,
			lengthofreference,errormessagelength);
	//long ,char*,char*,char*,long ,char*,long ,long ,long ,long 
	if (ierr != 0) printf("%s\n",herr);

	p=50e5;
	h=3e5;

	//wmix=1;
	//xkg=1;
	//XMOLEdll (x,&xkg,&wmix);
		
	XMOLEdll (x,xmol,&wm);
	//WMOLdll(x,&wm);
	//printf("WM, %10.4f\n",wm);

	hmol = h*wm/1000;
	pkpa = p/1000;

	printf("hmol, pkpa %10.4f, %10.4f\n",hmol,pkpa);

	PHFLSHdll (&pkpa,&hmol,xmol,&tt,&dd,&dl,&dv,xliq,xvap,&q,&e,&s,&cv,&cp,&w,&ierr,herr,errormessagelength);
    printf("T, D, Pkpa, hmol from PHFLSH   %10.4f,%10.4f,%10.4f,%10.4f\n",tt,dd,pkpa,hmol);


	dh=25e5;		
 	T_save = (double*) calloc(N,sizeof(double));
	h_in = (double*) calloc(N,sizeof(double));

	// Get the time, calculate and get the time again
	time_t tstart, tend;
	tstart = time(0);
  
	for (count = 0; count < N; count++)
	{
		h_in[count] = (h+dh*count/(N-1))*wm/1000;
		
		PHFLSHdll (&pkpa,&h_in[count],xmol,&tt,&dd,&dl,&dv,xliq,xvap,&q,&e,&s,&cv,&cp,&w,&ierr,herr,errormessagelength);
		
		T_save[count] = tt;
		
		//printf("T=%f [K]\n",T_save[count]);
	}
	tend = time(0);

	int deltat = difftime(tend, tstart)*1e3;
	printf("Time per call: %8.4f milliseconds\n", deltat/(N*1.0));

/*

	INFOdll(&info_index,&wm,&ttp,&tnbp,&tc,&pc,&dc,&zc,&acf,&dip,&rgas);
	printf("WM,ACF,DIP,TTP,TNBP   %10.4f,%10.4f,%10.4f,%10.4f,%10.4f\n",wm,acf,dip,ttp,tnbp);
	printf("TC,PC,DC,RGAS         %10.4f,%10.4f,%10.4f,%10.4f,%10.4f\n",tc,pc,dc,rgas);
//...Calculate molecular weight of a mixture
//     wm=WMOLdll(x)

//...Get saturation properties given t,x; for i=1: x is liquid phase
//.....                                   for i=2: x is vapor phase

     
	i=1;
      SATTdll(&t,x,&i,&p,&dl,&dv,xliq,xvap,&ierr,herr,errormessagelength);
      printf("P,Dl,Dv,xl[0],xv[0]   %10.4f,%10.4f,%10.4f,%10.4f,%10.4f\n",p,dl,dv,xliq[0],xvap[0]);
      i=2;
      SATTdll(&t,x,&i,&p,&dl,&dv,xliq,xvap,&ierr,herr,errormessagelength);
      printf("P,Dl,Dv,xl[0],xv[0]   %10.4f,%10.4f,%10.4f,%10.4f,%10.4f\n",p,dl,dv,xliq[0],xvap[0]);

//...Calculate saturation properties at a given p. i is same as SATT
      i=2;
      SATPdll(&p,x,&i,&t,&dl,&dv,xliq,xvap,&ierr,herr,errormessagelength);
      printf("T,Dl,Dv,xl(1),xv(1)   %10.4f,%10.4f,%10.4f,%10.4f,%10.4f\n",t,dl,dv,xliq[0],xvap[0]);

//...Other saturation routines are given in SAT_SUB.FOR

      t=300.0;
      p=20000.0;

//...Calculate d from t,p,x
//...If phase is known: (j=1: Liquid, j=2: Vapor)
      
      TPRHOdll(&t,&p,x,&j,&tmp_int,&d,&ierr,herr,errormessagelength);
      printf("T,P,D                 %10.4f,%10.4f,%10.4f\n",t,p,d);

//...If phase is not known, call TPFLSH
//...Calls to TPFLSH are much slower than TPRHO since SATT must be called first.
//.....(If two phase, quality is returned as q)
	  
      TPFLSHdll(&t,&p,x,&d,&dl,&dv,xliq,xvap,&q,&e,&h,&s,&cv,&cp,&w,&ierr,herr,errormessagelength);
      printf("T,P,D,H,CP            %10.4f,%10.4f,%10.4f,%10.4f,%10.4f\n",t,p,d,h,cp);

//...Calculate pressure (p), internal energy (e), enthalpy (h), entropy (s),
//.....isochoric (cv) and isobaric (cp) heat capacities, speed of sound (w),
//.....and Joule-Thomson coefficient (hjt) from t,d,x
//.....(subroutines THERM2 and THERM3 contain more properties, see PROP_SUB.FOR)
      THERMdll(&t,&d,x,&p,&e,&h,&s,&cv,&cp,&w,&hjt);

//...Calculate pressure
      PRESSdll(&t,&d,x,&p);

//...Calculate fugacity
      FGCTYdll(&t,&d,x,f);

//...Calculate second and third virial coefficients
      VIRBdll (&t,x,&b);
      VIRCdll (&t,x,&c);
      printf("F,B,C                 %10.4f,%10.4f,%10.4f\n",f[0],b,c);
//
//...Calculate the derivatives: dP/dD, d^2P/dD^2, dP/dT  (D indicates density)
//...(dD/dP, dD/dT, and dB/dT are also available, see PROP_SUB.FOR)
      DPDDdll (&t,&d,x,&dpdrho);
      DPDD2dll (&t,&d,x,&d2pdd2);
      DPDTdll (&t,&d,x,&dpdt);
      printf("dP/dD,d2P/dD2,dP/dT   %10.4f,%10.4f,%10.4f\n",dpdrho,d2pdd2,dpdt);


//...Calculate derivatives of enthalpy with respect to T, P, and D
      DHD1dll(&t,&d,x,&dhdt_d,&dhdt_p,&dhdd_t,&dhdd_p,&dhdp_t,&dhdp_d);
      printf("Enthalpy derivatives  %10.4f,%10.4f,%10.4f,%10.4f,%10.4f\n",
		       dhdt_d,dhdt_p,dhdd_t,dhdd_p/1000.0,dhdp_t);
//...Calculate surface tension
      SURFTdll (&t,&dl,x,&sigma,&ierr,herr,errormessagelength);
      printf("T,SURF. TN.           %10.4f,%10.4f\n",t,sigma);

//...Calculate viscosity (eta) and thermal conductivity (tcx)
      TRNPRPdll (&t,&d,x,&eta,&tcx,&ierr,herr,errormessagelength);
      printf("VIS.,TH.CND.          %10.4f,%10.4f\n",eta,tcx*1000.0);

//...General property calculation with inputs of t,d,x
      TDFLSHdll (&t,&d,x,&pp,&dl,&dv,xliq,xvap,&q,&e,&h1,&s,&cv,&cp,&w,&ierr,herr,errormessagelength);
      printf("T, D, P from TDFLSH   %10.4f,%10.4f,%10.4f\n",t,d,pp/1000.0);

//...General property calculation with inputs of p,d,x
      PDFLSHdll (&p,&d,x,&tt,&dl,&dv,xliq,xvap,&q,&e,&h1,&s,&cv,&cp,&w,&ierr,herr,errormessagelength);
      printf("T, D, P from PDFLSH   %10.4f,%10.4f,%10.4f\n",tt,d,p/1000.0);

//...General property calculation with inputs of p,h,x
      PHFLSHdll (&p,&h,x,&tt,&dd,&dl,&dv,xliq,xvap,&q,&e,&s,&cv,&cp,&w,&ierr,herr,errormessagelength);
      printf("T, D, P from PHFLSH   %10.4f,%10.4f,%10.4f\n",tt,dd,p/1000.0);

//...General property calculation with inputs of p,s,x
      PSFLSHdll (&p,&s,x,&tt,&dd,&dl,&dv,xliq,xvap,&q,&e,&h1,&cv,&cp,&w,&ierr,herr,errormessagelength);
	  printf("T, D, P from PSFLSH   %10.4f,%10.4f,%10.4f\n",tt,dd,p/1000.0);

//...General property calculation with inputs of d,h,x
      DHFLSHdll (&d,&h,x,&tt,&pp,&dl,&dv,xliq,xvap,&q,&e,&s,&cv,&cp,&w,&ierr,herr,errormessagelength);
      printf("T, D, P from DHFLSH   %10.4f,%10.4f,%10.4f\n",tt,d,pp/1000.0);

//...General property calculation with inputs of t,h,x
//     kr--flag specifying desired root for multi-valued inputs:
//         1=return lower density root
//         2=return higher density root
      
      THFLSHdll (&t,&h,x,
                  &kr,&pp,&dd,&dl,&dv,xliq,xvap,&q,&e,&s,&cv,&cp,&w,&ierr,herr,errormessagelength);
      printf("T, D, P from THFLSH   %10.4f,%10.4f,%10.4f\n",t,dd,pp/1000.0);

//...Other general property calculation routines are given in FLSH_SUB.FOR
//...and FLASH2.FOR

//...Calculate melting pressure
      t=100.0;
      MELTTdll (&t,x,&p,&ierr,herr,errormessagelength);
      printf("Melting pressure(MPa) %10.4f,%10.4f\n",p/1000.0,t);

//...Calculate melting temperature
      MELTPdll (&p,x,&tt,&ierr,herr,errormessagelength);
      printf("Melting temperature(K)%10.4f,%10.4f\n",tt,p/1000.0);

//...Calculate sublimation pressure
      t=200.0;
      SUBLTdll (&t,x,&p,&ierr,herr,errormessagelength);
      printf("Sublimation pr.(kPa)  %10.4f,%10.4f\n",p,t);

//...Calculate sublimation temperature
      SUBLPdll (&p,x,&tt,&ierr,herr,errormessagelength);
      printf("Sublimation temp.(K)  %10.4f,%10.4f\n",tt,p);

//...Get limits of the equations and check if t,d,p is a valid point
//...Equation of state
//     call LIMITK ('EOS',1,t,d,p,tmin,tmax,Dmax,pmax,ierr,herr)
//...Viscosity equation
//     call LIMITK ('ETA',1,t,d,p,tmin,tmax,Dmax,pmax,ierr,herr)
//...Thermal conductivity equation
//     call LIMITK ('TCX',1,t,d,p,tmin,tmax,Dmax,pmax,ierr,herr)

//...Other routines are given in UTILITY.FOR

*/

 return 0;
}
//---------------------------------------------------------------------------
