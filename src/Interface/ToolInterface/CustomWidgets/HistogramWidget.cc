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

#include <QtGui>
#include <time.h>
#include "ui_HistogramWidget.h"

// Interface includes
#include <Interface/ToolInterface/CustomWidgets/HistogramWidget.h>
#include <Interface/ToolInterface/CustomWidgets/Histogram.h>

namespace Seg3D
{

class HistogramWidgetPrivate 
{
public:
  Ui::HistogramWidget ui_;
  Histogram* histogram_;
};


HistogramWidget::HistogramWidget( QWidget *parent ) :
  QWidget( parent ),
    private_( new HistogramWidgetPrivate )
{
  this->private_->ui_.setupUi( this );
  
  this->private_->histogram_ = new Histogram( this );
  this->private_->ui_.histogramLayout->addWidget( this->private_->histogram_ );
  this->setMinimumHeight( 170 );
  this->private_->ui_.label_3->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
}

HistogramWidget::~HistogramWidget()
{
}

void HistogramWidget::set_histogram( std::vector< size_t > ints_bin, int min, int max,
  size_t min_bin, size_t max_bin )
{
  this->private_->histogram_->set_bins( ints_bin, min_bin, max_bin );
  this->private_->ui_.min->setText( QString::number( min ) );
  this->private_->ui_.max->setText( QString::number( max ) );
  this->private_->histogram_->repaint();
}

} // end namespace Seg3D