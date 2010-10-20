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

#ifndef INTERFACE_APPINTERFACE_KEYBOARDSHORTCUTS_H
#define INTERFACE_APPINTERFACE_KEYBOARDSHORTCUTS_H

// QT includes
#include <QtGui>

// STL includes
#include <string>

// Boost includes
#include <boost/shared_ptr.hpp>

// Application includes
#include <Application/Tool/ToolFWD.h>

// Core includes
#include <Core/Utils/ConnectionHandler.h>

namespace Seg3D
{

class AppShortcutsPrivate;

class AppShortcuts : public QDialog, private Core::ConnectionHandler
{
Q_OBJECT
  
public:
  AppShortcuts( QWidget *parent = 0 );
  virtual ~AppShortcuts();

private:
  boost::shared_ptr< AppShortcutsPrivate > private_;

private:
  void add_tool_shortcuts();
  
  static void ShowActiveToolControls( QPointer< AppShortcuts > qpointer, ToolHandle tool );
  void show_active_tool_contols( ToolHandle tool );
  

};

} // end namespace

#endif // MESSAGEHISTORYWIDGET_H
