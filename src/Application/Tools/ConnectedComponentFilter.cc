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
#include <Application/Layer/LayerFWD.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/ConnectedComponentFilter.h>
#include <Application/LayerManager/LayerManager.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, ConnectedComponentFilter )

namespace Seg3D 
{
  
ConnectedComponentFilter::ConnectedComponentFilter( const std::string& toolid ) :
  SeedPointsTool( toolid )
{
  std::vector< LayerIDNamePair > empty_names( 1, 
    std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );
  
  this->handle_layers_changed();
      
  this->add_connection ( LayerManager::Instance()->layers_changed_signal_.connect(
    boost::bind( &ConnectedComponentFilter::handle_layers_changed, this ) ) );
}

ConnectedComponentFilter::~ConnectedComponentFilter()
{ 
  this->disconnect_all();
}

void ConnectedComponentFilter::handle_layers_changed()
{
  std::vector< LayerIDNamePair > layer_names( 1, 
    std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );
  LayerManager::Instance()->get_layer_names( layer_names, Core::VolumeType::DATA_E );
  this->target_layer_state_->set_option_list( layer_names );
}

void ConnectedComponentFilter::activate()
{
}

void ConnectedComponentFilter::deactivate()
{
}
  
} // end namespace Seg3D


