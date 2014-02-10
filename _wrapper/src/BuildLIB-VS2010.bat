REM ******** set the variables ************
REM call both to ensure that one works
call "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"

REM ******* compile all the sources from CoolProp ***************
cl /c /Ox /fp:fast /MD /EHsc refprop_wrapper.cpp

lib refprop_wrapper.obj
erase *.obj

copy refprop_wrapper.lib "C:\Program Files (x86)\Dymola 2013\bin\lib\"
copy refprop_wrapper.h "C:\Program Files (x86)\Dymola 2013\Source\"
copy refprop_library.h "C:\Program Files (x86)\Dymola 2013\Source\"
copy refprop_wrapper.lib "C:\Program Files (x86)\Dymola 2014\bin\lib\"
copy refprop_wrapper.h "C:\Program Files (x86)\Dymola 2014\Source\"
copy refprop_library.h "C:\Program Files (x86)\Dymola 2014\Source\"