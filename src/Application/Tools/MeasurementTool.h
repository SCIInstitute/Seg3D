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

  // Note: All of the following functions ensure thread-safe access to state variables.

  // GET_MEASUREMENTS:
  // Get vector of all measurements
  std::vector< Core::Measurement > get_measurements() const;

  // SET_MEASUREMENT:
  // Set measurement at existing index.  Useful for modifying note or visibility of existing
  // measurement.
  void set_measurement( size_t index, const Core::Measurement& measurement );

  // ADD_MEASUREMENT:
  // Adds new measurement to end of list
  void add_measurement( const Core::Measurement& measurement );

  // REMOVE_MEASUREMENT:
  // Remove measurement matching this one.  By not passing index, we avoid issues with indices
  // changing as measurements are removed.
  void remove_measurement( const Core::Measurement& measurement );

  // GET_ACTIVE_INDEX:
  // Get index of active measurement in table, or -1 if none.
  int get_active_index() const;

  // SET_ACTIVE_INDEX:
  // Set index indicating active measurement in table.
  void set_active_index( int active_index );

  // GET_SHOW_WORLD_UNITS:
  // Get boolean indicating whether world units (true) or index units (false) should be displayed.
  bool get_show_world_units() const;

  double get_opacity() const;

  // GO_TO_ACTIVE_MEASUREMENT:
  // Do picking on all pickable views so that they move to the indicated point (0 or 1) in the
  // active measurement.
  void go_to_active_measurement( int point_index );

  // HANDLE_MOUSE_PRESS:
  // Called when a mouse button has been pressed.
  virtual bool handle_mouse_press( ViewerHandle viewer, 
    const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  // HANDLE_MOUSE_MOVE:
  // Called when the mouse moves in a viewer.
  virtual bool handle_mouse_move( ViewerHandle viewer, 
    const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  // HANDLE_MOUSE_LEAVE:
  // Called when the mouse has left a viewer.
  //virtual bool handle_mouse_leave( ViewerHandle viewer );

  // HANDLE_WHEEL:
  // Called when the mouse wheel has been rotated.
  virtual bool handle_wheel( ViewerHandle viewer, int delta, 
    int x, int y, int buttons, int modifiers );

  // HANDLE_KEY_PRESS:
  // Called when a key is pressed.
  virtual bool handle_key_press( ViewerHandle viewer, int key, int modifiers );

  // REDRAW:
  // Draw seed points in the specified viewer.
  // The function should only be called by the renderer, which has a valid GL context.
  virtual void redraw( size_t viewer_id, const Core::Matrix& proj_mat );
  virtual bool has_2d_visual();

  // -- signals --
public:
  // UNITS_CHANGED_SIGNAL:
  typedef boost::signals2::signal< void () > units_changed_signal_type;
  units_changed_signal_type units_changed_signal_;

  // -- state --
public:
  // List of measurements
  Core::StateMeasurementVectorHandle measurements_state_;

  // Index of active measurement in table
  Core::StateIntHandle active_index_state_;

  // Selection between display of index and world units.  Needed for radio button group.
  Core::StateLabeledOptionHandle units_selection_state_; 

  // Boolean indicating whether world units (true) or index units (false) should be displayed.
  Core::StateBoolHandle show_world_units_state_; 
  
  // The opacity of all the measurements for this tool
  Core::StateRangedDoubleHandle opacity_state_;

public:
  static const std::string INDEX_UNITS_C;
  static const std::string WORLD_UNITS_C;

private:
  MeasurementToolPrivateHandle private_;
};

} // end namespace

#endif
