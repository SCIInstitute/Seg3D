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

namespace Seg3D
{

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
SCI_REGISTER_ACTION(ImportLayer);

bool ActionImportLayer::validate( ActionContextHandle& context )
{
  boost::filesystem::path full_filename( filename_.value() );
  if ( !( boost::filesystem::exists ( full_filename ) ) )
  {
    context->report_error( std::string( "File '" ) + filename_.value() + "' does not exist." );
    return false;
  }

  if ( !( layer_importer_ ) )
  {
    if ( !( LayerIO::Instance()->create_importer( filename_.value(),  layer_importer_, 
      importer_.value() ) ) )
    {
      context->report_error( std::string( "Could not create importer with name '" ) +
        importer_.value() + "' for file '" + filename_.value() + "'." );
      return false;
    } 
  }

  LayerImporterMode mode = LayerImporterMode::DATA_E;
  if ( !( ImportFromString( mode_.value(), mode ) ) )
  {
    context->report_error( std::string( "Import mode '") +  mode_.value() + 
      "' is not a valid layer importer mode." );
    return false;
  }

  if ( !( layer_importer_->has_importer_mode( mode ) ) )
  {
    context->report_error( std::string( "Import mode '") +  mode_.value() + 
      "' is not available for this importer." );
    return false; 
  }

  return true; // validated
}

bool ActionImportLayer::run( ActionContextHandle& context, ActionResultHandle& result )
{
  LayerImporterMode mode( LayerImporterMode::DATA_E );
  ImportFromString( mode_.value(), mode );

  std::vector<LayerHandle> layers;
  layer_importer_->import_layer( layers, mode );
  
  SCI_LOG_DEBUG("Inserting layers");
  
  for (size_t j = 0; j < layers.size(); j++)
  {
    LayerManager::Instance()->insert_layer( layers[ j ] );
  }

  // As actions are only executed by one thread, modifications can be made. However this needs
  // to be in the part that only involves the execution and not the parameters of the action.
  layer_importer_.reset();

  return true;
}

ActionHandle ActionImportLayer::Create( const std::string& filename, const std::string& mode,
  const std::string importer )
{
  // Create new action
  ActionImportLayer* action = new ActionImportLayer;
  
  // Set action parameters
  action->filename_.value() = filename;
  action->mode_.value() = mode;
  action->importer_.value() = importer;
  
  // Post the new action
  return ActionHandle( action );
}

ActionHandle ActionImportLayer::Create( const LayerImporterHandle& importer, 
  LayerImporterMode mode )
{
  // Create new action
  ActionImportLayer* action = new ActionImportLayer;
  
  action->layer_importer_ = importer;

  // We need to fill in these to ensure the action can be replayed without the importer present
  action->filename_.value() = importer->get_filename();
  action->mode_.value()     = ExportToString(mode);
  action->importer_.value() = importer->name();
  
  // Post the new action
  return ActionHandle( action );
}

void ActionImportLayer::Dispatch( const std::string& filename, const std::string& mode,
  const std::string importer )
{
  Interface::PostAction( Create( filename, mode, importer ) );
}

void ActionImportLayer::Dispatch( const LayerImporterHandle& importer, LayerImporterMode mode )
{
  Interface::PostAction( Create( importer, mode ) );
}
  
} // end namespace Seg3D
