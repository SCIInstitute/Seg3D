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
#include <Core/Utils/Exception.h>
#include <Core/State/State.h>
#include <Core/State/StateEngine.h>

// Application includes
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/Tool/SliceTargetTool.h>
#include <Application/PreferencesManager/PreferencesManager.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class SliceTargetToolPrivate
//////////////////////////////////////////////////////////////////////////

class SliceTargetToolPrivate
{
public:
  void update_slice_type_labels();

  void handle_active_viewer_changed( int active_viewer );
  void handle_viewer_mode_changed( size_t viewer_id, std::string mode );
  void handle_use_active_viewer_changed( bool use_active_viewer );

  SliceTargetTool* tool_;
};

void SliceTargetToolPrivate::update_slice_type_labels()
{
  Core::OptionLabelPairVector label_options;
  label_options.push_back( std::make_pair( SliceTargetTool::AXIAL_C, 
    PreferencesManager::Instance()->z_axis_label_state_->get() ) );
  label_options.push_back( std::make_pair( SliceTargetTool::CORONAL_C, 
    PreferencesManager::Instance()->y_axis_label_state_->get() ) );
  label_options.push_back( std::make_pair( SliceTargetTool::SAGITTAL_C, 
    PreferencesManager::Instance()->x_axis_label_state_->get() ) );

  this->tool_->slice_type_state_->set_option_list( label_options );
}

void SliceTargetToolPrivate::handle_active_viewer_changed( int active_viewer )
{
  if ( !this->tool_->use_active_viewer_state_->get() )
  {
    return;
  }

  size_t viewer_id = static_cast< size_t >( active_viewer );
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
  if ( !viewer->is_volume_view() )
  {
    this->handle_viewer_mode_changed( viewer_id, viewer->view_mode_state_->get() );
  }
}

void SliceTargetToolPrivate::handle_viewer_mode_changed( size_t viewer_id, std::string mode )
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

  if ( mode == Viewer::AXIAL_C )
  {
    this->tool_->slice_type_state_->set( SliceTargetTool::AXIAL_C );
  }
  else if ( mode == Viewer::CORONAL_C )
  {
    this->tool_->slice_type_state_->set( SliceTargetTool::CORONAL_C );
  }
  else if ( mode == Viewer::SAGITTAL_C )
  {
    this->tool_->slice_type_state_->set( SliceTargetTool::SAGITTAL_C );
  }
}

void SliceTargetToolPrivate::handle_use_active_viewer_changed( bool use_active_viewer )
{
  if ( !use_active_viewer )
  {
    return;
  }

  ViewerHandle viewer = ViewerManager::Instance()->get_active_viewer();
  if ( !viewer->is_volume_view() )
  {
    this->handle_viewer_mode_changed( viewer->get_viewer_id(), viewer->view_mode_state_->get() );
  }
}

//////////////////////////////////////////////////////////////////////////
// Class SliceTargetTool
//////////////////////////////////////////////////////////////////////////

const std::string SliceTargetTool::AXIAL_C( "axial" );
const std::string SliceTargetTool::CORONAL_C( "coronal" );
const std::string SliceTargetTool::SAGITTAL_C( "sagittal" );

SliceTargetTool::SliceTargetTool(  int target_type, const std::string& tool_type ) :
  SingleTargetTool( target_type, tool_type ),
  private_( new SliceTargetToolPrivate )
{
  this->private_->tool_ = this;

  std::string sagittal = SAGITTAL_C + "=" + PreferencesManager::Instance()->x_axis_label_state_->get();
  std::string coronal = CORONAL_C + "=" + PreferencesManager::Instance()->y_axis_label_state_->get();
  std::string axial = AXIAL_C + "=" + PreferencesManager::Instance()->z_axis_label_state_->get();

  this->add_state( "slice_type", this->slice_type_state_, AXIAL_C,  axial + "|" + coronal 
    + "|" + sagittal );

  this->add_connection( PreferencesManager::Instance()->x_axis_label_state_->state_changed_signal_.
    connect( boost::bind( &SliceTargetToolPrivate::update_slice_type_labels, this->private_ ) ) );
  this->add_connection( PreferencesManager::Instance()->y_axis_label_state_->state_changed_signal_.
    connect( boost::bind( &SliceTargetToolPrivate::update_slice_type_labels, this->private_ ) ) );
  this->add_connection( PreferencesManager::Instance()->z_axis_label_state_->state_changed_signal_.
    connect( boost::bind( &SliceTargetToolPrivate::update_slice_type_labels, this->private_ ) ) );

  this->add_state( "use_active_viewer", this->use_active_viewer_state_, true );

  this->private_->handle_use_active_viewer_changed( true );

  this->add_connection( this->use_active_viewer_state_->value_changed_signal_.connect( 
    boost::bind( &SliceTargetToolPrivate::handle_use_active_viewer_changed, this->private_, _1 ) ) );
  this->add_connection( ViewerManager::Instance()->active_viewer_state_->value_changed_signal_.
    connect( boost::bind( &SliceTargetToolPrivate::handle_active_viewer_changed, 
    this->private_, _1 ) ) );
  size_t num_of_viewrs = ViewerManager::Instance()->number_of_viewers();

  for ( size_t i = 0; i < num_of_viewrs; ++i )
  {
    ViewerHandle viewer = ViewerManager::Instance()->get_viewer( i );
    this->add_connection( viewer->view_mode_state_->value_changed_signal_.connect( 
      boost::bind( &SliceTargetToolPrivate::handle_viewer_mode_changed, 
      this->private_, i, _2 ) ) );
  }
}

SliceTargetTool::~SliceTargetTool()
{
}

} // end namespace Seg3D
