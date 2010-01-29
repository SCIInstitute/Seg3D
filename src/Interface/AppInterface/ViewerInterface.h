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

#ifndef VIEWERINTERFACE_H
#define VIEWERINTERFACE_H

// QT includes
#include <QtGui>

// STL includes
#include <string>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace Seg3D {

class ViewerInterfacePrivate;
typedef boost::shared_ptr<ViewerInterfacePrivate> ViewerInterfacePrivateHandle;

class ViewerInterface : public QWidget {
    
    Q_OBJECT
// -- Private class containing all the widgets --
  private:
    ViewerInterfacePrivateHandle private_;

// -- Constructor/Destructor --
  public:
    ViewerInterface(QWidget *parent = 0);
    virtual ~ViewerInterface();
    
  //  void writeSizeSettings();
  //  void readSizeSettings();

// -- Setting widget state --    
    void set_layout(const std::string& layout);
    void set_active_viewer(int);

// -- Slots --    
  public:
    typedef QPointer<ViewerInterface> qpointer_type;
    
    // SetViewerLayout: (Thread safe slot)
    static void SetViewerLayout(qpointer_type qpointer, std::string layout);

    // SetActiveViewer: (Thread safe slot)
    static void SetActiveViewer(qpointer_type qpointer, int active_viewer);
};

} // end namespaceSeg3D

#endif
