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

#ifndef APPLICATION_TOOLS_FLIPTOOL_H
#define APPLICATION_TOOLS_FLIPTOOL_H

#include <Application/Tool/Tool.h>

namespace Seg3D {

class FlipTool : public Tool {
  SCI_TOOL_TYPE("FlipTool","Flip/Rotate","Alt+F",Tool::TOOL_E, QUrl::fromEncoded("http://seg3d.org/"))

// -- constructor/destructor --
  public:
    FlipTool(const std::string& toolid);
    virtual ~FlipTool();

// -- constraint parameters --

// -- handle updates from layermanager --    
    void handle_layers_changed();

// -- activate/deactivate tool --
  virtual void activate();
  virtual void deactivate();
  
// -- dispatch functions --

  void dispatch_flip_coronal() const;
  void dispatch_flip_sagittal() const;
  void dispatch_fip_axial() const;
  
  void dispatch_rotate90_coronal() const;
  void dispatch_rotate90_sagittal() const;
  void dispatch_rotate90_axial() const;

  void dispatch_negative_rotate90_coronal() const;
  void dispatch_negative_rotate90_sagittal() const;
  void dispatch_negative_rotate90_axial() const;
  
// -- state --
  public:
  
    // Layerid of the target layer
    StateOption::Handle              target_layer_;
    
};

} // end namespace

#endif
