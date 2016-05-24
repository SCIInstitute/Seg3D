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

#ifndef INTERFACE_APPLICATION_VIEWERWIDGET_H
#define INTERFACE_APPLICATION_VIEWERWIDGET_H

#ifndef Q_MOC_RUN

// QT includes
#include <QWidget>
#include <QPointer>

// STL includes
#include <string>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/Utils/ConnectionHandler.h>
#include <Core/Action/ActionContext.h>

// Application includes
#include <Application/Viewer/Viewer.h>

#endif

namespace Seg3D
{

// Forward declaration
class ViewerWidget;
typedef QPointer< ViewerWidget > ViewerWidgetQWeakHandle;

class ViewerWidgetPrivate;
typedef boost::shared_ptr< ViewerWidgetPrivate > ViewerWidgetPrivateHandle;

// Class definitions
class ViewerWidget : public QWidget, public Core::ConnectionHandler
{
  Q_OBJECT

  // -- constructor/destructor --
public:
  ViewerWidget( ViewerHandle viewer, QWidget *parent = 0 );
  virtual ~ViewerWidget();

public Q_SLOTS:

  /// SELECT:
  /// This slot is triggered when the widget is selected.
  void select();
  
  /// DESELECT:
  /// This slot is triggered if another viewer is activated and this one needs to be deselected.
  void deselect();
    
public:
  void image_mode( bool picture );

protected:

  /// RESIZEEVENT:
  /// Overloaded Qt function
  virtual void resizeEvent( QResizeEvent * event );
  
private:
  /// ADD_ICONS_TO_COMBOBOX:
  /// This function adds the proper Icons to the viewer states combobox
  void add_icons_to_combobox();
  
  /// GET_MINIMUM_SIZE;
  /// function returns the minimum size of that viewer widget needs before it needs to split
  int get_minimum_size();
  
private:
  friend class ViewerWidgetPrivate;
  /// Internals of the viewer widget, so most dependencies do not need to
  /// be included here.
  ViewerWidgetPrivateHandle private_;
};

} // end namespace Seg3D

#endif
