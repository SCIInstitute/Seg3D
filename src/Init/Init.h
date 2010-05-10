/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2009 Scientific Computing and Imaging Institute,
 University of Utah.


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */

#ifndef INIT_INIT_H
#define INIT_INIT_H

namespace Core
{

// INIT PLUGINS:
// This function serves only one purpose: to link in calls to all the plugin
// classes. This function will populate all the object factories with the 
// actions, tools, and UI pieces that can be configured on the fly.

// As the program is statically linked, plugins are optimized out of the program
// if they are not mentioned from any where in the main program. Hence this
// class will touch them all and will ensure that these plugins are available
// in the base classes. 

// The internals of the InitPlugins function are generated automatically by the
// make system. And hence to create a new plugin just add a registration tag
// inside the action/tool implementation class and use the right macro to build
// the library. This will then generate all the code needed to ensure these
// pieces are not optimized out of the code and that these pieces are part of
// of the main executable 

void InitPlugins();

}

#endif
