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

// Application includes
#include <Core/State/StateEngine.h>

// Interface includes
#include <Interface/Application/ControllerStateEngine.h>


namespace Seg3D
{

ControllerStateEngine::ControllerStateEngine(QObject* parent ) :
  QAbstractTableModel( parent )
{
}

ControllerStateEngine::~ControllerStateEngine()
{
}

int ControllerStateEngine::rowCount( const QModelIndex& ) const
{
  return ( static_cast< int > ( Core::StateEngine::Instance()->number_of_states() ) );
}

int ControllerStateEngine::columnCount( const QModelIndex& ) const
{
  return ( 2 );
}

QVariant ControllerStateEngine::data( const QModelIndex& index, int role ) const
{
  if ( !index.isValid() ) return QVariant();

  if ( role == Qt::TextAlignmentRole )
  {
    return int( Qt::AlignLeft | Qt::AlignVCenter );
  }
  else if ( role == Qt::DisplayRole )
  {
    Core::StateBaseHandle state;
    if ( Core::StateEngine::Instance()->get_state( index.row(), state ))
    {
      if ( index.column() == 0 )
      {
        return ( QString::fromStdString( state->get_stateid() ) );
      }
      else
      {
        return ( QString::fromStdString( state->export_to_string() ) );     
      }
    }
  }

  return QVariant();
}

QVariant ControllerStateEngine::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role != Qt::DisplayRole || orientation == Qt::Vertical )
  {
    return QVariant();
  }

  if ( section == 0 ) return QString( "State Entry" );
  else if ( section == 1 ) return QString( "State Value" );
  else return QVariant();
}

} // end namespace Seg3D
