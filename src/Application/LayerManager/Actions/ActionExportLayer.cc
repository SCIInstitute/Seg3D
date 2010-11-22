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
#include <Application/LayerManager/Actions/ActionExportLayer.h>
#include <Application/LayerIO/LayerIO.h>
#include <Application/LayerManager/LayerManager.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, ExportLayer )

namespace Seg3D
{

bool ActionExportLayer::validate( Core::ActionContextHandle& context )
{
  if( this->layer_.value() != "" )
  {
    std::vector< LayerHandle > layer_handles;
    
    LayerHandle temp_handle = LayerManager::Instance()->get_layer_by_name( this->layer_.value() );
    if( !temp_handle ) return false;
    else layer_handles.push_back( temp_handle );
  
    std::string extension = boost::filesystem::path( this->file_path_.value() ).extension(); 
    
    if( ! ( LayerIO::Instance()->create_exporter( this->layer_exporter_, layer_handles, this->exporter_.value(), extension ) ) )
    {
      return false;
    }
  } 
  
  // first we validate the path for saving the segmentation
  boost::filesystem::path segmentation_path( this->file_path_.value() );
  if ( !( boost::filesystem::exists ( segmentation_path.parent_path() ) ) )
  {
    context->report_error( std::string( "The path '" ) + this->file_path_.value() +
      "' does not exist." );
    return false;
  }

  return true; // validated
}

bool ActionExportLayer::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  boost::filesystem::path filename_and_path = boost::filesystem::path( this->file_path_.value() );
  std::string filename_without_extension = filename_and_path.filename();
  filename_without_extension = filename_without_extension.substr( 0, 
    filename_without_extension.find_last_of( "." ) );
    
  std::string message = std::string( "Exporting '" + filename_without_extension + "'" );

  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();
    
  this->layer_exporter_->export_layer( LayerExporterMode::DATA_E, 
    filename_and_path.parent_path().string(), filename_without_extension );

  progress->end_progress_reporting();

  return true;
}

Core::ActionHandle ActionExportLayer::Create( const LayerExporterHandle& exporter, 
  const std::string& file_path  )
{
  // Create new action
  ActionExportLayer* action = new ActionExportLayer;
  
  action->layer_exporter_ = exporter;
  action->file_path_.value() = file_path;
  
  // Post the new action
  return Core::ActionHandle( action );
}

void ActionExportLayer::clear_cache()
{
}

void ActionExportLayer::Dispatch( Core::ActionContextHandle context, 
  const LayerExporterHandle& exporter, const std::string& file_path )
{
  Core::ActionDispatcher::PostAction( Create( exporter, file_path ), context );
}

} // end namespace Seg3D
