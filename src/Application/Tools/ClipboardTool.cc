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

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Volume/VolumeSlice.h>
#include <Core/Volume/MaskVolumeSlice.h>

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/ClipboardTool.h>
#include <Application/Tools/Actions/ActionCopy.h>
#include <Application/Tools/Actions/ActionPaste.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/StatusBar/StatusBar.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, ClipboardTool )

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class ClipboardToolPrivate
//////////////////////////////////////////////////////////////////////////

class ClipboardToolPrivate
{
public:
  void update_slice_numbers();

  void handle_active_viewer_changed( int active_viewer );
  void handle_viewer_slice_changed( size_t viewer_id, int slice_num );
  void handle_use_active_viewer_changed( bool use_active_viewer );

  ClipboardTool* tool_;
};

void ClipboardToolPrivate::update_slice_numbers()
{
  ASSERT_IS_APPLICATION_THREAD();

  if ( this->tool_->target_layer_state_->get() == Tool::NONE_OPTION_C )
  {
    return;
  }

  bool zero_based = PreferencesManager::Instance()->zero_based_slice_numbers_state_->get();
  
  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  std::string slice_type_str = this->tool_->slice_type_state_->get();
  if ( slice_type_str == ClipboardTool::CORONAL_C )
  {
    slice_type = Core::VolumeSliceType::CORONAL_E;
  }
  else if ( slice_type_str == ClipboardTool::SAGITTAL_C )
  {
    slice_type = Core::VolumeSliceType::SAGITTAL_E;
  }

  MaskLayerHandle layer = boost::dynamic_pointer_cast< MaskLayer >( 
    LayerManager::Instance()->find_layer_by_id( this->tool_->target_layer_state_->get() ) );
  Core::MaskVolumeSliceHandle vol_slice( new Core::MaskVolumeSlice( 
    layer->get_mask_volume(), slice_type ) );
  int min_slice = zero_based ? 0 : 1;
  int max_slice = static_cast< int >( vol_slice->number_of_slices() );
  if ( zero_based ) max_slice -= 1;
  this->tool_->copy_slice_number_state_->set_range( min_slice, max_slice );
  this->tool_->paste_min_slice_number_state_->set_range( min_slice, max_slice );
  this->tool_->paste_min_slice_number_state_->set( min_slice );
  this->tool_->paste_max_slice_number_state_->set_range( min_slice, max_slice );
  this->tool_->paste_max_slice_number_state_->set( max_slice );

  if ( this->tool_->use_active_viewer_state_->get() )
  {
    ViewerHandle viewer = ViewerManager::Instance()->get_active_viewer();
    if ( !viewer->is_volume_view() )
    {
      int slice_num = viewer->slice_number_state_->get();
      if ( !zero_based )  slice_num += 1;
      this->tool_->copy_slice_number_state_->set( slice_num );
    }
  }
}

void ClipboardToolPrivate::handle_active_viewer_changed( int active_viewer )
{
  if ( !this->tool_->use_active_viewer_state_->get() )
  {
    return;
  }
  
  size_t viewer_id = static_cast< size_t >( active_viewer );
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
  if ( !viewer->is_volume_view() )
  {
    this->handle_viewer_slice_changed( viewer_id, viewer->slice_number_state_->get() );
  }
}

void ClipboardToolPrivate::handle_viewer_slice_changed( size_t viewer_id, int slice_num )
{
  if ( !this->tool_->use_active_viewer_state_->get() )
  {
    return;
  }

  size_t active_viewer = static_cast< size_t >( ViewerManager::Instance()->
    active_viewer_state_->get() );
  if ( viewer_id != active_viewer )
  {
    return;
  }

  if ( !PreferencesManager::Instance()->zero_based_slice_numbers_state_->get() )
  {
    slice_num += 1;
  }
  this->tool_->copy_slice_number_state_->set( slice_num );
}

void ClipboardToolPrivate::handle_use_active_viewer_changed( bool use_active_viewer )
{
  if ( !use_active_viewer )
  {
    return;
  }
  
  this->tool_->use_active_layer_state_->set( true );
  ViewerHandle viewer = ViewerManager::Instance()->get_active_viewer();
  if ( !viewer->is_volume_view() )
  {
    this->handle_viewer_slice_changed( viewer->get_viewer_id(), viewer->slice_number_state_->get() );
  }
}

//////////////////////////////////////////////////////////////////////////
// Class ClipboardTool
//////////////////////////////////////////////////////////////////////////

ClipboardTool::ClipboardTool( const std::string& toolid ) :
  SliceTargetTool( Core::VolumeType::MASK_E, toolid ),
  private_( new ClipboardToolPrivate )
{
  this->private_->tool_ = this;
      
  this->add_state( "copy_slice", this->copy_slice_number_state_, 1, 1, 1, 1 );
  this->add_state( "paste_min_slice", this->paste_min_slice_number_state_, 1, 1, 1, 1 );
  this->add_state( "paste_max_slice", this->paste_max_slice_number_state_, 1, 1, 1, 1 );

  this->private_->update_slice_numbers();
  this->private_->handle_use_active_viewer_changed( this->use_active_layer_state_->get() );

  this->add_connection( PreferencesManager::Instance()->zero_based_slice_numbers_state_->
    state_changed_signal_.connect( boost::bind( &ClipboardToolPrivate::update_slice_numbers,
    this->private_ ) ) );
  this->add_connection( this->target_layer_state_->state_changed_signal_.connect(
    boost::bind( &ClipboardToolPrivate::update_slice_numbers, this->private_ ) ) );
  this->add_connection( this->slice_type_state_->state_changed_signal_.connect(
    boost::bind( &ClipboardToolPrivate::update_slice_numbers, this->private_ ) ) );
    
  this->add_connection( this->use_active_viewer_state_->value_changed_signal_.connect( 
    boost::bind( &ClipboardToolPrivate::handle_use_active_viewer_changed, this->private_, _1 ) ) );
  this->add_connection( ViewerManager::Instance()->active_viewer_state_->value_changed_signal_.
    connect( boost::bind( &ClipboardToolPrivate::handle_active_viewer_changed, 
    this->private_, _1 ) ) );
  size_t num_of_viewrs = ViewerManager::Instance()->number_of_viewers();
  
  for ( size_t i = 0; i < num_of_viewrs; ++i )
  {
    ViewerHandle viewer = ViewerManager::Instance()->get_viewer( i );
    this->add_connection( viewer->slice_number_state_->value_changed_signal_.connect(
      boost::bind( &ClipboardToolPrivate::handle_viewer_slice_changed,
      this->private_, i, _1 ) ) );
  }
}

ClipboardTool::~ClipboardTool()
{
  this->disconnect_all();
}

void ClipboardTool::copy( Core::ActionContextHandle context )
{
  // NOTE: Post to the application thread to avoid sync issues
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &ClipboardTool::copy, this, context ) );
    return;
  }
  
  const std::string& layer_id = this->target_layer_state_->get();
  if ( layer_id == Tool::NONE_OPTION_C )
  {
    return;
  }
  
  int slice_num = this->copy_slice_number_state_->get();
  if ( !PreferencesManager::Instance()->zero_based_slice_numbers_state_->get() )
  {
    slice_num -= 1;
  }
  
  ActionCopy::Dispatch( context, layer_id, this->slice_type_state_->index(),
    static_cast< size_t >( slice_num ) );
}

void ClipboardTool::paste( Core::ActionContextHandle context )
{
  // NOTE: Post to the application thread to avoid sync issues
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &ClipboardTool::paste, this, context ) );
    return;
  }

  const std::string& layer_id = this->target_layer_state_->get();
  if ( layer_id == Tool::NONE_OPTION_C )
  {
    return;
  }

  int min_slice_num = this->paste_min_slice_number_state_->get();
  int max_slice_num = this->paste_max_slice_number_state_->get();
  if ( !PreferencesManager::Instance()->zero_based_slice_numbers_state_->get() )
  {
    min_slice_num -= 1;
    max_slice_num -= 1;
  }
  ActionPaste::Dispatch( context, layer_id, this->slice_type_state_->index(),
    static_cast< size_t >( min_slice_num ), static_cast< size_t >( max_slice_num ) );
}

void ClipboardTool::grab_min_paste_slice()
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &ClipboardTool::grab_min_paste_slice, this ) );
    return;
  }
  
  ViewerHandle viewer = ViewerManager::Instance()->get_active_viewer();
  if ( viewer->is_volume_view() )
  {
    CORE_LOG_ERROR( "Active viewer not in 2D view." );
    return;
  }

  int slice_num = viewer->slice_number_state_->get();
  if ( !PreferencesManager::Instance()->zero_based_slice_numbers_state_->get() )
  {
    slice_num += 1;
  }
  this->paste_min_slice_number_state_->set( slice_num );
}

void ClipboardTool::grab_max_paste_slice()
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &ClipboardTool::grab_max_paste_slice, this ) );
    return;
  }

  ViewerHandle viewer = ViewerManager::Instance()->get_active_viewer();
  if ( viewer->is_volume_view() )
  {
    CORE_LOG_ERROR( "Active viewer not in 2D view." );
    return;
  }

  int slice_num = viewer->slice_number_state_->get();
  if ( !PreferencesManager::Instance()->zero_based_slice_numbers_state_->get() )
  {
    slice_num += 1;
  }
  this->paste_max_slice_number_state_->set( slice_num );
}

} // end namespace Seg3D
