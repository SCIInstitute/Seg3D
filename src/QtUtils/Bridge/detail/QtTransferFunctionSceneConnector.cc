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

#include <QtUtils/Widgets/QtTransferFunctionCurve.h>
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Bridge/detail/QtTransferFunctionSceneConnector.h>

namespace QtUtils
{

QtTransferFunctionSceneConnector::QtTransferFunctionSceneConnector( 
  QtTransferFunctionScene* parent, Core::TransferFunctionHandle& tf ) :
  QtConnectorBase( parent ),
  parent_( parent ),
  transfer_function_( tf )
{
  QPointer< QtTransferFunctionSceneConnector > qpointer( this );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    
    this->add_connection( tf->feature_added_signal_.connect( boost::bind(
      &QtTransferFunctionSceneConnector::AddCurve, qpointer, _1 ) ) );
    this->add_connection( tf->feature_deleted_signal_.connect( boost::bind(
      &QtTransferFunctionSceneConnector::DeleteCurve, qpointer, _1 ) ) );
  }
}

QtTransferFunctionSceneConnector::~QtTransferFunctionSceneConnector()
{
  this->disconnect_all();
}

void QtTransferFunctionSceneConnector::AddCurve( 
  QPointer< QtTransferFunctionSceneConnector > qpointer, 
  Core::TransferFunctionFeatureHandle feature )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtTransferFunctionSceneConnector::AddCurve,
      qpointer, feature ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  QtTransferFunctionScene* scene = qpointer->parent_;
  QtTransferFunctionCurve* curve = new QtTransferFunctionCurve( feature->get_feature_id(), scene );
  scene->add_curve( curve );
  QtUtils::QtBridge::Connect( curve, feature );
}

void QtTransferFunctionSceneConnector::DeleteCurve( 
  QPointer< QtTransferFunctionSceneConnector > qpointer, 
  Core::TransferFunctionFeatureHandle feature )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtTransferFunctionSceneConnector::DeleteCurve,
      qpointer, feature ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->parent_->delete_curve( feature->get_feature_id() );
}

} // end namespace QtUtils
