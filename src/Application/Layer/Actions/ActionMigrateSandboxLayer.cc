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

// Core includes
#include <Core/Action/ActionFactory.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/Actions/ActionMigrateSandboxLayer.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, MigrateSandboxLayer )

namespace Seg3D
{

class ActionMigrateSandboxLayerPrivate
{
public:
  std::string layer_id_;
  SandboxID sandbox_;
  ProvenanceID prov_id_;

  LayerHandle layer_;
};

ActionMigrateSandboxLayer::ActionMigrateSandboxLayer() :
  private_( new ActionMigrateSandboxLayerPrivate )
{
  this->add_parameter( this->private_->layer_id_ );
  this->add_parameter( this->private_->sandbox_ );
  this->add_parameter( this->private_->prov_id_ );
}

ActionMigrateSandboxLayer::~ActionMigrateSandboxLayer()
{
}

bool ActionMigrateSandboxLayer::validate( Core::ActionContextHandle& context )
{
  if ( !LayerManager::CheckLayerExistence( this->private_->layer_id_, this->private_->sandbox_ ) )
    return false;
  
  if ( !LayerManager::CheckLayerAvailabilityForProcessing( this->private_->layer_id_,
    context, this->private_->sandbox_ ) )
    return false;
  
  // Look up the source layer
  this->private_->layer_ = LayerManager::FindLayer( this->private_->layer_id_, this->private_->sandbox_ );
  if ( !this->private_->layer_ )
  {
    context->report_error( "Layer '" + this->private_->layer_id_ + "' doesn't exist in sandbox " +
      Core::ExportToString( this->private_->sandbox_ ) + "." );
    return false;
  }

  // Make sure the layer is valid
  if ( !this->private_->layer_->has_valid_data() )
  {
    context->report_error( "Layer '" + this->private_->layer_id_ + "' doesn't have valid data." );
    return false;
  }

  return true; // validated
}

bool ActionMigrateSandboxLayer::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{ 
  // Remove the source layer from the sandbox
  LayerManager::Instance()->delete_layer( this->private_->layer_, this->private_->sandbox_ );
  // Update the provenance ID of the layer
  this->private_->layer_->provenance_id_state_->set( this->private_->prov_id_ == -1 ? 
    GenerateProvenanceID() : this->private_->prov_id_ );
  // Insert  the layer back to the normal context
  LayerManager::Instance()->insert_layer( this->private_->layer_ );
  // Report the layer ID to action result
  result.reset( new Core::ActionResult( this->private_->layer_->get_layer_id() ) );
  return true;
}

void ActionMigrateSandboxLayer::clear_cache()
{
  this->private_->layer_.reset();
}

} // end namespace Seg3D
