@echo off
setlocal
REM ============================================================================
REM Name        : Makefile.bat
REM Author      : Jorrit Wronski (jowr@mek.dtu.dk) 
REM Version     : 0.6
REM Copyright   : Use and modify at your own risk.
REM Description : Batch script to install the static wrapper library
REM ============================================================================
REM The installation procedure should be as follows:
REM 0) make sure to have the refprop.dll available on your system
REM 1) Check the paths in this file and correct them if necessary.
REM 2) run this file 
REM ============================================================================
REM To compile the sources you can use
cd src 
cl /c /EHsc refprop_wrapper.cpp
lib refprop_wrapper.obj
cd ..
REM ============================================================================
REM 
REM  Set the required directories, could be %ProgramFiles% or %ProgramFiles(x86)%
SET  PROG_DIR=%ProgramFiles%
SET  DYMO_VER=Dymola 2013
SET  DYMO_DIR=%PROG_DIR%\%DYMO_VER%
COPY "src\refprop_wrapper.lib"   "%DYMO_DIR%\bin\lib\"
COPY "src\refprop_wrapper.h" "%DYMO_DIR%\Source\"
COPY "src\refprop_library.h" "%DYMO_DIR%\Source\"
