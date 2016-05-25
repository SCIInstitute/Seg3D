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

#ifndef QTUTILS_WIDGETS_QTTRISTATETOOLBUTTON_H
#define QTUTILS_WIDGETS_QTTRISTATETOOLBUTTON_H

#ifndef Q_MOC_RUN

// boost includes
#include <boost/shared_ptr.hpp>

// QT includes
#include <QToolButton>

#endif

namespace QtUtils
{
  
class QtTristateToolButtonPrivate;
typedef boost::shared_ptr< QtTristateToolButtonPrivate > QtTristateToolButtonPrivateHandle;

class QtTristateToolButton : public QToolButton
{
  Q_OBJECT
  
public:
  QtTristateToolButton( QWidget *parent = 0 );
  virtual ~QtTristateToolButton() {}

public:
  enum State
  {
    UNCHECKED_E = 0,
    INTERMEDIATE_E,
    CHECKED_E
  };

  /// SET_ICONS:
  /// Set the icons to be used for each state.
  void set_icons( const QIcon& unchecked_icon, const QIcon& intermediate_icon,
    const QIcon& checked_icon );

  /// SET_STATE_STRINGS:
  /// Set the string representation of each state.
  void set_state_strings( const std::string& unchecked_str, 
    const std::string& intermediate_str, const std::string& checked_str );

  /// SET_STYLESHEETS:
  /// Set the stylesheet for each state.
  void set_stylesheets( const QString& unchecked_style,
    const QString& intermediate_style, const QString& checked_style );

  /// GET_STATE:
  /// Return the current state.
  QtTristateToolButton::State get_state();
  
Q_SIGNALS:
  void state_changed( int );
  void state_changed( std::string );

public:
  void set_state( QtTristateToolButton::State state );
  void set_state( const std::string& state );

protected:
  virtual void nextCheckState();
  
private:
  QtTristateToolButtonPrivateHandle private_;
};
  
} // end namespace QtUtils

#endif
