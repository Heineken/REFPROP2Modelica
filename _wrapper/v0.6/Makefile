# ============================================================================
# Name        : Makefile
# Author      : Jorrit Wronski (jowr@mek.dtu.dk) 
# Version     : 0.6
# Copyright   : Use and modify at your own risk.
# Description : Makefile for Refprop from Fortran and C++ tests.
# ============================================================================
# The installation procedure should be as follows:
# 0) make sure to have librefprop.so available on your system
# 1) make header library
# 2) sudo make install
# 3) sudo make fixit
# ============================================================================
# general commands:
RM    := rm -f
CP    := cp 
CH    := chmod 0644 
MK    := mkdir -p 
LD    := ldconfig 
LN    := ln -sf 

# used for the output
MAJORVERSION     =0
MINORVERSION     =6
THENAME          =refprop_wrapper
LIBRARYEXTENSION =.so
THETEST          =refpropwrappertest

# find . -type f -print0 | xargs -0 sed -i 's/!REFPROP_wrapper/refprop_wrapper/g' 
# find . -type f -print0 | xargs -0 sed -i 's/refprop_wrapper/refprop_wrapper/g' 
###########################################################
#  Setting the directories for library, header and 
#  binary files created in this makefile. 
###########################################################
SRCDIR     =./src
# DYMDIR     =/opt/dymola
# LIBINST    =$(DYMDIR)/bin/lib
# HEADINST   =$(DYMDIR)/source
LIBINST    =/usr/local/lib
HEADINST   =/usr/local/include
BINDIR     =./bin


LIBS       =-lrefprop -lPocoFoundation 
OPTFLAGS   =-O3 -ffast-math# -ffloat-store # optimisation, remove for debugging

###########################################################
#  Change these lines if you are using a different C++ 
#  compiler or if you would like to use other flags. 
###########################################################
CPPC         =g++
CPPFLAGS     =$(OPTFLAGS) -Wall -pedantic -fbounds-check -ansi -Wpadded -Wpacked -malign-double -mpreferred-stack-boundary=8
DEBUGFLAGS   =-g -O3

###########################################################
#  Change these lines if you have other needs regarding
#  the generated shared library file.  
###########################################################
LIBFILE         =$(THENAME)
LIBRARY         =lib$(THENAME)
LIBFLAGS        =-rdynamic -fPIC -shared -Wl,-soname,$(LIBRARY)$(LIBRARYEXTENSION).$(MAJORVERSION)
HEADERFILE      =$(THENAME)
HEADEREXTENSION =.h

###########################################################
#  Copy files to places recognised by the system.
###########################################################
.PHONY     : install
install    : install_dynamic

.PHONY     : install_dynamic
install_dynamic : header library
	$(MK) $(HEADINST) $(LIBINST)
	$(CP) $(BINDIR)/$(HEADERFILE)$(HEADEREXTENSION) $(HEADINST)/$(HEADERFILE)$(HEADEREXTENSION)
	$(CP) $(BINDIR)/$(LIBRARY)$(LIBRARYEXTENSION) $(LIBINST)/$(LIBRARY)$(LIBRARYEXTENSION).$(MAJORVERSION).$(MINORVERSION)
	$(CH) $(HEADINST)/$(HEADERFILE)$(HEADEREXTENSION)
	$(CH) $(LIBINST)/$(LIBRARY)$(LIBRARYEXTENSION).$(MAJORVERSION).$(MINORVERSION)
	$(LD) -l $(LIBINST)/$(LIBRARY)$(LIBRARYEXTENSION).$(MAJORVERSION).$(MINORVERSION)
	$(LN) $(LIBINST)/$(LIBRARY)$(LIBRARYEXTENSION).$(MAJORVERSION) $(LIBINST)/$(LIBRARY)$(LIBRARYEXTENSION)
	$(LD) 

.PHONY     : install_static
install_static : header $(LIBRARY).a 
	$(MK) $(HEADINST) $(LIBINST)
	$(CP) $(BINDIR)/$(HEADERFILE)$(HEADEREXTENSION) $(HEADINST)/$(HEADERFILE)$(HEADEREXTENSION)
	$(CP) $(LIBRARY).a  $(LIBINST)/$(LIBRARY).a 
	$(CH) $(HEADINST)/$(HEADERFILE)$(HEADEREXTENSION)
	$(CH) $(LIBINST)/$(LIBRARY).a 
	$(LD) 

.PHONY     : uninstall
uninstall  : unfixit
	$(RM) $(HEADINST)/$(HEADERFILE)$(HEADEREXTENSION)
	$(RM) $(LIBINST)/$(LIBRARY)$(LIBRARYEXTENSION)*
	$(RM) $(LIBINST)/$(LIBRARY).a

.PHONY     : all
all        : header library

.PHONY     : fixit
fixit      : install
	ln -sf $(HEADINST)/$(HEADERFILE)$(HEADEREXTENSION) $(HEADINST)/REFPROP_wrapper$(HEADEREXTENSION)
	ln -sf $(LIBINST)/$(LIBRARY)$(LIBRARYEXTENSION)    $(LIBINST)/libREFPROP_wrapper$(LIBRARYEXTENSION)
	ln -sf $(LIBINST)/$(LIBRARY).a                     $(LIBINST)/libREFPROP_wrapper

.PHONY     : unfixit
unfixit    : 
	$(RM) $(HEADINST)/REFPROP_wrapper$(HEADEREXTENSION)
	$(RM) $(LIBINST)/libREFPROP_wrapper$(LIBRARYEXTENSION)
	$(RM) $(LIBINST)/libREFPROP_wrapper

##########################################################
#  Compile the C++ sources into a library file that can 
#  be used as a shared object.
###########################################################
.PHONY     : header
header     : $(BINDIR)/$(HEADERFILE)$(HEADEREXTENSION)
$(BINDIR)/$(HEADERFILE)$(HEADEREXTENSION): $(SRCDIR)/$(HEADERFILE)$(HEADEREXTENSION)
	$(CP) $(SRCDIR)/$(HEADERFILE)$(HEADEREXTENSION) $(BINDIR)

.PHONY     : library
library    : $(BINDIR)/$(LIBRARY)$(LIBRARYEXTENSION)

$(BINDIR)/$(LIBRARY)$(LIBRARYEXTENSION): $(SRCDIR)/$(LIBFILE).o
	$(CPPC) $(LIBFLAGS) $(CPPFLAGS) -o $(BINDIR)/$(LIBRARY)$(LIBRARYEXTENSION) $(SRCDIR)/$(LIBFILE).o $(LIBS)


###########################################################
#  Compile the wrapper class tests.
###########################################################	
.PHONY     : test
test       : $(BINDIR)/$(THETEST)

$(BINDIR)/$(THETEST) : #$(SRCDIR)/$(THETEST).o 
	$(CPPC) $(DEBUGFLAGS) -o $(SRCDIR)/$(LIBFILE).o -c $(SRCDIR)/$(LIBFILE).cpp $(LIBS)
	$(CPPC) $(DEBUGFLAGS) -o $(BINDIR)/$(THETEST) $(SRCDIR)/$(THETEST).cpp $(SRCDIR)/$(LIBFILE).o $(LIBS) 

# .PHONY     : ian
# ian        : $(BINDIR)/$(THETEST).ian
# 
# $(BINDIR)/$(THETEST).ian : $(SRCDIR)/$(THETEST).ian.o $(SRCDIR)/$(LIBFILE).ian.o 
# 	$(CPPC) $(CPPFLAGS) -o $(BINDIR)/$(THETEST) $(SRCDIR)/$(THETEST).ian.o $(SRCDIR)/$(LIBFILE).ian.o -lPocoFoundation 

###########################################################
#  General rulesets for compilation.
###########################################################
$(SRCDIR)/%.o : $(SRCDIR)/%.cpp
	$(CPPC) $(CPPFLAGS) -o $(SRCDIR)/$*.o -c $<

$(SRCDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $(SRCDIR)/$*.o -c $<

$(SRCDIR)/%.o : $(SRCDIR)/%.FOR
	$(FC) $(FFLAGS) -o $(SRCDIR)/$*.o -c $<

.PHONY: clean
clean:
	$(RM) **.o **.so **.mod $(BINDIR)/* $(SRCDIR)/*.o 

###########################################################
#  Create the documentation from annotations in the source
#  files with DOXYGEN, a configuration file is needed.
###########################################################
.PHONY     : doc
doc        : doc/Doxyfile
	doxygen doc/Doxyfile
	cd doc/latex ; \
	make all 
	