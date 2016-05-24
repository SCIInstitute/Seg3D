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

#ifndef INTERFACE_APPLICATION_MESSAGEHISTORYWIDGET_H
#define INTERFACE_APPLICATION_MESSAGEHISTORYWIDGET_H

#ifndef Q_MOC_RUN

// QT includes
#include <QtCore/QPointer>

// STL includes
#include <string>

// Boost includes
#include <boost/shared_ptr.hpp>

// QtUtils includes
#include <QtUtils/Widgets/QtCustomDialog.h>

// Core includes
#include <Core/Utils/ConnectionHandler.h>

#endif

namespace Seg3D
{

class MessageWindowPrivate;

class MessageWindow : public QtUtils::QtCustomDialog, private Core::ConnectionHandler
{
Q_OBJECT
  
public:
  MessageWindow( QWidget *parent = 0 );
  virtual ~MessageWindow();

private:
  boost::shared_ptr< MessageWindowPrivate > private_;

private:
  /// ADDMESSAGE:
  /// static class for adding a message to the MessageWindow class
  static void AddMessage( QPointer< MessageWindow > qpointer, 
    int msg_type, std::string message );
  
  /// ADD_MESSAGE:
  /// member function that actually does the adding
  void add_message( int msg_type, std::string message );
};

} // end namespace Seg3D

#endif
