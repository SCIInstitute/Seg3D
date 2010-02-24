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

#ifndef APPLICATION_LAYER_MASKLAYER_H
#define APPLICATION_LAYER_MASKLAYER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Application includes
#include <Application/Layer/Layer.h>

namespace Seg3D {

// CLASS MaskLayer

// Forward declarations
class MaskLayer;
typedef boost::shared_ptr<MaskLayer> MaskLayerHandle;

// Class definition
class MaskLayer : public Layer {
    
// -- constructor/destructor --    
  public:

    MaskLayer(const std::string& name, const Utils::VolumeHandle& volume);
    virtual ~MaskLayer();

// -- state variables --
  public:

    // Which color to use for displaying the mask
    StateRangedIntHandle color_state_;
    
    // State that describes whether to use a fat border for the mask
    StateOptionHandle    border_state_;

    // State that describes whether to fill the mask solid
    StateOptionHandle    fill_state_;
        
    // State that describes whether to show the  isosurface state
    StateBoolHandle      show_isosurface_state_;
    
};

} // end namespace Seg3D

#endif
