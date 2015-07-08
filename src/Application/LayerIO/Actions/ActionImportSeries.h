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

#ifndef APPLICATION_LAYERIO_ACTIONS_ACTIONIMPORTSERIES_H
#define APPLICATION_LAYERIO_ACTIONS_ACTIONIMPORTSERIES_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

// Application includes
#include <Application/LayerIO/LayerIO.h>
#include <Application/LayerIO/LayerImporter.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{
  
class ActionImportSeries : public LayerAction
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "ImportSeries", "This action imports a series into the layer manager.")
  CORE_ACTION_ARGUMENT( "filenames", "The list of files to load." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "importer", "", "Optional name for a specific importer." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "mode", "data", "The mode to use: data, single_mask, bitplane_mask, or label_mask.")
  CORE_ACTION_OPTIONAL_ARGUMENT( "inputfiles_id", "-1" , "Location of the file if it is in the data cache of the project." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )  
  CORE_ACTION_CHANGES_PROJECT_DATA()
)

  // -- Constructor/Destructor --
public:
  ActionImportSeries()
  {
    this->add_parameter( this->filenames_ );
    this->add_parameter( this->importer_ );
    this->add_parameter( this->mode_ );
    this->add_parameter( this->inputfiles_id_ );
    this->add_parameter( this->sandbox_ );
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

  // The filename of the file to load
  std::vector< std::string > filenames_;

  // If the data is located in the data cache it is located in this directory
  ProvenanceID inputfiles_id_;
  
  // How should the file be loaded
  std::string mode_;

  // Which type of importer should we use
  std::string importer_;

  // The sandbox in which to run the action
  SandboxID sandbox_;

  // Short cut to the layer importer that has already loaded the data if the file
  // was read through the GUI
  LayerImporterHandle layer_importer_;
  
  // -- Dispatch this action from the interface --
public:
  // DISPATCH:
  // Create and dispatch action that moves the layer above 
  static void Dispatch( Core::ActionContextHandle context, 
    const std::vector< std::string >& filenames, 
    const std::string& mode = LayerIO::DATA_MODE_C, const std::string importer = "" );

  // DISPATCH:
  // To avoid reading a file twice, this action has a special option, so it can take an
  // importer that has already loaded the file. This prevents it from being read twice
  static void Dispatch( Core::ActionContextHandle context, 
    const LayerImporterHandle& importer, 
    const std::string& mode = LayerIO::DATA_MODE_C );
  
};
  
} // end namespace Seg3D

#endif
