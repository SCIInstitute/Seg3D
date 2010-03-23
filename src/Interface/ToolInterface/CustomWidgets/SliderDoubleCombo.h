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

#ifndef INTERFACE_TOOLINTERFACE_CUSTOMWIDGETS_SLIDERDOUBLECOMBO_H
#define INTERFACE_TOOLINTERFACE_CUSTOMWIDGETS_SLIDERDOUBLECOMBO_H

// QT Includes
#include <QtGui>

#include <boost/shared_ptr.hpp>

namespace Seg3D 
{

class SliderDoubleComboPrivate;

class SliderDoubleCombo : public QWidget
{
Q_OBJECT

Q_SIGNALS:
  void valueAdjusted( double );
    void rangeChanged( double, double );
// -- constructor/destructor --
public:
    SliderDoubleCombo( QWidget* parent = 0, bool edit_range = false );
    virtual ~SliderDoubleCombo();
    
public Q_SLOTS:
    void setStep( double );
  void setRange( double, double );
  void setCurrentValue( double );
  void set_all( double min, double max, double value );

    
// -- widget internals -- 
private:
    boost::shared_ptr< SliderDoubleComboPrivate > private_;
    
private Q_SLOTS:
    void edit_ranges( bool edit );
    void change_min( double new_min );
    void change_max( double new_max );
    void double_range();
    void half_range();
    void slider_signal( int value );
    void spinner_signal( double value );
  
};

}  // end namespace Seg3D

#endif