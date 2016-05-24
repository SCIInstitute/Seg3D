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

#include <QtUtils/Bridge/detail/QtAbstractButtonVectorConnector.h>

namespace QtUtils
{

QtAbstractButtonVectorConnector::QtAbstractButtonVectorConnector( 
  QAbstractButton* parent, std::vector<Core::StateBoolHandle>& state, 
  Core::StateIntSetHandle indices, bool blocking /*= true */ ) :
  QtConnectorBase( parent, blocking ),
  parent_( parent ),
  state_( state ),
  indices_( indices )
{
  QPointer< QtAbstractButtonVectorConnector > qpointer( this );

  this->parent_->setCheckable( true );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    const std::set< int >& index = this->indices_->get();
    bool combined_state = true;
    for ( std::set< int >::const_iterator it = index.begin();
      it != index.end(); ++it )
    {
      assert( ( *it ) >= 0 && ( *it ) < static_cast< int >( state.size() ) );
      combined_state = combined_state && this->state_[ *it ]->get();
    }
    
    parent->setChecked( combined_state );

    for ( size_t j = 0; j < this->state_.size(); j++)
    {
      this->add_connection( this->state_[ j ]->value_changed_signal_.connect(
        boost::bind( &QtAbstractButtonVectorConnector::SetActionChecked, qpointer, 
        static_cast< int >( j ), _1, _2 ) ) );
    }

    this->add_connection( this->indices_->value_changed_signal_.connect(
      boost::bind( &QtAbstractButtonVectorConnector::UpdateIndex, qpointer, _1, _2 ) ) );
  }

  this->connect( this->parent_, SIGNAL( toggled( bool ) ), SLOT( set_state( bool ) ) );
}


QtAbstractButtonVectorConnector::~QtAbstractButtonVectorConnector()
{
  this->disconnect_all();
}

void QtAbstractButtonVectorConnector::UpdateIndex( 
  QPointer< QtAbstractButtonVectorConnector > qpointer, 
  std::set< int > indices, Core::ActionSource source )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtAbstractButtonVectorConnector::UpdateIndex,
      qpointer, indices, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();

  bool combined_state = true;
  for ( std::set< int >::const_iterator it = indices.begin();
    it != indices.end(); ++it )
  {
    if ( ( *it ) >= 0 && ( *it ) < static_cast< int >( qpointer->state_.size() ) )
    {
      combined_state = combined_state && qpointer->state_[ *it ]->get();
    }
  }

  qpointer->parent_->setChecked( combined_state );

  qpointer->unblock();
}

void QtAbstractButtonVectorConnector::SetActionChecked( 
    QPointer< QtAbstractButtonVectorConnector > qpointer, int index,
    bool checked, Core::ActionSource source )
{
  if ( source == Core::ActionSource::INTERFACE_WIDGET_E )
  {
    return;
  }

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &QtAbstractButtonVectorConnector::SetActionChecked,
      qpointer, index, checked, source ) );
    return;
  }

  if ( qpointer.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  qpointer->block();
  std::set< int > indices;
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    indices = qpointer->indices_->get();
  }

  if ( indices.find( index ) != indices.end() )
  {
    bool combined_state = true;
    for ( std::set< int >::const_iterator it = indices.begin();
      it != indices.end(); ++it )
    {
      if ( ( *it ) >= 0 && ( *it ) < static_cast< int >( qpointer->state_.size() ) )
      {
        combined_state = combined_state && qpointer->state_[ *it ]->get();
      }
    }
    qpointer->parent_->setChecked( combined_state );
  }
  
  qpointer->unblock();
}

void QtAbstractButtonVectorConnector::set_state( bool value )
{
  if ( !this->is_blocked() )
  {
    std::set< int > indices;
    {
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
      indices = this->indices_->get();
    }

    for ( std::set< int >::const_iterator it = indices.begin();
      it != indices.end(); ++it )
    {
      if ( ( *it ) >= 0 && ( *it ) < static_cast< int >( this->state_.size() ) )
      {
        Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
          this->state_[ *it ], value );
      }
    }
  }
}

} // end namespace QtUtils
