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

#ifndef QTUTILS_BRIDGE_DETAIL_QTCONNECTORBASE_H
#define QTUTILS_BRIDGE_DETAIL_QTCONNECTORBASE_H

#ifndef Q_MOC_RUN

#include <QObject>

#include <Core/Utils/ConnectionHandler.h>

#endif

namespace QtUtils
{

class QtConnectorBase : public QObject, protected Core::ConnectionHandler
{
  Q_OBJECT
public:
  QtConnectorBase( QObject* parent = 0, bool blocking = true ) :
    QObject( parent ),
    blocking_( blocking ),
    blocked_( false ),
    disabled_( false )
  {
  }

  virtual ~QtConnectorBase()
  {
    this->disconnect_all();
  }
  
  void disable()
  {
    // Remove all Qt connections
    this->disconnect();
    // Stop forwarding information to the parent widget
    this->disabled_ = true;
  }

protected:
  // BLOCK:
  // Block forwarding Qt signals to the application
  void block()
  {
    if ( this->blocking_ ) 
    {
      this->blocked_ = true;
    }
  }

  // UNBLOCK:
  // Unblock forwarding Qt signals to the application
  void unblock()
  {
    if ( this->blocking_ ) 
    {
      this->blocked_ = false;
    }
  }

  bool is_blocked() const
  {
    return this->blocked_ || this->disabled_;
  }



private:
  // Whether the connector has a blocking mode
  bool blocking_;
  // Whether the connection is blocked
  bool blocked_;
  // Whether the connection is disabled
  bool disabled_;
};

} // end namespace QtUtils

#endif
