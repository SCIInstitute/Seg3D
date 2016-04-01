/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef APPLICATION_LAYERIO_ACTIONS_ACTIONEXPORTLAYER_H
#define APPLICATION_LAYERIO_ACTIONS_ACTIONEXPORTLAYER_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/LayerIO/LayerExporter.h>

namespace Seg3D
{
  
class ActionExportLayer : public Core::Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "ExportLayer", "This action exports a data layer to file.")
  CORE_ACTION_ARGUMENT( "layer", "The name of the data layer to be exported." )
  CORE_ACTION_ARGUMENT( "file_path", "A path, including the name of the file where the layer should be exported to." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "exporter", "", "Optional name for a specific exporter." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "extension", "", "Extension used for saving file." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
)

  // -- Constructor/Destructor --
public:
  ActionExportLayer()
  {
    this->add_parameter( this->layer_id_ );
    this->add_parameter( this->file_path_ );
    this->add_parameter( this->exporter_ );
    this->add_parameter( this->extension_ );
  }
  
  // -- Functions that describe action --
public:
  // VALIDATE:
  // Each action needs to be validated just before it is posted. This way we
  // enforce that every action that hits the main post_action signal will be
  // a valid action to execute.
  virtual bool validate( Core::ActionContextHandle& context );

  // RUN:
  // Each action needs to have this piece implemented. It spells out how the
  // action is run. It returns whether the action was successful or not.
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // CLEAR_CACHE:
  // Clear any objects that were given as a short cut to improve performance.
  virtual void clear_cache(); 

  void initialize_action( const std::string& layer_id,
                          const std::string& file_path,
                          LayerExporterHandle layer_exporter,
                          const std::string& extension );


  // -- Action parameters --
private:
  // Where the layer should be exported
  std::string file_path_;

  // Which type of exporter should we use
  std::string exporter_;
  
  // the name of the layer to be exported
  std::string layer_id_;

  // Extension for saving data
  std::string extension_;

  std::string filename_base_;
  
  // Short cut to the layer exporter that has already loaded the data if the file
  // was read through the GUI
  LayerExporterHandle layer_exporter_;
  
  
  // -- Dispatch this action from the interface --
public:

  // DISPATCH:
  static void Dispatch( Core::ActionContextHandle context, const std::string& layer, 
    const std::string& file_path, const std::string extension = "" );
    
  static void Dispatch( Core::ActionContextHandle context, const std::string& layer,
    LayerExporterHandle exporter, const std::string& file_path ); 
};
  
} // end namespace Seg3D

#endif
