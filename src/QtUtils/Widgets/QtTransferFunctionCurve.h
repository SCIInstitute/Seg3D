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

#ifndef QTUTILS_WIDGETS_QTTRANSFERFUNCTIONCURVE_H
#define QTUTILS_WIDGETS_QTTRANSFERFUNCTIONCURVE_H

#ifndef Q_MOC_RUN

#include <QObject>
#include <QGraphicsScene>

#include <Core/VolumeRenderer/TransferFunctionControlPoint.h>

#endif

namespace QtUtils
{

class QtTransferFunctionControlPoint;
class QtTransferFunctionCurvePrivate;

class QtTransferFunctionCurve : public QObject
{
  Q_OBJECT
public:
  explicit QtTransferFunctionCurve( const std::string& feature_id, QGraphicsScene* parent );
  ~QtTransferFunctionCurve();

  void add_control_point( const QPointF& pos );
  void set_control_points( const Core::TransferFunctionControlPointVector& points );
  void remove_control_point( QtTransferFunctionControlPoint* control_point );
  void set_active( bool active );
  void set_color( const QColor& color );
  void move_curve( const QPointF& offset );

  bool is_active() const;
  const std::string& get_feature_id() const;

Q_SIGNALS:
  void control_points_changed( const Core::TransferFunctionControlPointVector& points );
  void activated( bool active );

private Q_SLOTS:
  void handle_control_point_moved();

private:
  QtTransferFunctionCurvePrivate* private_;
};

} // end namespace QtUtils

#endif // QTTRANSFERFUNCTIONCURVE_H
