#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "refprop_wrapper.h"

//double density(char* fluidname_in, double p, double t, double* x, char* REFPROP_PATH);
//double density(char* fluidname_in, double p, double t);
//double density(double p, double t);
//char *str_replace(char *str, char *search, char *replace, long *count);

int main(int argc, char* argv[]){
	double p,t,d;
	char fluidname[255];
	char errormsg[255+1024];
	double* x;
	double *props;
	double sumx;
	int i;
	int nX = argc-5;
	int DEBUG = 1;
	
	if (argc<5){
//		printf("usage: refpropwrappertest.exe statevars fluidname1|fluidname2|... statevar1 statevar2 REFPROPdir massfractionComponent1 \nexample: refpropwrappertest \"pT\" \"isobutan|propane\" 1e5 293 \"d:\\Programme\\REFPROP\\\" .1");
		printf("usage: refpropwrappertest statevars fluidname1|fluidname2|... statevar1 statevar2 REFPROPdir massfractionComponent1 \nexample: refpropwrappertest \"pT\" \"ISOBUTAN|PROPANE\" 1e5 293 \"/home/jowr/Documents/Fluids/Refprop/v9.0/\" .1\n");
		return 1;
	}

	x = (double*) calloc(nX,sizeof(double));
	props=(double*) calloc(16+2*nX,sizeof(double));

	
	sumx = 0;
	for (i=0;i<nX-1;i++){
		x[i] = atof(argv[6+i]);
		sumx += x[i];
	}
	x[nX-1] = 1-sumx;
	
	
	//strcpy(fluidname,argv[1]);//water;
	/*p=atof(argv[2]);//2000;
	t=atof(argv[3]);//300.0;	*/
	//d = density(fluidname, p, t, x, ""); //d:\\Programme\REFPROP\\	
	//d = density(fluidname, p, t);	
	//d = density(p, t);	
	//props_REFPROP("", "pT"   , "isobutan|propane", props, 1e5          , 293.0        , x, 0, "d:\\Programme\\REFPROP\\", errormsg, DEBUGMODE);
	props_REFPROP  ("", argv[1], argv[2]           , props, atof(argv[3]), atof(argv[4]), x, 0, argv[5]                   , errormsg, DEBUG);
	printf("Errormessage: %s\n",errormsg);
	printf("%c,%c,D                 %10.4f,%10.4f,%10.4f\n",argv[1][0],argv[1][1],atof(argv[3]),atof(argv[4]),props[5]);
	/*	props[0] = ierr;//error code
	props[1] = p;//pressure in Pa
	props[2] = T;	//Temperature in K
	props[3] = wm;	//molecular weight
	props[4] = d;	//density
	props[5] = dl;	//density of liquid phase 
	props[6] = dv;	//density of liquid phase 
	props[7] = q;	//vapor quality on a mass basis [mass vapor/total mass] (q=0 indicates saturated liquid, q=1 indicates saturated vapor)
	props[8] = e;	//inner energy*/
	printf("h=%f J/kg\n",props[9]);	//specific enthalpy
	printf("MM=%f J/kg\n",props[3]);	//specific enthalpy
	/*props[10] = s;//specific entropy
	props[11] = cv;
	props[12] = cp;
	props[13] = w; //speed of sound
	props[14] = wmliq;
	props[15] = wmvap;*/
	for (int ii=0;ii<nX;ii++){
		printf("Xliq[%i]=%f\t",ii+1, props[16+ii]);
		printf("Xvap[%i]=%f\n",ii+1, props[16+nX+ii]);
	}

	//	INPUT:
	//		what: character specifying return value (p,T,h,s,d,wm,q,e,w) - Explanation of variables at the end of this function
	//		statevar: string of 1 variable out of p,T,h,s,d
	//		fluidnames: string containing names of substances in mixtured separated by |, substance names are identical to those of *.fld-files in REFPROP program directory
	//		statevarval: values of the variable specified in statevar
	//	 	x: array containing the mass fractions of the components of the mixture
	//	 	REFPROP_PATH: string defining the path of the refprop.dll
	//	OUTPUT
	//		return value: value of variable specified by the input variable what
	//		props: Array containing all calculated values
	//	 	errormsg: string containing error message
	//
	char what[255];
	strcpy(what, "T");
	char statevar[255];
	strcpy(statevar, "p");
	// fluidname defined above
	//strcpy(fluidname,argv[2]);
	strcpy(fluidname,"BUTANE");
	//props is defined
	double statevarval; // 1 bar
	statevarval = 1e5; // 1 bar
	// x is defined
	char REFPROP_PATH[255];
	strcpy(REFPROP_PATH,argv[5]);

	double T;
	T = satprops_REFPROP (what, statevar, fluidname, props, statevarval, x, REFPROP_PATH, errormsg, DEBUG);
	printf("Saturation conditions for %s\t",fluidname);
	printf("Saturation temperature =%f\n\n",T);
	
	
	strcpy(what, "T");
	strcpy(statevar, "p");
	strcpy(fluidname,"BUTANE");
	statevarval = 1.1e5; // 1 bar
	strcpy(REFPROP_PATH,argv[5]);

	T = satprops_REFPROP (what, statevar, fluidname, props, statevarval, x, REFPROP_PATH, errormsg, DEBUG);
	printf("Saturation conditions for %s\t",fluidname);
	printf("Saturation temperature =%f\n\n",T);
	
	
	printf("Errormessage: %s\n\n",errormsg);
	
	
// 	double dens;
// 	dens = REFPROP(char "D",char Name1, double Prop1, char Name2, double Prop2, char * Ref)
	

	return 0;
}
