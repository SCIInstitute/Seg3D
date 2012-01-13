#!/bin/bash
#  
#  For more information, please see: http://software.sci.utah.edu
#  
#  The MIT License
#  
#  Copyright (c) 2011 Scientific Computing and Imaging Institute,
#  University of Utah.
#  
#  License for the specific language governing rights and limitations under
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#  
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#  
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#  

# Seg3D build script
#
# This script will configure Seg3D builds using CMake
# By default, the CMake generator will be Unix Makefiles,
# in which case, the project will automatically be built using make.
#
# If an Xcode project is specified using the --xcode-build flag,
# the project will just be configured.
#
# Shortcuts for setting up the default OS X SDK build target and
# architecture are also available.

CMAKE_MAJOR_VERSION=2
CMAKE_MINOR_VERSION=8
CMAKE_PATCH_VERSION=5
CMAKE_VERSION="${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}"
CMAKE="cmake-${CMAKE_VERSION}"

OSX_TARGET_VERSION="10.5"
OSX_TARGET_VERSION_SDK="/Developer/SDKs/MacOSX10.5.sdk"
OSX_TARGET_ARCH="x86_64"

printhelp() {
    echo -e "build.sh: configure Seg3D with CMake and optionally build with GNU make."
    echo -e "If configured with --xcode-build, an Xcode project will be configured and the script will exit."
    echo -e "--debug\t\t\tBuilds Seg3D with debug symbols"
    echo -e "--release\t\tBuilds Seg3D without debug symbols (default)"
    echo -e "--set-osx-version-min\tTarget a minimum Mac OS X version (currently ${OSX_TARGET_VERSION}, ${OSX_TARGET_ARCH}) [OS X only]"
    echo -e "--xcode-build\t\tConfigure Xcode project [OS X only]"
    echo -e "-j#\t\t\tRuns # parallel make processes when building"
    echo -e "-?\t\t\tThis help"
    exit 0
}

# will cause the script to bailout if the passed in command fails
try () {
  $*
  if [[ $? != "0" ]]; then
      echo -e "\n***ERROR in build script\nThe failed command was:\n$*\n"
      exit 1
  fi
}

trybuild () {
  $*
  if [[ $? != "0" ]]; then
      echo -e "Building Seg3D returned an error\n"
      echo -e "Either the code failed to build properly or\n"
      echo -e "the testing programs failed to complete without\n"
      echo -e "every single test program passing the test.\n"
      exit 1
  fi
}

# functionally equivalent to try(),
# but it prints a different error message
ensure () {
  $* >& /dev/null
  if [[ $? != "0" ]]; then
      echo -e "\n***ERROR, $* is required but not found on this system\n"
      exit 1
  fi
}
    
# Try to find a version of cmake
find_cmake() {
    if [[ $getcmake != "1" ]]; then
        cmakebin=`which cmake`
    fi

    download=0
    #if it is not found 
    if [[ ! -e $cmakebin ]]; then
        download=1
    else
        # see if cmake is up-to-date
        version=`$cmakebin --version | cut -d ' ' -f 3 | sed -e "s/[^[:digit:].]//g"`
        echo "$cmakebin version $version found"
        major_version=${version:0:1}
        minor_version=${version:2:1}
        if [[ $major_version -le $CMAKE_MAJOR_VERSION && $minor_version -lt $CMAKE_MINOR_VERSION ]] ; then
            download=1
        fi 
    fi
      
    if [[ $download -eq 1 ]]; then
        # then look for our own copy made by this script previously
        cmakebin=$DIR/cmake/local/bin/cmake
        try mkdir -p $DIR/cmake/
        try cd $DIR/cmake 

        if [[ -e $cmakebin ]]; then
            # see if local cmake install is compatible
            version=`$cmakebin --version | cut -d ' ' -f 3 | sed -e "s/[^[:digit:].]//g"`
            echo "$cmakebin version $version found"
            major_version=${version:0:1}
            minor_version=${version:2:1}
            if [[ $major_version -ge $CMAKE_MAJOR_VERSION && $minor_version -ge $CMAKE_MINOR_VERSION ]] ; then
                download=0
            fi
        fi

        if [[ $download -eq 1 ]]; then
            # try to download and build our own copy in local
            try $getcommand http://www.cmake.org/files/v${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/${CMAKE}.tar.gz
            try tar xvzf ${CMAKE}.tar.gz
            try cd $CMAKE
            try ./bootstrap --prefix="${DIR}/cmake/local"
            try make $makeflags
            try make install
        fi
    fi

    echo "cmakebin=$cmakebin"
    ensure $cmakebin --version
}

configure_seg3d() {
    if [[ ! -d $builddir ]]; then
        echo "Creating build directory $builddir"
        try mkdir -p $builddir
    fi
    try cd $builddir

    local COMMON_BUILD_OPTS=""

    if [[ $setosxmin -eq 1 ]]; then
        COMMON_BUILD_OPTS="${COMMON_BUILD_OPTS} -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${OSX_TARGET_VERSION} -DCMAKE_OSX_SYSROOT:PATH=${OSX_TARGET_VERSION_SDK} -DCMAKE_OSX_ARCHITECTURES:STRING=${OSX_TARGET_ARCH}"
    fi

    if [[ $xcodebuild -eq 1 ]]; then
        try $cmakebin $DIR/src -G Xcode $COMMON_BUILD_OPTS $cmakeargs
    else
        # all Unix Makefile builds
        COMMON_BUILD_OPTS="${COMMON_BUILD_OPTS} -DCMAKE_BUILD_TYPE:STRING=${buildtype}  -DCMAKE_VERBOSE_MAKEFILE:BOOL=${verbosebuild}"
        try $cmakebin $DIR/src $COMMON_BUILD_OPTS $cmakeargs
    fi
}


build_seg3d() {
    if [[ $builddir != `pwd` ]]; then
      try cd $builddir
    fi

    echo "Building SCIRun using make..."
    trybuild make $makeflags
}


######### build.sh script execution starts here

export DIR=`pwd`

linux=0
osx=0

if [[ `uname` == "Darwin" ]]; then
    getcommand="curl -OL"
    osx=1
    echo "Build platform is OS X."
elif [[ `uname` == "Linux" ]]; then
    # TODO: could also try lsb_release (if available) to attempt to identify distro
    getcommand="wget"
    linux=1
    echo "Build platform is Linux."
else
    echo "Unsupported system.  Please run on OS X or Linux"
    exit 1
fi

buildtype="Release"
makeflags=""
cmakeflags=""
getcmake=0  
cmakeargs=""    
setosxmin=0
verbosebuild="OFF"
builddir="$DIR/bin"
xcodebuild=0

echo "Parsing arguments..."
while [[ $1 != "" ]]; do
    case "$1" in
        --debug)
            buildtype="Debug";;
        --release)
            buildtype="Release";;
        --get-cmake)
            getcmake=1;;
       --set-osx-version-min)
            if [[ $osx -eq 1 ]]; then
              setosxmin=1
            else
              echo "WARNING: Only OS X supports the --set-osx-version-min flag."
            fi;;
       --verbose)
            verbosebuild="ON";;
       --cmake-args=*)
            cmakeargs=`echo $1 | cut -c 14-`;;
       --custom-build-dir=*)
            dirarg=`echo $1 | cut -c 20-`
            first_char=${dirarg:0:1}
            last_char=${dirarg: -1:1}

            if [[ $first_char == '/' ]]; then
                builddir=$dirarg
            else
                if [[ $last_char == '/' ]]; then
                    builddir="${DIR}${dirarg}"
                else
                    builddir="${DIR}/${dirarg}"
                fi
            fi;;
        --xcode-build)
            if [[ $osx -eq 1 ]]; then
              xcodebuild=1
            else
              echo "WARNING: Only OS X supports the --xcode-build flag."
            fi;;
        -j*)
            makeflags="${makeflags} $1";;
        -D*)
            cmakeflags="${cmakeflags} $1";;
        -?|--?|-help|--help)
            printhelp;;
        *)
            echo "WARNING: \'$1\' parameter ignored";;
    esac
    shift 1
done

cmakeargs="${cmakeargs} ${cmakeflags}"

echo "CMake args: $cmakeargs"
echo "Get CMake: $getcmake"
echo "Make Flags: $makeflags"
if [[ $xcodebuild -eq 0 ]]; then
    echo "Build Type: $buildtype"
else
    echo "Generating Xcode project"
fi

ensure make --version # ensure make is on the system

find_cmake

configure_seg3d

# build using make if generator is not Xcode
if [[ $xcodebuild -eq 0 ]]; then
    build_seg3d
fi
