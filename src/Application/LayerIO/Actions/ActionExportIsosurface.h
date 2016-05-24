/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#ifndef APPLICATION_LAYERIO_ACTIONS_ACTIONEXPORTISOSURFACE_H
#define APPLICATION_LAYERIO_ACTIONS_ACTIONEXPORTISOSURFACE_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/LayerIO/LayerExporter.h>

namespace Seg3D
{

class ActionExportIsosurface : public Core::Action
{
  
CORE_ACTION( 
  CORE_ACTION_TYPE( "ExportIsosurface", "This action exports an isosurface to file.")
  CORE_ACTION_ARGUMENT( "layer", "layer to be exported." )
  CORE_ACTION_ARGUMENT( "file_path", "A path, including the name of the file where the layer should be exported to." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "name", "<none>", "Optional dataset name. Currently only used for STL files (defaults to layer ID if name is not set)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "binary", "false", "Optionally export binary file. Currently only available for STL files.")
  CORE_ACTION_CHANGES_PROJECT_DATA()
)
  
  // -- Constructor/Destructor --
public:
  ActionExportIsosurface()
  {
    this->add_parameter( this->layer_ );
    this->add_parameter( this->file_path_ );
    this->add_parameter( this->name_ );
    this->add_parameter( this->binary_file_export_ );
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
  
  // -- Action parameters --
private:
  
  // Where the layer should be exported
  std::string file_path_;
  
  // The layer to be exported
  std::string layer_;
  
  // Optional dataset name
  std::string name_;

  // Optionally export binary file format
  bool binary_file_export_;

  // -- Dispatch this action from the interface --
public:

  // DISPATCH:
  static void Dispatch( Core::ActionContextHandle context,
                       const std::string& layer_id, 
                       const std::string& file_path,
                       const std::string& name = "<none>",
                       const bool binary_file_export = false );
};

} // end namespace Seg3D

#endif
