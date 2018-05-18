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
#include <Core/State/StateEngine.h>

#include <QtUtils/Bridge/detail/QtTextMatrixConnector.h>

namespace QtUtils
{
	QtTextMatrixConnector::QtTextMatrixConnector(QPlainTextEdit* parent, 
		Core::StateDoubleVectorHandle& state, int dim1, int dim2) :
  QtConnectorBase( parent, true ),
  parent_( parent ),
  double_vector_state_( state ),
  dim1_(dim1),
  dim2_(dim2)
{
  QPointer< QtTextMatrixConnector > qpointer( this );

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  UpdateMatrixEntries( qpointer );
  this->add_connection( state->state_changed_signal_.connect( boost::bind(
    &QtTextMatrixConnector::UpdateMatrixEntries, qpointer ) ) );
}

QtTextMatrixConnector::~QtTextMatrixConnector()
{
  this->disconnect_all();
}

void QtTextMatrixConnector::UpdateMatrixEntries( 
  QPointer< QtTextMatrixConnector > qpointer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( 
      &QtTextMatrixConnector::UpdateMatrixEntries, qpointer ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  auto textwidget = qpointer->parent_;
  textwidget->clear();

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  const std::vector< double >& dbls = qpointer->double_vector_state_->get();
  lock.unlock();
  if (dbls.size() == qpointer->dim1_*qpointer->dim2_){
	  int inx = 0;
	  for (int i = 0; i < qpointer->dim1_; i++){
		  QString str;
		  for (int j = 0; j < qpointer->dim2_; j++){
			  if (j == 3){
				  str += QString::number(dbls[inx], 'f', 5);
			  }
			  else
			  {
				  str += QString::number(dbls[inx], 'f', 4) + "\t";
			  }
			  inx++;
		  }
		  textwidget->appendPlainText(str);
	  }
  }
}

} // end namespace QtUtils
