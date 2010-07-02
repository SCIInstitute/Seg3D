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
#include <Application/Tools/ThresholdTool.h>
#include <Application/LayerManager/LayerManager.h>

namespace Seg3D
{

const size_t ThresholdTool::VERSION_NUMBER_C = 1;

// Register the tool into the tool factory
SCI_REGISTER_TOOL(ThresholdTool)

ThresholdTool::ThresholdTool( const std::string& toolid, bool auto_number ) :
  Tool( toolid, VERSION_NUMBER_C, auto_number )
{
  // Need to set ranges and default values for all parameters
  add_state( "target", this->target_layer_state_, "<none>" );
  add_state( "upper_threshold", this->upper_threshold_state_, 1.0, 0.0, 1.0, .01 );
  add_state( "lower_threshold", this->lower_threshold_state_, 0.0, 0.0, 1.0, .01 );
  
  this->handle_layers_changed();

  // Add constaints, so that when the state changes the right ranges of
  // parameters are selected
  this->add_connection ( this->target_layer_state_->value_changed_signal_.connect( boost::bind(
      &ThresholdTool::target_constraint, this, _1 ) ) );

  // If a layer is added or deleted update the lists
  this->add_connection ( LayerManager::Instance()->layers_changed_signal_.connect(
    boost::bind( &ThresholdTool::handle_layers_changed, this ) ) );

}

ThresholdTool::~ThresholdTool()
{
  disconnect_all();
}

void ThresholdTool::handle_layers_changed()
{
  std::vector< LayerHandle > target_layers;
  LayerManager::Instance()->get_layers( target_layers );
  bool target_found = false;
  
  for( int i = 0; i < static_cast< int >( target_layers.size() ); ++i )
  {
    if( ( this->target_layer_state_->get() == "<none>" ) && ( target_layers[i]->type() == 
                                 Core::VolumeType::DATA_E ) )
    {
      this->target_layer_state_->set( target_layers[i]->get_layer_name(), 
        Core::ActionSource::NONE_E );
      target_found = true;
      break;
    }
    if( target_layers[i]->get_layer_name() == target_layer_state_->get() ) {
      target_found = true;
      break;
    }
  }
  
  if( !target_found )
    target_layer_state_->set( "", Core::ActionSource::NONE_E );
  
}

void ThresholdTool::target_constraint( std::string layerid )
{
}

void ThresholdTool::activate()
{
}

void ThresholdTool::deactivate()
{
}

} // end namespace Seg3D
