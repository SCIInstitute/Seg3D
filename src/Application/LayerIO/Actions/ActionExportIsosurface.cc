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
#include <Application/LayerIO/Actions/ActionExportIsosurface.h>
#include <Application/LayerIO/LayerIO.h>
#include <Application/Layer/LayerManager.h>
#include <Application/ProjectManager/ProjectManager.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, ExportIsosurface )

namespace Seg3D
{

bool ActionExportIsosurface::validate( Core::ActionContextHandle& context )
{
  
  LayerHandle temp_handle = LayerManager::Instance()->find_layer_by_id( this->layer_ );
  if ( ! temp_handle )
  {
    context->report_error( std::string( "No valid layer was given." ) );
    return false;
  }
  
  MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( temp_handle.get() );  
  
  if ( ! mask_layer->iso_generated_state_->get() )
  {
    context->report_error( std::string( "Isosurface not created for this layer.  Please create isosurface before exporting." ) );
    return false;
  }

  if ( mask_layer->get_isosurface()->surface_area() == 0 )
  {
    context->report_error( std::string( "Isosurface is empty." ) );
    return false;
  }
  
  boost::filesystem::path isosurface_path( this->file_path_ );
  if ( ! boost::filesystem::exists ( isosurface_path.parent_path() ) )
  {
    std::ostringstream error;
    error << "The path '" << this->file_path_ << "' does not exist.";
    context->report_error( error.str() );
    return false;
  }
  
  boost::filesystem::path extension = isosurface_path.extension();
  
  if (! ( (extension == ".fac") ||
          (extension == ".pts") ||
          (extension == ".val") ||
          (extension == ".stl") ||
          (extension == ".vtk") ) )
  {
    std::ostringstream error;
    error << extension << " is not supported for isosurface export.";
    context->report_error( error.str() );
  }
  
  if (this->name_ == "<none>")
  {
    // fall back to layer ID
    this->name_ = this->layer_;
  }
  
  return true; // validated
}

bool ActionExportIsosurface::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  std::string message = std::string( "Exporting your selected isosurface." );
  
  Core::ActionProgressHandle progress = 
  Core::ActionProgressHandle( new Core::ActionProgress( message ) );
  
  progress->begin_progress_reporting();
  
  boost::filesystem::path filename_and_path = boost::filesystem::path( this->file_path_ );
  boost::filesystem::path extension = filename_and_path.extension();
  
  LayerHandle temp_handle = LayerManager::Instance()->find_layer_by_id( this->layer_ );
  MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( temp_handle.get() );
  
  if ( (extension == ".fac") || (extension == ".pts") || (extension == ".val") )
  {
    boost::filesystem::path file_path = filename_and_path.parent_path();
    boost::filesystem::path file_prefix = filename_and_path.stem();
    mask_layer->get_isosurface()->export_legacy_isosurface(file_path, file_prefix.string());
  }
  else if (extension == ".stl")
  {
    mask_layer->get_isosurface()->export_stl_isosurface( filename_and_path, this->name_ );
  }
  else
  {
    mask_layer->get_isosurface()->export_vtk_isosurface( filename_and_path );
  }
  
  ProjectManager::Instance()->current_file_folder_state_->set( filename_and_path.parent_path().string() );
  
  ProjectManager::Instance()->checkpoint_projectmanager();
  
  progress->end_progress_reporting();
  
  return true;
}

void ActionExportIsosurface::clear_cache()
{
}

void ActionExportIsosurface::Dispatch( Core::ActionContextHandle context,
                                      const std::string& layer_id, 
                                      const std::string& file_path,
                                      const std::string& name )
{
  // Create new action
  ActionExportIsosurface* action = new ActionExportIsosurface;
  
  action->layer_ = layer_id;
  action->file_path_ = file_path;
  action->name_ = name;
  
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
