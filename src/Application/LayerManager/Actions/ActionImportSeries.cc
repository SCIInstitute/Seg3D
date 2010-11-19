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

// Boost includes
#include <boost/filesystem.hpp>

// Application includes
#include <Application/LayerManager/Actions/ActionImportSeries.h>
#include <Application/LayerIO/LayerIO.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/LayerManager/LayerUndoBufferItem.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, ImportSeries )

namespace Seg3D
{

bool ActionImportSeries::validate( Core::ActionContextHandle& context )
{
  boost::filesystem::path full_filename( filename_.value() );
  
  if ( !( boost::filesystem::exists ( full_filename ) ) )
  {
    context->report_error( std::string( "File '" ) + this->filename_.value() +
      "' does not exist." );
    return false;
  }

  if ( !this->layer_importer_ )
  {
    if ( !( LayerIO::Instance()->create_importer( this->filename_.value(),  
      this->layer_importer_, this->importer_.value() ) ) )
    {
      context->report_error( std::string( "Could not create importer with name '" ) +
        this->importer_.value() + "' for file '" + this->filename_.value() + "'." );
      return false;
    } 
  }

  LayerImporterMode mode = LayerImporterMode::INVALID_E;
  if ( !( ImportFromString( this->mode_.value(), mode ) ) )
  {
    context->report_error( std::string( "Import mode '") +  this->mode_.value() + 
      "' is not a valid layer importer mode." );
    return false;
  }

  if ( !( this->layer_importer_->import_header() ) )
  {
    context->report_error( std::string( "Could not interpret file '" +
      this->filename_.value() + "' with importer '" + this->importer_.value() + "'" ) );
    return false;
  }
  
  if ( !( this->layer_importer_->get_importer_modes() & mode ) )
  {
    context->report_error( std::string( "Import mode '") +  this->mode_.value() + 
      "' is not available for this importer." );
    return false; 
  }

  return true; // validated
}

bool ActionImportSeries::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  // Get the current counters for groups and layers, so we can undo the changes to those counters
  // NOTE: This needs to be done before a new layer is created
  LayerManager::id_count_type id_count = LayerManager::GetLayerIdCount();

  std::string file_or_folder_name;
  file_or_folder_name = boost::filesystem::path( 
    this->layer_importer_->get_filename() ).parent_path().filename();

  std::string message = std::string("Importing '") + file_or_folder_name + std::string("'");
    
  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();
  
  LayerImporterMode mode = LayerImporterMode::INVALID_E;
  ImportFromString( this->mode_.value(), mode );

  std::vector<LayerHandle> layers;
  if ( !( this->layer_importer_->import_layer( mode, layers ) ) ) 
  {
    context->report_error( "Layer importer failed" );
    return false;
  }
  
  for (size_t j = 0; j < layers.size(); j++)
  {
    if ( layers[ j ] ) LayerManager::Instance()->insert_layer( layers[ j ] );
  }

  progress->end_progress_reporting();

  // Create an undo item for this action
  LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "Import Layer" ) );
  // Tell which action has to be re-executed to obtain the result
  item->set_redo_action( this->shared_from_this() );
  // Tell which layer was added so undo can delete it
  for (size_t j = 0; j < layers.size(); j++)
  {
    item->add_layer_to_delete( layers[ j ] );
  }
  // Tell what the layer/group id counters are so we can undo those as well
  item->add_id_count_to_restore( id_count );
  // Add the complete record to the undo buffer
  UndoBuffer::Instance()->insert_undo_item( context, item );

  return true;
}

void ActionImportSeries::clear_cache()
{
  this->layer_importer_.reset();
}

Core::ActionHandle ActionImportSeries::Create( const std::string& filename, 
  const std::string& mode, const std::string importer )
{
  // Create new action
  ActionImportSeries* action = new ActionImportSeries;
  
  // Set action parameters
  action->filename_.value() = filename;
  action->mode_.value()   = mode;
  action->importer_.value() = importer;
    
  // Post the new action
  return Core::ActionHandle( action );
}

Core::ActionHandle ActionImportSeries::Create( const LayerImporterHandle& importer, 
  LayerImporterMode mode )
{
  // Create new action
  ActionImportSeries* action = new ActionImportSeries;
  
  // Fill in the short cut so the data that was already read is not lost
  action->layer_importer_ = importer;

  // We need to fill in these to ensure the action can be replayed without the importer present
  action->filename_.value() = importer->get_filename();
  action->mode_.value()     = ExportToString(mode);
  action->importer_.value() = importer->name();

  // Post the new action
  return Core::ActionHandle( action );
}

void ActionImportSeries::Dispatch( Core::ActionContextHandle context, const std::string& filename, 
  const std::string& mode, const std::string importer )
{
  Core::ActionDispatcher::PostAction( Create( filename, mode, importer ), context );
}

void ActionImportSeries::Dispatch( Core::ActionContextHandle context, 
  const LayerImporterHandle& importer, LayerImporterMode mode )
{
  Core::ActionDispatcher::PostAction( Create( importer, mode ), context );
}
  
} // end namespace Seg3D
