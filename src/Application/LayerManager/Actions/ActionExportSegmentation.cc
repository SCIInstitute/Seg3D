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
  // first we validate the path for saving the segmentation
  boost::filesystem::path segmentation_path( this->file_path_.value() );
  if ( !( boost::filesystem::exists ( segmentation_path ) ) )
  {
    context->report_error( std::string( "The path '" ) + this->file_path_.value() +
      "' does not exist." );
    return false;
  }
  
  // next we validate the layers
  std::vector< std::string > masks_vector = Core::SplitString( this->masks_.value(), "|" );
  for( int i = 1; i < static_cast< int >( masks_vector.size() ); ++i )
  {
    std::vector< std::string > mask_name = Core::SplitString( masks_vector[ i ], "," );
    if( !( LayerManager::Instance()->get_layer_by_name( mask_name[ 0 ] ) ) )
    {
      context->report_error( std::string( "The mask '" ) + mask_name[ 0 ] +
        "' does not exist." );
      return false;
    }
  }

  return true; // validated
}

bool ActionExportSegmentation::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  std::string message = std::string( "Exporting your segmentation..." );
    
  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();

  std::vector< std::string > masks_vector = Core::SplitString( this->masks_.value(), "|" );
  
  LayerManager::Instance()->export_segmentation( masks_vector, this->file_path_.value(), 
    this->segmentation_name_.value(), this->single_file_.value() );

  progress->end_progress_reporting();

  return true;
}

Core::ActionHandle ActionExportSegmentation::Create( const std::string& masks, 
  const std::string& file_path, bool single_file  )
{
  // Create new action
  ActionExportSegmentation* action = new ActionExportSegmentation;
  
  if( single_file )
  {
    action->file_path_.value() = boost::filesystem::path( file_path ).parent_path().string();
    action->segmentation_name_.value() = boost::filesystem::path( file_path ).filename();
  }
  else
  {
    action->file_path_.value() = file_path;
    action->segmentation_name_.value() = "unused";
  }
  
  action->masks_.value() = masks;
  action->single_file_.value() = single_file;
  
  // Post the new action
  return Core::ActionHandle( action );
}

void ActionExportSegmentation::Dispatch( Core::ActionContextHandle context, 
  const std::string& masks, const std::string& file_path, bool single_file )
{
  Core::ActionDispatcher::PostAction( Create( masks, file_path, single_file ), context );
}
  
} // end namespace Seg3D
