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

#ifndef APPLICATION_LAYER_LAYER_H
#define APPLICATION_LAYER_LAYER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// STL includes
#include <string>
#include <deque>
#include <vector>

// Boost includes 
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

// DataVolume includes
//#include <DataVolume/DataVolume.h>

// Application includes
#include <Application/Application/Application.h>
#include <Application/Interface/Interface.h>
#include <Application/Action/Action.h>
#include <Application/State/State.h>

namespace Seg3D {

// Forward declarations
class Layer;
typedef boost::shared_ptr<Layer> LayerHandle;

class Layer : public StateHandler {
    
// -- constructor/destructor --    
  public:

    Layer(std::string& name);
    virtual ~Layer();

// -- state variables --
  public:
    // The name of the layer 
    StateStringHandle               name_state_;
    
    StateBoolHandle                 lock_state_;
    StateIntHandle                  color_index_state_;

    StateDoubleHandle               contrast_state_;
    StateDoubleHandle               brightness_state_;
    
    // Per viewer control of state
    std::vector<StateDoubleHandle>  opacity_state_;
    std::vector<StateBoolHandle>    visibility_state_;
    
    std::vector<StateBoolHandle>    border_state_;
    std::vector<StateBoolHandle>    fill_state_;
    
};

} // end namespace Seg3D

#endif
