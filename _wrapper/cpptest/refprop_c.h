
#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__WIN64__)
#	define __ISWINDOWS__
#	include <windows.h>
#	define _CRT_SECURE_NO_WARNINGS
#elif __APPLE__
#	define __ISAPPLE__
#elif __linux
#	define __ISLINUX__
#endif

#if defined(__ISWINDOWS__)
#include <windows.h>
#elif defined(__ISLINUX__)
#include <dlfcn.h>
#endif

#if defined(__ISWINDOWS__)
// For C calling conventions, replaced all "double &" with "double *", and "long &" with "long *"
typedef void (__stdcall *fp_PHFLSHdllTYPE)(double *,double *,double *,double *,double *,double *,double *,double *,double *,double *,double *,double *,double *,double *,double *,long *,char*,long );
typedef void (__stdcall *fp_SETUPdllTYPE)(long *,char*,char*,char*,long *,char*,long ,long ,long ,long );
typedef void (__stdcall *fp_XMOLEdllTYPE)(double *,double *,double *);
#elif defined(__ISLINUX__)
// For C calling conventions, replaced all "double &" with "double *", and "long &" with "long *"
typedef void ( *fp_PHFLSHdllTYPE)(double *,double *,double *,double *,double *,double *,double *,double *,double *,double *,double *,double *,double *,double *,double *,long *,char*,long );
typedef void ( *fp_SETUPdllTYPE)(long *,char*,char*,char*,long *,char*,long ,long ,long ,long );
typedef void ( *fp_XMOLEdllTYPE)(double *,double *,double *);
#endif

//Define explicit function pointers
fp_PHFLSHdllTYPE PHFLSHdll;
fp_SETUPdllTYPE SETUPdll;
fp_XMOLEdllTYPE XMOLEdll;
