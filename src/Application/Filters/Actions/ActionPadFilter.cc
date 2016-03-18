/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

  // -- internal variables --
public:
  Core::Point range_min_;
  Core::Point range_max_;
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

  double offset = transform.get_originally_node_centered() ? 0.0 : 1.0;
  Core::Point start( -this->private_->pad_level_[0], -this->private_->pad_level_[1], -this->private_->pad_level_[2] );
  Core::Point end( transform.get_nx() + this->private_->pad_level_[0] - offset,
                   transform.get_ny() + this->private_->pad_level_[1] - offset,
                   transform.get_nz() + this->private_->pad_level_[2] - offset );

  this->private_->range_min_ = transform * start;
  this->private_->range_max_ = transform * end;

std::cerr << start << std::endl;
std::cerr << end << std::endl;
std::cerr << this->private_->range_min_ << std::endl;
std::cerr << this->private_->range_max_ << std::endl;

  return false;
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
