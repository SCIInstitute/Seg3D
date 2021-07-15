Build Seg3D
=================

<!-- * [Installing Seg3D from source](#installing-seg3d-from-source)
  * [Compiler Requirements](#compiler-requirements)
    * [Windows](#windows)
    * [Mac OS X](#mac-os-x)
    * [Linux](#linux)
  * [Dependencies](#dependencies)
    * [Qt](#qt)
      * [Seg3D 2.4 And Newer](#seg3d-24-and-newer)
        * [Mac OS X](#mac-os-x-2)
        * [Linux](#linux-2)
      * [Seg3D 2.0 to 2.3](#seg3d-20-to-23)
        * [Windows](#windows)
        * [Mac OS X](#mac-os-x-1)
        * [Linux](#linux-1)
  * [Compiling Seg3D](#compiling-seg3d)
    * [Seg3D 2.4 And Newer](#seg3d-24-and-newer-1)
    * [Optional Image Registration Tools](#optional-image-registration-tools)
  * [Seg3D Packages](#seg3d-packages)
* [Seg3D Data](#seg3d-data)
* [Seg3D Support](#seg3d-support) -->

<!-- Created by [gh-md-toc](https://github.com/ekalinin/github-markdown-toc) -->

## Installing Seg3D from source

### Compiler Requirements

**At least C++11 64-bit compiler support is required.**

#### Windows

The current source code must be compiled with the 64-bit version of Visual Studio 2013.
Visual Studio 2015 support is coming soon.
If building the installer using CPack, NSIS is required.
The code has not been tested under Cygwin or MinGW compilers.

#### Mac OS X

The source code base was built with Xcode 5-7 as well as GNU Make and Ninja and works for both environments on OS X 10.8+.
If building the installer using CPack, PackageMaker is required.

#### Linux

The code base has been tested for use with GCC, and this is the recommended compiler for
linux. GNU Make and Ninja build systems can be used. **Compiler must support C++11.**

### Dependencies

#### Qt

##### Seg3D 2.4 And Newer

Before building Seg3D, please make sure that **Qt 5.9** has been installed on your system.

###### Windows

A typical Qt 5.9 build is as follows:

+ Qt will need to be built from the command line using [Visual C++ command line tools](https://msdn.microsoft.com/en-us/library/f35ctcxw(v=vs.120).aspx). Troubleshooting tips can be found [here](http://stackoverflow.com/questions/21476588/where-is-developer-command-prompt-for-vs2013).
+ Perl and Python are required before building Qt from source. The link below includes download links for both dependencies.
+ Refer to the [Qt docs](http://doc.qt.io/qt-5/windows-requirements.html#building-from-source) for requirements for building from source.
+ Run ```configure``` from this new directory with the following parameters (using the ***-fast*** flag to speed up build process can be helpful too):

```
configure -platform win32-msvc2013 -no-gif -qt-libpng -qt-zlib -qt-libjpeg -no-openssl -skip qtwebengine -skip qtwebkit -D _BIND_TO_CURRENT_VCLIBS_VERSION=1 -debug-and-release -opensource -opengl desktop
```

+ You will be asked a few questions on license before configuring.
+ Now run nmake and compile all of Qt (this is a long build).

###### Mac OS X

Qt binaries are available on the Qt website or can be built from source code.

###### Linux

Qt 5 static libraries are **NOT** supported. If building from source, ensure that the shared library build is enabled.

If building from source, use the following:

```
./configure -qt-xcb -qt-zlib -nomake examples -nomake tests -skip qtwayland -skip qtwebengine -skip qtwebkit -opengl desktop
```

##### Seg3D 2.0 to 2.3

Before building Seg3D, please make sure that **Qt 4.7-4.8** has been installed on your system.

###### Windows

A Visual Studio binary build is available.
To our knowledge the Windows Visual Studio development libraries are only available in a 32-bit version.
A 64-bit version can be built from the source code download, configuring it as described on the Qt webpage.
A typical Qt build is as follows:

+ Download the open source version of Seg3D2. [Qt 4.8](http://download.qt.io/archive/qt/4.8/) is recommended.
+ Qt will need to be built from the command line using [Visual C++ command line tools](https://msdn.microsoft.com/en-us/library/f35ctcxw(v=vs.120).aspx). Troubleshooting tips can be found [here](http://stackoverflow.com/questions/21476588/where-is-developer-command-prompt-for-vs2013).
+ Create a new directory called ***C:\Qt\4.8-64bit*** (or name of your choice), change into that directory and copy all the source files into it.
+ Now, run ```configure.exe``` from this new directory with the following parameters (using the ***-fast*** flag to speed up build process can be helpful too):

```
configure.exe -platform win32-msvc2013 -no-gif -qt-libpng -qt-zlib -no-libmng -qt-libtiff -qt-libjpeg -no-openssl
-no-phonon -no-phonon-backend -no-qt3support -D _BIND_TO_CURRENT_VCLIBS_VERSION=1
-debug-and-release -opensource
```

+ You will be asked a few questions on license before configuring.
+ Now run nmake and compile all of Qt (this is a long build).

###### Mac OS X

Qt binaries are available on the Qt website or can be built from source code.

###### Linux

Qt is available from most package managers. Look for Qt 4.7-4.8.

#### CMake

[CMake](https://cmake.org/) versions 2.8 - 3.4 are supported.


### Compiling Seg3D

Once you have obtained a compatible compiler and installed Qt on your system, you need to
download and install CMake (<http://www.cmake.org>) to actually build the software.
CMake is a platform independent configuring system that is used for generating Makefiles,
Visual Studio project files, or Xcode project files.

Once CMake has been installed, run CMake from your build (***bin***) directory and give a path to the CMake ***Superbuild*** directory containing the master CMakeLists.txt file.

You may build Seg3D with [Ninja](https://ninja-build.org) by adding the tag "`-G Ninja`" to your cmake command. Note: Mac users will have to install Ninja with [Homebrew](https://brew.sh).

#### Seg3D 2.4 And Newer

CMake requires a path to the Qt 5 install directory. Typically, this directory will contain ***bin*** and ***lib*** directories.
For example, for Qt installed on Mac OS X from the Qt 5 binaries on the Qt site:

```
cd bin
```

```
cmake -DQt5_PATH:PATH=/Users/test/Qt5.5.1/5.5/clang_64 ../Superbuild
```

If building on Linux, the path may be under /usr/lib or /usr/lib64.

The console version ``ccmake``, or GUI version can also be used.
Set the **Qt5_PATH** variable to the location of the Qt 5 installation.

After configuration is done, generate the make files or project files for your favorite
development environment and build.

The build script can also be used:

```
./build.sh --qt5=/Users/test/Qt5.5.1/5.5/clang_64
```

The Seg3D application will be built in bin/Seg3D.

#### Seg3D 2.0 to 2.3

Running cmake on the command line:

```
cd bin
```

```
cmake ../Superbuild
```

Following the previous example, the Seg3D application will be built in bin/Seg3D.

The console version ``ccmake``, or GUI version can also be used.
You may be prompted to specify your location of the Qt installation.
If you installed Qt in the default location, it should find Qt automatically.

After configuration is done, generate the make files or project files for your favorite
development environment and build.

A bash build script (build.sh) is also available for Linux and Mac OS X to simplify the process.
Usage information is available using the ***--help*** flag:

```
./build.sh --help
```

#### Optional Image Registration Tools

To build the optional [python-based image registrations tools](ir-tools), the CMake option **BUILD_MOSAIC_TOOLS** must be turned on, either on the command line with the flag **-DBUILD_MOSAIC_TOOLS:BOOL=ON**, or in the console or GUI interfaces.

### Seg3D Packages

Build the ***package*** target in Visual Studio or Xcode, or ``make package`` using GNU make on Mac OS X.

## Seg3D Data

Seg3D sample data (also used in the tutorial) is available [here](https://github.com/CIBC-Internal/Seg3DData/releases).

## Seg3D Support

For questions and issues regarding building the software from source,
please email our support list: [seg3d@sci.utah.edu](mailto:seg3d@sci.utah.edu)
