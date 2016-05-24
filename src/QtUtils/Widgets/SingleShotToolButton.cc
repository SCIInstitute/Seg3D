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

#include <cassert>

// Qt includes
#include <QtGui/QMouseEvent>

// QtUtils includes
#include <QtUtils/Widgets/SingleShotToolButton.h>

namespace QtUtils
{

SingleShotToolButton::SingleShotToolButton( QWidget *parent ) :
  QToolButton( parent )
{
  this->setCheckable( true );
}

SingleShotToolButton::~SingleShotToolButton()
{
}

void SingleShotToolButton::mouseReleaseEvent( QMouseEvent *e )
{
  if ( e->button() == Qt::LeftButton )
  {
    bool checked = this->isChecked();
    bool signals_blocked = this->blockSignals( true );
    QToolButton::mouseReleaseEvent( e );
    if ( e->isAccepted() )
    {
      // Make sure the button is checked
      this->setChecked( true );
      // Restore signal blocking state
      this->blockSignals( signals_blocked );      
      
      // If the button was previously not checked, emit the toggled signal
      if ( !checked )
      {
        assert( this->isChecked() );
        this->toggled( true );
      }
    }
    else
    {
      this->blockSignals( signals_blocked );
    }
  }
  else
  {
    QToolButton::mouseReleaseEvent( e );
  }
}

} // end namespace QtUtils
