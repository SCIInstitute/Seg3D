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
  Core::ActionParameter< int > x0_;
  Core::ActionParameter< int > y0_;
  Core::ActionParameter< int > x1_;
  Core::ActionParameter< int > y1_;
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
  Core::ActionCachedHandle< Core::MaskVolumeSliceHandle > target_slice_;
  Core::ActionCachedHandle< Core::DataVolumeSliceHandle > data_constraint_slice_;
  Core::ActionCachedHandle< Core::MaskVolumeSliceHandle > mask_constraint1_slice_;
  Core::ActionCachedHandle< Core::MaskVolumeSliceHandle > mask_constraint2_slice_;
};

ActionPaint::ActionPaint() :
  private_( new ActionPaintPrivate )
{
  this->add_argument( this->private_->target_layer_id_ );
  this->add_argument( this->private_->slice_type_ );
  this->add_argument( this->private_->slice_number_ );
  this->add_argument( this->private_->x0_ );
  this->add_argument( this->private_->y0_ );
  this->add_argument( this->private_->x1_ );
  this->add_argument( this->private_->y1_ );
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

  this->add_cachedhandle( this->private_->target_slice_ );
  this->add_cachedhandle( this->private_->data_constraint_slice_ );
  this->add_cachedhandle( this->private_->mask_constraint1_slice_ );
  this->add_cachedhandle( this->private_->mask_constraint2_slice_ );
}

ActionPaint::~ActionPaint()
{
}

bool ActionPaint::validate( Core::ActionContextHandle& context )
{
  if ( this->private_->brush_radius_.value() < 0 )
  {
    context->report_error( "Invalid brush size " + Core::ExportToString(
      this->private_->brush_radius_.value() ) );
    return false;
  }
  
  if ( !this->private_->target_slice_.handle() )
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
    this->private_->target_slice_.handle().reset( new Core::MaskVolumeSlice( 
      target_layer->get_mask_volume(), static_cast< Core::VolumeSliceType::enum_type >(
      this->private_->slice_type_.value() ) ) );
    if ( this->private_->slice_number_.value() >= 
      this->private_->target_slice_.handle()->number_of_slices() )
    {
      context->report_error( "Slice number " + Core::ExportToString(
        this->private_->slice_number_.value() ) + " is out of range" );
      return false;
    }
    this->private_->target_slice_.handle()->set_slice_number( 
      this->private_->slice_number_.value() );
  }

  if ( this->private_->target_slice_.handle()->out_of_boundary() )
  {
    context->report_error( "The target slice is out of boundary" );
    return false;
  }
  
  if ( !this->private_->data_constraint_slice_.handle() &&
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
    
    this->private_->data_constraint_slice_.handle().reset( new Core::DataVolumeSlice(
      data_constraint_layer->get_data_volume(), 
      this->private_->target_slice_.handle()->get_slice_type(),
      this->private_->slice_number_.value() ) );
  }

  if ( !this->private_->mask_constraint1_slice_.handle() &&
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
    this->private_->mask_constraint1_slice_.handle().reset( new Core::MaskVolumeSlice(
      mask_constraint_layer->get_mask_volume(), 
      this->private_->target_slice_.handle()->get_slice_type(),
      this->private_->slice_number_.value() ) );
  }
  
  if ( !this->private_->mask_constraint2_slice_.handle() &&
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
    this->private_->mask_constraint2_slice_.handle().reset( new Core::MaskVolumeSlice(
      mask_constraint_layer->get_mask_volume(), 
      this->private_->target_slice_.handle()->get_slice_type(),
      this->private_->slice_number_.value() ) );
  }

  return true;
}

bool ActionPaint::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  PaintToolHandle paint_tool( this->private_->paint_tool_weak_handle_.lock() );
  if ( !paint_tool )
  {
    static PaintToolHandle static_paint_tool_s( new PaintTool( "staticpainttool" ) );
    paint_tool = static_paint_tool_s;
  }

  PaintInfo paint_info;
  paint_info.target_slice_ = this->private_->target_slice_.handle();
  paint_info.data_constraint_slice_ = this->private_->data_constraint_slice_.handle();
  paint_info.min_val_ = this->private_->min_val_.value();
  paint_info.max_val_ = this->private_->max_val_.value();
  paint_info.negative_data_constraint_ = this->private_->negative_data_constraint_.value();
  paint_info.mask_constraint1_slice_ = this->private_->mask_constraint1_slice_.handle();
  paint_info.negative_mask_constraint1_ = this->private_->negative_mask_constraint1_.value();
  paint_info.mask_constraint2_slice_ = this->private_->mask_constraint2_slice_.handle();
  paint_info.negative_mask_constraint2_ = this->private_->negative_mask_constraint2_.value();
  paint_info.x0_ = this->private_->x0_.value();
  paint_info.y0_ = this->private_->y0_.value();
  paint_info.x1_ = this->private_->x1_.value();
  paint_info.y1_ = this->private_->y1_.value();
  paint_info.brush_radius_ = this->private_->brush_radius_.value();
  paint_info.erase_ = this->private_->erase_.value();
  paint_info.inclusive_ = ( context->source() != Core::ActionSource::INTERFACE_MOUSE_E ||
    ( paint_info.x0_ == paint_info.x1_ && paint_info.y0_ == paint_info.y1_ ) );


  // Create undo for this action
  LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "Paint" ) );

/*
  int axis = 0;
  if ( this->private_->slice_type_.value() == Core::VolumeSliceType::AXIAL_E ) axis = 3;
  if ( this->private_->slice_type_.value() == Core::VolumeSliceType::CORONAL_E ) axis = 2;
  if ( this->private_->slice_type_.value() == Core::VolumeSliceType::SAGITTAL_E ) axis = 1;
  int slice = this->private_->slice_number_.value();
  
  LayerHandle layer = LayerManager::Instance()->get_layer_by_id( 
    this->private_->target_layer_id_.value() );
  LayerCheckPointHandle check_point( new LayerCheckPoint( layer, slice, axis ) );

  item->set_redo_action( this->shared_from_this() );
  item->add_layer_to_restore( layer, check_point );

  LayerUndoBuffer::Instance()->insert_undo_item( context, item );
*/

  bool success = paint_tool->paint( paint_info );
  if ( context->source() != Core::ActionSource::INTERFACE_MOUSE_E )
  {
    paint_info.target_slice_->release_cached_data();
  }
  
  return success;
}

Core::ActionHandle ActionPaint::Create( const PaintToolHandle& paint_tool, 
                     const PaintInfo& paint_info )
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
  action->private_->x0_.set_value( paint_info.x0_ );
  action->private_->y0_.set_value( paint_info.y0_ );
  action->private_->x1_.set_value( paint_info.x1_ );
  action->private_->y1_.set_value( paint_info.y1_ );
  action->private_->brush_radius_.value() = paint_info.brush_radius_;
  action->private_->erase_.value() = paint_info.erase_;

  action->private_->paint_tool_weak_handle_ = paint_tool;
  action->private_->target_slice_.handle() = paint_info.target_slice_;
  action->private_->data_constraint_slice_.handle() = paint_info.data_constraint_slice_;
  action->private_->mask_constraint1_slice_.handle() = paint_info.mask_constraint1_slice_;
  action->private_->mask_constraint2_slice_.handle() = paint_info.mask_constraint2_slice_;

  return Core::ActionHandle( action );
}

void ActionPaint::Dispatch( Core::ActionContextHandle context, 
               const PaintToolHandle& paint_tool, const PaintInfo& paint_info )
{
  Core::ActionDispatcher::PostAction( Create( paint_tool, paint_info ), context );
}

} // end namespace Seg3D