@echo off
set builddir=build
rmdir /S /Q %builddir%
mkdir %builddir%
mkdir %builddir%\Release
xcopy *.cl %builddir%\Release
xcopy *.conf %builddir%\Release
xcopy windows\lib\x86\*.dll %builddir%\Release\
cd %builddir%
cmake -G "Visual Studio 10" -D CMAKE_BUILD_TYPE=Release -D "CMAKE_PREFIX_PATH=windows;%AMDAPPSDKROOT%." -D LIB_SUBDIRECTORY_NAME=x86 ..
cd ..
set builddir=build64
rmdir /S /Q %builddir%
mkdir %builddir%
mkdir %builddir%\Release
xcopy *.cl %builddir%\Release
xcopy *.conf %builddir%\Release
xcopy windows\lib\x86_64\*.dll %builddir%\Release\
cd %builddir%
cmake -G "Visual Studio 10 Win64" -D CMAKE_BUILD_TYPE=Release -D "CMAKE_PREFIX_PATH=windows;%AMDAPPSDKROOT%." -D LIB_SUBDIRECTORY_NAME=x86_64 ..
cd ..
