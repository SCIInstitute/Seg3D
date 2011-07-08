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

#ifndef INTERFACE_APPLICATION_PROVENANCEDOCKWIDGET_H
#define INTERFACE_APPLICATION_PROVENANCEDOCKWIDGET_H

// QT includes
#include <QtCore/QPointer>

// STL includes
#include <string>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/Utils/ConnectionHandler.h>

// Application includes
#include <Application/Project/Project.h>
#include <Application/Provenance/Provenance.h>

// QtUtils includes
#include <QtUtils/Widgets/QtCustomDockWidget.h>

namespace Seg3D
{
  
class ProvenanceDockWidgetPrivate;

class ProvenanceDockWidget : public QtUtils::QtCustomDockWidget, public Core::ConnectionHandler
{
  Q_OBJECT
  
public:
  ProvenanceDockWidget( QWidget *parent = 0 );
  virtual ~ProvenanceDockWidget();
  
private:
  typedef QPointer< ProvenanceDockWidget > qpointer_type;

  // HANDLEPROVENANCERESULT:
  // A function that handles the signal that contains the provenance of a particular layer.
  static void HandleProvenanceResult( qpointer_type qpointer, ProvenanceTrailHandle provenance_trail );
  
  // HANDLEPROJECTCHANGED:
  // A function that handles reconnecting the provenance dock widget to the current project when it has changed
  static void HandleProjectChanged( qpointer_type qpointer );
  
  // POPULATE_PROVENANCE_LIST:
  // this handles the actual updating of the ui
  void populate_provenance_list( ProvenanceTrailHandle provenance_trail );
  
  // CONNECT_PROJECT:
  // this function does the actual reconnecting
  void connect_project();

private Q_SLOTS:
  // ENABLE_DISABLE_REPLAY_BUTTON:
  // Enable or disable the 'Recreate' button based on the current selected row.
  void enable_disable_replay_button();

  // DISPATCH_RECREATE_PROVENANCE:
  // Dispatch an ActionRecreateLayer if the current selected row has a valid provenance ID of interest.
  void dispatch_recreate_provenance();

private:
  boost::shared_ptr< ProvenanceDockWidgetPrivate > private_;
};
  
} // end namespace Seg3D

#endif 
