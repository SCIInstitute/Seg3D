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

/*
 *****************************************************************************
 *
 *   Layer.h
 *
 *   Also see: 
 *
 *   Authors:
 *      Kristen Zygmunt   -- initial attempt      11/10/2009
 *
 *    
 *****************************************************************************
 */

#ifndef APPLICATION_LAYER_LAYER_H
#define APPLICATION_LAYER_LAYER_H 1

//#ifdef (_MSC_VER) && (_MSC_VER >= 1020)
//# pragma once
//#endif

// STL includes
#include <string>
#include <deque>

// Boost includes 
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

// Application includes
//#include <DataVolume/DataVolume.h>
#include <Utils/Stubs/Stubs.h>

namespace Seg3D {

// Forward declarations
class Layer;

// typedefs
typedef boost::shared_ptr<Layer> LayerHandle;
typedef std::deque<LayerHandle> LayerGroup;

// Class declarations
class LayerProperties {
  public:
    enum {
      NONE_E = 0x0000,
      // MASKLAYER is a type of layer with 0/1 data
      MASKLAYER_E = 0x0001,
      // DATAGRAY is a layer with grayscale data
      DATAGRAY_E = 0x0002,
      // DATARGB is a layer with RGB data
      DATARGB_E = 0x0004,
      // ACTIVE is a layer that is currently active
      ACTIVE_E = 0x0008
    };
    
    int kind_;
    Color label_color_; 
    BoundingBox bbox_;
    bool operator==(const LayerProperties& other) {
      return (bbox_ == other.bbox_);
    }
};

class Layer {
    
  public:

    Layer(std::string& name, DataVolumeHandle& data);
    ~Layer();

    LayerProperties get_properties() const { return properties_; }
    DataVolumeHandle get_data() const { return data_handle_; }
    LayerHandle clone(std::string& clone_name) const;

    bool is_locked() const;

  private:
    std::string name_;
    boost::mutex thread_lock_; // used for indicating whether or not the layer is being looked at by another thread
    boost::mutex user_lock_; // used for indicating whether the user has said this thread cannot be modified
    bool dirty_;
    DataVolumeHandle data_handle_; // data volume has a lock for whether or not the data can be viewed/accessed
    //Isosurface iso_; // belongs in viewer
    MaskIndex mask_index_;
    LayerProperties properties_;
};

} // end namespace Seg3D

#endif
