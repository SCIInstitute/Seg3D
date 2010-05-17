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

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>
#include <Core/State/State.h>
#include <Core/State/StateEngine.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Tool/Tool.h>
#include <Application/Tool/ToolFactory.h>

namespace Seg3D
{

const std::string Tool::NONE_OPTION_C( "<none>" );

Tool::Tool( const std::string& tool_type ) :
  StateHandler( tool_type, true )
{
}

Tool::~Tool()
{
}

void Tool::close()
{
  this->disconnect_all();
}

void Tool::activate()
{
  // Defaults to doing nothing
}

void Tool::deactivate()
{
  // Defaults to doing nothing
}

void Tool::CreateLayerNameList( std::vector<std::string> &layer_names, 
                 Core::VolumeType type )
{
  std::vector< LayerHandle > layers;
  LayerManager::Instance()->get_layers( layers );
  size_t num_of_layers = layers.size();
  for ( size_t i = 0; i < num_of_layers; i++ )
  {
    if ( layers[ i ]->type() == type )
    {
      layer_names.push_back( layers[ i ]->get_layer_name() );
    }
  }
}

void Tool::CreateLayerNameList( std::vector<std::string> &layer_names, 
                 Core::VolumeType type, LayerGroupHandle layer_group )
{
  layer_list_type layer_list;
  {
    LayerManager::lock_type lock( LayerManager::Instance()->get_mutex() );
    layer_list = layer_group->get_layer_list();
  }

  size_t num_of_layers = layer_list.size();
  layer_list_type::iterator it = layer_list.begin();
  for ( ; it != layer_list.end(); it++ )
  {
    std::string layer_name = ( *it )->get_layer_name();
    if ( ( *it )->type() == type )
    {
      layer_names.push_back( layer_name );
    }
  }
}

void Tool::CreateLayerNameList( std::vector<std::string> &layer_names, 
  Core::VolumeType type, LayerGroupHandle layer_group, std::string exclude )
{
  layer_list_type layer_list;
  {
    LayerManager::lock_type lock( LayerManager::Instance()->get_mutex() );
    layer_list = layer_group->get_layer_list();
  }

  size_t num_of_layers = layer_list.size();
  layer_list_type::iterator it = layer_list.begin();
  for ( ; it != layer_list.end(); it++ )
  {
    std::string layer_name = ( *it )->get_layer_name();
    if ( ( *it )->type() == type )
    {
      if ( layer_name != exclude )
      {
        layer_names.push_back( layer_name );
      }
    }
  }
}

} // end namespace Seg3D
