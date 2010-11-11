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
#include <Application/LayerManager/Actions/ActionImportLayer.h>
#include <Application/LayerIO/LayerIO.h>
#include <Application/LayerManager/LayerManager.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, ImportLayer )

namespace Seg3D
{

bool ActionImportLayer::validate( Core::ActionContextHandle& context )
{
  boost::filesystem::path full_filename( filename_.value() );
  
  if ( !( boost::filesystem::exists ( full_filename ) ) )
  {
    context->report_error( std::string( "File '" ) + this->filename_.value() +
      "' does not exist." );
    return false;
  }

  if ( !( this->layer_importer_ ) )
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

  if ( !( this->layer_importer_->get_importer_modes() & mode ) )
  {
    context->report_error( std::string( "Import mode '") +  this->mode_.value() + 
      "' is not available for this importer." );
    return false; 
  }

  return true; // validated
}

bool ActionImportLayer::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  std::string file_or_folder_name;
  if( this->series_import_.value() == true )
  {
    file_or_folder_name = boost::filesystem::path( 
      this->layer_importer_->get_filename() ).parent_path().filename();
  }
  else
  {
    file_or_folder_name = this->layer_importer_->get_filename();
  }

  std::string message = std::string("Importing '") + file_or_folder_name + std::string("'");
    
  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();
  
  LayerImporterMode mode = LayerImporterMode::INVALID_E;
  ImportFromString( this->mode_.value(), mode );

  std::vector<LayerHandle> layers;
  bool succeed = this->layer_importer_->import_layer( mode, layers );
    
  for (size_t j = 0; j < layers.size(); j++)
  {
    if ( layers[ j ] ) LayerManager::Instance()->insert_layer( layers[ j ] );
  }

  progress->end_progress_reporting();

  return succeed;
}

void ActionImportLayer::clear_cache()
{
  this->layer_importer_.reset();
}

Core::ActionHandle ActionImportLayer::Create( const std::string& filename, 
  const std::string& mode, const std::string importer )
{
  // Create new action
  ActionImportLayer* action = new ActionImportLayer;
  
  // Set action parameters
  action->filename_.value() = filename;
  action->mode_.value()   = mode;
  action->importer_.value() = importer;
  action->series_import_.value() = false;
  
  // Post the new action
  return Core::ActionHandle( action );
}

Core::ActionHandle ActionImportLayer::Create( const LayerImporterHandle& importer, 
  LayerImporterMode mode, bool series_import )
{
  // Create new action
  ActionImportLayer* action = new ActionImportLayer;
  
  // Fill in the short cut so the data that was already read is not lost
  action->layer_importer_ = importer;

  // We need to fill in these to ensure the action can be replayed without the importer present
  action->filename_.value() = importer->get_filename();
  action->mode_.value()     = ExportToString(mode);
  action->importer_.value() = importer->name();
  action->series_import_.value() = series_import;
  
  // Post the new action
  return Core::ActionHandle( action );
}

void ActionImportLayer::Dispatch( Core::ActionContextHandle context, const std::string& filename, 
  const std::string& mode, const std::string importer )
{
  Core::ActionDispatcher::PostAction( Create( filename, mode, importer ), context );
}

void ActionImportLayer::Dispatch( Core::ActionContextHandle context, 
  const LayerImporterHandle& importer, LayerImporterMode mode, bool series_import )
{
  Core::ActionDispatcher::PostAction( Create( importer, mode, series_import ), context );
}
  
} // end namespace Seg3D
