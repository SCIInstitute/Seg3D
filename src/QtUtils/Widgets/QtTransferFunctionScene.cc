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

#include <boost/foreach.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QPointer>

#include <Core/Interface/Interface.h>

#include <QtUtils/Widgets/QtTransferFunctionScene.h>
#include <QtUtils/Widgets/QtTransferFunctionCurve.h>
#include <QtUtils/Widgets/QtTransferFunctionControlPoint.h>
#include <QtUtils/Widgets/QtTransferFunctionEdge.h>
#include <QtUtils/Utils/QtPointer.h>

namespace QtUtils
{

//////////////////////////////////////////////////////////////////////////
// Class QtTransferFunctionScenePrivate
//////////////////////////////////////////////////////////////////////////

typedef std::map< std::string, QtTransferFunctionCurve* > curve_map_type;

class QtTransferFunctionScenePrivate
{
public:
  void set_active_curve( QtTransferFunctionCurve* curve );

  curve_map_type curve_map_;
  QtTransferFunctionCurve* active_curve_;
  QTransform view_transform_;
  bool dragging_;
  QPointF prev_pos_;

public:
  static void HandleReset( QPointer< QtTransferFunctionScene > qpointer );
};

void QtTransferFunctionScenePrivate::set_active_curve( QtTransferFunctionCurve* curve )
{
  if ( this->active_curve_ == curve )
  {
    return;
  }

  if ( this->active_curve_ != 0 )
  {
    this->active_curve_->set_active( false );
  }

  this->active_curve_ = curve;
  if ( this->active_curve_ != 0 )
  {
    this->active_curve_->set_active( true );
  }
}

void QtTransferFunctionScenePrivate::HandleReset( QPointer< QtTransferFunctionScene > qpointer )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, 
    boost::bind( &QtTransferFunctionScene::reset, qpointer.data() ) ) );
}

//////////////////////////////////////////////////////////////////////////
// Class QtTransferFunctionScene
//////////////////////////////////////////////////////////////////////////

QtTransferFunctionScene::QtTransferFunctionScene(QObject *parent) :
  QGraphicsScene( 0, 0, 256, 256, parent ),
  private_( new QtTransferFunctionScenePrivate )
{
  this->private_->active_curve_ = 0;
  this->private_->dragging_ = false;

  this->add_connection( Core::Application::Instance()->reset_signal_.connect( boost::bind( 
    &QtTransferFunctionScenePrivate::HandleReset, 
    QPointer< QtTransferFunctionScene >( this ) ) ) );
}

QtTransferFunctionScene::~QtTransferFunctionScene()
{
  this->disconnect_all();
  delete this->private_;
}

void QtTransferFunctionScene::mousePressEvent( QGraphicsSceneMouseEvent* mouseEvent )
{
  QGraphicsItem* item = this->itemAt( mouseEvent->scenePos(), this->private_->view_transform_ );
  if ( item == 0 )
  {
    if ( this->private_->active_curve_ != 0 && mouseEvent->button() == Qt::LeftButton )
    {
      this->private_->active_curve_->add_control_point( mouseEvent->scenePos() );
      mouseEvent->accept();
      return;
    }
  }
  else
  {
    QtTransferFunctionControlPoint* control_point =
      dynamic_cast< QtTransferFunctionControlPoint* >( item );
    
    if ( control_point != 0 )
    {
      this->private_->set_active_curve( control_point->get_curve() );

      if ( mouseEvent->button() == Qt::RightButton )
      {
        control_point->get_curve()->remove_control_point( control_point );
      }
      }
    else
    {
      QtTransferFunctionEdge* edge = dynamic_cast< QtTransferFunctionEdge* >( item );
      if ( edge != 0 )
      {
        this->private_->set_active_curve( edge->get_curve() );
      }
    }

    if ( this->private_->active_curve_ != 0 && mouseEvent->modifiers() == Qt::ShiftModifier )
    {
      this->private_->dragging_ = true;
      this->private_->prev_pos_ = mouseEvent->scenePos();
      mouseEvent->accept();
      return;
  }
  }

  QGraphicsScene::mousePressEvent( mouseEvent );
}

void QtTransferFunctionScene::mouseMoveEvent( QGraphicsSceneMouseEvent* mouseEvent )
{
  if ( this->private_->dragging_ && this->private_->active_curve_ != 0 )
  {
    this->private_->active_curve_->move_curve( mouseEvent->scenePos() - this->private_->prev_pos_ );
    this->private_->prev_pos_ = mouseEvent->scenePos();
    mouseEvent->accept();
    return;
  }

  QGraphicsScene::mouseMoveEvent( mouseEvent );
}

void QtTransferFunctionScene::mouseReleaseEvent( QGraphicsSceneMouseEvent* mouseEvent )
{
  if ( this->private_->dragging_ && mouseEvent->button() == Qt::LeftButton )
  {
    this->private_->dragging_ = false;
    mouseEvent->accept();
    return;
  }

  QGraphicsScene::mouseReleaseEvent( mouseEvent );
}

void QtTransferFunctionScene::set_view_transform( const QTransform& matrix )
{
  this->private_->view_transform_ = matrix;
}

void QtTransferFunctionScene::add_curve( QtTransferFunctionCurve* curve )
{
  this->private_->curve_map_[ curve->get_feature_id() ] = curve;
  this->private_->set_active_curve( curve );
}

void QtTransferFunctionScene::delete_curve( const std::string& feature_id )
{
  curve_map_type::iterator it = this->private_->curve_map_.find( feature_id );
  if ( it != this->private_->curve_map_.end() )
  {
    QtTransferFunctionCurve* curve = ( *it ).second;
    this->private_->curve_map_.erase( it );
    if ( this->private_->active_curve_ == curve )
    {
      if ( !this->private_->curve_map_.empty() )
      {
        this->private_->set_active_curve( ( *this->private_->curve_map_.begin() ).second );
      }
      else
      {
        this->private_->set_active_curve( 0 );
      }
    }
    curve->deleteLater();
  }
  else
  {
    assert( false );
  }
}

QtTransferFunctionCurve* QtTransferFunctionScene::get_curve( const std::string& feature_id )
{
  curve_map_type::iterator it = this->private_->curve_map_.find( feature_id );
  if ( it != this->private_->curve_map_.end() )
  {
    return ( *it ).second;
  }

  return 0;
}

QtTransferFunctionCurve* QtTransferFunctionScene::get_active_curve()
{
  return this->private_->active_curve_;
}

void QtTransferFunctionScene::reset()
{
  BOOST_FOREACH( curve_map_type::value_type curve_entry, this->private_->curve_map_ )
  {
    delete curve_entry.second;
  }
  this->private_->active_curve_ = 0;
  this->private_->curve_map_.clear();
}

} // end namespace QtUtils
