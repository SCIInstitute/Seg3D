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

#ifndef INTERFACE_TOOLINTERFACE_POINTSSELECTTOOLINTERFACE_H
#define INTERFACE_TOOLINTERFACE_POINTSSELECTTOOLINTERFACE_H

#ifndef Q_MOC_RUN

// Qt includes
#include <QtCore/QPointer>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

// Base class of the tool widget include
#include <Interface/Application/ToolWidget.h>

#endif

namespace Seg3D
{

class PointsSelectToolInterfacePrivate;
typedef boost::shared_ptr< PointsSelectToolInterfacePrivate > PointsSelectToolInterfacePrivateHandle;


class PointsSelectToolInterface : public ToolWidget
{
Q_OBJECT

public:
  /// Constructor/destructor
  PointsSelectToolInterface();
  virtual ~PointsSelectToolInterface();
  virtual bool build_widget( QFrame* frame );

private:
    PointsSelectToolInterfacePrivateHandle private_;

public:
  typedef QPointer< PointsSelectToolInterface > qpointer_type;

  /// TOGGLESAVEBUTTONENABLED
  /// Enable or disable save points button depending on whether points are available.
  /// Locks: StateEngine
  static void ToggleSaveButtonEnabled( qpointer_type points_select_interface );

  /// UPDATETABLE
  /// Update table containing list of points when seed points change.
  static void UpdateTable( qpointer_type points_select_interface );
};

} // end namespace Seg3D

#endif
