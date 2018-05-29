/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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
#include <limits>

// Boost includes 
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/State/StateIO.h>
#include <Core/Utils/Exception.h>
#include <Core/Utils/ScopedCounter.h>
#include <Core/Utils/Log.h>

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/Layer/LargeVolumeLayer.h>
#include <Application/PreferencesManager/PreferencesManager.h>

namespace Seg3D
{

class LargeVolumeLayerPrivate
{
public:
  void update_data_info();
  void update_display_value_range();

  void handle_contrast_brightness_changed();
  void handle_display_value_range_changed();

  LargeVolumeLayer* layer_;
  Core::LargeVolumeHandle volume_;
  size_t signal_block_count_;
};

void LargeVolumeLayerPrivate::update_data_info()
{
  if ( !this->volume_ ||
    !this->volume_->is_valid() )
  {
    this->layer_->data_type_state_->set( "unknown" );
    this->layer_->min_value_state_->set( std::numeric_limits< double >::quiet_NaN() );
    this->layer_->max_value_state_->set( std::numeric_limits< double >::quiet_NaN() );
    return;
  }
  
  this->layer_->centering_state_->set( 
    this->layer_->get_grid_transform().get_originally_node_centered() ? "node" : "cell" );

  switch ( this->layer_->get_data_type() )
  {
  case Core::DataType::CHAR_E:
    this->layer_->data_type_state_->set( "char" );
    break;
  case Core::DataType::UCHAR_E:
    this->layer_->data_type_state_->set( "unsigned char" );
    break;
  case Core::DataType::SHORT_E:
    this->layer_->data_type_state_->set( "short" );
    break;
  case Core::DataType::USHORT_E:
    this->layer_->data_type_state_->set( "unsigned short" );
    break;
  case Core::DataType::INT_E:
    this->layer_->data_type_state_->set( "int" );
    break;
  case Core::DataType::UINT_E:
    this->layer_->data_type_state_->set( "unsigned int" );
    break;
  case Core::DataType::FLOAT_E:
    this->layer_->data_type_state_->set( "float" );
    break;
  case Core::DataType::DOUBLE_E:
    this->layer_->data_type_state_->set( "double" );
    break;
  }
  
  this->layer_->min_value_state_->set( this->volume_->get_min() );
  this->layer_->max_value_state_->set( this->volume_->get_max() );
}

void LargeVolumeLayerPrivate::update_display_value_range()
{
  if ( !this->layer_->has_valid_data() )  return;
  
  {
    Core::ScopedCounter signal_block( this->signal_block_count_ );
    double min_val = this->volume_->get_min();
    double max_val = this->volume_->get_max();
    this->layer_->display_min_value_state_->set_range( min_val, max_val );
    this->layer_->display_max_value_state_->set_range( min_val, max_val );
  }

  this->handle_contrast_brightness_changed();
}

void LargeVolumeLayerPrivate::handle_contrast_brightness_changed()
{
  if ( this->signal_block_count_ > 0 || !this->layer_->has_valid_data() )
  {
    return;
  }
  
  Core::ScopedCounter signal_block( this->signal_block_count_ );

  // Convert contrast to range ( 0, 1 ] and brightness to [ 0, 2 ]
  double contrast = ( 1 - this->layer_->contrast_state_->get() / 101 );
  double brightness = this->layer_->brightness_state_->get() / 50.0;

  double min_val, max_val;
  this->layer_->display_min_value_state_->get_range( min_val, max_val );
  double mid_val = max_val - brightness * 0.5 * ( max_val - min_val );
  double window_size = ( max_val - min_val ) * contrast;
  this->layer_->display_min_value_state_->set( mid_val - window_size * 0.5 );
  this->layer_->display_max_value_state_->set( mid_val + window_size * 0.5 );
}

void LargeVolumeLayerPrivate::handle_display_value_range_changed()
{
  if ( this->signal_block_count_ > 0 || !this->layer_->has_valid_data() )
  {
    return;
  }

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  double min_val, max_val;
  this->layer_->display_min_value_state_->get_range( min_val, max_val );
  double display_min = this->layer_->display_min_value_state_->get();
  double display_max = this->layer_->display_max_value_state_->get();
  if ( display_min > display_max )
  {
    std::swap( display_min, display_max );
  }
  
  double contrast = 1.0 - ( display_max - display_min ) / ( max_val - min_val );
  double brightness = ( max_val * 2 - display_min - display_max ) / ( max_val - min_val );
  this->layer_->contrast_state_->set( contrast * 100 );
  this->layer_->brightness_state_->set( brightness * 50 );
}


LargeVolumeLayer::LargeVolumeLayer( const std::string& name, Core::LargeVolumeSchemaHandle schema ) :
  Layer( name ),
  private_( new LargeVolumeLayerPrivate )
{
  this->private_->volume_ = Core::LargeVolumeHandle( new Core::LargeVolume( schema ) );
  this->private_->layer_ = this;
  this->private_->signal_block_count_ = 0;
  this->initialize_states();
  this->dir_name_state_->set( schema->get_dir().string() );
  this->private_->update_display_value_range();
}

LargeVolumeLayer::LargeVolumeLayer( const std::string& name, Core::LargeVolumeSchemaHandle schema, 
  const Core::GridTransform& crop_trans ) :
  Layer( name ),
  private_( new LargeVolumeLayerPrivate )
{
  this->private_->volume_ = Core::LargeVolumeHandle( new Core::LargeVolume( schema, crop_trans ) );
  this->private_->layer_ = this;
  this->private_->signal_block_count_ = 0;
  this->initialize_states();
  this->dir_name_state_->set( schema->get_dir().string() );
  this->crop_volume_state_->set( true );
  this->cropped_grid_state_->set( crop_trans );
  this->private_->update_display_value_range();
}

LargeVolumeLayer::LargeVolumeLayer( const std::string& state_id ) :
  Layer( "not initialized", state_id ),
  private_( new LargeVolumeLayerPrivate )
{
  this->private_->layer_ = this;
  this->private_->signal_block_count_ = 0;
  this->initialize_states();
}

LargeVolumeLayer::~LargeVolumeLayer()
{
  // Disconnect all current connections
  this->disconnect_all();
}

void LargeVolumeLayer::initialize_states()
{
  // NOTE: This function allows setting of state variables outside of application thread
  this->set_initializing( true ); 

  this->add_state( "dir_name", this->dir_name_state_, "" );
     
  // == The brightness of the layer ==
  this->add_state( "brightness", brightness_state_, 50.0, 0.0, 100.0, 0.1 );

  // == The contrast of the layer ==
  this->add_state( "contrast", contrast_state_, 0.0, 0.0, 100.0, 0.1 );

  this->add_state( "display_min", this->display_min_value_state_, 0.0, 0.0, 1.0, 1.0 );
  this->display_min_value_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );
  this->add_state( "display_max", this->display_max_value_state_, 1.0, 0.0, 1.0, 1.0 );
  this->display_max_value_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );

  this->add_state( "adjust_minmax", this->adjust_display_min_max_state_, false );
  this->add_state("pick_color_bool", this->pick_color_state_, false);

  this->add_state( "data_type", this->data_type_state_, "unknown" );
  this->add_state( "min", this->min_value_state_, std::numeric_limits< double >::quiet_NaN() );
  this->add_state( "max", this->max_value_state_, std::numeric_limits< double >::quiet_NaN() );

  this->add_state( "crop_volume", this->crop_volume_state_, false );
  this->add_state( "cropped_grid", this->cropped_grid_state_, Core::GridTransform() );

  this->add_connection( this->contrast_state_->state_changed_signal_.connect( boost::bind(
    &LargeVolumeLayerPrivate::handle_contrast_brightness_changed, this->private_ ) ) );
  this->add_connection( this->brightness_state_->state_changed_signal_.connect( boost::bind(
    &LargeVolumeLayerPrivate::handle_contrast_brightness_changed, this->private_ ) ) );
  this->add_connection( this->display_min_value_state_->state_changed_signal_.connect( boost::bind(
    &LargeVolumeLayerPrivate::handle_display_value_range_changed, this->private_ ) ) );
  this->add_connection( this->display_max_value_state_->state_changed_signal_.connect( boost::bind(
    &LargeVolumeLayerPrivate::handle_display_value_range_changed, this->private_ ) ) );

  this->private_->update_data_info();



  this->set_initializing( false );
}

Core::GridTransform LargeVolumeLayer::get_grid_transform() const 
{ 
  Layer::lock_type lock( Layer::GetMutex() );

  if ( this->private_->volume_ )
  {
    return this->private_->volume_->get_grid_transform();
  }
  else
  {
    return Core::GridTransform();
  }
}

void LargeVolumeLayer::set_grid_transform( const Core::GridTransform& grid_transform, 
  bool preserve_centering )
{
  Layer::lock_type lock( Layer::GetMutex() );

  if ( this->private_->volume_ )
  {
    this->private_->volume_->set_grid_transform(grid_transform, preserve_centering);
  }
}

Core::DataType LargeVolumeLayer::get_data_type() const
{
  Layer::lock_type lock( Layer::GetMutex() );

  if (this->private_->volume_)
  {
    return this->private_->volume_->get_data_type();
  }
  
  return Core::DataType::UNKNOWN_E;
}

size_t LargeVolumeLayer::get_byte_size() const
{
  Core::IndexVector size = this->private_->volume_->get_schema()->get_size();
  Core::DataType type = this->get_data_type();

  return size[0] * size[1] * size[2] * Core::GetSizeDataType( type );
}

bool LargeVolumeLayer::has_valid_data() const
{
  return true;
}

Core::VolumeHandle LargeVolumeLayer::get_volume() const
{
  return this->private_->volume_;
}

bool LargeVolumeLayer::pre_save_states( Core::StateIO& state_io )
{ 
  return true;
}

bool LargeVolumeLayer::post_load_states( const Core::StateIO& state_io )
{
  Core::LargeVolumeSchemaHandle schema( new Core::LargeVolumeSchema);
  schema->set_dir( this->dir_name_state_->get() );
  std::string error;
  
  if (! schema->load( error) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  if (this->crop_volume_state_->get())
  {
    this->private_->volume_ = Core::LargeVolumeHandle( new Core::LargeVolume( schema, this->cropped_grid_state_->get() ) );
  }
  else
  {
    this->private_->volume_ = Core::LargeVolumeHandle( new Core::LargeVolume( schema ) );
  }

  this->private_->update_data_info();
  this->private_->update_display_value_range();

  return true;
}
  
void LargeVolumeLayer::clean_up()
{
  // Abort any filter still using this layer
  this->abort_signal_();
    
  // Remove all the connections
  this->disconnect_all();   
}

LayerHandle LargeVolumeLayer::duplicate() const
{
  CORE_THROW_NOTIMPLEMENTEDERROR("LargeVolumeLayer does not implement duplicate.");
}

Core::LargeVolumeSchemaHandle LargeVolumeLayer::get_schema() const
{
  return this->private_->volume_->get_schema();
}

} // end namespace Seg3D

