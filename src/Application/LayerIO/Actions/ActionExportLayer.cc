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

#include <sstream>

// Application includes
#include <Application/LayerIO/Actions/ActionExportLayer.h>
#include <Application/LayerIO/LayerIO.h>
#include <Application/Layer/LayerManager.h>
#include <Application/ProjectManager/ProjectManager.h>


// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, ExportLayer )

namespace Seg3D
{

bool ActionExportLayer::validate( Core::ActionContextHandle& context )
{
  // Check whether the layer exists and is of the right type and return an
  // error if not
  if ( ! LayerManager::CheckLayerExistence( this->layer_id_, context ) ) return false;

    std::vector< LayerHandle > layer_handles;
  
    LayerHandle layer = LayerManager::Instance()->find_layer_by_id( this->layer_id_ );
    if (! layer ) 
    {
      return false;
    }

    if ( layer->get_type() == Core::VolumeType::DATA_E )
    {
      layer_handles.push_back( layer );
    }
    else
    {
      context->report_error("ExportLayer exports a data layer to file. Use ExportSegmentation for mask layers.");
      return false;
    }
  
    if ( this->extension_.empty() )
    {
      this->extension_ = boost::filesystem::extension( boost::filesystem::path( this->file_path_ ) );
    }
    else
    {
      // TODO: test this!!!
      this->file_path_ += this->extension_;
    }
  
    if ( this->exporter_.empty() )
    {
      if ( this->extension_ == ".nrrd" ) this->exporter_ = "NRRD Exporter";
      else if ( this->extension_ == ".mat" ) this->exporter_ = "Matlab Exporter";
      else if ( this->extension_ == ".dcm" ) this->exporter_ = "ITK Data Exporter";
      else if ( this->extension_ == ".mrc" ) this->exporter_ = "MRC Exporter";
      else if ( this->extension_ != "" ) this->exporter_ = "ITK Data Exporter";
    }
  
    if ( ! ( LayerIO::Instance()->create_exporter( this->layer_exporter_, layer_handles, 
      this->exporter_, this->extension_ ) ) )
    {
      context->report_error( "Could not create the specified exporter." );
      return false;
    }
  
    // first we validate the path for saving the segmentation
    boost::filesystem::path segmentation_path( this->file_path_ );
    if ( !( boost::filesystem::exists ( segmentation_path.parent_path() ) ) )
    {
      std::ostringstream error;
      error << "The path '" << this->file_path_ << "' does not exist.";
      context->report_error( error.str() );
      return false;
    }

    return true; // validated
}

bool ActionExportLayer::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  boost::filesystem::path filename_and_path = boost::filesystem::path( this->file_path_ );
  std::string filename_without_extension = filename_and_path.filename().string();
  filename_without_extension = filename_without_extension.substr( 0, 
    filename_without_extension.find_last_of( "." ) );
    
  std::string message = std::string( "Exporting '" + filename_without_extension + "'" );

  Core::ActionProgressHandle progress =
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();
    
  this->layer_exporter_->export_layer( LayerIO::DATA_MODE_C, filename_and_path.parent_path().string(),
    filename_without_extension );

  ProjectManager::Instance()->current_file_folder_state_->set( 
    filename_and_path.parent_path().string() );
  ProjectManager::Instance()->checkpoint_projectmanager();

  progress->end_progress_reporting();

  return true;
}

void ActionExportLayer::clear_cache()
{
  this->layer_exporter_.reset();
}


void ActionExportLayer::Dispatch( Core::ActionContextHandle context, const std::string& layer,
  const std::string& file_path, const std::string extension )
{
  // Create new action
  ActionExportLayer* action = new ActionExportLayer;

  action->layer_id_ = layer;
  action->file_path_ = file_path;
  action->extension_ = extension;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

void ActionExportLayer::Dispatch( Core::ActionContextHandle context, const std::string& layer, 
  LayerExporterHandle exporter, const std::string& file_path )
{
  // Create new action
  ActionExportLayer* action = new ActionExportLayer;

  action->layer_id_ = layer;
  action->layer_exporter_ = exporter;
  action->file_path_ = file_path;
  action->extension_ = "";

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
