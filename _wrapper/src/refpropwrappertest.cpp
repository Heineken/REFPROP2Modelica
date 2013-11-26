#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include "refprop_wrapper.h"
#include "refprop_library.h"
#include <math.h>
#include <ctime>



//double density(char* fluidname_in, double p, double t, double* x, char* REFPROP_PATH);
//double density(char* fluidname_in, double p, double t);
//double density(double p, double t);
//char *str_replace(char *str, char *search, char *replace, long *count);

int main(int argc, char* argv[]){
	double p,t,d;
	char fluidname[255];
	char errormsg[255];
	double* x;
	double *props;
	double *ders;
	double *trns;
	double sumx;
	int i;
//	int nX = argc-5;
	int DEBUG = 0;

	
	
/*	
  int count;
  printf ("This program was called with \"%s\".\n",argv[0]);
	if (argc > 1)
    {
      for (count = 1; count < argc; count++)
	{
	  printf("argv[%d] = %s\n", count, argv[count]);
	}
    }
  else
    {
      printf("The command had no other arguments.\n");
    }
printf("argc is %li \n",argc); 
*/


/*
	if (argc<5){
		printf("usage: refpropwrappertest.exe statevars fluidname1|fluidname2|... statevar1 statevar2 REFPROPdir massfractionComponent1 \nexample: refpropwrappertest \"pT\" \"isobutan|propane\" 1e5 293 \"d:\\Programme\\REFPROP\\\" .1");
//		printf("usage: refpropwrappertest statevars fluidname1|fluidname2|... statevar1 statevar2 REFPROPdir massfractionComponent1 \nexample: ./bin/refpropwrappertest \"pT\" \"ISOBUTAN|PROPANE\" 1e5 293 \"/opt/refprop/\" .1\n");
		return 1;
	}
*/
// usage
// refpropwrappertest "pT" "isobutan|propane" 1e5 293 "c:\\Program Files (x86)\\REFPROP" .1	

int nX=2;

	x = (double*) calloc(nX,sizeof(double));
	props=(double*) calloc(16+2*nX,sizeof(double));
	ders=(double*) calloc(21,sizeof(double));
	trns=(double*) calloc(3,sizeof(double));

x[0] = 0.5;
x[1] = 1.0-x[0];
//x[2] = 1-x[1];





/*
	sumx = 0;
	for (i=0;i<nX-1;i++){
		x[i] = atof(argv[6+i]);
		sumx += x[i];
	}
	x[nX-1] = 1-sumx;
*/

//	double p;
    double h;
	double dh;
	int N=500;

	double* T_save;
	double* h_in;
	T_save = (double*) calloc(N,sizeof(double));
	h_in = (double*) calloc(N,sizeof(double));

p=50e5;
h=3e5;
dh=25e5;

props_REFPROP  ("", "ph", "ammoniaL|water"           , ders, trns, props, p, h, x, 0, "c:\\Program Files (x86)\\Refprop"     , errormsg, DEBUG);

	//printf("q=%f [kg/kg]\n",props[7]);

//Then at the beginning:
 time_t tstart, tend; 
 tstart = time(0);
 
  int count;
	for (count = 0; count < N; count++)
	{
		h_in[count] = h+dh*count/(N-1);
		props_REFPROP  ("", "ph", "ammoniaL|water"           , ders, trns, props, p, h_in[count], x, 0, "c:\\Program Files (x86)\\Refprop"     , errormsg, DEBUG);
		T_save[count] = props[2];

	//printf("T=%f [K]\n",props[2]);
	}

// And finally before the end:
 tend = time(0); 
 std::ostringstream myString1;
 myString1 << "It took "<< difftime(tend, tstart) <<" second(s)."<< std::endl;
printf("%s",myString1.str().c_str());

	std::ostringstream s;
	for (count = 0; count < N; count++)
	{
	s << h_in[count] << " " << T_save[count] << std::endl;
	}

	FILE * f;
	f = fopen("test.txt", "wb"); // wb -write binary
	if (f != NULL) 
	{
    fputs (s.str().c_str(),f);
    fclose(f);
	}
	else
	{
	//failed to create the file
	printf("failed to create the file");
	}


	//strcpy(fluidname,argv[1]);//water;
	/*p=atof(argv[2]);//2000;
	t=atof(argv[3]);//300.0;	*/
	//d = density(fluidname, p, t, x, ""); //d:\\Programme\REFPROP\\
	//d = density(fluidname, p, t);
	//d = density(p, t);
	//props_REFPROP("", "pT"   , "isobutan|propane", props, 1e5          , 293.0        , x, 0, "d:\\Programme\\REFPROP\\", errormsg, DEBUGMODE);
	// props_REFPROP  ("", argv[1], argv[2]           , props, atof(argv[3]), atof(argv[4]), x, 0, argv[5]                   , errormsg, DEBUG);

//    props_REFPROP  ("", "pT", "isobutan|propane"           , ders, trns, props, 1e5, 293, x, 0, "c:\\Program Files (x86)\\Refprop"     , errormsg, DEBUG);

	//props_REFPROP  ("", argv[1], argv[2]           , ders, trns, props, atof(argv[3]), atof(argv[4]), x, 0, argv[5]                   , errormsg, DEBUG);
//	props_REFPROP("", "pT"   , "isobutan|propane", props, 1e5          , 293.0        , x, 0, "/opt/refprop/", errormsg, DEBUG);
//	printf("Errormessage: %s\n",errormsg);
	//printf("%c,%c,D                 %10.4f,%10.4f,%10.4f \n",argv[1][0],argv[1][1],atof(argv[3]),atof(argv[4]),props[5]);
	/*	props[0] = ierr;//error code
	props[1] = p;//pressure in Pa
	props[2] = T;	//Temperature in K
	props[3] = wm;	//molecular weight
	props[4] = d;	//density
	props[5] = dl;	//density of liquid phase
	props[6] = dv;	//density of liquid phase
	props[7] = q;	//vapor quality on a mass basis [mass vapor/total mass] (q=0 indicates saturated liquid, q=1 indicates saturated vapor)
	props[8] = e;	//inner energy*/
/*
    printf("\nwm=%f molecular weight [kg/mol]\n",props[3]);
	printf("d=%f kg/m3\n",props[4]);
	printf("molar density=%f mol/L\n",props[4]/props[3]/1000);
	printf("dl=%f kg/m3\n",props[5]);
	printf("dv=%f kg/m3\n",props[6]);
	printf("q=%f [kg/kg]\n",props[7]);
	printf("e=%f J/kg\n",props[8]);
	printf("h=%f J/kg\n",props[9]);	//specific enthalpy
	printf("MM=%f kg/mol  I think this must be kg/kmol... \n",props[3]);	//molar weight
	printf("s=%f J/kg\n",props[10]);	//specific enthalpy
	printf("a=%f J/kg\n",ders[2]);	//helmholtz energy
	printf("f=%f J/kg\n",ders[3]);	//gibbs energy
	printf("dhdd_T=%f J/kg m3/kg\n",ders[15]);	//dhdd_T
	printf("dhdd_p=%f J/kg m3/kg\n",ders[16]);	//dhdd_p
	printf("eta=%f Pa.s \n",trns[1]);	//
	printf("lambda=%f W/m.K\n",trns[2]);	//
	printf("dddh_p=%f \n",ders[19]);	//
	printf("dddp_h=%f \n\n",ders[20]);	//
*/

//	/*props[10] = s;//specific entropy
//	props[11] = cv;
//	props[12] = cp;
//	props[13] = w; //speed of sound
//	props[14] = wmliq;
//	props[15] = wmvap;*/
//	//for (int ii=0;ii<nX;ii++){
//	//	printf("Xliq[%i]=%f\t",ii+1, props[16+ii]);
//	//	printf("Xvap[%i]=%f\n",ii+1, props[16+nX+ii]);
//	//}
//
//	props_REFPROP  ("l", argv[1], argv[2]           , ders, trns, props, atof(argv[3]), atof(argv[4]), x, 0, argv[5]                   , errormsg, DEBUG);
//	//	props_REFPROP("", "pT"   , "isobutan|propane", props, 1e5          , 293.0        , x, 0, "/opt/refprop/", errormsg, DEBUG);
//		printf("Errormessage: %s\n",errormsg);
//
//		printf("h=%f J/kg\n",props[9]);	//specific enthalpy
//		printf("MM=%f kg/mol\n",props[3]);	//molar weight
//		printf("s=%f J/kg\n",props[10]);	//specific enthalpy
//		printf("a=%f J/kg\n",ders[2]);	//helmholtz energy
//		printf("f=%f J/kg\n",ders[3]);	//gibbs energy
//		printf("dhdd_T=%f J/kg m3/kg\n",ders[15]);	//dhdd_T
//		printf("dhdd_p=%f J/kg m3/kg\n",ders[16]);	//dhdd_p
//		printf("eta=%f Pa.s \n",trns[1]);	//
//		printf("lambda=%f W/m.K\n",trns[2]);	//
//		printf("dddh_p=%f \n",ders[19]);	//
//		printf("dddp_h=%f \n\n",ders[20]);	//
//
//	//	for (int ii=0;ii<nX;ii++){
//	//		printf("Xliq[%i]=%f\t",ii+1, props[16+ii]);
//	//		printf("Xvap[%i]=%f\n",ii+1, props[16+nX+ii]);
//	//	}
//
//	//	INPUT:
//	//		what: character specifying return value (p,T,h,s,d,wm,q,e,w) - Explanation of variables at the end of this function
//	//		statevar: string of 1 variable out of p,T,h,s,d
//	//		fluidnames: string containing names of substances in mixtured separated by |, substance names are identical to those of *.fld-files in REFPROP program directory
//	//		statevarval: values of the variable specified in statevar
//	//	 	x: array containing the mass fractions of the components of the mixture
//	//	 	REFPROP_PATH: string defining the path of the refprop.dll
//	//	OUTPUT
//	//		return value: value of variable specified by the input variable what
//	//		props: Array containing all calculated values
//	//	 	errormsg: string containing error message
//	//
//	char what[255];
//	strcpy(what, "T");
//	char statevar[255];
//	strcpy(statevar, "p");
//	// fluidname defined above
//	//strcpy(fluidname,argv[2]);
//	strcpy(fluidname,"BUTANE");
//	//props is defined
//	double statevarval; // 1 bar
//	statevarval = 1e5; // 1 bar
//	// x is defined
//	char REFPROP_PATH[255];
//	strcpy(REFPROP_PATH,argv[5]);
//
//	double T;
//	T = satprops_REFPROP (what, statevar, fluidname, ders, trns, props, statevarval, x, REFPROP_PATH, errormsg, DEBUG);
//	printf("Saturation conditions for %s\t",fluidname);
//	printf("Saturation temperature =%f\n\n",T);
//
//
//	strcpy(what, "T");
//	strcpy(statevar, "p");
//	strcpy(fluidname,"BUTANE");
//	statevarval = 1.1e5; // 1 bar
//	strcpy(REFPROP_PATH,argv[5]);
//
//	T = satprops_REFPROP (what, statevar, fluidname, ders, trns, props, statevarval, x, REFPROP_PATH, errormsg, DEBUG);
//	printf("Saturation conditions for %s\t",fluidname);
//	printf("Saturation temperature =%f\n\n",T);
//
//
//	printf("Errormessage: %s\n\n",errormsg);
//
//
//// 	double dens;
//// 	dens = REFPROP(char "D",char Name1, double Prop1, char Name2, double Prop2, char * Ref)
//

//	system("pause");




	return 0;
}
