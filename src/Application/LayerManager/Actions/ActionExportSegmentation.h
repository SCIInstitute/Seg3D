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

#ifndef APPLICATION_LAYERMANAGER_ACTIONS_ACTIONEXPORTSEGMENTATION_H
#define APPLICATION_LAYERMANAGER_ACTIONS_ACTIONEXPORTSEGMENTATION_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

// Application includes
#include <Application/LayerIO/LayerImporter.h>

namespace Seg3D
{
  
class ActionExportSegmentation : public Core::Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "ExportSegmentation", "This action imports a layer into the layer manager.")
  CORE_ACTION_ARGUMENT( "segmentationname", "The name to be used for the segmentation." )
  CORE_ACTION_ARGUMENT( "masks", "A comma delimited list of layers to export as a segmentation." )
  CORE_ACTION_ARGUMENT( "path", "A path where the segmentation should be exported to." )
)

  // -- Constructor/Destructor --
public:
  ActionExportSegmentation()
  {
    add_argument( this->segmentation_name_ );
    add_argument( this->masks_ );
    add_argument( this->path_ );
  }
  
  virtual ~ActionExportSegmentation()
  {
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // -- Action parameters --
private:

  // The name for the segmentation
  Core::ActionParameter< std::string > segmentation_name_;

  // A comma delimited list of mask names to export
  Core::ActionParameter< std::string > masks_;

  // Where the segmentation should be saved
  Core::ActionParameter< std::string > path_;

  // -- Dispatch this action from the interface --
public:
  // CREATE:
  // Create action that exports a segmentation
  static Core::ActionHandle Create( const std::string& segmentation_name, 
    const std::string& masks, const std::string& path );

  // DISPATCH:
  // To avoid reading a file twice, this action has a special option, so it can take an
  // importer that has already loaded the file. This prevents it from being read twice
  static void Dispatch( Core::ActionContextHandle context, const std::string& segmentation_name, 
    const std::string& masks, const std::string& path );
  
};
  
} // end namespace Seg3D

#endif
