
#Welcome to REFPROP2Modelica!
This piece of software enables the user to access the Refprop fluid property database from within Modelica. The aim is to develop wrapper classes and integrate them with the `Media` framework inside Modelica. It has only been tested with Dymola sofar. 

## Installation Instructions

### Windows
For Windows, please follow these instructions

1.  After downloading and unzipping rename folder containing these files to `MediaTwoPhaseMixture`.
2.  Copy `\wrapper\v0.5\REFPROP_WRAPPER.LIB` to `%DYMOLADIR%\\BIN\\LIB\` (%DYMOLADIR% is DYMOLA's program directory)
3.  Copy `\wrapper\v0.5\REFPROP_WRAPPER.H` to `%DYMOLADIR%\\SOURCE\\`
4.  Set the path to the REFPROP program directory with the constant String REFPROP_PATH (at the beginning of the Modelica package). Make sure you mask the backslashes. It should look something like: constant String REFPROP_PATH = `C:\\Program Files\\REFPROP\\`;

### Linux
For installing on a Linux machine, please follow the instructions in the Makefile provided in the directory containing the Linux version of the wrapper class. You only have to type in the right directories and install all the compilers / libraries required. 

1.  After downloading and unzipping rename folder containing these files to `MediaTwoPhaseMixture`.
2.  Change the paths in `_wrapper/v0.6/Makefile` to your needs.
3.  Make sure to have the shared library `librefprop.so` available in the same directory as the `fluids` folder. You might want to check https://github.com/jowr/librefprop.so for details. 
4.  Got to the directory `_wrapper/v0.6/` and call `make all` and `sudo make install` as well as `sudo make fixit` to compile and install the wrapper library.
5.  Set the path to the REFPROP program directory with the constant String REFPROP_PATH (at the beginning of the Modelica package). It should look something like: constant String REFPROP_PATH = "/opt/refprop/"; 

To remove the files from your system, please use `sudo make uninstall`. Please be aware that you need a copy of the Poco C++ framework to compile future versions of the wrapper files yourself. You can find it at: http://pocoproject.org/. The general idea is to use this framework to implement platform independent library loading and caching, but it is still a long way to go. 

## General Remarks
Please note that you need a working and licensed copy of Refprop in order to use the software provided here. This is not a replacement for Refprop.

## Links
If you are interested in this kind of software, you might also consider the following projects:

1.  https://github.com/Heineken/REFPROP2Modelica (The original version of this library)
2.  https://github.com/thorade/HelmholtzMedia (A Modelica implementation the Helmholtz functions)
3.  http://coolprop.sourceforge.net/ (A free fluid property library also based on Helmholtz formulation)