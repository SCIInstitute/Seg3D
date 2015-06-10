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
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/State/StateIO.h>
#include <Core/Utils/AtomicCounter.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Log.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class LayerPrivate
//////////////////////////////////////////////////////////////////////////

class LayerPrivate : public Core::ConnectionHandler
{
public:
  void handle_locked_state_changed( bool locked );
  void handle_data_state_changed( std::string data_state );
  void handle_visible_state_changed( size_t viewer_id, bool visible );

  // Function for setting abort flag
  void handle_abort();
  
  // Function for setting stop flag
  void handle_stop();

public:
  // Pointer back to the main class
  Layer* layer_;

  // Handle to the layer group (this one needs to be weak to ensure objects are not persistent
  // due to a circular dependency)
  LayerGroupWeakHandle layer_group_;

  // Identifier for keeping track which data_processing action is running. 
  std::set<Layer::filter_key_type> keys_;
  
  // WeakHandle to the algorithm that is currently creating or processing this layer
  LayerAbstractFilterWeakHandle filter_;
  
  // Abort flag
  bool abort_;

  // Stop processing flag
  bool stop_;
};

void LayerPrivate::handle_locked_state_changed( bool locked )
{
  if ( locked && !this->layer_->show_information_state_->get() )
  {
    this->layer_->gui_state_group_->clear_selection();
  }
}

void LayerPrivate::handle_data_state_changed( std::string data_state )
{
  bool show_progress = ( data_state == Layer::CREATING_C ||
    data_state == Layer::PROCESSING_C );
  this->layer_->show_progress_bar_state_->set( show_progress );
  if ( show_progress )
  {
    this->layer_->update_progress_signal_( 0.0 );
  }
  this->layer_->show_abort_message_state_->set( false );
}

void LayerPrivate::handle_visible_state_changed( size_t viewer_id, bool visible )
{
  if ( ViewerManager::Instance()->get_viewer( viewer_id )->
    viewer_visible_state_->get() && visible )
  {
    this->layer_->master_visible_state_->set( true );
  }
}

void LayerPrivate::handle_abort()
{
  Layer::lock_type lock( Layer::GetMutex() );
  this->abort_ = true;
}

void LayerPrivate::handle_stop()
{
  Layer::lock_type lock( Layer::GetMutex() );
  this->stop_ = true;
}

//////////////////////////////////////////////////////////////////////////
// Class Layer
//////////////////////////////////////////////////////////////////////////

const std::string Layer::CREATING_C( "creating" );
const std::string Layer::PROCESSING_C( "processing" );
const std::string Layer::AVAILABLE_C( "available" );
const std::string Layer::IN_USE_C( "inuse" );

Layer::Layer( const std::string& name, bool creating ) :
  StateHandler( "layer",  true ),
  private_( new LayerPrivate )
{ 
  this->initialize_states( name, creating );
}

Layer::Layer( const std::string& name, const std::string& state_id, bool creating ) :
  StateHandler( state_id, true ),
  private_( new LayerPrivate )
{
  this->initialize_states( name, creating );
}
  
Layer::~Layer()
{
  this->private_->disconnect_all();
  // Disconnect all current connections
  this->disconnect_all();
}

LayerGroupHandle Layer::get_layer_group() 
{ 
  return this->private_->layer_group_.lock(); 
}
  
void Layer::set_layer_group( LayerGroupWeakHandle layer_group )
{
  this->private_->layer_group_ = layer_group;
}

std::string Layer::get_layer_id() const
{
  return this->get_statehandler_id();
}
  
std::string Layer::get_layer_name() const
{
  return this->name_state_->get();
}
  
Core::DataBlock::generation_type Layer::get_generation() const
{
  return this->generation_state_->get();
}

Layer::mutex_type& Layer::GetMutex()
{
  return Core::StateEngine::GetMutex();
}

void Layer::initialize_states( const std::string& name, bool creating )
{
  // Setup the default values for private class
  this->private_->layer_ = this;

  this->private_->abort_ = false;
  this->private_->stop_ = false;
  
  //  Build the layer specific state variables

  // Ensure that the states are encoded as states that change the project
  this->mark_as_project_data();
  
  // == The name of the layer ==
  this->add_state( "name", name_state_, name );

  // == Visibility information for this layer per viewer ==
  this->visible_state_.resize( ViewerManager::Instance()->number_of_viewers() );
  for ( size_t j = 0; j < this->visible_state_.size(); j++ )
  {
    this->add_state( std::string( "visible" ) + Core::ExportToString( j ), 
      this->visible_state_[ j ], true );
  }

  this->add_state( "visible", this->master_visible_state_, true );
  this->master_visible_state_->set_is_project_data( false );

  // == The state of the lock ==
  this->add_state( "visual_lock", this->locked_state_, false );
  this->locked_state_->set_is_project_data( false );

  // == The opacity of the layer ==
  this->add_state( "opacity", opacity_state_, 
    PreferencesManager::Instance()->default_layer_opacity_state_->get(), 0.0, 1.0, 0.01 );

  // == The generation number of the data, or -1 if there is no data ==
  this->add_state( "generation", this->generation_state_, -1 );

  // == The generation number of the data, or -1 if there is no data ==
  this->add_state( "provenance_id", this->provenance_id_state_, -1 );
  
  // == The meta data for this dataset ==
  this->add_state( "metadata", this->meta_data_state_, "" );

  // == Information string for the metadata of this dataset ==
  this->add_state( "metadata_info", this->meta_data_info_state_, "" );
  
  // == The layer state indicating whether data is bein processed ==
  this->add_state( "data", this->data_state_,  creating ? CREATING_C : AVAILABLE_C  , 
    AVAILABLE_C + "|" + CREATING_C + "|" + PROCESSING_C + "|" + IN_USE_C );
  this->data_state_->set_is_project_data( false );
  
  // == Centering (node vs. cell) ==
  this->add_state( "centering", this->centering_state_, "node" );
  this->centering_state_->set_is_project_data( false );

  this->add_state( "show_info", this->show_information_state_, false );
  this->add_state( "show_advanced_visibility", this->show_advanced_visibility_state_, false );
  this->add_state( "show_opacity", this->show_opacity_state_, false );
  this->add_state( "show_appearance", this->show_appearance_state_, false );
  this->add_state( "show_progress", this->show_progress_bar_state_, creating );
  this->add_state( "show_abort", this->show_abort_message_state_, false );
  this->add_state( "show_stop_button", this->show_stop_button_state_, false );
  
  this->show_information_state_->set_is_project_data( false );
  this->show_advanced_visibility_state_->set_is_project_data( false );
  this->show_opacity_state_->set_is_project_data( false );
  this->show_appearance_state_->set_is_project_data( false );
  this->show_progress_bar_state_->set_is_project_data( false );
  this->show_abort_message_state_->set_is_project_data( false );
  this->show_stop_button_state_->set_is_project_data( false );

  // These states do not need to be loaded from file -- just reset to defaults.
  this->data_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );
  this->show_abort_message_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );
  this->show_progress_bar_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );

  // NOTE: Use the private class to keep track of the connections so they won't
  // be accidentally deleted by subclasses.
  this->private_->add_connection( this->locked_state_->value_changed_signal_.connect(
    boost::bind( &LayerPrivate::handle_locked_state_changed, this->private_, _1 ) ) );
  this->private_->add_connection( this->data_state_->value_changed_signal_.connect( 
    boost::bind( &LayerPrivate::handle_data_state_changed, this->private_, _1 ) ) );

  // Make sure handle_abort is called when the signal is triggered
  this->private_->add_connection( this->abort_signal_.connect( boost::bind(
    &LayerPrivate::handle_abort, this->private_ ) ) );

  this->private_->add_connection( this->stop_signal_.connect( boost::bind(
    &LayerPrivate::handle_stop, this->private_ ) ) );

  size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
  for ( size_t i = 0; i < num_of_viewers; ++i )
  {
    this->private_->add_connection( this->visible_state_[ i ]->value_changed_signal_.connect(
      boost::bind( &LayerPrivate::handle_visible_state_changed, this->private_, i, _1 ) ) );
  }

  this->gui_state_group_.reset( new Core::BooleanStateGroup );
  this->gui_state_group_->add_boolean_state( this->show_information_state_ );
  this->gui_state_group_->add_boolean_state( this->show_advanced_visibility_state_ );
  this->gui_state_group_->add_boolean_state( this->show_opacity_state_ );
  this->gui_state_group_->add_boolean_state( this->show_appearance_state_ );
}

bool Layer::post_save_states( Core::StateIO& state_io )
{
  TiXmlElement* layer_element = state_io.get_current_element();
  assert( this->get_statehandler_id() == layer_element->Value() );
  std::string layer_type;
  switch ( this->get_type() )
  {
  case Core::VolumeType::DATA_E:
    layer_type = "data";
    break;
  case Core::VolumeType::MASK_E:
    layer_type = "mask";
    break;
  case Core::VolumeType::LABEL_E:
    layer_type = "label";
    break;
  case Core::VolumeType::LARGE_DATA_E:
    layer_type = "large";
    break;
  default:
    CORE_LOG_ERROR( "Unknown layer type" );
    assert( false );
  }
  
  layer_element->SetAttribute( "type", layer_type );
  return true;
}

void Layer::update_progress( double amount, double progress_start /*= 0.0f*/, double progress_amount /*= 1.0f */ )
{
  this->update_progress_signal_( progress_start + amount * progress_amount );
}

bool Layer::is_visible( size_t viewer_id ) const
{
  lock_type lock( Layer::GetMutex() );

  return this->master_visible_state_->get() && this->visible_state_[ viewer_id ]->get();
}

LayerMetaData Layer::get_meta_data() const
{
  LayerMetaData meta_data;
  meta_data.meta_data_ = this->meta_data_state_->get();
  meta_data.meta_data_info_ = this->meta_data_info_state_->get();
  return meta_data;
}

void Layer::set_meta_data( const LayerMetaData& meta_data )
{
  this->meta_data_state_->set( meta_data.meta_data_ );
  this->meta_data_info_state_->set( meta_data.meta_data_info_ );
}

bool Layer::check_filter_key( filter_key_type key ) const
{
  // key 0 is always assumed to be there
  if ( key == 0 ) return true;
  return ( this->private_->keys_.find( key ) != this->private_->keys_.end() );
}

void Layer::add_filter_key( filter_key_type key )
{
  if ( key == 0 ) return; 
  this->private_->keys_.insert( key );
}

void Layer::remove_filter_key( filter_key_type key )
{
  if ( key == 0 ) return; 
  this->private_->keys_.erase( key );
}

void Layer::clear_filter_keys()
{
  this->private_->keys_.clear();
}

size_t Layer::num_filter_keys() const
{
  return this->private_->keys_.size();
}

void Layer::set_filter_handle( LayerAbstractFilterHandle layer )
{
  this->private_->filter_ = LayerAbstractFilterWeakHandle( layer );
}
  
void Layer::reset_filter_handle()
{
  this->private_->filter_.reset();
}

LayerAbstractFilterHandle Layer::get_filter_handle()
{
  return this->private_->filter_.lock();
}

Layer::filter_key_type Layer::GenerateFilterKey()
{
  static boost::mutex filter_key_mutex;
  static long long filter_key_count = 1;

  boost::unique_lock<boost::mutex> lock( filter_key_mutex );
  return filter_key_count++;
}

void Layer::reset_abort()
{
  lock_type lock( GetMutex() );
  this->private_->abort_ = false;
}

bool Layer::check_abort()
{
  lock_type lock( GetMutex() );
  return this->private_->abort_;
}

void Layer::reset_stop()
{
  lock_type lock( GetMutex() );
  this->private_->stop_ = false;
}

bool Layer::check_stop()
{
  lock_type lock( GetMutex() );
  return this->private_->stop_;
}

void Layer::set_allow_stop()
{
  this->show_stop_button_state_->set( true );
}

void Layer::reset_allow_stop()
{
  this->show_stop_button_state_->set( false );
}

int Layer::get_version()
{
  return 2;
}

} // end namespace Seg3D
