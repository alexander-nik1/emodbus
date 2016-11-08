
*******************************************************************

To build this project you neet to:
(suppose, you are in the project directory)

mkdir build
cd build
cmake -DPACKAGE_ARCH=i386 -DTARGET_PLATFORM=posix -DPREFIX=/usr ../emodbus
make
make package

If all steps was finished without errors, then you 
have the DEB packages in a build directory, and you can install them.

*******************************************************************

About a cmake's options:

The PACKAGE_ARCH is a architecture that will be used
in a DEB package Architecture.
This option is required.

The TARGET_PLATFORM is a trarget platform to select type of 
building and packaging. (Now, a valid value is a 'posix')
This option is required.

The PREFIX is a install prefix for all data in DEB packages.
This option is not required, and if you is not set it, it will
be defaulted to the '/usr' value.

*******************************************************************
