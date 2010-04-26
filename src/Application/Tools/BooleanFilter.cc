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

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/BooleanFilter.h>
#include <Application/LayerManager/LayerManager.h>

namespace Seg3D
{

// Register the tool into the tool factory
SCI_REGISTER_TOOL(BooleanFilter)

BooleanFilter::BooleanFilter( const std::string& toolid ) :
  Tool( toolid )
{
  // add default values for the the states
  add_state( "mask_a", this->mask_a_state_, "<none>" );
  add_state( "mask_b", this->mask_b_state_, "<none>" );
  add_state( "mask_c", this->mask_c_state_, "<none>" );
  add_state( "mask_d", this->mask_d_state_, "<none>" );
  add_state( "example_expressions", this->example_expressions_state_, "<none>", "<none>" );
  add_state( "replace", this->replace_state_, false );

  this->handle_layers_changed();
  
  // Add constaints, so that when the state changes the right ranges of
  // parameters are selected
  this->add_connection ( this->mask_a_state_->value_changed_signal_.connect( boost::bind( 
    &BooleanFilter::target_constraint, this, _1 ) ) );
  this->add_connection ( this->mask_b_state_->value_changed_signal_.connect( boost::bind( 
    &BooleanFilter::target_constraint, this, _1 ) ) );
  this->add_connection ( this->mask_c_state_->value_changed_signal_.connect( boost::bind( 
    &BooleanFilter::target_constraint, this, _1 ) ) );
  this->add_connection ( this->mask_d_state_->value_changed_signal_.connect( boost::bind( 
    &BooleanFilter::target_constraint,this, _1 ) ) );
  
  this->add_connection ( LayerManager::Instance()->layers_changed_signal_.connect(
    boost::bind( &BooleanFilter::handle_layers_changed, this ) ) );
}


BooleanFilter::~BooleanFilter()
{
  disconnect_all();
}
  
void BooleanFilter::handle_layers_changed()
{
  std::vector< LayerHandle > target_layers;
  LayerManager::Instance()->get_layers( target_layers );
  bool mask_a_found = false;
  bool mask_b_found = false;
  bool mask_c_found = false;
  bool mask_d_found = false;
  
  for( int i = 0; i < static_cast< int >( target_layers.size() ); ++i )
  {
    if( ( this->mask_a_state_->get() == "<none>" ) && ( target_layers[i]->type() == 
                           Utils::VolumeType::MASK_E ) )
    {
      this->mask_a_state_->set( target_layers[i]->get_layer_name(), ActionSource::NONE_E );
    }
    if( ( this->mask_b_state_->get() == "<none>" ) && ( target_layers[i]->type() == 
                           Utils::VolumeType::MASK_E ) )
    {
      this->mask_b_state_->set( target_layers[i]->get_layer_name(), ActionSource::NONE_E );
    }
    if( ( this->mask_c_state_->get() == "<none>" ) && ( target_layers[i]->type() == 
                           Utils::VolumeType::MASK_E ) )
    {
      this->mask_c_state_->set( target_layers[i]->get_layer_name(), ActionSource::NONE_E );
    }
    if( ( this->mask_d_state_->get() == "<none>" ) && ( target_layers[i]->type() == 
                           Utils::VolumeType::MASK_E ) )
    {
      this->mask_d_state_->set( target_layers[i]->get_layer_name(), ActionSource::NONE_E );
    }

  
    if( target_layers[i]->get_layer_name() == this->mask_a_state_->get() ) 
      mask_a_found = true;
    
    if( target_layers[i]->get_layer_name() == this->mask_b_state_->get() )
      mask_b_found = true;
    
    if( target_layers[i]->get_layer_name() == this->mask_c_state_->get() ) 
      mask_c_found = true;
    
    if( target_layers[i]->get_layer_name() == this->mask_d_state_->get() )
      mask_d_found = true;
  }
  
  if( !mask_a_found )
    this->mask_a_state_->set( "", ActionSource::NONE_E );
  
  if( !mask_b_found )
    this->mask_b_state_->set( "", ActionSource::NONE_E );
  
  if( !mask_c_found )
    this->mask_c_state_->set( "", ActionSource::NONE_E );
  
  if( !mask_d_found )
    this->mask_d_state_->set( "", ActionSource::NONE_E );
  
}


void BooleanFilter::target_constraint( std::string layerid )
{
}

void BooleanFilter::activate()
{
}

void BooleanFilter::deactivate()
{
}

} // end namespace Seg3D


