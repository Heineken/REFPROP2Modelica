#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <sstream>
#include "../src/refprop_wrapper.h"
#include "../src/refprop_library.h"
#include <math.h>
#include <ctime>


int main(int argc, char* argv[]){
	//char fluidname[255];
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
#	elif defined(__ISLINUX__)
		char thepathChar[255] = "/opt/refprop";
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
	int N     = 50;

	double res  = 0.0;
	double h_in = 0.0;

	// Format the output properly
	int decimals = 4;
	int width    = 10;
	char buffer [50];
	//n=sprintf (buffer, "%d plus %d is %d", a, b, a+b);
	// printf ("[%s] is a string %d chars long\n",buffer,n);

	std::ostringstream out1,out2;

	// Then at the beginning:
	time_t tstart, tend;
	tstart = time(0);
 
	for (int count = 0; count < N; count++) {
		h_in = h + dh * count / (N - 1);
		res = props_REFPROP((char*)"u", (char*)"ph", fluidname, ders, trns, props, p, h_in, x, 0, thepathChar, errormsg, DEBUG);
		sprintf (buffer, "d = %8.4f [kg/m3]",props[4]);
		out1 << buffer << std::endl;
		sprintf (buffer, "T = %8.4f [K]",props[2]);
		out1 << buffer << std::endl << std::endl;
	}

	// And finally before the end:
	tend = time(0);
	out2 << "It took " << difftime(tend, tstart) << " second(s)." << std::endl;


	//printf("%s", out1.str().c_str());

	printf("%s", out2.str().c_str());


//	myString1 << "It took " << difftime(tend, tstart) << " second(s)."
//			<< std::endl;
//	printf("%s", myString1.str().c_str());




	return 0;
}
