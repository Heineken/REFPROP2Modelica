#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <sstream>
#include <math.h>
#include <ctime>
#include "cpp_wrapped.h"
//#include "../src/refprop_library.h"
#include <windows.h>


int main(int argc, char* argv[]){

	// Allocate space for objects
	int nX=2; // Number of components
	double* x;
	x     = (double*) calloc(nX,sizeof(double));

	char thepathChar[255] = "c:\\Program Files (x86)\\Refprop";
	char theSepChar[2]    = "\\";
	x[0] = 0.5;
	x[1] = 1.0 - x[0];
	double p  = 50.e5;
    double h  = 3.0e5;

	char hf[refpropcharlength*ncmax], hrf[lengthofreference+1],
	      herr[errormessagelength+1],hfmix[refpropcharlength+1];
	double xmol[ncmax],xliq[ncmax],xvap[ncmax];
	double wm;
	long i=0, ierr=0;
	double q,e,s,cv,cp,w,tt,dd,dl,dv;

	strcpy(hf,thepathChar);
	strcat(hf,theSepChar);
	strcat(hf,"fluids");
	strcat(hf,theSepChar);
	strcat(hf,"ammoniaL.fld|");
	strcat(hf,thepathChar);
	strcat(hf,theSepChar);
	strcat(hf,"fluids");
	strcat(hf,theSepChar);
	strcat(hf,"water.fld");

	strcpy(hfmix,thepathChar);
	strcat(hfmix,theSepChar);
	strcat(hfmix,"fluids");
	strcat(hfmix,theSepChar);
	strcat(hfmix,"HMX.BNC");

	strcpy(hrf,"DEF");
	strcpy(herr,"Ok");

	i = nX;

    HINSTANCE RefpropdllInstance;
    RefpropdllInstance = LoadLibrary("C:\\Program Files (x86)\\REFPROP\\refprop.dll");

    TDFLSHdll = (TDFLSHdll_POINTER) GetProcAddress(RefpropdllInstance,"TDFLSHdll");
    PHFLSHdll = (PHFLSHdll_POINTER) GetProcAddress(RefpropdllInstance,"PHFLSHdll");
	SETUPdll = (SETUPdll_POINTER) GetProcAddress(RefpropdllInstance,"SETUPdll");
	WMOLdll = (WMOLdll_POINTER) GetProcAddress(RefpropdllInstance,"WMOLdll");
	XMOLEdll = (XMOLEdll_POINTER) GetProcAddress(RefpropdllInstance,"XMOLEdll");



	/*
 	//...Call SETUP to initialize the program
	//long ,char*,char*,char*,long ,char*,long ,long ,long ,long
	SETUPdll(i, hf, hfmix, hrf, ierr, herr,refpropcharlength*ncmax,refpropcharlength,lengthofreference,errormessagelength);

	if (ierr != 0) printf("%s\n",herr);
	//WMOLdll(x,wm);
	XMOLEdll(x,xmol,wm);

	double hmol  = h*wm/1000.0;
	double pkpa  = p/1000.0;

	PHFLSHdll(pkpa,hmol,xmol,tt,dd,dl,dv,xliq,xvap,q,e,s,cv,cp,w,ierr,herr,errormessagelength);

    printf("T = %f\n",tt);
    printf("D = %f\n",dd);
    printf("xmole = %f,%f\n",xmol[0],xmol[1]);

    double pnew,hnew;
	TDFLSHdll(tt,dd,xmol,pnew,dl,dv,xliq,xvap,q,e,hnew,s,cv,cp,w,ierr,herr,errormessagelength);
//			 ( t, D, z,   p,  Dl,Dv, x,   y,  q,e,h,s,cv,cp,w,ierr,herr)
    printf("p = %f\n",pnew);
    printf("h = %f\n",hnew/wm*1000);

    */


    // new fluid

	strcpy(hf,thepathChar);
	strcat(hf,theSepChar);
	strcat(hf,"fluids");
	strcat(hf,theSepChar);
	strcat(hf,"methane.fld|");
	strcat(hf,thepathChar);
	strcat(hf,theSepChar);
	strcat(hf,"fluids");
	strcat(hf,theSepChar);
	strcat(hf,"ethane.fld");

 	//...Call SETUP to initialize the program
	//long ,char*,char*,char*,long ,char*,long ,long ,long ,long
	SETUPdll(i, hf, hfmix, hrf, ierr, herr,refpropcharlength*ncmax,refpropcharlength,lengthofreference,errormessagelength);
	printf("ierr = %ld\n",ierr);


    xmol[0]=0.5;
    xmol[1]=0.5;
    tt=250;
    dd=15; //mol/dm^3

    double h1,h2,p1,p2,dpdx1_tv,dhdx1_tv;
    double xmolnew[ncmax];
    double dx=1e-7;
    xmolnew[0]=0.5+dx;
    xmolnew[1]=0.5;

    TDFLSHdll(tt,dd,xmol,p1,dl,dv,xliq,xvap,q,e,h1,s,cv,cp,w,ierr,herr,errormessagelength);
	printf("ierr = %ld\n",ierr);
    printf("p1 = %f\n",p1);
    printf("h1 = %f\n",h1);
    printf("xmole1 = %1.10f,%1.10f\n",xmol[0],xmol[1]);

    TDFLSHdll(tt,dd,xmolnew,p2,dl,dv,xliq,xvap,q,e,h2,s,cv,cp,w,ierr,herr,errormessagelength);
	printf("ierr = %ld\n",ierr);
    printf("p2 = %f\n",p2);
    printf("h2 = %f\n",h2);
    printf("xmole2 = %1.10f,%1.10f\n",xmolnew[0],xmolnew[1]);

    dpdx1_tv = (p2-p1)/dx;
    dhdx1_tv = (h2-h1)/dx;
    printf("dpdx1_tv = %f\n",dpdx1_tv);
    printf("dhdx1_tv = %f\n",dhdx1_tv);

    double dpdx2_tv,dhdx2_tv;
    xmolnew[0]=0.5;
    xmolnew[1]=0.5+dx;

    TDFLSHdll(tt,dd,xmol,p1,dl,dv,xliq,xvap,q,e,h1,s,cv,cp,w,ierr,herr,errormessagelength);
	printf("ierr = %ld\n",ierr);
    printf("p1 = %f\n",p1);
    printf("h1 = %f\n",h1);
    printf("xmole1 = %1.10f,%1.10f\n",xmol[0],xmol[1]);

    TDFLSHdll(tt,dd,xmolnew,p2,dl,dv,xliq,xvap,q,e,h2,s,cv,cp,w,ierr,herr,errormessagelength);
	printf("ierr = %ld\n",ierr);
    printf("p2 = %f\n",p2);
    printf("h2 = %f\n",h2);
    printf("xmole2 = %1.10f,%1.10f\n",xmolnew[0],xmolnew[1]);

    dpdx2_tv = (p2-p1)/dx;
    dhdx2_tv = (h2-h1)/dx;
    printf("dpdx2_tv = %f\n",dpdx2_tv);
    printf("dhdx2_tv = %f\n",dhdx2_tv);

	return 0;
}
