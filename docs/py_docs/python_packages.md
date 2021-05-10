# Python Packages and Matlab

## Installing packages

If there is an installation of the packages with other python builds on the machine, the system path can be used to use those packages.
There are many ways to install packages in Python. pip is an easy way that is usually included with Python.

### Installing pip

If pip is not installed for the Seg3D installation of python, follow these [directions](https://pip.pypa.io/en/stable/installing/).
Be sure to use the python included in the Seg3D application.
With OS x versions, it is in `Seg3D2.app/Contents/Frameworks/Python.framework/Versions/Current/bin`.

#### Installing numpy, scipy, and other major packages

* Make sure that pip is installed
* in the terminal:
```
cd Seg3D2.app/Contents/Frameworks/Python.framework/Versions/Current/bin
```
```
./python3 -m pip install numpy
```

Make sure that the package is found in `Seg3D2.app/Contents/Frameworks/Python.framework/Versions/Current/lib/python3.4/site-packages`.

##### Installing Matlab engine for Python in Seg3D

In the terminal:
```
cd "matlab_root"/extern/engines/python
```
```
Seg3D2.app/Contents/Frameworks/Python.framework/Versions/Current/bin/python3.4 setup.py build --build-base="builddir" install --prefix="installdir"
```

Make sure that the package (matlab and matlabengineforpython-…) is found in `Seg3D2.app/Contents/Frameworks/Python.framework/Versions/Current/lib/python3.4/site-packages`

Full instructions are located on the [mathworks site](http://www.mathworks.com/help/matlab/matlab_external/install-the-matlab-engine-for-python.html).

## Matlab engine in Seg3D (through Python)

In Seg3D, Matlab code and functions can be run using the Matlab engine for Python in the python console or Python interface.
To do so, make sure that the Matlab engine is installed (previous section).

To run the Matlab engine in the Python console in Seg3D, the **DYLD_LIBRARY_PATH** environment variable needs to be set to find python libraries and the matlab engine:

```
export DYLD_LIBRARY_PATH="path"/Seg3D2.app/Contents/Frameworks/Python.framework/Versions/Current/lib: "matlab_root"/extern/engines/python/dist/matlab/
```

This can be added to the .bashrc file or you could try the /etc/launchd.conf file.

Full documentation for the Matlab engine can be found on the [mathworks site](http://www.mathworks.com/help/matlab/matlab-engine-for-python.html).
