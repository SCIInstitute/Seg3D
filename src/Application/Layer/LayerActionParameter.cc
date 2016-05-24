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
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerActionParameter.h>

namespace Seg3D
{

LayerActionParameter::~LayerActionParameter()
{
}

bool LayerActionParameter::has_extension() const
{
  return true;
}

LayerActionLayerID::LayerActionLayerID( std::string& layer_id ) :
  layer_id_( layer_id ),
  provenance_id_( -1 )
{
}

LayerActionLayerID::~LayerActionLayerID()
{
}

bool LayerActionLayerID::import_from_string( const std::string& str )
{
  return Core::ImportFromString( str, this->layer_id_ );
}

std::string LayerActionLayerID::export_to_string() const
{
  return Core::ExportToString( this->layer_id_ );
}

bool LayerActionLayerID::translate_provenance( ProvenanceIDList& input_provenance )
{
  if ( this->layer_id_ == "" || this->layer_id_ == "<none>" )
  {
    // Nothing to translate
    return true;
  }
  
  LayerHandle layer = LayerManager::FindLayer( this->layer_id_ );
  if ( !layer )
  {
    ProvenanceID prov_id;
    if ( Core::ImportFromString( this->layer_id_, prov_id ) )
    {
      layer = LayerManager::FindLayer( prov_id );
    }
  }
  
  if ( layer ) 
  {
    this->provenance_id_ = layer->provenance_id_state_->get();
    input_provenance.push_back( this->provenance_id_ );
    return true;
  }
  else
  {
    return false;
  } 
}

std::string LayerActionLayerID::export_to_provenance_string( 
  size_t& input_counter, bool single_input ) const
{
  if ( this->layer_id_ == "" || this->layer_id_ == "<none>" )
  {
    return "'" + this->layer_id_ + "'";
  }
  
  return "${" + Core::ExportToString( input_counter++ ) + "}";
}


LayerActionGroupID::LayerActionGroupID( std::string& group_id ) :
  group_id_( group_id ),
  provenance_id_( -1 )
{
}

LayerActionGroupID::~LayerActionGroupID()
{
}

bool LayerActionGroupID::import_from_string( const std::string& str )
{
  return Core::ImportFromString( str, this->group_id_ );
}

std::string LayerActionGroupID::export_to_string() const
{
  return Core::ExportToString( this->group_id_ );
}

bool LayerActionGroupID::translate_provenance( ProvenanceIDList& input_provenance )
{
  if ( this->group_id_ == "" || this->group_id_ == "<none>" )
  {
    // Nothing to translate
    return true;
  }
  
  LayerGroupHandle group = LayerManager::FindGroup( this->group_id_ );
  if ( !group )
  {
    ProvenanceID prov_id;
    if ( Core::ImportFromString( this->group_id_, prov_id ) )
    {
      group = LayerManager::FindGroup( prov_id );
    }
  }
  
  if ( group ) 
  {
    this->provenance_id_ = group->provenance_id_state_->get();
    input_provenance.push_back( this->provenance_id_ );
    return true;
  }
  else
  {
    return false;
  } 
}

std::string LayerActionGroupID::export_to_provenance_string(
  size_t& input_counter, bool single_input ) const
{
  if ( this->group_id_ == "" || this->group_id_ == "<none>" )
  {
    return "'" + this->group_id_ + "'";
  }

  return "${" + Core::ExportToString( input_counter++ ) + "}";
}

LayerActionLayerIDList::LayerActionLayerIDList( std::vector<std::string>& layer_id_list ) :
  layer_id_list_( layer_id_list )
{
}

LayerActionLayerIDList::~LayerActionLayerIDList()
{
}

bool LayerActionLayerIDList::import_from_string( const std::string& str )
{
  return Core::ImportFromString( str, this->layer_id_list_ );
}

std::string LayerActionLayerIDList::export_to_string() const
{
  return Core::ExportToString( this->layer_id_list_ );
}

bool LayerActionLayerIDList::translate_provenance( ProvenanceIDList& input_provenance )
{
  for ( size_t j = 0; j < this->layer_id_list_.size(); j++ )
  {
    if ( this->layer_id_list_[ j ] == "" || this->layer_id_list_[ j ] == "<none>" ) continue;
    
    LayerHandle layer = LayerManager::FindLayer( this->layer_id_list_[ j ] );
    if ( !layer )
    {
      ProvenanceID prov_id;
      if ( Core::ImportFromString( layer_id_list_[ j ], prov_id ) )
      {
        layer = LayerManager::FindLayer( prov_id );
      }
    }
    
    if ( layer ) 
    {
      ProvenanceID prov_id = layer->provenance_id_state_->get();
      this->provenance_id_list_.push_back( prov_id ); 
      input_provenance.push_back( prov_id );
    }
    else
    {
      return false;
    }
  }

  return true;
}

std::string LayerActionLayerIDList::export_to_provenance_string( 
  size_t& input_counter, bool single_input ) const
{
  size_t num_ids = this->layer_id_list_.size();
  assert( num_ids > 0 );
  std::string str = "[";
  if ( single_input )
  {
    str += "${" + Core::ExportToString( input_counter++ ) + "}]";
  }
  else
  {
    for ( size_t i = 0; i < num_ids; ++i )
    {
      if ( this->layer_id_list_[ i ] == "" || this->layer_id_list_[ i ] == "<none>" )
      {
        str += "'<none>',";
      }
      else
      {
        str += "${" + Core::ExportToString( input_counter++ ) + "},";
      }
    }
    str[ str.size() - 1 ] = ']' ;
  }

  return str;
}

} // namespace Seg3D
