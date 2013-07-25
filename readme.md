#Welcome to REFPROP2Modelica!
This piece of software enables the user to access the Refprop fluid property database from within Modelica. Even though the code base is not very mature yet, this package seems to be the only solution to access properties of mixtures from within Modelica on both Windows and Linux systems. The aim is to develop wrapper classes and integrate them with the `Media` framework inside Modelica. It has only been tested with Dymola sofar. 

## Installation Instructions
You can use the provided makefiles to compile and install the wrapper. The process should be straightforward. You can get the latest release from https://github.com/jowr/REFPROP2Modelica/releases

### Windows
Since I do not run Windows regularly, I am not able to test many things myself. Could you, dear Windows users, please 
provide me with feedback?

1.  After downloading, unpack and rename the folder to `REFPROP2Modelica`.
2.  Run `_wrapper\v0.7\Makefile.bat` from your Visual Studio console, tested with VS9.0 (2008).
3.  Make sure to have the shared library `refprop.dll` available on your system.
4.  Set the path to the REFPROP program directory with the constant String REFPROP_PATH (at the beginning of the Modelica package). Make sure you mask the backslashes. It should look something like: constant String REFPROP_PATH = `C:\\Program Files\\REFPROP\\`;


### Linux
For installing on a Linux machine, please follow these instructions

1.  After downloading, unpack and rename the folder to `REFPROP2Modelica`.
2.  Open a command prompt in `_wrapper/v0.7/`, then run `make all` and `sudo make install_static`.
3.  Make sure to have the shared library `librefprop.so` available on your system. You might want to check https://github.com/jowr/librefprop.so for details. 
4.  Set the path to the REFPROP program directory with the constant String REFPROP_PATH (at the beginning of the Modelica package). It should look something like: constant String REFPROP_PATH = "/opt/refprop/"; 
5.  If you experience problems with older versions of this package, you can run `sudo make fixit` to create additonal aliases. This compiles and installs a dynamic library on your system. 
6.  ... and finally to remove the files from your system, please use `sudo make uninstall`.


## General Remarks
Please note that you need a working and licensed copy of Refprop in order to use the software provided here. 
This is not a replacement for Refprop, only a wrapper to use it with Modelica.

## Links
If you are interested in this kind of software, you might also want to consider the following projects:

1.  https://github.com/Heineken/REFPROP2Modelica (The original version of this library)
2.  https://github.com/thorade/HelmholtzMedia (A Modelica implementation the Helmholtz functions)
3.  http://coolprop.sourceforge.net/ (A free fluid property library also based on Helmholtz formulation)
