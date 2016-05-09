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

#include <QCoreApplication>

#include <Core/Interface/Interface.h>
#include <Core/State/Actions/ActionSet.h>

#include <QtUtils/Bridge/detail/QtTransferFunctionCurveConnector.h>

namespace QtUtils
{

QtTransferFunctionCurveConnector::QtTransferFunctionCurveConnector( 
  QtTransferFunctionCurve* parent, 
  Core::TransferFunctionFeatureHandle& tf_feature, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  tf_feature_( tf_feature )
{
  QPointer< QtTransferFunctionCurveConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    parent->set_control_points( tf_feature->control_points_state_->get() );
    QColor color( tf_feature->diffuse_color_red_state_->get(),
      tf_feature->diffuse_color_green_state_->get(),
      tf_feature->diffuse_color_blue_state_->get() );
    parent->set_color( color );

    this->add_connection( tf_feature->control_points_state_->value_changed_signal_.connect(
      boost::bind( &QtTransferFunctionCurveConnector::SetCurveControlPoints, qpointer, _1, _2 ) ) );
    this->add_connection( tf_feature->diffuse_color_red_state_->state_changed_signal_.connect(
      boost::bind( &QtTransferFunctionCurveConnector::UpdateCurveColor, qpointer ) ) );
    this->add_connection( tf_feature->diffuse_color_green_state_->state_changed_signal_.connect(
      boost::bind( &QtTransferFunctionCurveConnector::UpdateCurveColor, qpointer ) ) );
    this->add_connection( tf_feature->diffuse_color_blue_state_->state_changed_signal_.connect(
      boost::bind( &QtTransferFunctionCurveConnector::UpdateCurveColor, qpointer ) ) );
  }

  this->connect( parent, SIGNAL( control_points_changed( 
    const Core::TransferFunctionControlPointVector& ) ), 
    SLOT( set_control_points_state( const Core::TransferFunctionControlPointVector& ) ) );
}

QtTransferFunctionCurveConnector::~QtTransferFunctionCurveConnector()
{
  this->disconnect_all();
}

void QtTransferFunctionCurveConnector::SetCurveControlPoints( 
  QPointer< QtTransferFunctionCurveConnector > qpointer, 
  Core::TransferFunctionControlPointVector control_points, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( 
      &QtTransferFunctionCurveConnector::SetCurveControlPoints,
      qpointer, control_points, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  qpointer->parent_->set_control_points( control_points );
  qpointer->unblock();
}

void QtTransferFunctionCurveConnector::UpdateCurveColor( 
  QPointer< QtTransferFunctionCurveConnector > qpointer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( 
      &QtTransferFunctionCurveConnector::UpdateCurveColor,
      qpointer ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  QColor color( qpointer->tf_feature_->diffuse_color_red_state_->get(),
    qpointer->tf_feature_->diffuse_color_green_state_->get(),
    qpointer->tf_feature_->diffuse_color_blue_state_->get() );
  lock.unlock();
  qpointer->parent_->set_color( color );
}

void QtTransferFunctionCurveConnector::set_control_points_state( 
  const Core::TransferFunctionControlPointVector& control_points )
{
  if ( !this->is_blocked() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->tf_feature_->control_points_state_, control_points );
  }
}

} // end namespace QtUtils
