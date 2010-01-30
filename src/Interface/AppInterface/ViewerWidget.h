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


#ifndef INTERFACE_APPINTERFACE_VIEWERWIDGET_H
#define INTERFACE_APPINTERFACE_VIEWERWIDGET_H

#include <Application/Viewer/ViewerRenderer.h>

// QT includes
#include <QFrame>

// STL includes
#include <string>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace Seg3D {

// Forward declaration
class ViewerWidgetPrivate;
typedef boost::shared_ptr<ViewerWidgetPrivate> ViewerWidgetPrivateHandle;

// Class definitions
class ViewerWidget : public QFrame {
    
    Q_OBJECT
  public:
    ViewerWidget(int viewer_id, QWidget *parent = 0);
    virtual ~ViewerWidget();
    
    ViewerRendererHandle renderer();
  
  Q_SIGNALS:
    void selected(int);
    void changed_viewer_type(int);

  public Q_SLOTS:  
    void select();
    void deselect();

  private:
    // Internals of the viewer widget, so most dependencies do not need to
    // be included here
    ViewerWidgetPrivateHandle private_;
    
    int viewer_id_;
};

} // end namespace Seg3D

#endif
