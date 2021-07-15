# Specifications

## Minimum recommended system configuration:

+ CPU: Core Duo or higher, recommended i5 or i7
+ Memory: 4Gb, recommended 8Gb or more
+ Graphics Memory: minimum 128MB, recommended 256MB or more

## Windows

Seg3D will install on Windows 7-10 (64-bit only).

## Mac OS X

The Seg3D application will run on Mac OS X 10.8+.

The Seg3D 2 installer is not currently signed. The unsigned installer can be run by control-clicking (or right-clicking if 2 button mouse support is enabled) on the installer package and selecting open. The unsigned Seg3D application can also be run for the first time by control-clicking (or right-clicking if 2 button mouse support is enabled) on the installer package and selecting open.

## Linux specifications

### Build from source

Seg3D must be [compiled](build.md) from source on Linux platforms (OpenSuSE, Ubuntu etc.) and requires at least the following:

+ GCC or clang compiler that supports C++11
+ Git 1.8 or greater  (system default packages are usually sufficient)
+ CMake (tested with 2.8 - 3.4)
+ NVIDIA card and drivers for Linux
+ Graphics cards must support OpenGL 2.0 or greater (not available on older Intel embedded graphics cards).

Consult the distribution-specific section for additional package information and the developer documentation for build instructions.

#### Ubuntu

We are currently testing on 64-bit Ubuntu 14.04.2 LTS and 13.04. We usually build Seg3D using the GCC C++ compiler, so we recommend installing the package that provides that compiler. Ubuntu comes with a relatively minimal install, so several packages are required.

Ubuntu 14.x and 13.x Packages
It is easy to get the necessary packages listed below via the Synaptic package manager or the command line instructions below:

```
sudo apt-get install cmake-qt-gui cmake-curses-gui build-essential \
libxft-dev libxi-dev libxmu-headers freeglut3-dev \
libtiff4-dev git
```

Qt SDK
Version 4.8 is recommended. The libqt4-dev package provides Qt 4.8 on Ubuntu 14.x and 13.x.

If the libpng library is not installed by the Qt SDK packages, it will have to be installed.

#### OpenSuSE

We are currently testing on 64-bit Leap 42.1 (OpenSuSE package repository information).

OpenSuSE RPMs:

+ gcc
+ gcc-c++
+ Make
+ CMake
+ git
+ glu-devel
+ libXmu-devel
+ CMake-gui
+ CMake

Qt SDK
Version 4.8 is recommended.

If the libpng library is not installed by the Qt SDK packages, it will have to be installed. The libqt4 and libqt4-devel packages provide Qt 4.8.
