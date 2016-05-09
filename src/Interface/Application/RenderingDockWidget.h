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

#ifndef INTERFACE_APLICATION_RENDERINGDOCKWIDGET_H
#define INTERFACE_APLICATION_RENDERINGDOCKWIDGET_H

#ifndef Q_MOC_RUN

// Boost includes
#include <boost/shared_ptr.hpp>

// QT includes
#include <QtCore/QPointer>

// Core includes
#include <Core/Utils/ConnectionHandler.h>
#include <Core/VolumeRenderer/TransferFunctionFeature.h>

// QtUtils includes
#include <QtUtils/Widgets/QtCustomDockWidget.h>

#endif

namespace Seg3D
{

class RenderingDockWidgetPrivate;

class RenderingDockWidget : public QtUtils::QtCustomDockWidget, private Core::ConnectionHandler
{

Q_OBJECT

public:
  RenderingDockWidget( QWidget *parent = 0 );
  ~RenderingDockWidget();
  
private:
  void update_tab_appearance( bool enabled, int index  );
  void update_tool_appearance();
  void handle_feature_added( Core::TransferFunctionFeatureHandle feature );
  void handle_feature_deleted( Core::TransferFunctionFeatureHandle feature );
  void handle_volume_rendering_target_changed( std::string target_id );
  void handle_reset();

public Q_SLOTS:
  void delete_active_curve();
  void set_histogram_mode( int mode );

private:
  boost::shared_ptr< RenderingDockWidgetPrivate > private_;
  
private:
  typedef QPointer< RenderingDockWidget > qpointer_type;

  static void HandleClippingPlanesStateChanged( qpointer_type qpointer, bool state, int index );
  static void HandleUpdateToolAppearance( qpointer_type qpointer );
  static void HandleFeatureAdded( qpointer_type qpointer, Core::TransferFunctionFeatureHandle feature );
  static void HandleFeatureDeleted( qpointer_type qpointer, Core::TransferFunctionFeatureHandle feature );
  static void HandleVolumeRenderingTargetChanged( qpointer_type qpointer, std::string target_id );
  static void HandleReset( qpointer_type qpointer );
};

} // end namespace Seg3D

#endif
