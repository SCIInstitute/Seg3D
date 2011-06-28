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

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerActionParameter.h>

namespace Seg3D
{

class LayerActionPrivate
{
public:
  // Input provenance ids.
  ProvenanceIDList input_provenance_ids_;

  // Output provenance ids
  ProvenanceIDList output_provenance_ids_;
  
  // Keep track of whether the supplied output provenance ids
  // have been used before
  bool use_provenance_ids_list_;
};


LayerAction::LayerAction() :
  private_( new LayerActionPrivate )
{
  this->private_->use_provenance_ids_list_ = false; 
}

void LayerAction::add_group_id( std::string& group_id )
{
  this->add_param( new LayerActionGroupID( group_id ) );
}

void LayerAction::add_layer_id( std::string& layer_id )
{
  this->add_param( new LayerActionLayerID( layer_id ) );
}

void LayerAction::add_layer_id_list( std::vector<std::string>& layer_id_list )
{
  this->add_param( new LayerActionLayerIDList( layer_id_list ) );
}


bool LayerAction::translate( Core::ActionContextHandle& context )
{
  // Check whether we should remove the supplied provenance ids for the output
  // If we redoing an action it should generate the same outputs, like wise from
  // the provenance buffer it should generate output with the same provenance ids.
  // Hence we special case these circumstances here, and we do not clear the ids
  // provided. If the action is however replayed but not from any of those buffers
  // the list should be cleared.
  if( this->private_->use_provenance_ids_list_ == true ||
    context->source() == Core::ActionSource::UNDOBUFFER_E ||
    context->source() == Core::ActionSource::PROVENANCE_E )
  {
    this->private_->use_provenance_ids_list_ = false;
  }
  else
  {
    this->private_->output_provenance_ids_.clear();
  }
  
  this->private_->input_provenance_ids_.clear();
  
  size_t num_params = this->num_params();
  for ( size_t j = 0; j < num_params; j++ )
  {
    Core::ActionParameterBase* param = this->get_param( j );
    if ( param->has_extension() )
    {
      LayerActionParameter* layer_param = static_cast< LayerActionParameter* >( param );
      if (! ( layer_param->translate_provenance( this->private_->input_provenance_ids_ ) ) )
      {
        return false;
      }
    }
  }
  
  return true;
}

ProvenanceIDList LayerAction::get_input_provenance_ids()
{
  return this->private_->input_provenance_ids_;
}

void LayerAction::set_output_provenance_ids( const ProvenanceIDList& provenance_ids )
{
  this->private_->output_provenance_ids_ = provenance_ids;
  this->private_->use_provenance_ids_list_ = true;
}

ProvenanceID LayerAction::get_output_provenance_id( size_t index )
{
  while ( index >= this->private_->output_provenance_ids_.size() )
  {
    this->private_->output_provenance_ids_.push_back( GenerateProvenanceID() ); 
  }
  
  return this->private_->output_provenance_ids_[ index ];
}

ProvenanceIDList LayerAction::get_output_provenance_ids( size_t num_provenance_ids )
{
  while ( num_provenance_ids > this->private_->output_provenance_ids_.size() )
  {
    this->private_->output_provenance_ids_.push_back( GenerateProvenanceID() ); 
  }

  return this->private_->output_provenance_ids_;
}

void LayerAction::generate_output_provenance_ids( size_t num_provenance_ids )
{
  while ( num_provenance_ids > this->private_->output_provenance_ids_.size() )
  {
    this->private_->output_provenance_ids_.push_back( GenerateProvenanceID() ); 
  }
}

std::string LayerAction::export_to_provenance_string() const
{
  // Add action name to string
  std::string command = std::string( this->get_type() ) + " ";

  // Loop through all the arguments and add them  
  size_t num_params = this->num_params();
    
  for ( size_t j = 0; j < num_params; j++ )
  {
    Core::ActionParameterBase* param = this->get_param( j );
    if ( param->has_extension() )
    {
      LayerActionParameter* layer_param = reinterpret_cast< LayerActionParameter* >( param );
      command += this->get_key( j ) + "=" + 
        layer_param->export_to_provenance_string() + " ";
    }
    else
    {
      command += this->get_key( j ) + "=" + param->export_to_string() + " ";
    }
  }

  // Return the command
  return command;
}

} // end namespace Seg3D
