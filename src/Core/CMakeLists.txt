#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2016 Scientific Computing and Imaging Institute,
#  University of Utah.
# 
#  
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

add_subdirectory(Action)
add_subdirectory(Application)
add_subdirectory(DataBlock)
add_subdirectory(EventHandler)
add_subdirectory(Geometry)
add_subdirectory(Interface)
add_subdirectory(Isosurface)
add_subdirectory(LargeVolume)
add_subdirectory(Log)
add_subdirectory(Parser)
add_subdirectory(State)
add_subdirectory(TextRenderer)
add_subdirectory(Utils)
add_subdirectory(Viewer)
add_subdirectory(Volume)

# These next three depend on OpenGL and should only be compiled when the renderer is compiled
add_subdirectory(Graphics)
add_subdirectory(RenderResources)
add_subdirectory(Renderer)
add_subdirectory(VolumeRenderer)

if(BUILD_MOSAIC_TOOLS)
  add_subdirectory(ITKCommon)
endif()

if(BUILD_WITH_PYTHON)
  add_subdirectory(Python)
endif()
