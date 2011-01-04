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
#include <Application/LayerManager/Actions/ActionExportSegmentation.h>
#include <Application/LayerIO/LayerIO.h>
#include <Application/LayerManager/LayerManager.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, ExportSegmentation )

namespace Seg3D
{

bool ActionExportSegmentation::validate( Core::ActionContextHandle& context )
{
  if( this->mode_.value() == ExportToString( LayerExporterMode::DATA_E ) )
  {
    context->report_error( std::string( "You cannot export data as a segmention." ) );
      return false;
  } 
  
  if( this->layers_.value() != "" )
  {
    
    std::vector< std::string > layer_names = Core::SplitString( this->layers_.value(), "|" );
    std::vector< LayerHandle > layer_handles;

    for( size_t i = 0; i < layer_names.size(); ++i )
    {
    
      LayerHandle temp_handle = LayerManager::Instance()->get_layer_by_name( layer_names[ i ] );
      if( !temp_handle ) return false;
      else layer_handles.push_back( temp_handle );
    }

    if( this->extension_.value() == ".nrrd" )
    {
      if( ! LayerIO::Instance()->create_exporter( this->layer_exporter_, layer_handles, 
        "NRRD Exporter", this->extension_.value() ) )
      {
        return false;
      }
    }
    else
    {
      if( ! LayerIO::Instance()->create_exporter( this->layer_exporter_, layer_handles, 
        "ITK Mask Exporter", this->extension_.value() ) )
      {
        return false;
      }
    }
  } 

  boost::filesystem::path segmentation_path( this->file_path_.value() );
  if ( !( boost::filesystem::exists ( segmentation_path.parent_path() ) ) )
  {
    context->report_error( std::string( "The path '" ) + this->file_path_.value() +
      "' does not exist." );
    return false;
  }

  return true; // validated
}

bool ActionExportSegmentation::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  LayerExporterMode mode = LayerExporterMode::INVALID_E;
  ImportFromString( this->mode_.value(), mode );

  std::string message = std::string( "Exporting your selected segmentations." );
    
  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();
    
  boost::filesystem::path filename_and_path = boost::filesystem::path( this->file_path_.value() );
  std::string filename_without_extension = filename_and_path.filename();
  filename_without_extension = filename_without_extension.substr( 0, 
    filename_without_extension.find_last_of( "." ) );

  if( mode == LayerExporterMode::SINGLE_MASK_E )
  {
    this->layer_exporter_->export_layer( mode, filename_and_path.string(), "unused" );
  }
  else
  {
    this->layer_exporter_->export_layer( mode, 
      filename_and_path.parent_path().string(), filename_without_extension );
  }

  progress->end_progress_reporting();

  return true;
}

Core::ActionHandle ActionExportSegmentation::Create( const LayerExporterHandle& exporter, 
  LayerExporterMode mode, const std::string& file_path, std::string extension )
{
  // Create new action
  ActionExportSegmentation* action = new ActionExportSegmentation;
  
  action->extension_.value() = extension;
  action->layer_exporter_ = exporter;
  action->mode_.value() = ExportToString( mode );
  action->file_path_.value() = file_path;
  
  // Post the new action
  return Core::ActionHandle( action );
}
  
Core::ActionHandle ActionExportSegmentation::Create( const std::string& layer_id, 
  LayerExporterMode mode, const std::string& file_path, std::string extension  )
{
  // Create new action
  ActionExportSegmentation* action = new ActionExportSegmentation;
  
  action->extension_.value() = extension;
  action->layers_.value() = layer_id;
  action->mode_.value() = ExportToString( mode );
  action->file_path_.value() = file_path;
  
  // Post the new action
  return Core::ActionHandle( action );
}

void ActionExportSegmentation::clear_cache()
{
}

void ActionExportSegmentation::Dispatch( Core::ActionContextHandle context, 
  const LayerExporterHandle& exporter, LayerExporterMode mode, const std::string& file_path, 
  std::string extension )
{
  Core::ActionDispatcher::PostAction( Create( exporter, mode, file_path, extension ), context );
}

void ActionExportSegmentation::Dispatch( Core::ActionContextHandle context, const std::string& layer_id, 
  LayerExporterMode mode, const std::string& file_path, std::string extension )
{
  Core::ActionDispatcher::PostAction( Create( layer_id, mode, file_path, extension ), context );
}


} // end namespace Seg3D
