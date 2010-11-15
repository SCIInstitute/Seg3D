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

#include <Core/Volume/VolumeSlice.h>
#include <Core/Volume/MaskVolumeSlice.h>

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/ClipboardTool.h>
#include <Application/Tools/Actions/ActionCopy.h>
#include <Application/Tools/Actions/ActionPaste.h>
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>

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

  ClipboardTool* tool_;
};

void ClipboardToolPrivate::update_slice_numbers()
{
  ASSERT_IS_APPLICATION_THREAD();

  if ( this->tool_->target_layer_state_->get() == Tool::NONE_OPTION_C )
  {
    return;
  }
  
  Core::VolumeSliceType slice_type( static_cast< Core::VolumeSliceType::enum_type >(
    this->tool_->slice_type_state_->index() ) );
  MaskLayerHandle layer = boost::dynamic_pointer_cast< MaskLayer >( 
    LayerManager::Instance()->get_layer_by_id( this->tool_->target_layer_state_->get() ) );
  Core::MaskVolumeSliceHandle vol_slice( new Core::MaskVolumeSlice( 
    layer->get_mask_volume(), slice_type ) );
  this->tool_->copy_slice_number_state_->set_range( 1, static_cast< int >(
    vol_slice->number_of_slices() ) );
  this->tool_->paste_min_slice_number_state_->set_range( 1, static_cast< int >(
    vol_slice->number_of_slices() ) );
  this->tool_->paste_max_slice_number_state_->set_range( 1, static_cast< int >(
    vol_slice->number_of_slices() ) );
}

//////////////////////////////////////////////////////////////////////////
// Class ClipboardTool
//////////////////////////////////////////////////////////////////////////

const std::string ClipboardTool::AXIAL_C( "axial" );
const std::string ClipboardTool::CORONAL_C( "coronal" );
const std::string ClipboardTool::SAGITTAL_C( "sagittal" );


ClipboardTool::ClipboardTool( const std::string& toolid ) :
  SingleTargetTool( Core::VolumeType::MASK_E, toolid ),
  private_( new ClipboardToolPrivate )
{
  this->private_->tool_ = this;
  
  std::string sagittal = SAGITTAL_C + "=" + PreferencesManager::Instance()->x_axis_label_state_->get();
  std::string coronal = CORONAL_C + "=" + PreferencesManager::Instance()->y_axis_label_state_->get();
  std::string axial = AXIAL_C + "=" + PreferencesManager::Instance()->z_axis_label_state_->get();

  this->add_state( "slice_type", this->slice_type_state_, "axial",  sagittal + "|" + coronal 
    + "|" + axial );
    
  this->add_connection( PreferencesManager::Instance()->x_axis_label_state_->state_changed_signal_.
    connect( boost::bind( &ClipboardTool::set_slice_type_labels, this ) ) );
  this->add_connection( PreferencesManager::Instance()->y_axis_label_state_->state_changed_signal_.
    connect( boost::bind( &ClipboardTool::set_slice_type_labels, this ) ) );
  this->add_connection( PreferencesManager::Instance()->z_axis_label_state_->state_changed_signal_.
    connect( boost::bind( &ClipboardTool::set_slice_type_labels, this ) ) );  
    
  this->add_state( "copy_slice", this->copy_slice_number_state_, 1, 1, 1, 1 );
  this->add_state( "paste_min_slice", this->paste_min_slice_number_state_, 1, 1, 1, 1 );
  this->add_state( "paste_max_slice", this->paste_max_slice_number_state_, 1, 1, 1, 1 );

  this->add_connection( this->target_layer_state_->state_changed_signal_.connect(
    boost::bind( &ClipboardToolPrivate::update_slice_numbers, this->private_ ) ) );
  this->add_connection( this->slice_type_state_->state_changed_signal_.connect(
    boost::bind( &ClipboardToolPrivate::update_slice_numbers, this->private_ ) ) );

  this->private_->update_slice_numbers();
}

ClipboardTool::~ClipboardTool()
{
  this->disconnect_all();
}

void ClipboardTool::set_slice_type_labels()
{
  Core::OptionLabelPairVector label_options;
  label_options.push_back( std::make_pair( SAGITTAL_C, 
    PreferencesManager::Instance()->x_axis_label_state_->get() ) );
  label_options.push_back( std::make_pair( CORONAL_C, 
    PreferencesManager::Instance()->y_axis_label_state_->get() ) );
  label_options.push_back( std::make_pair( AXIAL_C, 
    PreferencesManager::Instance()->z_axis_label_state_->get() ) );

  this->slice_type_state_->set_option_list( label_options );
}

void ClipboardTool::copy( Core::ActionContextHandle context )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  const std::string& layer_id = this->target_layer_state_->get();
  if ( layer_id == Tool::NONE_OPTION_C )
  {
    return;
  }
  
  ActionCopy::Dispatch( context, layer_id, this->slice_type_state_->index(),
    static_cast< size_t >( this->copy_slice_number_state_->get() - 1 ) );
}

void ClipboardTool::paste( Core::ActionContextHandle context )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  const std::string& layer_id = this->target_layer_state_->get();
  if ( layer_id == Tool::NONE_OPTION_C )
  {
    return;
  }

  ActionPaste::Dispatch( context, layer_id, this->slice_type_state_->index(),
    static_cast< size_t >( this->paste_min_slice_number_state_->get() - 1 ),
    static_cast< size_t >( this->paste_max_slice_number_state_->get() - 1 ) );
}

} // end namespace Seg3D
