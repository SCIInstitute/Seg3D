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

#ifndef APPLICATION_TOOLS_MEASUREMENTTOOL_H
#define APPLICATION_TOOLS_MEASUREMENTTOOL_H

// Application includes
#include <Application/Tool/Tool.h>


// Core includes
#include <Core/Geometry/Measurement.h>
#include <Core/State/StateVector.h>

namespace Seg3D
{

class MeasurementTool;
typedef boost::shared_ptr< MeasurementTool > MeasurementToolHandle;

class MeasurementToolPrivate;
typedef boost::shared_ptr< MeasurementToolPrivate > MeasurementToolPrivateHandle;

class MeasurementTool : public Tool
{
friend class MeasurementTableModel;

SEG3D_TOOL
(
  SEG3D_TOOL_NAME( "MeasurementTool", "Tool for creating measurements in slices" )
  SEG3D_TOOL_MENULABEL( "Measure --TEST--" )
  SEG3D_TOOL_MENU( "Tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "Ctrl+ALT+0" )
  SEG3D_TOOL_URL( "http://www.sci.utah.edu/SCIRunDocs/index.php/CIBC:Seg3D2:MeasurementTool:1" )
)

  // -- constructor/destructor --
public:
  MeasurementTool( const std::string& toolid );
  virtual ~MeasurementTool();

  // EXECUTE:
  // Fire off the action that executes the filter
  virtual void execute( Core::ActionContextHandle context );

  std::vector< Core::Measurement > get_measurements() const;
  void set_measurement( size_t index, const Core::Measurement& measurement );

  int get_active_index() const;
  void set_active_index( int active_index );

  // -- state --
public:
  Core::StateMeasurementVectorHandle measurements_state_;
  Core::StateIntHandle active_index_state_;

public:
  static const int INVALID_ACTIVE_INDEX_C;

private:
  MeasurementToolPrivateHandle private_;
};

} // end namespace

#endif
