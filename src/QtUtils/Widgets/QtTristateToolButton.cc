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

#include <QtUtils/Widgets/QtTristateToolButton.h>

namespace QtUtils
{
  
class QtTristateToolButtonPrivate
{
public:
  QIcon icons_[ 3 ];
  std::string state_strings_[ 3 ];
  QString stylesheets_[ 3 ];
  QtTristateToolButton::State state_;
};

QtTristateToolButton::QtTristateToolButton( QWidget *parent ) :
  QToolButton( parent ),
  private_( new QtTristateToolButtonPrivate )
{
  this->setCheckable( false );

  this->private_->state_strings_[ 0 ] = "unchecked";
  this->private_->state_strings_[ 1 ] = "intermediate";
  this->private_->state_strings_[ 2 ] = "checked";
  this->private_->stylesheets_[ 0 ] = QString::fromUtf8(
    " background-color: rgba(0,0,0,0); "
    );
  this->private_->stylesheets_[ 1 ] = QString::fromUtf8(
    " background-color: rgba(0,0,0,0); "
    );
  this->private_->stylesheets_[ 2 ] = QString::fromUtf8(
    " background-color: rgb(0,0,0,0); "
    );
  
  this->private_->state_ = UNCHECKED_E;
}

void QtTristateToolButton::set_icons( const QIcon& unchecked_icon, 
                   const QIcon& intermediate_icon, 
                   const QIcon& checked_icon )
{
  this->private_->icons_[ 0 ] = unchecked_icon;
  this->private_->icons_[ 1 ] = intermediate_icon;
  this->private_->icons_[ 2 ] = checked_icon;
}

void QtTristateToolButton::set_state_strings( const std::string& unchecked_str, 
                       const std::string& intermediate_str, 
                       const std::string& checked_str )
{
  this->private_->state_strings_[ 0 ] = unchecked_str;
  this->private_->state_strings_[ 1 ] = intermediate_str;
  this->private_->state_strings_[ 2 ] = checked_str;
}

void QtTristateToolButton::set_stylesheets( const QString& unchecked_style,
                       const QString& intermediate_style,
                       const QString& checked_style )
{
  this->private_->stylesheets_[ 0 ] = unchecked_style;
  this->private_->stylesheets_[ 1 ] = intermediate_style;
  this->private_->stylesheets_[ 2 ] = checked_style;
}

QtTristateToolButton::State QtTristateToolButton::get_state()
{
  return this->private_->state_;
}

void QtTristateToolButton::set_state( QtTristateToolButton::State state )
{
  this->private_->state_ = state;
  this->setIcon( this->private_->icons_[ state ] );
  this->setStyleSheet( this->private_->stylesheets_[ state ] );

  Q_EMIT this->state_changed( state );
  Q_EMIT this->state_changed( this->private_->state_strings_[ state ] );
}

void QtTristateToolButton::set_state( const std::string& state )
{
  for ( int i = 0; i < 3; ++i )
  {
    if ( this->private_->state_strings_[ i ] == state )
    {
      this->set_state( static_cast< State >( i ) );
      return;
    }
  }

  assert( false );
}

void QtTristateToolButton::nextCheckState()
{
  if ( this->private_->state_ == CHECKED_E )
  {
    this->set_state( UNCHECKED_E );
  }
  else
  {
    this->set_state( CHECKED_E );
  }
}

} // end namespace QtUtils
