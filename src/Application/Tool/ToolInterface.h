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

#ifndef APPLICATION_TOOL_TOOLINTERFACE_H
#define APPLICATION_TOOL_TOOLINTERFACE_H

#include <Application/Tool/Tool.h>

namespace Seg3D {

// CLASS TOOLINTERFACE:
// Base class of each tool interface. These are not included inside the tool
// to ensure that the GUI is propery separated from the application.

class ToolInterface : public boost::noncopyable {

// -- constructor/destructor --
  public:
    ToolInterface();
    virtual ~ToolInterface();

// Store the parent tool this interface is intended for.
  public:
  
    // SET_TOOL
    // Insert the tool into the interface
    void set_tool(ToolHandle tool) { tool_ = tool; }

    // TOOL
    // Get the current tool handle stored in this interface class
    ToolHandle tool() const        { return tool_; }
    
    // TOOLID
    // This is a shortcut function to the toolid this interface is associated
    // with
    std::string toolid() const     { return tool_->toolid(); }
  
// -- internals of this class --  
  protected:
    // Handle to the underlying tool
    ToolHandle tool_;
};

}

#endif
