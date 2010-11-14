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

#include <Core/Action/ActionFactory.h>

#include <Application/ToolManager/ToolManager.h>
#include <Application/Tools/Actions/ActionPaint.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/DataLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/LayerUndoBuffer.h>
#include <Application/LayerManager/LayerUndoBufferItem.h>

CORE_REGISTER_ACTION( Seg3D, Paint )

namespace Seg3D
{

class ActionPaintPrivate
{
public:
  Core::ActionParameter< std::string > target_layer_id_;
  Core::ActionParameter< int > slice_type_;
  Core::ActionParameter< size_t > slice_number_;
  Core::ActionParameter< std::vector<int> > x_;
  Core::ActionParameter< std::vector<int> > y_;
  Core::ActionParameter< int > brush_radius_;

  Core::ActionParameter< std::string > data_constraint_layer_id_;
  Core::ActionParameter< double > min_val_;
  Core::ActionParameter< double > max_val_;
  Core::ActionParameter< bool > negative_data_constraint_;
  Core::ActionParameter< std::string > mask_constraint1_layer_id_;
  Core::ActionParameter< bool > negative_mask_constraint1_;
  Core::ActionParameter< std::string > mask_constraint2_layer_id_;
  Core::ActionParameter< bool > negative_mask_constraint2_;
  Core::ActionParameter< bool > erase_;

  PaintToolWeakHandle paint_tool_weak_handle_;
  Core::MaskVolumeSliceHandle target_slice_;
  Core::DataVolumeSliceHandle data_constraint_slice_;
  Core::MaskVolumeSliceHandle mask_constraint1_slice_;
  Core::MaskVolumeSliceHandle mask_constraint2_slice_;
};

ActionPaint::ActionPaint() :
  private_( new ActionPaintPrivate )
{
  this->add_argument( this->private_->target_layer_id_ );
  this->add_argument( this->private_->slice_type_ );
  this->add_argument( this->private_->slice_number_ );
  this->add_argument( this->private_->x_ );
  this->add_argument( this->private_->y_ );
  this->add_argument( this->private_->brush_radius_ );

  this->add_key( this->private_->data_constraint_layer_id_ );
  this->add_key( this->private_->min_val_ );
  this->add_key( this->private_->max_val_ );
  this->add_key( this->private_->negative_data_constraint_ );
  this->add_key( this->private_->mask_constraint1_layer_id_ );
  this->add_key( this->private_->negative_mask_constraint1_ );
  this->add_key( this->private_->mask_constraint2_layer_id_ );
  this->add_key( this->private_->negative_mask_constraint2_ );
  this->add_key( this->private_->erase_ );
}

ActionPaint::~ActionPaint()
{
}

bool ActionPaint::validate( Core::ActionContextHandle& context )
{
  // The paint interface will update the painting
  if ( context->source() == Core::ActionSource::INTERFACE_MOUSE_E ) return true;

  if ( this->private_->brush_radius_.value() < 0 )
  {
    context->report_error( "Invalid brush size " + Core::ExportToString(
      this->private_->brush_radius_.value() ) );
    return false;
  }
  
  if ( !this->private_->target_slice_ )
  {
    MaskLayerHandle target_layer = boost::dynamic_pointer_cast< MaskLayer >(
      LayerManager::Instance()->get_layer_by_id( this->private_->target_layer_id_.value() ) );
    if ( !target_layer )
    {
      context->report_error( "Invalid target layer '" + 
        this->private_->target_layer_id_.value() + "'" );
      return false;
    }
    if ( this->private_->slice_type_.value() != Core::VolumeSliceType::AXIAL_E &&
      this->private_->slice_type_.value() != Core::VolumeSliceType::CORONAL_E &&
      this->private_->slice_type_.value() != Core::VolumeSliceType::SAGITTAL_E )
    {
      context->report_error( "Invalid slice type " + Core::ExportToString( 
        this->private_->slice_type_.value() ) );
      return false;
    }
    this->private_->target_slice_.reset( new Core::MaskVolumeSlice( 
      target_layer->get_mask_volume(), static_cast< Core::VolumeSliceType::enum_type >(
      this->private_->slice_type_.value() ) ) );
    if ( this->private_->slice_number_.value() >= 
      this->private_->target_slice_->number_of_slices() )
    {
      context->report_error( "Slice number " + Core::ExportToString(
        this->private_->slice_number_.value() ) + " is out of range" );
      return false;
    }
    this->private_->target_slice_->set_slice_number( this->private_->slice_number_.value() );
  }

  if ( this->private_->target_slice_->out_of_boundary() )
  {
    context->report_error( "The target slice is out of boundary" );
    return false;
  }
  
  if ( !this->private_->data_constraint_slice_ &&
    this->private_->data_constraint_layer_id_.value() != Tool::NONE_OPTION_C )
  {
    DataLayerHandle data_constraint_layer = boost::dynamic_pointer_cast< DataLayer >(
      LayerManager::Instance()->get_layer_by_id( 
      this->private_->data_constraint_layer_id_.value() ) );
    if ( !data_constraint_layer )
    {
      context->report_error( "Invalid data constraint layer '" +
        this->private_->data_constraint_layer_id_.value() + "'" );
      return false;
    }
    
    this->private_->data_constraint_slice_.reset( new Core::DataVolumeSlice(
      data_constraint_layer->get_data_volume(), 
      this->private_->target_slice_->get_slice_type(), 
      this->private_->slice_number_.value() ) );
  }

  if ( !this->private_->mask_constraint1_slice_ &&
    this->private_->mask_constraint1_layer_id_.value() != Tool::NONE_OPTION_C )
  {
    MaskLayerHandle mask_constraint_layer = boost::dynamic_pointer_cast< MaskLayer >(
      LayerManager::Instance()->get_layer_by_id( 
      this->private_->mask_constraint1_layer_id_.value() ) );
    if ( !mask_constraint_layer )
    {
      context->report_error( "Invalid mask constraint layer '" +
        this->private_->mask_constraint1_layer_id_.value() + "'" );
      return false;
    }
    this->private_->mask_constraint1_slice_.reset( new Core::MaskVolumeSlice(
      mask_constraint_layer->get_mask_volume(), 
      this->private_->target_slice_->get_slice_type(),
      this->private_->slice_number_.value() ) );
  }
  
  if ( !this->private_->mask_constraint2_slice_ &&
    this->private_->mask_constraint2_layer_id_.value() != Tool::NONE_OPTION_C )
  {
    MaskLayerHandle mask_constraint_layer = boost::dynamic_pointer_cast< MaskLayer >(
      LayerManager::Instance()->get_layer_by_id( 
      this->private_->mask_constraint2_layer_id_.value() ) );
    if ( !mask_constraint_layer )
    {
      context->report_error( "Invalid mask constraint layer '" +
        this->private_->mask_constraint2_layer_id_.value() + "'" );
      return false;
    }
    this->private_->mask_constraint2_slice_.reset( new Core::MaskVolumeSlice(
      mask_constraint_layer->get_mask_volume(), 
      this->private_->target_slice_->get_slice_type(),
      this->private_->slice_number_.value() ) );
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
  // Create a new undo item
  LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "Paint Stroke" ) );
  
  // Add redo information to undo item
  item->set_redo_action( this->shared_from_this() );

  // Add check point for the slice we are painting on
  LayerHandle layer = LayerManager::Instance()->get_layer_by_id( 
    this->private_->target_layer_id_.value() );
  Core::SliceType slice_type = static_cast<Core::SliceType::enum_type>( 
    this->private_->slice_type_.value() );
  int index = this->private_->slice_number_.value();
  
  LayerCheckPointHandle check_point( new LayerCheckPoint( layer, slice_type, index ) );
  item->add_layer_to_restore( layer, check_point );

  LayerUndoBuffer::Instance()->insert_undo_item( context, item );

  // Painting will already have been done by the interface
  if ( context->source() == Core::ActionSource::INTERFACE_MOUSE_E ) return true;

  static PaintToolHandle static_paint_tool( new PaintTool( "staticpainttool" ) );

  PaintInfo paint_info;
  paint_info.target_slice_ = this->private_->target_slice_;
  paint_info.data_constraint_slice_ = this->private_->data_constraint_slice_;
  paint_info.min_val_ = this->private_->min_val_.value();
  paint_info.max_val_ = this->private_->max_val_.value();
  paint_info.negative_data_constraint_ = this->private_->negative_data_constraint_.value();
  paint_info.mask_constraint1_slice_ = this->private_->mask_constraint1_slice_;
  paint_info.negative_mask_constraint1_ = this->private_->negative_mask_constraint1_.value();
  paint_info.mask_constraint2_slice_ = this->private_->mask_constraint2_slice_;
  paint_info.negative_mask_constraint2_ = this->private_->negative_mask_constraint2_.value();

  paint_info.x_ = this->private_->x_.value();
  paint_info.y_ = this->private_->y_.value();
  paint_info.brush_radius_ = this->private_->brush_radius_.value();
  paint_info.erase_ = this->private_->erase_.value();

  bool success = static_paint_tool->paint( paint_info );
  paint_info.target_slice_->release_cached_data();
  
  return success;
}

Core::ActionHandle ActionPaint::Create( const PaintInfo& paint_info )
{
  ActionPaint* action = new ActionPaint;

  action->private_->target_layer_id_.value() = paint_info.target_layer_id_;
  action->private_->slice_type_.value() = paint_info.target_slice_->get_slice_type();
  action->private_->slice_number_.value() = paint_info.target_slice_->get_slice_number();
  action->private_->data_constraint_layer_id_.value() = paint_info.data_constraint_layer_id_;
  action->private_->min_val_.value() = paint_info.min_val_;
  action->private_->max_val_.value() = paint_info.max_val_;
  action->private_->negative_data_constraint_.value() = paint_info.negative_data_constraint_;
  action->private_->mask_constraint1_layer_id_.value() = paint_info.mask_constraint1_layer_id_;
  action->private_->negative_mask_constraint1_.value() = paint_info.negative_mask_constraint1_;
  action->private_->mask_constraint2_layer_id_.value() = paint_info.mask_constraint2_layer_id_;
  action->private_->negative_mask_constraint2_.value() = paint_info.negative_mask_constraint2_;
  action->private_->x_.set_value( paint_info.x_ );
  action->private_->y_.set_value( paint_info.y_ );
  action->private_->brush_radius_.value() = paint_info.brush_radius_;
  action->private_->erase_.value() = paint_info.erase_;

  action->private_->target_slice_ = paint_info.target_slice_;
  action->private_->data_constraint_slice_ = paint_info.data_constraint_slice_;
  action->private_->mask_constraint1_slice_ = paint_info.mask_constraint1_slice_;
  action->private_->mask_constraint2_slice_ = paint_info.mask_constraint2_slice_;

  return Core::ActionHandle( action );
}

void ActionPaint::Dispatch( Core::ActionContextHandle context, const PaintInfo& paint_info )
{
  Core::ActionDispatcher::PostAction( Create( paint_info ), context );
}

} // end namespace Seg3D
