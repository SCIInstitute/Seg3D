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

// Qt includes
#include <QPainter>
#include <QLinearGradient>

// Interface includes
#include <QtInterface/Widgets/QtHistogramGraph.h>

namespace Core
{

QtHistogramGraph::QtHistogramGraph( QWidget *parent )
    : QWidget( parent )
{
    setBackgroundRole( QPalette::Base );
    setAutoFillBackground( true );
}

QtHistogramGraph::~QtHistogramGraph()
{
}

void QtHistogramGraph::set_histogram( const Histogram& histogram )
{
  this->histogram_ = histogram;
}

void QtHistogramGraph::paintEvent(QPaintEvent * event )
{
    QPainter painter( this );
    painter.setRenderHint( QPainter::Antialiasing, true );

    double histogram_width = this->width();
    double bins_size = this->histogram_.get_size();
    double percent_of_max = this->height() /
    static_cast< double >( this->histogram_.get_max_bin() );

  const std::vector<size_t>& bins = this->histogram_.get_bins();
    for( int i = 0; i < bins_size; ++i )
    {
    // Break it down so it's easier to understand
    int adjusted_bin_size = static_cast< double >( bins[ i ] ) * percent_of_max;
    int rect_left =  i * ( histogram_width / bins_size );
    int rect_top = this->height() - adjusted_bin_size;
    int rect_width = histogram_width / bins_size;
    int rect_height = adjusted_bin_size;
    
    QLinearGradient linearGradient( QPointF( rect_left, rect_top ), 
      QPointF( rect_left+rect_width, this->height() ) );
    linearGradient.setColorAt( 0.0, Qt::lightGray );
    linearGradient.setColorAt( 0.5, Qt::darkGray );
    linearGradient.setColorAt( 1.0, Qt::gray );
    painter.setBrush( linearGradient );
        painter.drawRect( rect_left, rect_top, rect_width, rect_height );
    }
}

} // end namespace Core
