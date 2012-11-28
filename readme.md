
#Welcome to REFPROP2Modelica!
This piece of software enables the user to access the Refprop fluid property database from within Modelica. The aim is to develop wrapper classes and integrate them with the `Media` framework inside Modelica. It has only been tested with Dymola sofar. 

## Installation Instructions
If you only want to run the wrapper without compiling it from source, you can use the provided makefiles to install the precompiled static libraries. 

### Windows
**Since I do not run Windows myself, I am not able to try this myself. Could you, dear Windows users, please provide me with feedback and the actual `.lib` files needed for the instructions below to work.**

> For Windows, please follow these instructions
> 
> 1.  After downloading and unzipping rename folder containing these files to `MediaTwoPhaseMixture`.
> 2.  Double click on `_wrapper\v0.6\Makefile.bat`
> 3.  Set the path to the REFPROP program directory with the constant String REFPROP_PATH (at the beginning of the Modelica package). Make sure you mask the backslashes. It should look something like: constant String REFPROP_PATH = `C:\\Program Files\\REFPROP\\`;
> 

### Linux
For installing on a Linux machine, please follow these instructions

1.  After downloading and unzipping rename folder containing these files to `MediaTwoPhaseMixture`.
2.  Open a command prompt in `_wrapper/v0.6/` and run `sudo make install_static`.
3.  Make sure to have the shared library `librefprop.so` available in the same directory as the `fluids` folder. You might want to check https://github.com/jowr/librefprop.so for details. 
4.  Set the path to the REFPROP program directory with the constant String REFPROP_PATH (at the beginning of the Modelica package). It should look something like: constant String REFPROP_PATH = "/opt/refprop/"; 

## Compiling

### Windows
Please feel free to contribute to this part of the work. I was told that the code works with VisualStudio and VisualStudio Express, but I cannot provide instructions or feedback.

### Linux
For compiling on a Linux machine, please follow the instructions in the Makefile provided in the directory of the wrapper sources. You might have to adjust some directorie names and install all the compilers and libraries required. 

Afterwards, you can go to the directory `_wrapper/v0.6/` and call `make all` and `sudo make install` as well as `sudo make fixit` to compile and install the wrapper library. This compiles and installs a dynamic library on your system. To remove the files from your system, please use `sudo make uninstall`. Please be aware that you need a copy of the Poco C++ framework v1.3.6 or newer to compile future versions of the wrapper files yourself. You can find it at: http://pocoproject.org/ and https://github.com/pocoproject/poco or install it via your package manager with `sudo apt-get install libpoco-dev` .The general idea is to use this framework to implement platform independent library loading and caching, but it is still a long way to go. 

## General Remarks
Please note that you need a working and licensed copy of Refprop in order to use the software provided here. This is not a replacement for Refprop.

## Links
If you are interested in this kind of software, you might also consider the following projects:

1.  https://github.com/Heineken/REFPROP2Modelica (The original version of this library)
2.  https://github.com/thorade/HelmholtzMedia (A Modelica implementation the Helmholtz functions)
3.  http://coolprop.sourceforge.net/ (A free fluid property library also based on Helmholtz formulation)