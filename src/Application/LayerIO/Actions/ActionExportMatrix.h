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

#include <Application/LayerIO/Actions/ActionExportVector.h>

namespace Seg3D
{
class MatrixDoubleWriter
{
public:
  MatrixDoubleWriter() {}
  MatrixDoubleWriter(int dim1, int dim2) : dim1_(dim1), dim2_(dim2) {}
  void operator()(std::ostream& o, const double& x);
private:
  int dim1_{ 0 }, dim2_{ 0 };
  int index_{ 0 };
};

class ActionExportMatrix : public ActionExportVector<double, MatrixDoubleWriter>
{
  CORE_ACTION(
    CORE_ACTION_TYPE("ExportMatrix", "This action exports a maxtrix of a given size.")
    CORE_ACTION_ARGUMENT("file_path", "Path to the file that the matrix will be written to.")
    CORE_ACTION_ARGUMENT("matrix", "matrix")
    CORE_ACTION_CHANGES_PROJECT_DATA()
    )


public:
  using Base = ActionExportVector<double, MatrixDoubleWriter>;
  ActionExportMatrix(const std::string& file_path, const std::vector<double>& matrix, int dim1, int dim2) :
    Base(file_path, matrix, MatrixDoubleWriter(dim1, dim2))
  {
    init_parameters();
  }
  ActionExportMatrix()
  {
    init_parameters();
  }

  // DISPATCH:
  static void Dispatch(Core::ActionContextHandle context,
    const std::string& file_path,
    const std::vector<double>& matrix, int dim1, int dim2)
  {
    auto action = new ActionExportMatrix(file_path, matrix, dim1, dim2);
    Core::ActionDispatcher::PostAction(Core::ActionHandle(action), context);
  }
};

} // end namespace Seg3D

#endif
