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

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/Filters/Actions/ActionResample.h>
#include <Application/Filters/NrrdResampleFilter.h>
#include <Application/Filters/ITKResampleFilter.h>
#include <Application/Filters/Utils/PadValues.h>

#include <Core/Utils/Log.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, Resample )

namespace Seg3D
{

using namespace Filter;

//////////////////////////////////////////////////////////////////////////
// Class ActionResamplePrivate
//////////////////////////////////////////////////////////////////////////

class ActionResamplePrivate
{
  // -- action parameters --
public:
  std::vector< std::string > layer_ids_;
  unsigned int x_;
  unsigned int y_;
  unsigned int z_;
  bool crop_;
  Core::Point range_min_;
  Core::Point range_max_;
  std::string padding_;
  std::string kernel_;
  double gauss_sigma_;
  double gauss_cutoff_;
  int spline_order_;
  bool replace_;
  SandboxID sandbox_;

  // -- internal variables --
public:
  bool match_grid_transform_;
  Core::GridTransform grid_transform_;

};

//////////////////////////////////////////////////////////////////////////
// Class ActionResample
//////////////////////////////////////////////////////////////////////////

ActionResample::ActionResample() :
  private_( new ActionResamplePrivate )
{
  // Action arguments
  this->add_layer_id_list( this->private_->layer_ids_ );
  this->add_parameter( this->private_->x_ );
  this->add_parameter( this->private_->y_ );
  this->add_parameter( this->private_->z_ );
  this->add_parameter( this->private_->crop_ );
  this->add_parameter( this->private_->range_min_ );
  this->add_parameter( this->private_->range_max_ );
  this->add_parameter( this->private_->padding_ );
  this->add_parameter( this->private_->kernel_ );
  this->add_parameter( this->private_->gauss_sigma_ );
  this->add_parameter( this->private_->gauss_cutoff_ );
  this->add_parameter( this->private_->spline_order_ );
  this->add_parameter( this->private_->replace_ );
  this->add_parameter( this->private_->sandbox_ );

  this->private_->match_grid_transform_ = false;
}

bool ActionResample::validate( Core::ActionContextHandle& context )
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
         this->private_->replace_, context, this->private_->sandbox_ ) ) return false;
  }

  if ( this->private_->x_ < 1 ||
       this->private_->y_ < 1 ||
       this->private_->z_ < 1 )
  {
    context->report_error( "Invalid resample size" );
    return false;
  }

  if ( this->private_->match_grid_transform_ )
  {
    this->private_->crop_ = true;

    // Compute the boundary of the new grid transform in world space
    // NOTE: If the destination grid is cell centered, the actual range of the grid should be
    // extended by half a voxel in each direction
    double offset = this->private_->grid_transform_.get_originally_node_centered() ? 0.0 : 0.5;
    Core::Point start( -offset, -offset, -offset );
    Core::Point end( this->private_->x_ - 1 + offset, this->private_->y_ - 1 + offset, this->private_->z_ - 1 + offset );
    this->private_->range_min_ = this->private_->grid_transform_ * start;
    this->private_->range_max_ = this->private_->grid_transform_ * end;

    // Compute the resample range relative to the input in index space
    LayerHandle layer = LayerManager::FindLayer( layer_ids[ 0 ] );
    Core::Transform inverse_src_trans = layer->get_grid_transform().get_inverse();
    this->private_->range_min_ = inverse_src_trans * this->private_->range_min_;
    this->private_->range_max_ = inverse_src_trans * this->private_->range_max_;
  }
  
  if ( this->private_->crop_ )
  {
    if ( this->private_->range_max_[ 0 ] <= this->private_->range_min_[ 0 ] ||
         this->private_->range_max_[ 1 ] <= this->private_->range_min_[ 1 ] ||
         this->private_->range_max_[ 2 ] <= this->private_->range_min_[ 2 ] )
    {
      context->report_error( "Invalid resample range." );
      return false;
    }
    
    if ( ! PadValues::IsValidPadding( this->private_->padding_ ) )
    {
      context->report_error( "Unknown padding option" );
      return false;
    }
  }
  
  if ( ! ( NrrdResampleFilter::IsNrrdResample( this->private_->kernel_ ) ||
           ITKResampleFilter::IsITKResample( this->private_->kernel_ ) ) )
  {
    context->report_error( "Unknown kernel type" );
    return false;
  }

  if ( ( this->private_->kernel_ == ITKResampleFilter::B_SPLINE_C ) &&
       ( this->private_->spline_order_ < 0 || this->private_->spline_order_ > 5 ) )
  {
    context->report_error( "Spline order must be between 0 and 5." );
    return false;
  }

  // Validation successful
  return true;
}

bool ActionResample::run( Core::ActionContextHandle& context, 
                          Core::ActionResultHandle& result )
{
  if ( NrrdResampleFilter::IsNrrdResample( this->private_->kernel_ ) )
  {
    // Create algorithm
    boost::shared_ptr< NrrdResampleFilter > algo( new NrrdResampleFilter( this->private_->kernel_, this->private_->gauss_sigma_, this->private_->gauss_cutoff_, this->private_->replace_, this->private_->crop_, this->private_->padding_, this->private_->range_min_, this->private_->range_max_, this->private_->sandbox_ ) );

    algo->setup_layers( this->private_->layer_ids_, this->private_->match_grid_transform_, this->private_->grid_transform_, this->private_->x_, this->private_->y_, this->private_->z_ );

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
  }
  else // ITK
  {
    // Create algorithm
    boost::shared_ptr< ITKResampleFilter > algo( new ITKResampleFilter( this->private_->kernel_, this->private_->spline_order_, this->private_->replace_, this->private_->crop_, this->private_->padding_, this->private_->range_min_, this->private_->range_max_, this->private_->sandbox_ ) );

    algo->setup_layers( this->private_->layer_ids_, this->private_->match_grid_transform_, this->private_->grid_transform_, this->private_->x_, this->private_->y_, this->private_->z_ );

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
  }

  return true;
}

// used by resample tool
void ActionResample::Dispatch( Core::ActionContextHandle context,
                               const std::vector< std::string >& layer_ids,
                               int x, int y, int z,
                               const std::string& kernel,
                               double gauss_sigma,
                               double gauss_cutoff,
                               int spline_order,
                               bool replace
                             )
{
  ActionResample* action = new ActionResample;
  action->private_->layer_ids_ = layer_ids;
  action->private_->x_ = static_cast< unsigned int >(x);
  action->private_->y_ = static_cast< unsigned int >(y);
  action->private_->z_ = static_cast< unsigned int >(z);
  action->private_->kernel_ = kernel;
  action->private_->gauss_sigma_ = gauss_sigma;
  action->private_->gauss_cutoff_ = gauss_cutoff;
  action->private_->spline_order_ = spline_order;
  action->private_->replace_ = replace;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

// used by resample tool
void ActionResample::Dispatch( Core::ActionContextHandle context,
                               const std::vector< std::string >& layer_ids,
                               const Core::GridTransform& grid_trans,
                               const std::string& padding,
                               const std::string& kernel,
                               double gauss_sigma,
                               double gauss_cutoff,
                               int spline_order,
                               bool replace
                             )
{
  ActionResample* action = new ActionResample;

  action->private_->layer_ids_ = layer_ids;
  action->private_->x_ = static_cast< unsigned int >( grid_trans.get_nx() );
  action->private_->y_ = static_cast< unsigned int >( grid_trans.get_ny() );
  action->private_->z_ = static_cast< unsigned int >( grid_trans.get_nz() );
  action->private_->crop_ = true;
  action->private_->padding_ = padding;

  action->private_->match_grid_transform_ = true;
  action->private_->grid_transform_ = grid_trans;

  action->private_->kernel_ = kernel;
  action->private_->gauss_sigma_ = gauss_sigma;
  action->private_->gauss_cutoff_ = gauss_cutoff;
  action->private_->spline_order_ = spline_order;
  action->private_->replace_ = replace;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

// used by layer resampler
void ActionResample::Dispatch( Core::ActionContextHandle context,
                               const std::string& src_layer,
                               const std::string& dst_layer,
                               const std::string& padding,
                               const std::string& kernel,
                               double gauss_sigma,
                               double gauss_cutoff,
                               int spline_order,
                               bool replace
                             )
{
  LayerHandle layer = LayerManager::FindLayer( dst_layer );
  if ( layer )
  {
    std::vector< std::string > layer_ids( 1, src_layer );
    ActionResample::Dispatch( context, layer_ids, layer->get_grid_transform(),
      padding, kernel, gauss_sigma, gauss_cutoff, spline_order, replace );
  }
}

} // end namespace Seg3D
