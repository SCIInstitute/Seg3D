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

#include <QtUtils/Bridge/detail/QtActionVectorConnector.h>

namespace QtUtils
{

QtActionVectorConnector::QtActionVectorConnector( QAction* parent, std::vector<Core::StateBoolHandle>& state,
    Core::StateIntHandle index, bool blocking ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state ),
  index_( index )
{
  QPointer< QtActionVectorConnector > qpointer( this );

  this->parent_->setCheckable( true );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    int index = this->index_->get();
    if ( index == -1 )
    {
      bool combined_state = true;
      for ( size_t j = 0; j < this->state_.size(); j++ )
      {
        combined_state = combined_state && this->state_[j]->get();
      }
      parent->setChecked( combined_state );
    }
    else if ( index >= 0 && index < static_cast<int>( this->state_.size() ) )
    {
      parent->setChecked( this->state_[index]->get() );
    }

    for ( size_t j = 0; j < this->state_.size(); j++)
    {
      this->add_connection( this->state_[ j ]->value_changed_signal_.connect(
        boost::bind( &QtActionVectorConnector::SetActionChecked, qpointer, 
        static_cast< int >( j ), _1, _2 ) ) );
    }
    
    this->add_connection( this->index_->value_changed_signal_.connect(
      boost::bind( &QtActionVectorConnector::UpdateIndex, qpointer, _1, _2 ) ) );
  }

  this->connect( this->parent_, SIGNAL( toggled( bool ) ), SLOT( set_state( bool ) ) );
  
}

QtActionVectorConnector::~QtActionVectorConnector()
{
  this->disconnect_all();
}

void QtActionVectorConnector::UpdateIndex( QPointer< QtActionVectorConnector > qpointer,
    int index, Core::ActionSource source )
{

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtActionVectorConnector::UpdateIndex,
      qpointer, index, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  if ( index == -1 )
  {
    bool combined_state = true;
    for ( size_t j = 0; j < qpointer->state_.size(); j++ )
    {
      combined_state = combined_state && qpointer->state_[ j ]->get();
    }
    qpointer->parent_->setChecked( combined_state );  
  }
  else if ( index >= 0 && index < static_cast<int>( qpointer->state_.size() ) )
  {
    qpointer->parent_->setChecked(  qpointer->state_[ index ]->get() );
  }
  
}

void QtActionVectorConnector::SetActionChecked( 
    QPointer< QtActionVectorConnector > qpointer, int index,
    bool checked, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtActionVectorConnector::SetActionChecked,
      qpointer, index, checked, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  if ( qpointer->index_->get() == -1 )
  {
    bool combined_state = true;
    for ( size_t j = 0; j < qpointer->state_.size(); j++ )
    {
      combined_state = combined_state && qpointer->state_[ j ]->get();
    }
    qpointer->parent_->setChecked( combined_state );
  }
  else
  {
    if ( index == qpointer->index_->get() ) qpointer->parent_->setChecked( checked );
  }
  
  qpointer->unblock();
}

void QtActionVectorConnector::set_state( bool value )
{
  if ( !this->is_blocked() )
  {
    int index = this->index_->get();
    if ( index == -1 )
    {
      for ( size_t j = 0; j< state_.size(); j++ )
      {
        Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
          this->state_[ j ], value );
      }
    }
    else if ( index >= 0 && index < static_cast<int>( this->state_.size() ) )
    {
      Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
        this->state_[ index ], value );
    }
  }
}

} // end namespace QtUtils
