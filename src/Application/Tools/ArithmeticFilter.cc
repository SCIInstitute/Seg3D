/*
 For more information, please see: http://software.sci.utah.edu
 ArithmeticFilter
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

#include <Core/Utils/ScopedCounter.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/ArithmeticFilter.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Filters/Actions/ActionArithmeticFilter.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, ArithmeticFilter )

namespace Seg3D
{

ArithmeticFilter::ArithmeticFilter( const std::string& toolid ) :
  SingleTargetTool( Core::VolumeType::DATA_E|Core::VolumeType::MASK_E,  toolid )
{
  // Create an empty list of label options
  std::vector< LayerIDNamePair > empty_list( 1, 
    std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );
  
  this->add_state( "input_b", this->input_b_state_, Tool::NONE_OPTION_C, empty_list );
  this->add_extra_layer_input( this->input_b_state_, Core::VolumeType::ALL_REGULAR_E );
  this->add_state( "input_c", this->input_c_state_, Tool::NONE_OPTION_C, empty_list );
  this->add_extra_layer_input( this->input_c_state_, Core::VolumeType::ALL_REGULAR_E );
  this->add_state( "input_d", this->input_d_state_, Tool::NONE_OPTION_C, empty_list );
  this->add_extra_layer_input( this->input_d_state_, Core::VolumeType::ALL_REGULAR_E );
  
  this->add_state( "expressions", this->expressions_state_, "" );

  this->add_state( "output_type", this->output_type_state_, ActionArithmeticFilter::DATA_C, 
    ActionArithmeticFilter::DATA_C + "=Data Layer|" + ActionArithmeticFilter::MASK_C + 
    "=Mask Layer" );

  this->add_state( "replace", this->replace_state_, false );
  this->add_state( "preserve_data_format", this->preserve_data_format_state_, false );
  this->add_state( "output_is_data", this->output_is_data_state_, false );  
  this->add_state( "input_matches_output", this->input_matches_output_state_, false );  
  
  this->add_connection( this->target_layer_state_->state_changed_signal_.connect( 
    boost::bind( &ArithmeticFilter::update_output_type, this ) ) );

  this->add_connection( this->output_type_state_->state_changed_signal_.connect( 
    boost::bind( &ArithmeticFilter::update_replace_options, this ) ) );

  // Make sure output type matches initial input type 
  this->update_output_type();
} 

ArithmeticFilter::~ArithmeticFilter()
{
  this->disconnect_all();
}

void ArithmeticFilter::update_output_type()
{
  LayerHandle layer = LayerManager::Instance()->find_layer_by_id( this->target_layer_state_->get() );
  if ( layer )
  {
    if ( layer->get_type() == Core::VolumeType::DATA_E ) 
    {
      this->output_type_state_->set( ActionArithmeticFilter::DATA_C );
    }
    else
    {
      this->output_type_state_->set( ActionArithmeticFilter::MASK_C );  
    }
  }

  // If output type didn't actually change, state_changed_signal won't be emitted, so this 
  // function needs to be called manually
  this->update_replace_options();
}

void ArithmeticFilter::update_replace_options()
{
  LayerHandle layer = LayerManager::Instance()->find_layer_by_id( this->target_layer_state_->get() );
  if( layer )
  {
    if( ( layer->get_type() == Core::VolumeType::DATA_E && 
      this->output_type_state_->get() == ActionArithmeticFilter::DATA_C ) ||
      ( layer->get_type() == Core::VolumeType::MASK_E && 
      this->output_type_state_->get() == ActionArithmeticFilter::MASK_C ) )
    {
      this->input_matches_output_state_->set( true );
    }
    else 
    {
      this->input_matches_output_state_->set( false );
    }
  }
  else
  {
    this->input_matches_output_state_->set( false );
  }

  this->output_is_data_state_->set( 
    this->output_type_state_->get() == ActionArithmeticFilter::DATA_C );
}

void ArithmeticFilter::execute( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  // Get action inputs from state engine
  std::vector< std::string > layer_ids;
  
  layer_ids.push_back( this->target_layer_state_->get() );
  layer_ids.push_back( this->input_b_state_->get() );
  layer_ids.push_back( this->input_c_state_->get() );
  layer_ids.push_back( this->input_d_state_->get() );

  ActionArithmeticFilter::Dispatch( context, layer_ids, this->expressions_state_->get(),
    this->output_type_state_->get(), this->replace_state_->get(), 
    this->preserve_data_format_state_->get() );
}

} // end namespace Seg3D
