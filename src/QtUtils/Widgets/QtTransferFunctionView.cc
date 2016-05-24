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

#include <QResizeEvent>

#include <QtUtils/Widgets/QtTransferFunctionView.h>
#include <QtUtils/Widgets/QtTransferFunctionScene.h>

namespace QtUtils
{

class QtTransferFunctionViewPrivate
{
public:
  QtTransferFunctionScene* scene_;
};

QtTransferFunctionView::QtTransferFunctionView( QWidget* parent ) :
  QGraphicsView( parent ),
  private_( new QtTransferFunctionViewPrivate )
{
  this->private_->scene_ = new QtTransferFunctionScene( this );
  this->setScene( this->private_->scene_ );
}

QtTransferFunctionView::~QtTransferFunctionView()
{
  delete this->private_;
}

void QtTransferFunctionView::resizeEvent( QResizeEvent* event )
{
  QGraphicsView::resizeEvent( event );
  QRectF scene_rect = this->private_->scene_->sceneRect();
  this->setTransform( QTransform::fromScale( event->size().width() / scene_rect.width(),
    event->size().height() / scene_rect.height() ) );
  this->private_->scene_->set_view_transform( this->transform() );
}

QtTransferFunctionScene* QtTransferFunctionView::get_scene() const
{
  return this->private_->scene_;
}

} // end namespace QtUtils
