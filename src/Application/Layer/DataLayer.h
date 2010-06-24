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

#ifndef APPLICATION_LAYER_DATALAYER_H
#define APPLICATION_LAYER_DATALAYER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Application includes
#include <Application/Layer/Layer.h>
#include <Core/Volume/DataVolume.h>

namespace Seg3D
{

// CLASS DataLayer

// Forward declarations
class DataLayer;
typedef boost::shared_ptr< DataLayer > DataLayerHandle;

// Class definition
class DataLayer : public Layer
{

  // -- constructor/destructor --
public:

  DataLayer( const std::string& name, const Core::DataVolumeHandle& volume );
  DataLayer( const std::string& state_id );
  virtual ~DataLayer();

  virtual Core::VolumeType type() const { return Core::VolumeType::DATA_E; }

  virtual const Core::GridTransform& get_grid_transform() const 
  { 
    return data_volume_->get_grid_transform(); 
  }
  
  Core::DataVolumeHandle get_data_volume()
  {
    return this->data_volume_;
  }

  void set_data_volume( Core::DataVolumeHandle data_volume )
  { 
    this->data_volume_ = data_volume; 
  } 
  
  // -- state variables --
public:

  // State describing contrast
  Core::StateRangedDoubleHandle contrast_state_;

  // State describing brightness
  Core::StateRangedDoubleHandle brightness_state_;

  // State describing whether volume is volume rendered
  Core::StateBoolHandle volume_rendered_state_;

protected:
  // PRE_SAVE_STATES:
  // this function synchronize the generation number for the session saving
  virtual bool pre_save_states();

  virtual bool post_load_states();

private:
  void initialize_states();

private:
  Core::DataVolumeHandle data_volume_;
  const static size_t version_number_;
};

} // end namespace Seg3D

#endif
