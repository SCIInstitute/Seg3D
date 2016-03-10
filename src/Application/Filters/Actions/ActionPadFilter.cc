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

CORE_REGISTER_ACTION( Seg3D, PadFilter )

namespace Seg3D
{

using namespace Filter;

class ActionPadFilterPrivate
{
  // -- action parameters --
public:
  std::vector< std::string > layer_ids_;
  int pad_level_x_;
  int pad_level_y_;
  int pad_level_z_;
  Core::Point range_min_;
  Core::Point range_max_;
  std::string padding_;
  bool replace_;
  SandboxID sandbox_;

  // -- internal variables --
public:
  bool match_grid_transform_;
  Core::GridTransform grid_transform_;
  
};

ActionPadFilter::ActionPadFilter() :
private_( new ActionPadFilterPrivate )
{
  // Action arguments
  this->add_layer_id_list( this->private_->layer_ids_ );
  this->add_parameter( this->private_->pad_level_x_ );
  this->add_parameter( this->private_->pad_level_y_ );
  this->add_parameter( this->private_->pad_level_z_ );
  this->add_parameter( this->private_->range_min_ );
  this->add_parameter( this->private_->range_max_ );
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

  const std::vector< std::string >& layer_ids = this->private_->layer_ids_;
  if ( layer_ids.size() == 0 )
  {
    context->report_error( "No input layers specified" );
    return false;
  }

  for ( size_t i = 0; i < layer_ids.size(); ++i )
  {
    // Check for layer existence and type information
    if ( ! LayerManager::CheckLayerExistence( layer_ids[ i ], context, this->private_->sandbox_ ) )
      return false;

    // Check for layer availability
    if ( ! LayerManager::CheckLayerAvailability( layer_ids[ i ],
                                                 this->private_->replace_,
                                                 context,
                                                 this->private_->sandbox_ ) )
      return false;
  }

  if ( this->private_->range_max_[ 0 ] <= this->private_->range_min_[ 0 ] ||
       this->private_->range_max_[ 1 ] <= this->private_->range_min_[ 1 ] ||
       this->private_->range_max_[ 2 ] <= this->private_->range_min_[ 2 ] )
  {
    context->report_error( "Invalid resample range." );
    return false;
  }

  if ( this->private_->pad_level_x_ < 1 ||
       this->private_->pad_level_y_ < 1 ||
       this->private_->pad_level_z_ < 1 )
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
  return false;
}

void ActionPadFilter::Dispatch( Core::ActionContextHandle context,
                                const std::vector< std::string >& layer_ids,
                                int pad_level_x, int pad_level_y, int pad_level_z,
                                const std::string& padding,
                                bool replace)
{
  return;
}

}
