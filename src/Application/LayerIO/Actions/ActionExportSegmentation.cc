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
#include <Application/LayerIO/Actions/ActionExportSegmentation.h>
#include <Application/LayerIO/LayerIO.h>
#include <Application/Layer/LayerManager.h>
#include <Application/ProjectManager/ProjectManager.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, ExportSegmentation )

namespace Seg3D
{

// TODO: needs complete overhaul

bool ActionExportSegmentation::validate( Core::ActionContextHandle& context )
{
  if ( this->mode_ == LayerIO::DATA_MODE_C )
  {
    context->report_error( std::string( "You cannot export data as a segmention." ) );
    return false;
  }

  if ( this->layer_exporter_.get() == nullptr && this->layers_.empty() )
  {
    context->report_error( std::string("Either provide a list of layers, or an exporter to this action.") );
    return false;
  }

  if ( ! this->layers_.empty() )
  {
    std::vector< LayerHandle > layer_handles;
    std::vector< double > values;

    if ( this->mode_ == LayerIO::LABEL_MASK_MODE_C )
    {
      // insert empty background layer - datablock will be filled in by exporter
      Core::MaskLayerHandle background_layer;
      layer_handles.push_back(background_layer);
    }

    // TODO: hardcode for now, list of values needs to be optional argument
    int valuesCounter = 0; // background label = 0 matches segmentation export wizard default
    values.push_back(valuesCounter);
    valuesCounter += 1;

    for ( size_t i = 0; i < this->layers_.size(); ++i )
    {
      LayerHandle temp_handle = LayerManager::Instance()->find_layer_by_id( layers_[ i ] );

      if ( ! temp_handle )
      {
        context->report_error( std::string( "No valid layer ids were given" ) );
        return false;
      }

      if ( temp_handle->get_type() == Core::VolumeType::MASK_E )
      {
        layer_handles.push_back( temp_handle );
        values.push_back(valuesCounter);
        valuesCounter += 1;
      }
      else
      {
        context->report_error("ExportSegmentation exports mask layers to file. Use ExportLayer for data layers.");
        return false;
      }
    }

    // TODO: too much repeated code - refactor (see ActionExportLayer)
    // Also, this should be in run()
    if ( this->extension_ == ".nrrd" )
    {
      if ( ! LayerIO::Instance()->create_exporter( this->layer_exporter_, layer_handles,
                                                  "NRRD Exporter", this->extension_ ) )
      {
        context->report_error( "Could not create NRRD exporter." );
        return false;
      }
    }
    else if ( this->extension_ == ".mat" )
    {
      if( ! LayerIO::Instance()->create_exporter( this->layer_exporter_, layer_handles,
                                                 "Matlab Exporter", this->extension_ ) )
      {
        context->report_error( "Could not create Matlab exporter." );
        return false;
      }
    }
    else if ( this->extension_ == ".mrc" )
    {
      if( ! LayerIO::Instance()->create_exporter( this->layer_exporter_, layer_handles,
                                                 "MRC Exporter", this->extension_ ) )
      {
        context->report_error( "Could not create MRC2000 exporter." );
        return false;
      }
    }
    else
    {
      if ( ! LayerIO::Instance()->create_exporter( this->layer_exporter_, layer_handles,
                                                 "ITK Mask Exporter", this->extension_ ) )
      {
        context->report_error( "Could not create ITK exporter." );
        return false;
      }
    }

    // TODO: hack!
    if ( ( ! this->layer_exporter_->label_layer_values_set() ) &&
         ( this->mode_ == LayerIO::LABEL_MASK_MODE_C ) )
    {
      this->layer_exporter_->set_label_layer_values( values );
    }
  }
  
  boost::filesystem::path segmentation_path( this->file_path_ );
  if ( ! boost::filesystem::exists( segmentation_path.parent_path() ) )
  {
    std::ostringstream error;
    error << "The path '" << this->file_path_ << "' does not exist.";
    context->report_error( error.str() );
    return false;
  }
  
  return true; // validated
}

bool ActionExportSegmentation::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  std::string message = std::string( "Exporting your selected segmentations." );
    
  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();
    
  boost::filesystem::path filename_and_path = boost::filesystem::path( this->file_path_ );
  std::string filename_without_extension = filename_and_path.filename().string();
  filename_without_extension = filename_without_extension.substr( 0, 
    filename_without_extension.find_last_of( "." ) );

  if ( this->mode_ == LayerIO::SINGLE_MASK_MODE_C )
  {
    this->layer_exporter_->export_layer( this->mode_, filename_and_path.string(), "unused" );

    ProjectManager::Instance()->current_file_folder_state_->set( 
      filename_and_path.parent_path().string() );
  }
  else
  {
    this->layer_exporter_->export_layer( this->mode_, filename_and_path.parent_path().string(), 
      filename_without_extension );

    ProjectManager::Instance()->current_file_folder_state_->set(
      filename_and_path.parent_path().string() );
  }

  ProjectManager::Instance()->checkpoint_projectmanager();

  progress->end_progress_reporting();

  return true;
}

void ActionExportSegmentation::clear_cache()
{
  this->layer_exporter_.reset();
}

void ActionExportSegmentation::Dispatch( Core::ActionContextHandle context, 
                                         const LayerExporterHandle& exporter,
                                         const std::string& mode,
                                         const std::string& file_path,
                                         const std::string& extension )
{
  // Create new action
  ActionExportSegmentation* action = new ActionExportSegmentation;
  
  action->extension_ = extension;
  action->layer_exporter_ = exporter;
  action->mode_ = mode;
  action->file_path_ = file_path;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

void ActionExportSegmentation::Dispatch( Core::ActionContextHandle context, 
                                         const std::vector< std::string >& layers,
                                         const std::string& mode,
                                         const std::string& file_path,
   const std::string&extension )
{
  // Create new action
  ActionExportSegmentation* action = new ActionExportSegmentation;
  
  action->extension_ = extension;
  action->layers_ = layers;
  action->mode_ = mode;
  action->file_path_ = file_path;
  
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

void ActionExportSegmentation::Dispatch( Core::ActionContextHandle context,
                                         const std::string& layer,
                                         const std::string& mode,
                                         const std::string& file_path,
                                         const std::string& extension )
{
  std::vector< std::string > layers;
  layers.push_back( layer );
  Dispatch( context, layers, mode, file_path, extension );
}


} // end namespace Seg3D
