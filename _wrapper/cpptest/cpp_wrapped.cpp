#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <sstream>
#include <math.h>
#include <ctime>
#include "../src/refprop_library.h"
#include "../src/refprop_wrapper.h"
#include "cpp_wrapped.h"

int main(int argc, char* argv[]){
	char errormsg[255];
	double* x;
	double *props;
	double *ders;
	double *trns;
	int DEBUG = 1;
	int transport = 0;
	int partialDersInputChoice= 3;

	double *satprops;
	double *critprops;
	double *transprops;

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
	props = (double*) calloc(18+2*nX,sizeof(double));
	ders  = (double*) calloc(12,sizeof(double));
	trns  = (double*) calloc(3,sizeof(double));

	satprops = (double*) calloc(12+nX,sizeof(double));

	critprops = (double*) calloc(3+nX,sizeof(double));
	transprops = (double*) calloc(2,sizeof(double));

	char fluidname[255] = "ammoniaL|water";
	x[0] = 0.5;
	x[1] = 1.0 - x[0];

	double p  = 50.e5;
    double h  = 3.0e5;
	double dh = 1500e3;//2.5e6;

	int N     = 2; // steps in enthalpy
	int M     =   1; // repetitions

	double res  = 0.0;
	double h_in = 0.0;

	// Format the output properly
	char buffer [100];
	int limit    = 105;

	std::ostringstream out;
	out << std::endl;

	// Get the time, calculate and get the time again
	time_t tstart, tend;
	tstart = time(0);
	for (int i = 0; i < M; i++) {
		for (int count = 0; count < N; count++) {
			h_in = h + dh * count / (N - 1);

			res = props_REFPROP((char*)"u", (char*)"ph", fluidname, ders, trns, props, p, h_in, x, 0, thepathChar, errormsg, DEBUG, transport, partialDersInputChoice);

			if (N*M<limit) {
				sprintf (buffer, "d = %8.4f [kg/m3]",props[4]);
				out << buffer << std::endl;
				sprintf (buffer, "T = %8.4f [K]",props[2]);
				out << buffer << std::endl;

			res = satprops_REFPROP((char*)"p", (char*)"t", fluidname, satprops, props[2], 0,  x, thepathChar, errormsg, DEBUG, transport);

				sprintf (buffer, "Pl(T) = %8.4f [Pa]",satprops[3]);
				out << buffer << std::endl;
				sprintf (buffer, "Pv(T) = %8.4f [Pa]",satprops[4]);
				out << buffer << std::endl;

				res = satprops_REFPROP((char*)"t", (char*)"p", fluidname, satprops, p, 0,  x, thepathChar, errormsg, DEBUG, transport);

				sprintf (buffer, "Tl(p) = %8.4f [K]",satprops[1]);
				out << buffer << std::endl;
				sprintf (buffer, "Tv(p) = %8.4f [K]",satprops[2]);
				out << buffer << std::endl;


				sprintf (buffer, "Ders = %8.0f , %8.12f , %8.1f , %8.1f , %8.12f , %8.8f , %8.1f , %8.2f , %8.2f , %8.9f , %8.9f , %8.9f, %8.1f",ders[0],ders[1],ders[2],ders[3],ders[4],ders[5],ders[6],ders[7],ders[8],ders[18],ders[19],ders[20],ders[6]);
				out << buffer << std::endl << std::endl;
			}
		}
	}
	tend = time(0);

	if (N*M<limit) {
		printf("%s", out.str().c_str());
	} else {
		sprintf(buffer, "\nThere were %d calls to the wrapper.", N*M);
		out << buffer << std::endl;
	}
	int deltat = difftime(tend, tstart);
	sprintf(buffer,"It took %d second(s) with an average time per call of %8.4f ms.",deltat,deltat*1e3/N/M);
	out << buffer << std::endl << std::endl << std::endl;


	// Do the same for the unwrapped functions
	char hf[refpropcharlength*ncmax], hrf[lengthofreference+1],
	      herr[errormessagelength+1],hfmix[refpropcharlength+1];

	double xmol[ncmax],xliq[ncmax],xvap[ncmax],f[ncmax];
	double wm,ttp,tnbp,tc,pc,dc,zc,acf,dip,rgas;
	double t=100.0;
	double dl,dv;
	long j=1 ,lerr = 0, i=0, ierr=0;
	double d=0.0,q=0.0,e=0.0,s=0.0,cv=0.0,cp=0.0,w=0.0,b=0.0,c=0.0,
		  dpdrho,d2pdd2,dpdt,dhdt_d,dhdt_p,dhdp_t,dhdp_d,
		  sigma,dhdd_t,dhdd_p,eta,tcx,pp,tt,hjt,h1,dd;

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

	//...Call SETUP to initialize the program
	//long ,char*,char*,char*,long ,char*,long ,long ,long ,long
	SETUPdll(i, hf, hfmix, hrf, ierr, herr,refpropcharlength*ncmax,refpropcharlength,lengthofreference,errormessagelength);

	if (ierr != 0) printf("%s\n",herr);

	//WMOLdll(x,wm);
	XMOLEdll(x,xmol,wm);

	double hmol  = h*wm/1000.0;
	double dhmol = dh*wm/1000.0;
	double pkpa  = p/1000.0;

	tstart = time(0);
	for (int i = 0; i < M; i++) {
		for (int count = 0; count < N; count++) {
			h_in = hmol + dhmol * count / (N - 1);
			PHFLSHdll(pkpa,h_in,xmol,tt,dd,dl,dv,xliq,xvap,q,e,s,cv,cp,w,ierr,herr,errormessagelength);
		//	if (N*M<limit) {
		//		sprintf (buffer, "d = %8.4f [kg/m3]",dd*wm);
		//		out << buffer << std::endl;
		//		sprintf (buffer, "T = %8.4f [K]",tt);
		//		out << buffer << std::endl << std::endl;
		//	}
		}
	}
	tend = time(0);

	if (N*M<limit) {
		//printf("%s", out.str().c_str());
	} else {
		sprintf(buffer, "\nThere were %d calls to REFPROP.", N*M);
		out << buffer << std::endl;
	}

	deltat = difftime(tend, tstart);
	sprintf(buffer, "It took %d second(s) with an average time per call of %8.4f ms.",deltat,deltat*1e3/N/M);
	out << buffer << std::endl << std::endl << std::endl;

	printf("%s",out.str().c_str());


	x[0]=0;
	x[1]=1;

	res = critprops_REFPROP(fluidname, critprops, x, thepathChar, errormsg, DEBUG);

	printf("%f\n",critprops[0]);
	printf("%f\n",critprops[1]);
	printf("%f\n",critprops[2]);
	printf("%f\n",critprops[3]);
	printf("%f\n",critprops[4]);

	res = critprops_REFPROP(fluidname, critprops, x, thepathChar, errormsg, DEBUG);

	printf("%f\n",critprops[0]);
	printf("%f\n",critprops[1]);
	printf("%f\n",critprops[2]);
	printf("%f\n",critprops[3]);
	printf("%f\n",critprops[4]);

	x[0]=1;
	x[1]=0;

	return 0;

}
