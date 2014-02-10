#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <sstream>
#include <math.h>
#include <ctime>
#include "cpp_wrapped2.h"
//#include "../src/refprop_library.h"
#include <windows.h>


int main(int argc, char* argv[]){

	// Allocate space for objects
	int nX=2; // Number of components
	double* x;
	x     = (double*) calloc(nX,sizeof(double));
	x[0] = 0.25;
	x[1] = 1.0 - x[0];

	double href[ncmax];
	href[0] = 8295.6883966232;
	href[1] = 14873.1879732343;


	char thepathChar[255] = "c:\\Program Files (x86)\\Refprop";
	char theSepChar[2]    = "\\";
	char hf[refpropcharlength*ncmax], hrf[lengthofreference+1],
	      herr[errormessagelength+1],hfmix[refpropcharlength+1];
	long i=0, ierr=0;

    HINSTANCE RefpropdllInstance;
    RefpropdllInstance = LoadLibrary("C:\\Program Files (x86)\\REFPROP\\REFPROP.dll");

    TDFLSHdll = (TDFLSHdll_POINTER) GetProcAddress(RefpropdllInstance,"TDFLSHdll");
    PHFLSHdll = (PHFLSHdll_POINTER) GetProcAddress(RefpropdllInstance,"PHFLSHdll");
	SETUPdll = (SETUPdll_POINTER) GetProcAddress(RefpropdllInstance,"SETUPdll");
	WMOLdll = (WMOLdll_POINTER) GetProcAddress(RefpropdllInstance,"WMOLdll");
	XMOLEdll = (XMOLEdll_POINTER) GetProcAddress(RefpropdllInstance,"XMOLEdll");

	RMIX2dll = (RMIX2dll_POINTER) GetProcAddress(RefpropdllInstance,"RMIX2dll");
	RDXHMXdll = (RDXHMXdll_POINTER) GetProcAddress(RefpropdllInstance,"RDXHMXdll");
	PHIXdll = (PHIXdll_POINTER) GetProcAddress(RefpropdllInstance,"PHIXdll");
	PHI0dll = (PHI0dll_POINTER) GetProcAddress(RefpropdllInstance,"PHI0dll");


	PRESSdll = (PRESSdll_POINTER) GetProcAddress(RefpropdllInstance,"PRESSdll");
	ENTHALdll = (ENTHALdll_POINTER) GetProcAddress(RefpropdllInstance,"ENTHALdll");
	CRITPdll = (CRITPdll_POINTER) GetProcAddress(RefpropdllInstance,"CRITPdll");
	XMASSdll = (XMASSdll_POINTER) GetProcAddress(RefpropdllInstance,"XMASSdll");



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

	strcpy(hfmix,thepathChar);
	strcat(hfmix,theSepChar);
	strcat(hfmix,"fluids");
	strcat(hfmix,theSepChar);
	strcat(hfmix,"HMX.BNC");

	strcpy(hrf,"DEF");
	strcpy(herr,"Ok");

	i = nX;

 	//...Call SETUP to initialize the program
	SETUPdll(i, hf, hfmix, hrf, ierr, herr,refpropcharlength*ncmax,refpropcharlength,lengthofreference,errormessagelength);
	printf("ierr = %ld\n",ierr);

	double wm,xkg[ncmax];
//	WMOLdll(x,wm);
	XMASSdll(x,xkg,wm);
	printf("mass fraction [0] = %2.10f\n",xkg[0]);
	printf("mass fraction [1] = %2.10f\n",xkg[1]);
	printf("molecular weight = %2.10f\n\n",wm);
// here derivatives start

	int n = 2; // derivative wrt component n
	double xn[ncmax],xp[ncmax];
    double delx = 0.00001;
    double delx2 = 2 * delx;

    for (i = 0; i < nX; i++)  {
       xn[i] = x[i];
       xp[i] = x[i];
    }
    xn[n-1] = xn[n-1] - delx;
    xp[n-1] = xp[n-1] + delx;
    double t = 250;
    double d = 15;

	 long im1=-1;
	 long i0=0;
	 long ip1=1;
	 double R, t0, d0, phi01, phi10, phig10, tau, delta;
	 double pn,pp,dpdxi1,hn,hp,dhdxi1,hrn,hrp,dhrdxi1;
     double dpdxi2;
     double dhdxi2;

	 double hrefdiff;

 // test pressure equation..
     PRESSdll(t,d,x,pn);
     printf("PRESSdll, p = %f\n",pn);

	 RMIX2dll(x, R);
	 RDXHMXdll(&im1,&i0,&i0,x,t0,d0,ierr,herr,errormessagelength);
	 tau = t0 / t;
	 delta =d / d0;
	 PHIXdll(&i0,&ip1, tau, delta, x,phi01);
	 pn = d * R * t * (1 + phi01);
	 printf("Definition, p = %f\n",pn);

 // test enthalpy equation..
	 ENTHALdll(t,d,x,hn);
	 printf("ENTHALdll, h = %f\n",hn);

	 RMIX2dll(x, R);
	 RDXHMXdll(&im1,&i0,&i0,x,t0,d0,ierr,herr,errormessagelength);
	 tau = t0 / t;
	 delta =d / d0;
	 PHIXdll(&i0,&ip1, tau, delta, x,phi01);
	 PHIXdll(&ip1,&i0, tau, delta, x,phi10);
     PHI0dll(&ip1, &i0, t, d, x, phig10);
     hrefdiff = hn;
     hn = R * t * (1 + phig10 + phi10 + phi01);
     hrefdiff = hrefdiff - hn;
	 printf("Definition, h = %f\n",hn);
	 printf("hrefdiff = %5.10f\n",hrefdiff);

 // new test with new computed enthalpy reference from above at another temperature..
	 t=t+50;
	 ENTHALdll(t,d,x,hn);
	 printf("ENTHALdll, h = %f\n",hn);
	 RMIX2dll(x, R);
	 RDXHMXdll(&im1,&i0,&i0,x,t0,d0,ierr,herr,errormessagelength);
	 tau = t0 / t;
	 delta =d / d0;
	 PHIXdll(&i0,&ip1, tau, delta, x,phi01);
	 PHIXdll(&ip1,&i0, tau, delta, x,phi10);
     PHI0dll(&ip1, &i0, t, d, x, phig10);
     hn = R * t * (1 + phig10 + phi10 + phi01) + x[0]*href[0] + x[1]*href[1];
     printf("Definition, h = %f\n",hn);

     t=250; // set back tempereture
     d=15;



     printf("\n definitions work, proceed with numerical derivatives\n\n");

// PERFORM DERIVATIVES...


 	// negative increment
	 RMIX2dll(x, R); // R must be constant for some reason?
	 RDXHMXdll(&im1,&i0,&i0,xn,t0,d0,ierr,herr,errormessagelength);
	 tau = t0 / t;
	 delta =d / d0;
	 PHIXdll(&i0,&ip1, tau, delta, xn, phi01);
	 PHIXdll(&ip1,&i0, tau, delta, xn, phi10);
     PHI0dll(&ip1, &i0, t, d, xn, phig10);
	 pn = d * R * t * (1 + phi01);
	 hrn = R * t * (1 + phi10 + phi01);
     hn = R * t * (1 + phig10 + phi10 + phi01) + xn[0]*href[0] + xn[1]*href[1];

    // positive increment
     RMIX2dll(x, R); // R must be constant for some reason?
     RDXHMXdll(&im1,&i0,&i0,xp,t0,d0,ierr,herr,errormessagelength);
	 tau = t0 / t;
	 delta =d / d0;
	 PHIXdll(&i0,&ip1, tau, delta, xp, phi01);
	 PHIXdll(&ip1,&i0, tau, delta, xp, phi10);
     PHI0dll(&ip1, &i0, t, d, xp, phig10);
	 pp = d * R * t * (1 + phi01);
     hrp = R * t * (1 + phi10 + phi01);
     hp = R * t * (1 + phig10 + phi10 + phi01) + xp[0]*href[0] + xp[1]*href[1];

    // derivatives..
     dpdxi2 = (pp - pn) / delx2;
//     dhrdxi2 = (hrp - hrn) / delx2;
     dhdxi2 = (hp - hn) / delx2;
     printf("dpdx2 = %f\n",dpdxi2);
//     printf("dhrdxi = %f\n",dhrdxi2);
     printf("dhdx2 = %f\n",dhdxi2);



 	n = 1; // derivative wrt component n
 	for (i = 0; i < nX; i++)  {
        xn[i] = x[i];
        xp[i] = x[i];
    }
 	xn[n-1] = xn[n-1] - delx;
    xp[n-1] = xp[n-1] + delx;

	// negative increment
	 RMIX2dll(x, R); // R must be constant for some reason?
	 RDXHMXdll(&im1,&i0,&i0,xn,t0,d0,ierr,herr,errormessagelength);
	 tau = t0 / t;
	 delta =d / d0;
	 PHIXdll(&i0,&ip1, tau, delta, xn, phi01);
	 PHIXdll(&ip1,&i0, tau, delta, xn, phi10);
     PHI0dll(&ip1, &i0, t, d, xn, phig10);
	 pn = d * R * t * (1 + phi01);
	 hrn = R * t * (1 + phi10 + phi01);
     hn = R * t * (1 + phig10 + phi10 + phi01)  + xn[0]*href[0] + xn[1]*href[1];;

    // positive increment
     RMIX2dll(x, R); // R must be constant for some reason?
	 RDXHMXdll(&im1,&i0,&i0,xp,t0,d0,ierr,herr,errormessagelength);
	 tau = t0 / t;
	 delta =d / d0;
	 PHIXdll(&i0,&ip1, tau, delta, xp, phi01);
	 PHIXdll(&ip1,&i0, tau, delta, xp, phi10);
     PHI0dll(&ip1, &i0, t, d, xp, phig10);
	 pp = d * R * t * (1 + phi01);
     hrp = R * t * (1 + phi10 + phi01);
     hp = R * t * (1 + phig10 + phi10 + phi01)  + xp[0]*href[0] + xp[1]*href[1];;

    // derivatives..
     dpdxi1 = (pp - pn) / delx2;
//     dhrdxi1 = (hrp - hrn) / delx2;
     dhdxi1= (hp - hn) / delx2;
     printf("dpdx1 = %f\n",dpdxi1);
     //printf("dhrdxi = %f\n",dhrdxi1);
     printf("dhdx1 = %f\n",dhdxi1);

     printf("dpdx1-dpdx2 = %f\n",dpdxi1-dpdxi2);
     printf("dhdx1-dhdx2 = %f\n",dhdxi1-dhdxi2);


     //	'  calculate d(p)/d(xi) and d(h)/d(xi) at constant T and V and
     	  //	'  The following do not work because R in d*R*T is changed as a function of x
          double xnn[ncmax],xpp[ncmax];
          double dpdxi12,dhdxi12;

          xpp[0] = x[0]+delx;
          xpp[1] = x[1]-delx;
          xnn[0] = x[0]-delx;
          xnn[1] = x[1]+delx;

          PRESSdll(t,d,xnn,pn);
          PRESSdll(t,d,xpp,pp);
          ENTHALdll(t,d,xnn,hn);
          ENTHALdll(t,d,xpp,hp);
          dpdxi12 = (pp - pn) / delx2;
          printf("dpdx1-dpdx2 using PRESSdll = %f\n",dpdxi12);
          dhdxi12 = (hp - hn) / delx2;
          printf("dhdx1-dhdx2 using ENTHALdll = %f\n",dhdxi12);


//
//
//

     printf("\n");
     printf(" lets try mass basis, i.e. compute xmol from xkg +/- increments\n");
     printf(" NOTE that when we want to keep density on mass basis constant, then the molar density mol/L must be corrected when computing +/- increments\n");
     printf("      This is because when concentration (mass or mole) changes, so does molecular weight \n");
     printf("      (i.e. either moler density or mass density must change when keeping the other constant)\n");
     printf("\n");
     //	'  calculate d(p)/d(xi) and d(h)/d(xi) at constant T and V and
    	  //	'  The following do not work because R in d*R*T is changed as a function of x
    double xkgn[ncmax],xkgp[ncmax],wmn,wmp;

	XMASSdll(x,xkg,wm);
	XMASSdll(x,xkgn,wm);
	XMASSdll(x,xkgp,wm);

	xkgp[0] = xkg[0]+delx;
	xkgp[1] = xkg[1]-delx;
	xkgn[0] = xkg[0]-delx;
    xkgn[1] = xkg[1]+delx;
    XMOLEdll(xkgn,xnn,wmn);
    XMOLEdll(xkgp,xpp,wmp);

    // diff dxdz numerically
    double dxdz_num = (xkgp[0]-xkgn[0])/(xpp[0]-xnn[0]);
    printf("dxdz_num = %f\n",dxdz_num);

    printf("dpdx_num = %f\n",(dpdxi1-dpdxi2)/dxdz_num);





/*
    printf("wmn = %f\n",wmn);
    printf("wmp = %f\n",wmp);

    printf("xkgp[0] = %f\n",xkgp[0]);
    printf("xkgp[1] = %f\n",xkgp[1]);
    printf("xkgn[0] = %f\n",xkgn[0]);
    printf("xkgn[1] = %f\n",xkgn[1]);

    printf("xpp[0] = %f\n",xpp[0]);
    printf("xpp[1] = %f\n",xpp[1]);
    printf("xnn[0] = %f\n",xnn[0]);
    printf("xnn[1] = %f\n",xnn[1]);
*/

    //double dpdxi12;
    double dn=d;//*wm/wmn; // mass density should be kept constant even through we change concentration..
    PRESSdll(t,dn,xnn,pn);
    double dp=d;//*wm/wmp; // mass density should be kept constant even through we change concentration..
    PRESSdll(t,dp,xpp,pp);

    dpdxi12 = (pp - pn) / (delx2);
//    printf("pn = %f\n", pn);
//    printf("pp = %f\n", pp);
    printf("dpdx1 - dpdx2 using PRESSdll = %f\n",dpdxi12);

// try do the conversion your self...
    double wm1;
    xkgp[0] = 1;
    xkgp[1] = 0;
    WMOLdll(xkgp,wm1);
    printf("wm1 = %f\n",wm1);
    double wm2;
    xkgp[0] = 0;
    xkgp[1] = 1;
    WMOLdll(xkgp,wm2);
    printf("wm2 = %f\n",wm2);
    printf("wm = %f\n",wm);

//    printf("ratio %f\n",dpdxi12/(dpdxi1-dpdxi2));

    /*
    printf("x[0] = %f\n",x[0]);
    printf("x[1] = %f\n",x[1]);
    printf("xkg[0] = %f\n",xkg[0]);
    printf("xkg[1] = %f\n",xkg[1]);
    printf("dpdxi1 = %f\n",dpdxi1);
    printf("dpdxi2 = %f\n",dpdxi2);

    */

    double dxdz1,dxdz2,dzdx1,dzdx2;
    dxdz1=wm1*x[1]*wm2/(wm*wm);	// this one from maple
    dzdx1=wm1*wm2*xkg[1]/((xkg[0]*wm2+xkg[1]*wm1)*(xkg[0]*wm2+xkg[1]*wm1));
    printf("dxdz1 = %f\n",dxdz1);
    printf("1/dzdx1 = %f\n",1/dzdx1);
    dxdz2=wm1*x[0]*wm2/(wm*wm); // this one from maple
    dzdx2=wm1*wm2*xkg[0]/((xkg[0]*wm2+xkg[1]*wm1)*(xkg[0]*wm2+xkg[1]*wm1));
    printf("dxdz2 = %f\n",dxdz2);
    printf("1/dzdx2 = %f\n",1/dzdx2);
    printf("dpdx = %f\n",dpdxi1/dzdx1-dpdxi2/dzdx2);
    printf("dpdx = %f\n",dpdxi1/dxdz1-dpdxi2/dxdz2);
    printf("dpdxhmm = %f\n",(dpdxi1-dpdxi2)/(dxdz1-dxdz2)/2);

    dxdz1 = wm1/wm - x[0]*wm1*(wm1-wm2)/(wm*wm);
    dxdz2 = wm2/wm - x[1]*wm2*(wm2-wm1)/(wm*wm);
    printf("dxdz1 = %f\n",dxdz1);
    printf("dxdz2 = %f\n",dxdz2);

    printf("dpdx = %f\n",dpdxi1*dxdz1-dpdxi2*dxdz2);
    printf("dpdx = %f\n",dpdxi1/dxdz1-dpdxi2/dxdz2);
//
    printf("\n");
//
//    printf("dpdx = %f\n",dpdxi1*wm1/wm-dpdxi2*wm2/wm);
//    printf("dpdx = %f\n",dpdxi1*wm/wm1-dpdxi2*wm/wm2);




// printf("\n");
// printf(" I expect that dpdxi1 and dpdxi2 have some small error when transforming to mass basis, due to that molecular weight is changed when performing these numerical derivatives");


   double p;
   double h;
   PRESSdll(t,d,x,p);
   ENTHALdll(t,d,x,h);






/////////////////////////////////////////////////////////
/*
    printf("\n try something else \n\n");


    n=2;
    for (i = 0; i < nX; i++)  {
       xkgn[i] = xkg[i];
       xkgp[i] = xkg[i];
    }
    xkgn[n-1] = xkgn[n-1] - delx;
    xkgp[n-1] = xkgp[n-1] + delx;


    xnn[0] = xkgn[0] *wm/wm1;
    xnn[1] = xkgn[1] *wm/wm2;
    xpp[0] = xkgp[0] *wm/wm1;
    xpp[1] = xkgp[1] *wm/wm2;


    printf("xkgp[0] = %f\n",xkgp[0]);
    printf("xkgp[1] = %f\n",xkgp[1]);
    printf("xkgn[0] = %f\n",xkgn[0]);
    printf("xkgn[1] = %f\n",xkgn[1]);

    printf("xpp[0] = %f\n",xpp[0]);
    printf("xpp[1] = %f\n",xpp[1]);
    printf("xnn[0] = %f\n",xnn[0]);
    printf("xnn[1] = %f\n",xnn[1]);

    //    XMOLEdll(xkgn,xnn,wmn);
    //    XMOLEdll(xkgp,xpp,wmp);

	 RMIX2dll(x, R);
	 double Rkg=R/wm;
	 double dkg=d*wm;


	 RDXHMXdll(&im1,&i0,&i0,xnn,t0,d0,ierr,herr,errormessagelength);
	 tau = t0 / t;
	 delta =dkg/wmn / d0;
	 PHIXdll(&i0,&ip1, tau, delta, xnn, phi01);
	 pn = dkg * Rkg * t * (1 + phi01);

    // positive increment
     //RMIX2dll(x, R); // R must be constant for some reason?
     RDXHMXdll(&im1,&i0,&i0,xpp,t0,d0,ierr,herr,errormessagelength);
	 tau = t0 / t;
	 delta =dkg/wmp / d0;
	 PHIXdll(&i0,&ip1, tau, delta, xpp, phi01);
	 pp = dkg * Rkg * t * (1 + phi01);

    // derivatives..
     dpdxi2 = (pp - pn) / delx2;
     printf("dpdx2 = %f\n",dpdxi2);

     n=1;
   for (i = 0; i < nX; i++)  {
        xkgn[i] = xkg[i];
        xkgp[i] = xkg[i];
     }
     xkgn[n-1] = xkgn[n-1] - delx;
     xkgp[n-1] = xkgp[n-1] + delx;
 //    double t = 250;
  //   XMOLEdll(xkgn,xnn,wmn);
  //   XMOLEdll(xkgp,xpp,wmp);
     xnn[0] = xkgn[0] *wm/wm1;
     xnn[1] = xkgn[1] *wm/wm2;
     xpp[0] = xkgp[0] *wm/wm1;
     xpp[1] = xkgp[1] *wm/wm2;

	// negative increment
	 //RMIX2dll(x, R); // R must be constant for some reason?
	 RDXHMXdll(&im1,&i0,&i0,xnn,t0,d0,ierr,herr,errormessagelength);
	 tau = t0 / t;
	 delta = dkg/wmn / d0;
	 PHIXdll(&i0,&ip1, tau, delta, xnn, phi01);
	 pn = dkg * Rkg * t * (1 + phi01);

    // positive increment
    // RMIX2dll(x, R); // R must be constant for some reason?
	 RDXHMXdll(&im1,&i0,&i0,xp,t0,d0,ierr,herr,errormessagelength);
	 tau = t0 / t;
	 delta =dkg/wmp / d0;
	 PHIXdll(&i0,&ip1, tau, delta, xp, phi01);
	 pp = dkg * Rkg * t * (1 + phi01);

    // derivatives..
     dpdxi1 = (pp - pn) / delx2;

     printf("dpdx1 = %f\n",dpdxi1);

     printf("dpdx1-dpdx2 = %f\n",dpdxi1-dpdxi2);

*/




	return 0;
}
