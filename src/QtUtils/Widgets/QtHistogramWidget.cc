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

// UI includes
#include "ui_QtHistogramWidget.h"

// Core includes
#include <Core/Math/MathFunctions.h>

// QtUtils includes
#include <QtUtils/Widgets/QtHistogramWidget.h>
#include <QtUtils/Widgets/QtHistogramGraph.h>

namespace QtUtils
{

class QtHistogramWidgetPrivate 
{
public:
  Ui::HistogramWidget ui_;
  QtHistogramGraph* histogram_graph_;
  int bins_;
};


QtHistogramWidget::QtHistogramWidget( QWidget *parent, QtSliderDoubleCombo* upper_threshold, 
  QtSliderDoubleCombo* lower_threshold ) :
  QWidget( parent ),
    private_( new QtHistogramWidgetPrivate )
{
  this->private_->ui_.setupUi( this );
  
  this->lower_threshold_ = lower_threshold;
  this->upper_threshold_ = upper_threshold;
  
  this->private_->histogram_graph_ = new QtHistogramGraph( this );
  this->private_->ui_.histogramLayout->addWidget( this->private_->histogram_graph_ );
  
  connect( this->private_->histogram_graph_, SIGNAL( lower_position( int ) ), this, 
    SLOT( handle_right_button_click( int ) ) );
    
  connect( this->private_->histogram_graph_, SIGNAL( upper_position( int ) ), this, 
    SLOT( handle_left_button_click( int ) ) );
    
  if( this->upper_threshold_ != 0 )
  {
    connect( this->upper_threshold_, SIGNAL( valueAdjusted( double ) ), 
      this, SLOT( set_max( double ) ) );
  } 
    
  if( this->lower_threshold_ != 0 )
  {
    connect( this->lower_threshold_, SIGNAL( valueAdjusted( double ) ), 
      this, SLOT( set_min( double ) ) );
  }
    
  this->min_bar_ = new QWidget( this );
  this->max_bar_ = new QWidget( this );
  this->min_bar_->hide();
  this->max_bar_->hide();

  this->setMinimumHeight( 170 );
  this->private_->ui_.label_3->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
}

QtHistogramWidget::~QtHistogramWidget()
{
}

void QtHistogramWidget::set_histogram( const Core::Histogram& histogram )
{
  this->private_->histogram_graph_->set_histogram( histogram );
  this->private_->bins_ =  static_cast< int >( histogram.get_size() );
  this->private_->ui_.min->setText( QString::number( histogram.get_min() ) );
  this->private_->ui_.max->setText( QString::number( histogram.get_max() ) );

  if( this->min_bar_ ) this->min_bar_->deleteLater();
  if( this->max_bar_ ) this->max_bar_->deleteLater();
  
  this->min_bar_ = new QWidget( this );
  this->max_bar_ = new QWidget( this );
  this->min_bar_->hide();
  this->max_bar_->hide();

  this->private_->histogram_graph_->repaint();
}

void QtHistogramWidget::reset_histogram( )
{
  this->private_->histogram_graph_->reset_histogram();
  this->private_->ui_.min->setText( "-" );
  this->private_->ui_.max->setText( "-" );
  this->private_->histogram_graph_->repaint();
}

void QtHistogramWidget::set_min( double min )
{
  if( this->min_bar_->isHidden() ) this->min_bar_->show();
  if( this->max_bar_->isHidden() ) this->max_bar_->show();
  if( this->get_histogram_min() < 0 )
  {
    min = min + Core::Abs( this->get_histogram_min() );
  }
  
  double percentage = min / ( this->get_histogram_max() - this->get_histogram_min() );
  
  double min_location = ( ( this->private_->histogram_graph_->width() - 5 ) * percentage ) + 4;
  this->min_bar_->setGeometry( min_location, 4 , 4, this->private_->histogram_graph_->height() );
  this->min_bar_->setStyleSheet( QString::fromUtf8( "border-left: 2px solid rgb( 237, 148, 31 );"
    "border-bottom: 2px solid rgb( 237, 148, 31 );"
    "border-top: 2px solid rgb( 237, 148, 31 );" ) );
}

void QtHistogramWidget::set_max( double max )
{
  if( this->max_bar_->isHidden() ) this->max_bar_->show();
  if( this->min_bar_->isHidden() ) this->min_bar_->show();
  if( this->get_histogram_min() < 0 )
  {
    max = max + Core::Abs( this->get_histogram_min() );
  }
  
  double percentage = max / ( this->get_histogram_max() - this->get_histogram_min() );
  double max_location = ( ( this->private_->histogram_graph_->width() - 5 ) * percentage ) + 4;
  this->max_bar_->setGeometry( max_location, 4, 4, this->private_->histogram_graph_->height() );
  this->max_bar_->setStyleSheet( QString::fromUtf8( "border-right: 2px solid rgb( 237, 148, 31 );"
    "border-bottom: 2px solid rgb( 237, 148, 31 );"
    "border-top: 2px solid rgb( 237, 148, 31 );" ) );
}

void QtHistogramWidget::handle_left_button_click( int lower_location )
{
  if( this->lower_threshold_ == 0 ) return;
  if( this->max_bar_->isHidden() ) this->max_bar_->show();
  if( this->min_bar_->isHidden() ) this->min_bar_->show();
  double percent_of_width = lower_location / 
    double ( this->private_->histogram_graph_->width( ) );
  double current_value = ( this->get_histogram_max() - 
    this->get_histogram_min() ) * percent_of_width;
  if( this->get_histogram_min() < 0 ) current_value = current_value + this->get_histogram_min();
    
  this->lower_threshold_->setCurrentValue( current_value );
}

void QtHistogramWidget::handle_right_button_click( int upper_location )
{
  if( this->upper_threshold_ == 0 ) return;
  if( this->max_bar_->isHidden() ) this->max_bar_->show();
  if( this->min_bar_->isHidden() ) this->min_bar_->show();
  double percent_of_width = upper_location / 
    double ( this->private_->histogram_graph_->width() );
  double current_value = ( this->get_histogram_max() - 
    this->get_histogram_min() ) * percent_of_width;
  if( this->get_histogram_min() < 0 ) current_value = current_value + this->get_histogram_min();
  this->upper_threshold_->setCurrentValue( current_value );
}

double QtHistogramWidget::get_histogram_min()
{
  return this->private_->ui_.min->text().toDouble();
}

double QtHistogramWidget::get_histogram_max()
{
  return this->private_->ui_.max->text().toDouble();
}


} // end namespace QtUtils
