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

#ifndef APPLICATION_TOOL_ACTIONS_ACTIONIMPORTLAYER_H
#define APPLICATION_TOOL_ACTIONS_ACTIONIMPORTLAYER_H

// Application includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>
#include <Application/LayerIO/LayerImporter.h>

namespace Seg3D
{
  
class ActionImportLayer : public Core::Action
{
  CORE_ACTION( "ImportLayer", "ImportLayer <filename>  [mode=data|single_mask|biplane_mask|label_mask] [importer=name_importer]" );
  
  // -- Constructor/Destructor --
public:
  ActionImportLayer() :
    filename_(""),
    mode_("data"),
    importer_("")
  {
    add_argument( filename_ );
    add_parameter( "mode", mode_ );
    add_parameter( "importer", importer_ );
  }
  
  virtual ~ActionImportLayer()
  {
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // -- Action parameters --
private:
  // ToolID that is requested
  Core::ActionParameter< std::string > filename_;
  Core::ActionParameter< std::string > mode_;
  Core::ActionParameter< std::string > importer_;
  
  // TODO: possible memory leak here ...
  LayerImporterHandle layer_importer_;
  
  // -- Dispatch this action from the interface --
public:
  // CREATE:
  // Create action that moves the layer above
  static Core::ActionHandle Create( const std::string& filename, const std::string& mode = "data",
    const std::string importer = "" );

  // CREATE:
  // Create action that moves the layer above
  static Core::ActionHandle Create( const LayerImporterHandle& importer, LayerImporterMode mode );
  
  // DISPATCH:
  // Create and dispatch action that moves the layer above 
  static void Dispatch( const std::string& filename, const std::string& mode = "data",
    const std::string importer = "" );

  // DISPATCH:
  // To avoid reading a file twice, this action has a special option, so it can take an
  // importer that has already loaded the file. This prevents it from being read twice
  static void Dispatch( const LayerImporterHandle& importer, LayerImporterMode mode );
  
};
  
} // end namespace Seg3D

#endif