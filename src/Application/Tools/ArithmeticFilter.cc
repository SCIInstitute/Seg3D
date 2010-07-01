/*
 For more information, please see: http://software.sci.utah.edu
 ArithmeticFilter
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

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/ArithmeticFilter.h>
#include <Application/LayerManager/LayerManager.h>

namespace Seg3D
{

const size_t ArithmeticFilter::version_number_ = 1;

// Register the tool into the tool factory
SCI_REGISTER_TOOL( ArithmeticFilter )

// Constructor, set default values
ArithmeticFilter::ArithmeticFilter( const std::string& toolid, bool auto_number ) :
  Tool( toolid, version_number_, auto_number )
{
  // add default values for the the states
  add_state( "volume_a", this->volume_a_state_, "<none>" );
  add_state( "volume_b", this->volume_b_state_, "<none>" );
  add_state( "volume_c", this->volume_c_state_, "<none>" );
  add_state( "example_expressions", this->example_expressions_state_, "<none>", "<none>" );
  add_state( "replace", this->replace_state_, false );

  this->handle_layers_changed();

  // Add constaints, so that when the state changes the right ranges of
  // parameters are selected
  this->add_connection ( this->volume_a_state_->value_changed_signal_.connect( boost::bind(
      &ArithmeticFilter::target_constraint, this, _1 ) ) );
  this->add_connection ( this->volume_b_state_->value_changed_signal_.connect( boost::bind(
      &ArithmeticFilter::target_constraint, this, _1 ) ) );
  this->add_connection ( this->volume_c_state_->value_changed_signal_.connect( boost::bind(
      &ArithmeticFilter::target_constraint, this, _1 ) ) );
  
  this->add_connection ( LayerManager::Instance()->layers_changed_signal_.connect(
    boost::bind( &ArithmeticFilter::handle_layers_changed, this ) ) );

} // end constructor

ArithmeticFilter::~ArithmeticFilter()
{
  disconnect_all();
}
  
void ArithmeticFilter::handle_layers_changed()
{
  std::vector< LayerHandle > target_layers;
  LayerManager::Instance()->get_layers( target_layers );
  bool volume_a_found = false;
  bool volume_b_found = false;
  bool volume_c_found = false;
  
  for( int i = 0; i < static_cast< int >( target_layers.size() ); ++i )
  {
    if( ( this->volume_a_state_->get() == "<none>" ) && ( target_layers[i]->type() == 
                                 Core::VolumeType::DATA_E ) )
    {
      this->volume_a_state_->set( target_layers[i]->get_layer_name(), Core::ActionSource::NONE_E );
    }
    if( ( this->volume_b_state_->get() == "<none>" ) && ( target_layers[i]->type() == 
                               Core::VolumeType::DATA_E ) )
    {
      this->volume_b_state_->set( target_layers[i]->get_layer_name(), Core::ActionSource::NONE_E );
    }
    if( ( this->volume_c_state_->get() == "<none>" ) && ( target_layers[i]->type() == 
                               Core::VolumeType::DATA_E ) )
    {
      this->volume_c_state_->set( target_layers[i]->get_layer_name(), Core::ActionSource::NONE_E );
    }
  
    if( target_layers[i]->get_layer_name() == this->volume_a_state_->get() ) 
      volume_a_found = true;
    
    if( target_layers[i]->get_layer_name() == this->volume_b_state_->get() )
      volume_b_found = true;
    
    if( target_layers[i]->get_layer_name() == this->volume_c_state_->get() )
      volume_c_found = true;
  }
  
  if( !volume_a_found )
    this->volume_a_state_->set( "", Core::ActionSource::NONE_E );
  
  if( !volume_b_found )
    this->volume_b_state_->set( "", Core::ActionSource::NONE_E );
  
  if( !volume_c_found )
    this->volume_c_state_->set( "", Core::ActionSource::NONE_E );
  
}

void ArithmeticFilter::target_constraint( std::string layerid )
{
}
  
void ArithmeticFilter::activate()
{
}

void ArithmeticFilter::deactivate()
{
}

} // end namespace Seg3D


