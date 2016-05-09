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

#include <Core/Action/ActionFactory.h>

#include <Application/ToolManager/ToolManager.h>
#include <Application/Tools/Actions/ActionPaint.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/Layer/LayerUndoBufferItem.h>
#include <Application/Provenance/ProvenanceStep.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>

CORE_REGISTER_ACTION( Seg3D, Paint )

namespace Seg3D
{

class ActionPaintPrivate
{
public:
  std::string target_layer_id_;
  int slice_type_;
  size_t slice_number_;
  std::vector<int> x_;
  std::vector<int> y_;
  int brush_radius_;

  std::string data_constraint_layer_id_;
  double min_val_;
  double max_val_;
  bool negative_data_constraint_;
  std::string mask_constraint1_layer_id_;
  bool negative_mask_constraint1_;
  std::string mask_constraint2_layer_id_;
  bool negative_mask_constraint2_;
  bool erase_;
  SandboxID sandbox_;

  Core::MaskVolumeSliceHandle target_slice_;
  Core::DataVolumeSliceHandle data_constraint_slice_;
  Core::MaskVolumeSliceHandle mask_constraint1_slice_;
  Core::MaskVolumeSliceHandle mask_constraint2_slice_;
};

ActionPaint::ActionPaint() :
  private_( new ActionPaintPrivate )
{
  this->add_layer_id( this->private_->target_layer_id_ );
  this->add_parameter( this->private_->slice_type_ );
  this->add_parameter( this->private_->slice_number_ );
  this->add_parameter( this->private_->x_ );
  this->add_parameter( this->private_->y_ );
  this->add_parameter( this->private_->brush_radius_ );

  this->add_layer_id( this->private_->data_constraint_layer_id_ );
  this->add_parameter( this->private_->min_val_ );
  this->add_parameter( this->private_->max_val_ );
  this->add_parameter( this->private_->negative_data_constraint_ );
  this->add_layer_id( this->private_->mask_constraint1_layer_id_ );
  this->add_parameter( this->private_->negative_mask_constraint1_ );
  this->add_layer_id( this->private_->mask_constraint2_layer_id_ );
  this->add_parameter( this->private_->negative_mask_constraint2_ );
  this->add_parameter( this->private_->erase_ );
  this->add_parameter( this->private_->sandbox_ );
}

bool ActionPaint::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->private_->sandbox_, context ) )
  {
    return false;
  }

  // The paint interface will update the painting
  if ( context->source() == Core::ActionSource::INTERFACE_MOUSE_E ) return true;

  if ( this->private_->brush_radius_ < 0 )
  {
    context->report_error( "Invalid brush size " + Core::ExportToString(
      this->private_->brush_radius_ ) );
    return false;
  }
  
  MaskLayerHandle target_layer = LayerManager::FindMaskLayer( 
    this->private_->target_layer_id_, this->private_->sandbox_ );
  if ( !target_layer )
  {
    context->report_error( "Invalid target layer '" + this->private_->target_layer_id_ + "'" );
    return false;
  }

  // Make sure the layer is available for processing
  if ( !LayerManager::CheckLayerAvailabilityForProcessing( this->private_->target_layer_id_,
    context, this->private_->sandbox_) )
  {
    return false;
  }

  if ( !this->private_->target_slice_ )
  {
    if ( this->private_->slice_type_ != Core::VolumeSliceType::AXIAL_E &&
      this->private_->slice_type_ != Core::VolumeSliceType::CORONAL_E &&
      this->private_->slice_type_ != Core::VolumeSliceType::SAGITTAL_E )
    {
      context->report_error( "Invalid slice type " + Core::ExportToString( 
        this->private_->slice_type_ ) );
      return false;
    }
    this->private_->target_slice_.reset( new Core::MaskVolumeSlice( 
      target_layer->get_mask_volume(), static_cast< Core::VolumeSliceType::enum_type >(
      this->private_->slice_type_ ) ) );
    if ( this->private_->slice_number_ >= 
      this->private_->target_slice_->number_of_slices() )
    {
      context->report_error( "Slice number " + Core::ExportToString(
        this->private_->slice_number_ ) + " is out of range" );
      return false;
    }
    this->private_->target_slice_->set_slice_number( this->private_->slice_number_ );
  }

  if ( this->private_->target_slice_->out_of_boundary() )
  {
    context->report_error( "The target slice is out of boundary" );
    return false;
  }
  
  if ( !this->private_->data_constraint_slice_ &&
    this->private_->data_constraint_layer_id_ != Tool::NONE_OPTION_C )
  {
    DataLayerHandle data_constraint_layer = LayerManager::FindDataLayer(
      this->private_->data_constraint_layer_id_, this->private_->sandbox_ );
    if ( !data_constraint_layer )
    {
      context->report_error( "Invalid data constraint layer '" +
        this->private_->data_constraint_layer_id_ + "'" );
      return false;
    }

    // Make sure the layer is available for use
    if ( !LayerManager::CheckLayerAvailabilityForUse( this->private_->data_constraint_layer_id_,
      context, this->private_->sandbox_) )
    {
      return false;
    }

    // Make sure the constraint layer has the same grid transform as the target
    if ( data_constraint_layer->get_grid_transform() != target_layer->get_grid_transform() )
    {
      context->report_error( "Data constraint layer and target layer have different grid transform." );
      return false;
    }
    
    this->private_->data_constraint_slice_.reset( new Core::DataVolumeSlice(
      data_constraint_layer->get_data_volume(), 
      this->private_->target_slice_->get_slice_type(), 
      this->private_->slice_number_ ) );
  }

  if ( !this->private_->mask_constraint1_slice_ &&
    this->private_->mask_constraint1_layer_id_ != Tool::NONE_OPTION_C )
  {
    MaskLayerHandle mask_constraint_layer = LayerManager::FindMaskLayer( 
      this->private_->mask_constraint1_layer_id_, this->private_->sandbox_ );
    if ( !mask_constraint_layer )
    {
      context->report_error( "Invalid mask constraint layer '" +
        this->private_->mask_constraint1_layer_id_ + "'" );
      return false;
    }

    // Make sure the layer is available for use
    if ( !LayerManager::CheckLayerAvailabilityForUse( this->private_->mask_constraint1_layer_id_,
      context, this->private_->sandbox_) )
    {
      return false;
    }

    // Make sure the constraint layer has the same grid transform as the target
    if ( mask_constraint_layer->get_grid_transform() != target_layer->get_grid_transform() )
    {
      context->report_error( "Mask constraint layer and target layer have different grid transform." );
      return false;
    }

    this->private_->mask_constraint1_slice_.reset( new Core::MaskVolumeSlice(
      mask_constraint_layer->get_mask_volume(), 
      this->private_->target_slice_->get_slice_type(),
      this->private_->slice_number_ ) );
  }
  
  if ( !this->private_->mask_constraint2_slice_ &&
    this->private_->mask_constraint2_layer_id_ != Tool::NONE_OPTION_C )
  {
    MaskLayerHandle mask_constraint_layer = LayerManager::FindMaskLayer( 
      this->private_->mask_constraint2_layer_id_, this->private_->sandbox_ );
    if ( !mask_constraint_layer )
    {
      context->report_error( "Invalid mask constraint layer '" +
        this->private_->mask_constraint2_layer_id_ + "'" );
      return false;
    }

    // Make sure the layer is available for use
    if ( !LayerManager::CheckLayerAvailabilityForUse( this->private_->mask_constraint2_layer_id_,
      context, this->private_->sandbox_) )
    {
      return false;
    }

    // Make sure the constraint layer has the same grid transform as the target
    if ( mask_constraint_layer->get_grid_transform() != target_layer->get_grid_transform() )
    {
      context->report_error( "Mask constraint layer and target layer have different grid transform." );
      return false;
    }

    this->private_->mask_constraint2_slice_.reset( new Core::MaskVolumeSlice(
      mask_constraint_layer->get_mask_volume(), 
      this->private_->target_slice_->get_slice_type(),
      this->private_->slice_number_ ) );
  }

  return true;
}

void ActionPaint::clear_cache()
{
  this->private_->target_slice_.reset();
  this->private_->data_constraint_slice_.reset();
  this->private_->mask_constraint1_slice_.reset();
  this->private_->mask_constraint2_slice_.reset();
}

bool ActionPaint::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  LayerHandle layer = LayerManager::FindLayer( this->private_->target_layer_id_, 
    this->private_->sandbox_ );

  if ( this->private_->sandbox_ == -1 )
  {
    // Create a provenance record
    ProvenanceIDList deleted_prov_ids( 1, layer->provenance_id_state_->get() );
    ProvenanceStepHandle provenance_step( new ProvenanceStep );
    provenance_step->set_input_provenance_ids( this->get_input_provenance_ids() );
    provenance_step->set_output_provenance_ids( this->get_output_provenance_ids( 1 ) );
    provenance_step->set_replaced_provenance_ids( deleted_prov_ids );
    provenance_step->set_action_name( this->get_type() );
    provenance_step->set_action_params( this->export_params_to_provenance_string() );
    ProvenanceStepID prov_step_id = ProjectManager::Instance()->get_current_project()->
      add_provenance_record( provenance_step );

    if ( PreferencesManager::Instance()->enable_undo_state_->get() )
    {
      // Create a new undo item
      LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "Paint Stroke" ) );
      
      // Add redo information to undo item
      item->set_redo_action( this->shared_from_this() );

      // Add check point for the slice we are painting on
      Core::SliceType slice_type = static_cast<Core::SliceType::enum_type>( 
        this->private_->slice_type_ );
      int index = static_cast< int >( this->private_->slice_number_ );
      
      LayerCheckPointHandle check_point( new LayerCheckPoint( layer, slice_type, index ) );
      item->add_layer_to_restore( layer, check_point );
      item->set_provenance_step_id( prov_step_id );

      UndoBuffer::Instance()->insert_undo_item( context, item );
    }

    // Update the provenance ID of the target mask
    layer->provenance_id_state_->set( this->get_output_provenance_id() );
  }


  // Painting will already have been done by the interface
  if ( context->source() == Core::ActionSource::INTERFACE_MOUSE_E ) return true;

  static PaintToolHandle static_paint_tool( new PaintTool( "staticpainttool" ) );

  PaintInfo paint_info;
  paint_info.target_slice_ = this->private_->target_slice_;
  paint_info.data_constraint_slice_ = this->private_->data_constraint_slice_;
  paint_info.min_val_ = this->private_->min_val_;
  paint_info.max_val_ = this->private_->max_val_;
  paint_info.negative_data_constraint_ = this->private_->negative_data_constraint_;
  paint_info.mask_constraint1_slice_ = this->private_->mask_constraint1_slice_;
  paint_info.negative_mask_constraint1_ = this->private_->negative_mask_constraint1_;
  paint_info.mask_constraint2_slice_ = this->private_->mask_constraint2_slice_;
  paint_info.negative_mask_constraint2_ = this->private_->negative_mask_constraint2_;

  paint_info.x_ = this->private_->x_;
  paint_info.y_ = this->private_->y_;
  paint_info.brush_radius_ = this->private_->brush_radius_;
  paint_info.erase_ = this->private_->erase_;

  bool success = static_paint_tool->paint( paint_info );
  paint_info.target_slice_->release_cached_data();

  result.reset( new Core::ActionResult( this->private_->target_layer_id_ ) );
  
  return success;
}

void ActionPaint::Dispatch( Core::ActionContextHandle context, const PaintInfo& paint_info )
{
  ActionPaint* action = new ActionPaint;

  action->private_->target_layer_id_ = paint_info.target_layer_id_;
  action->private_->slice_type_ = paint_info.target_slice_->get_slice_type();
  action->private_->slice_number_ = paint_info.target_slice_->get_slice_number();
  action->private_->data_constraint_layer_id_ = paint_info.data_constraint_layer_id_;
  action->private_->min_val_ = paint_info.min_val_;
  action->private_->max_val_ = paint_info.max_val_;
  action->private_->negative_data_constraint_ = paint_info.negative_data_constraint_;
  action->private_->mask_constraint1_layer_id_ = paint_info.mask_constraint1_layer_id_;
  action->private_->negative_mask_constraint1_ = paint_info.negative_mask_constraint1_;
  action->private_->mask_constraint2_layer_id_ = paint_info.mask_constraint2_layer_id_;
  action->private_->negative_mask_constraint2_ = paint_info.negative_mask_constraint2_;
  action->private_->x_ = paint_info.x_;
  action->private_->y_ = paint_info.y_;
  action->private_->brush_radius_ = paint_info.brush_radius_;
  action->private_->erase_ = paint_info.erase_;

  action->private_->target_slice_ = paint_info.target_slice_;
  action->private_->data_constraint_slice_ = paint_info.data_constraint_slice_;
  action->private_->mask_constraint1_slice_ = paint_info.mask_constraint1_slice_;
  action->private_->mask_constraint2_slice_ = paint_info.mask_constraint2_slice_;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
