/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

// System includes
#include <algorithm>

// Qt includes
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenu>

// Interface includes
#include <Interface/ToolInterface/detail/CopyTableView.h>

namespace Seg3D
{

CopyTableView::CopyTableView( QWidget* parent ) : 
  QTableView( parent ),
  triggered_action_( 0 )
{
  this->copy_action_ = new QAction( tr( "&Copy" ), this ) ;
  //this->copy_action_->setIcon( QIcon( ":/trolltech/styles/commonstyle/images/up-128.png" ) ); 
}

void CopyTableView::copy() const
{
  QItemSelectionModel* selection = this->selectionModel(); 
  QModelIndexList indexes = selection->selectedIndexes();
  
  if( indexes.size() < 1 ) return; 

  // QModelIndex::operator < sorts first by row, then by column.  
  // this is what we need 
  std::sort( indexes.begin(), indexes.end() ); 

  // You need a pair of indexes to find the row changes 
  QModelIndex previous = indexes.first(); 
  indexes.removeFirst(); 
  QString selected_text; 
  QModelIndex current; 
  Q_FOREACH( current, indexes ) 
  { 
    // If this is the first column and this row is selected, append the row header to the string
    if ( previous.column() == 0 )
    {
      if( selection->isRowSelected( previous.row(), QModelIndex() ) )
      {
        QString header_text = 
          this->model()->headerData( previous.row(), Qt::Vertical ).toString();
        selected_text.append( header_text );
      }
    }

    QVariant data = this->model()->data( previous ); 
    QString text = data.toString(); 
    // At this point `text` contains the text in one cell 
    selected_text.append( text ); 
    // If you are at the start of the row the row number of the previous index 
    // isn't the same.  Text is followed by a row separator, which is a newline. 
    if ( current.row() != previous.row() ) 
    { 
      selected_text.append( QLatin1Char('\n') ); 
    } 
    // Otherwise it's the same row, so append a column separator, which is a tab. 
    else 
    { 
      selected_text.append( QLatin1Char('\t') ); 
    } 
    previous = current; 
  } 

  // add last element 
  selected_text.append( model()->data( previous ).toString() ); 
  selected_text.append( QLatin1Char('\n') ); 
  qApp->clipboard()->setText( selected_text );
}

void CopyTableView::keyPressEvent( QKeyEvent * event )
{
  if ( event->matches( QKeySequence::Copy ) ) 
  { 
    copy(); 
  } 
  else 
  { 
    QTableView::keyPressEvent( event ); 
  }
}

} // end namespace Seg3D