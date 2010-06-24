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

// STL includes

// Boost includes 

// Core includes
#include <Core/Application/Application.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/Volume/MaskVolume.h>

// Application includes
#include <Application/Layer/MaskLayer.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/ProjectManager/ProjectManager.h>

namespace Seg3D
{

const size_t MaskLayer::version_number_ = 1;

MaskLayer::MaskLayer( const std::string& name, const Core::MaskVolumeHandle& volume ) :
  Layer( name, version_number_ ), 
  mask_volume_( volume )
{
  this->initialize_states();
  this->bit_state_->set( static_cast< int >( volume->mask_data_block()->get_mask_bit() ) );
}

MaskLayer::MaskLayer( const std::string& name, const Core::GridTransform& grid_transform ) :
    Layer( name, version_number_ ), 
  mask_volume_( new Core::MaskVolume( grid_transform ) )
{
    this->initialize_states();
  this->bit_state_->set( static_cast< int >( this->mask_volume_->
    mask_data_block()->get_mask_bit() ) );
}

MaskLayer::MaskLayer( const std::string& state_id ) :
  Layer( "not_initialized", version_number_, state_id )
{
  this->initialize_states();
}





MaskLayer::~MaskLayer()
{
  // Disconnect all current connections
  disconnect_all();
}

void MaskLayer::initialize_states()
{
    // Step (1) : Build the layer specific state variables

    // == Color of the layer ==
  

    add_state( "color", color_state_, static_cast< int >( color_count_ %  PreferencesManager::Instance()->
    color_states_.size() ) );

  color_count_++;

    // == What border is used ==
    add_state( "border", border_state_, PreferencesManager::Instance()->
    default_mask_border_state_->export_to_string(), PreferencesManager::Instance()->
    default_mask_border_state_->export_list_to_string() );

    // == How is the segmentation filled in ==
    add_state( "fill", fill_state_, PreferencesManager::Instance()->
    default_mask_fill_state_->export_to_string(), PreferencesManager::Instance()->
    default_mask_fill_state_->export_list_to_string() );

    // == Whether the isosurface is shown in the volume display ==
    add_state( "isosurface", show_isosurface_state_, false );

  add_state( "bit", this->bit_state_, 0 );
    
}

Core::AtomicCounter MaskLayer::color_count_;
bool MaskLayer::pre_save_states()
{
  this->generation_state_->set( static_cast< int >( this->get_mask_volume()->
    mask_data_block()->get_data_block()->get_generation() ) );
  return true;
}

bool MaskLayer::post_load_states()
{
  Core::DataBlock::generation_type generation = this->generation_state_->get();
  unsigned int bit = static_cast< unsigned int >( this->bit_state_->get() );
  Core::MaskDataBlockHandle mask_data_block;
  Core::GridTransform grid_transform;
  bool success = Core::MaskDataBlockManager::Instance()->
    create( generation, bit, grid_transform, mask_data_block );
  if ( !success )
  {
    Core::DataVolumeHandle data_volume;
    boost::filesystem::path volume_path = ProjectManager::Instance()->get_project_data_path() /
      ( this->generation_state_->export_to_string() + ".nrrd" );
    std::string error;

    if( Core::DataVolume::LoadDataVolume( volume_path, data_volume, error ) )
    {
      Core::MaskDataBlockManager::Instance()->register_data_block( data_volume->data_block(),
        data_volume->get_grid_transform() );
      success = Core::MaskDataBlockManager::Instance()->
        create( generation, bit, grid_transform, mask_data_block );
    }
  }

  if ( success )
  {
    this->mask_volume_ = Core::MaskVolumeHandle( new Core::MaskVolume( 
      grid_transform, mask_data_block ) );
  }
  
  return success;
}



} // end namespace Seg3D

