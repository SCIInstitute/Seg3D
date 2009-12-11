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

#ifndef INTERFACE_APPINTERFACE_APPMENU_H
#define INTERFACE_APPINTERFACE_APPMENU_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#include <QMenuBar>
#include <QAction>
#include <QMainWindow>
#include <Interface/AppInterface/ViewerInterface.h>
#include <Interface/AppInterface/ViewAction.h>


namespace Seg3D {

class AppMenu : public QObject
{
  Q_OBJECT
// -- constructor / destructor --    
  public:
    AppMenu(QMainWindow* parent = 0);
    AppMenu(QMainWindow* parent, ViewerInterface* view_ptr); 
    virtual ~AppMenu();

// -- functions for building menus --
  public:

    void create_file_menu(QMenu* menu);
    void create_edit_menu(QMenu* menu);
    void create_view_menu(QMenu* menu);

    void create_tool_menu(QMenu* menu);
    void create_filter_menu(QMenu* menu);

    void create_window_menu(QMenu* menu);
  
    ViewerInterface *viewer_pointer_;

// -- Menu pointers --
  private:

    QMenu* file_menu_;
    QMenu* edit_menu_;
    QMenu* view_menu_;
    QMenu* tool_menu_;
    QMenu* filter_menu_;
    QMenu* window_menu_;
    
       


};

} // end namespace Seg3D

#endif

