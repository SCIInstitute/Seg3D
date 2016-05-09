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

#include <Application/Filters/Actions/ActionPadFilter.h>
#include <Application/Filters/PadFilter.h>

#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerGroup.h>

CORE_REGISTER_ACTION( Seg3D, PadFilter )

namespace Seg3D
{

using namespace Filter;

class ActionPadFilterPrivate
{
  // -- action parameters --
public:
  std::vector< std::string > layer_ids_;
  Core::Vector pad_level_;
  std::string padding_;
  bool replace_;
  SandboxID sandbox_;
};

ActionPadFilter::ActionPadFilter() :
private_( new ActionPadFilterPrivate )
{
  // Action arguments
  this->add_layer_id_list( this->private_->layer_ids_ );
  this->add_parameter( this->private_->pad_level_ );
  this->add_parameter( this->private_->padding_ );
  this->add_parameter( this->private_->replace_ );
  this->add_parameter( this->private_->sandbox_ );
}

bool ActionPadFilter::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( ! LayerManager::CheckSandboxExistence( this->private_->sandbox_, context ) )
  {
    return false;
  }

  if ( this->private_->layer_ids_.size() == 0 )
  {
    context->report_error( "No input layers specified" );
    return false;
  }

  for ( size_t i = 0; i < this->private_->layer_ids_.size(); ++i )
  {
    std::string layerid = this->private_->layer_ids_[i];
    if ( ! LayerManager::Instance()->find_layer_by_id( layerid ) )
    {
      LayerHandle layer = LayerManager::Instance()->find_layer_by_name( layerid );
      if ( layer )
      {
        // If they passed the name instead, then we'll take the opportunity to get the id instead.
        layerid = layer->get_layer_id();
        this->private_->layer_ids_[i] = layerid;
      }
    }

    // Check for layer existence and type information
    if ( ! LayerManager::CheckLayerExistence( layerid, context, this->private_->sandbox_ ) )
      return false;

    // Check for layer availability
    if ( ! LayerManager::CheckLayerAvailability( layerid,
                                                 this->private_->replace_,
                                                 context,
                                                 this->private_->sandbox_ ) )
      return false;
  }

  if ( this->private_->pad_level_[ 0 ] < 1 ||
       this->private_->pad_level_[ 1 ] < 1 ||
       this->private_->pad_level_[ 2 ] < 1 )
  {
    context->report_error( "Invalid pad size" );
    return false;
  }

  if ( ! PadValues::IsValidPadding( this->private_->padding_ ) )
  {
    context->report_error( "Unknown padding option" );
    return false;
  }

  // Validation successful
  return true;
}

bool ActionPadFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  LayerHandle layer = LayerManager::Instance()->find_layer_by_id( this->private_->layer_ids_[0] );
  Core::GridTransform transform = layer->get_layer_group()->get_grid_transform();
  Core::Point start( -this->private_->pad_level_[0], -this->private_->pad_level_[1], -this->private_->pad_level_[2] );

  size_t padded_dim_x = transform.get_nx() + 2 * this->private_->pad_level_[0];
  size_t padded_dim_y = transform.get_ny() + 2 * this->private_->pad_level_[1];
  size_t padded_dim_z = transform.get_nz() + 2 * this->private_->pad_level_[2];

  Core::Point new_origin = transform * start;
  Core::GridTransform new_transform( padded_dim_x, padded_dim_y, padded_dim_z );
  new_transform.set_originally_node_centered( transform.get_originally_node_centered() );
  new_transform.load_basis( new_origin, Core::Vector( transform.spacing_x(), 0, 0 ),
                                        Core::Vector( 0, transform.spacing_y(), 0 ),
                                        Core::Vector( 0, 0, transform.spacing_z() ) );

  // Create algorithm
  boost::shared_ptr< PadFilter > algo( new PadFilter( this->private_->replace_,
                                                      this->private_->padding_,
                                                      this->private_->sandbox_ ) );
  algo->setup_layers( this->private_->layer_ids_, new_transform );

  // Return the ids of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( algo->get_dst_layer_ids() ) );
  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
       context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }

  // Build the undo-redo record
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this(), true );

  // Start the filter.
  Core::Runnable::Start( algo );

  return true;
}

void ActionPadFilter::Dispatch( Core::ActionContextHandle context,
                                const std::vector< std::string >& layer_ids,
                                const Core::Vector& pad_level,
                                const std::string& padding,
                                bool replace )
{
  ActionPadFilter* action = new ActionPadFilter;
  action->private_->layer_ids_ = layer_ids;
  action->private_->pad_level_ = pad_level;
  action->private_->padding_ = padding;
  action->private_->replace_ = replace;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

}
