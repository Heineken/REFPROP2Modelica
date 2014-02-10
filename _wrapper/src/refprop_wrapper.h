/*
	header file for refprop_wrapper.cpp
	
	This file is released under the Modelica License 2.
	
	Coded in 2010 by
	Henning Francke
	francke@gfz-potsdam.de
	
	Helmholtz Centre Potsdam
	GFZ German Research Centre for Geosciences
	Telegrafenberg, D-14473 Potsdam


	Modified for Linux and extended in 2012
	and in 2013 by
	Jorrit Wronski (jowr@mek.dtu.dk)
	DTU Mechanical Engineering
*/

#ifndef REFPROP_WRAPPER
#define REFPROP_WRAPPER

#if defined(__ISWINDOWS__)
#  if defined(__cplusplus)
#    define EXPCONV __declspec(dllexport)
#  else
#    define EXPCONV
#  endif // __cplusplus
#else
#  define EXPCONV
#endif

#define OK 0
#define FAIL 1

//std::vector<std::string> strsplit(std::string s, char del);
//std::string get_REFPROP_fluid_path(std::string rpPath);
//std::string resolve_error(std::string in1, long lerr, char* errormsg);
//bool load_REFPROP(char* error);

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus
	EXPCONV double props_REFPROP(char* what, char* statevars, char* fluidnames, double *ders, double *trns, double *props, double statevar1, double statevar2, double* x, int phase, char* REFPROP_PATH, char* errormsg, int DEBUGMODE, int calcTransport, int partialDersInputChoice); //declaration;
	EXPCONV double satprops_REFPROP(char* what, char* statevar, char* fluidnames, double *satprops, double statevarval, double Tsurft, double* x, char* REFPROP_PATH, char* errormsg, int DEBUGMODE,int calcTransport); //declaration;
#if defined(__cplusplus)
}
#endif // __cplusplus
#endif /*REFPROP_WRAPPER*/
