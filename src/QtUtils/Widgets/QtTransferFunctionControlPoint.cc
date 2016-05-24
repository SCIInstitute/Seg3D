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

#include <QtUtils/Widgets/QtTransferFunctionControlPoint.h>
#include <QtUtils/Widgets/QtTransferFunctionCurve.h>

namespace QtUtils
{

class QtTransferFunctionControlPointPrivate
{
public:
  QtTransferFunctionCurve* curve_;
};

QtTransferFunctionControlPoint::QtTransferFunctionControlPoint( QtTransferFunctionCurve* curve ) :
  QObject( curve ),
  QGraphicsEllipseItem(0),
  private_( new QtTransferFunctionControlPointPrivate )
{
  this->private_->curve_ = curve;
  this->setFlag( ItemSendsGeometryChanges, true );
  this->setFlag( ItemIsMovable );
  this->setFlag( ItemIgnoresTransformations );
}

QtTransferFunctionControlPoint::~QtTransferFunctionControlPoint()
{
  delete this->private_;
}

QVariant QtTransferFunctionControlPoint::itemChange( GraphicsItemChange change, const QVariant& value )
{
  if ( change == ItemPositionChange && this->scene() )
  {
    QPointF new_pos = value.toPointF();
    QRectF rect = this->scene()->sceneRect();
    if ( !rect.contains( new_pos ) )
    {
      new_pos.setX( qMin( rect.right(), qMax( new_pos.x(), rect.left() ) ) );
      new_pos.setY( qMin( rect.bottom(), qMax( new_pos.y(), rect.top() ) ) );
      return new_pos;
    }
  }
  else if ( change == ItemPositionHasChanged )
  {
    QPointF new_pos = value.toPointF();
    Q_EMIT this->position_changed( new_pos );
    return value;
  }

  return QGraphicsItem::itemChange( change, value );
}

QtTransferFunctionCurve* QtTransferFunctionControlPoint::get_curve() const
{
  return this->private_->curve_;
}

} // end namespace QtUtils
