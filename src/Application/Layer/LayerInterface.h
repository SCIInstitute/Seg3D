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

#ifndef APPLICATION_LAYER_LAYERINTERFACE_H
#define APPLICATION_LAYER_LAYERINTERFACE_H

#include <Application/Layer/Layer.h>

namespace Seg3D {
  
  // CLASS LayerInterface:
  // Base class of each layer interface. These are not included inside the layer
  // to ensure that the GUI is propery separated from the application.
  
  
  class LayerInterface : public boost::noncopyable {
    
    // -- constructor/destructor --
  public:
    LayerInterface();
    virtual ~LayerInterface();
    
    // Store the parent layer this interface is intended for.
  public:
    
    void set_layer(LayerHandle layer) { layer_ = layer; }
    LayerHandle layer() const { return layer_; }
    LayerProperties properties() const { return layer_->get_properties(); }
    DataVolumeHandle data() const { return layer_-> get_data(); }
    bool get_locked() const { return layer_->is_locked(); }
        
  protected:
    LayerHandle layer_;
  };
  
} // end namespace Seg3D

#endif

