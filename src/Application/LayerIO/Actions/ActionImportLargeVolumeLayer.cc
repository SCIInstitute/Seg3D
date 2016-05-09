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

// Boost includes
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Action/ActionDispatcher.h>
#include <Core/Action/ActionFactory.h>
#include <Core/LargeVolume/LargeVolumeSchema.h>


// Application includes
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/LayerIO/LayerIO.h>
#include <Application/LayerIO/Actions/ActionImportLargeVolumeLayer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LargeVolumeLayer.h>
#include <Application/Layer/LayerUndoBufferItem.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, ImportLargeVolumeLayer )

namespace Seg3D
{

bool ActionImportLargeVolumeLayer::validate( Core::ActionContextHandle& context )
{
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) )
  {
    return false;
  }
  
  // Validate whether the filename is actually valid
  
  // Convert the file to a boost filename
  boost::filesystem::path full_filename( this->dir_ );
    
  // Check whether the file exists
  if ( !( boost::filesystem::exists( full_filename ) ) )
  {
    context->report_error( std::string( "File '" ) + this->dir_ + "' does not exist." );
    return false;
  }

  // Update the filename to include the full path, so the filename has an absolute path
  try
  {
    full_filename = boost::filesystem::absolute( full_filename );
  }
  catch ( ... )
  {
    context->report_error(  std::string( "Could not determine full path of '" ) +
      this->dir_ + "'." );
    return false;
  }
  
  // Reinsert the filename in the action
  this->dir_ = full_filename.string();

  return true; // validated
}


bool ActionImportLargeVolumeLayer::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  // Get the current counters for groups and layers, so we can undo the changes to those counters
  // NOTE: This needs to be done before a new layer is created
  LayerManager::id_count_type id_count = LayerManager::GetLayerIdCount();

  // Forwarding a message to the UI that we are importing a layer. This generates a progress bar
  std::string message = std::string( "Importing '" ) + this->dir_ + std::string( "'" );
  Core::ActionProgressHandle progress;

  // Progress reporting is only needed if not running in a sandbox
  if ( this->sandbox_ == -1 )
  {
    progress.reset( new Core::ActionProgress( message ) );
    // Indicate that we have started the process
    progress->begin_progress_reporting();
  }
  
  // The ImporterFileData is an abstraction of all the data can be extracted from the file
  LayerImporterFileDataHandle data;
  
  // Get the data from the file
  Core::LargeVolumeSchemaHandle schema(new Core::LargeVolumeSchema);
  schema->set_dir( this->dir_ );
  std::string error;
  if (! schema->load( error ) )
  {
    context->report_error( error );

    // We are done processing
    
    progress->end_progress_reporting();
    return false;
  }

  LayerHandle layer( new LargeVolumeLayer( boost::filesystem::path( this->dir_ ).stem().string(), schema ) );

  // Now insert the layers one by one into the layer manager.
  std::string layer_id;

  if ( this->sandbox_ == -1 ) layer->provenance_id_state_->set( this->get_output_provenance_id( 0 ) );
  LayerManager::Instance()->insert_layer( layer, this->sandbox_ );
  layer_id = layer->get_layer_id();

  // Report the layer IDs to action result
  result.reset( new Core::ActionResult( layer_id ) );

  if ( this->sandbox_ != -1 ) {
    // We are done processing
    progress->end_progress_reporting();
    return true;
  }

  // Now the layers are properly inserted, generate the undo item that will undo this action.
  {
    // Create a provenance record
    ProvenanceStepHandle provenance_step( new ProvenanceStep );
    
    // Get the input provenance ids from the translate step
    provenance_step->set_input_provenance_ids( this->get_input_provenance_ids() );
    
    // Get the output and replace provenance ids from the analysis above
    provenance_step->set_output_provenance_ids( this->get_output_provenance_ids() );
      
    // Get the action and turn it into provenance 
    provenance_step->set_action_name( this->get_type() );
    provenance_step->set_action_params( this->export_params_to_provenance_string() );   
    
    // Add step to provenance record
    ProvenanceStepID step_id = ProjectManager::Instance()->get_current_project()->
      add_provenance_record( provenance_step );   

    // Create an undo item for this action
    LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "Import Large Volume Layer" ) );

    // Tell which action has to be re-executed to obtain the result
    item->set_redo_action( this->shared_from_this() );

    // Tell which provenance record to delete when undone
    item->set_provenance_step_id( step_id );

    // Tell which layer was added so undo can delete it
    item->add_layer_to_delete( layer );

    // Tell what the layer/group id counters are so we can undo those as well
    item->add_id_count_to_restore( id_count );
    
    // Add the complete record to the undo buffer
    UndoBuffer::Instance()->insert_undo_item( context, item );
  }
  
  boost::filesystem::path full_filename( this->dir_ );
  ProjectManager::Instance()->current_file_folder_state_->set( 
    full_filename.parent_path().string() );
  ProjectManager::Instance()->checkpoint_projectmanager();

  // We are done processing
  progress->end_progress_reporting();

  return true;
}

void ActionImportLargeVolumeLayer::Dispatch( Core::ActionContextHandle context, const std::string& dir )
{
  // Create new action
  ActionImportLargeVolumeLayer* action = new ActionImportLargeVolumeLayer;
  
  // Set action parameters
  action->dir_ = dir;
  
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
