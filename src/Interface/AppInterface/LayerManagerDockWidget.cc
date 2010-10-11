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

// Core includes
#include <Core/Interface/Interface.h>

// Interface includes
#include <Interface/AppInterface/LayerManagerDockWidget.h>
#include <Interface/AppInterface/LayerManagerWidget.h>

namespace Seg3D
{

LayerManagerDockWidget::LayerManagerDockWidget( QWidget *parent ) :
  QDockWidget( "LayerManagerDockWidget", parent )
{
  this->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  this->setWindowTitle( "Layer Manager" );

//  this->resize( 240, 640 );
//  QSizePolicy sizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
//  sizePolicy.setHorizontalStretch(0);
//  sizePolicy.setVerticalStretch(0);
//  setSizePolicy( sizePolicy );
  this->setMinimumSize( QSize( 260, 313 ) );
  
  this->setWidget( new LayerManagerWidget( this ) );
}

LayerManagerDockWidget::~LayerManagerDockWidget()
{
}

}  // end namespace Seg3D
