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

#ifndef APPLICATION_LAYERIO_ACTIONS_ACTIONEXPORTMATRIX_H
#define APPLICATION_LAYERIO_ACTIONS_ACTIONEXPORTMATRIX_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

//#include <Core/Geometry/Point.h>

namespace Seg3D
{

class ActionExportMatrix : public Core::Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "ExportMatrix", "This action exports a matrix of a given size to file.")
  CORE_ACTION_ARGUMENT( "file_path", "Path to the file that the points will be written to." )
  CORE_ACTION_ARGUMENT( "matrix", "Matrix of a given size." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
)

  // -- Constructor/Destructor --
public:
  ActionExportMatrix( const std::string& file_path, const std::vector<double>& matrix ) :
    file_path_(file_path),
    matrix_(matrix)
  {
    init_parameters();
  }

  ActionExportMatrix()
  {
    init_parameters();
  }

  // -- Functions that describe action --
public:
  // VALIDATE:
  // Each action needs to be validated just before it is posted. This way we
  // enforce that every action that hits the main post_action signal will be
  // a valid action to execute.
  virtual bool validate( Core::ActionContextHandle& context ) override;
  
  // RUN:
  // Each action needs to have this piece implemented. It spells out how the
  // action is run. It returns whether the action was successful or not.
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result ) override;

  void set_file_path( const std::string& file_path ) { file_path_ = file_path; }
  void set_matrix( const std::vector<double> matrix ) { matrix_ = matrix; }

  // -- Action parameters --
private:
  void init_parameters()
  {
    this->add_parameter( this->file_path_ );
    this->add_parameter( this->matrix_ );
  }

  // Where the layer should be exported
  std::string file_path_;
  // Matrix to be exported
  std::vector<double> matrix_;
  
  // -- Dispatch this action from the interface --
public:
  
  // DISPATCH:
  static void Dispatch( Core::ActionContextHandle context,
                        const std::string& file_path,
                        const std::vector<double>& matrix_ );
  
};

} // end namespace Seg3D

#endif
