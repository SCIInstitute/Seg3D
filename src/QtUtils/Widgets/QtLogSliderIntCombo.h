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

#ifndef QTUTILS_WIDGETS_QTLOGSLIDERINTCOMBO_H
#define QTUTILS_WIDGETS_QTLOGSLIDERINTCOMBO_H

#ifndef Q_MOC_RUN

// QT Includes
#include <QWidget>

// Boost includes
#include <boost/shared_ptr.hpp>

#endif

namespace QtUtils 
{

// Forward declaration
class QtLogSliderIntCombo;
class QtLogSliderIntComboPrivate;
typedef boost::shared_ptr< QtLogSliderIntComboPrivate > QtLogSliderIntComboPrivateHandle;

// Class definition
class QtLogSliderIntCombo : public QWidget
{
Q_OBJECT

// -- constructor/destructor --
public:
    QtLogSliderIntCombo( QWidget* parent = 0 );
    virtual ~QtLogSliderIntCombo();

Q_SIGNALS:
  void valueAdjusted( int );
    void valueChanged( int );
  
public Q_SLOTS:
    void setStep( int );
  void setRange( int, int );
  void setCurrentValue( int );
  
public:
  int get_value() { return value_; }
  void set_description( std::string description );

// -- widget internals -- 
private:
    QtLogSliderIntComboPrivateHandle private_;
  int value_;  
    
private Q_SLOTS:
    void change_min( int new_min );
    void change_max( int new_max );
    void slider_signal( int value );
    void spinner_signal( int value );
    
private:
  void block_signals( bool block );
};

}  // end namespace QtUtils

#endif
