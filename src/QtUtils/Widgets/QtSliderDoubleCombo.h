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

#ifndef QTUTILS_WIDGETS_QTSLIDERDOUBLECOMBO_H
#define QTUTILS_WIDGETS_QTSLIDERDOUBLECOMBO_H

#ifndef Q_MOC_RUN

// QT includes
#include <QWidget>

// Boost includes
#include <boost/shared_ptr.hpp>

#endif

namespace QtUtils
{

// Forward declarations
class QtSliderDoubleComboPrivate;
typedef boost::shared_ptr< QtSliderDoubleComboPrivate > QtSliderDoubleComboPrivateHandle;

class QtSliderDoubleCombo : public QWidget
{
Q_OBJECT

Q_SIGNALS:
  void valueAdjusted( double );
    void rangeChanged( double, double );
  
// -- constructor/destructor --
public:
  QtSliderDoubleCombo( QWidget* parent = 0 );
    virtual ~QtSliderDoubleCombo();
    
public Q_SLOTS:
  void setStep( double );
  void setRange( double, double );
  void setCurrentValue( double );

public:
  double get_value(){ return value_; }

  void set_description( const std::string& description );
  void set_tooltip( const std::string& tooltip );

  void connect_min( QtSliderDoubleCombo* min );
  void connect_max( QtSliderDoubleCombo* max );
    
// -- widget internals -- 
private:
    double value_;
    QtSliderDoubleComboPrivateHandle private_;
    
private Q_SLOTS:
  void change_min( double new_min );
  void change_max( double new_max );
  void slider_signal( int percentage );
  void spinner_signal( double value );
  void handle_min_signal( double value );
  void handle_max_signal( double value );

private:
    void block_signals( bool block );    
  
};

}  // end namespace QtUtils

#endif
