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

#ifndef INTERFACE_APPPREFERENCES_APPPREFERENCES_H
#define INTERFACE_APPPREFERENCES_APPPREFERENCES_H

// Boost includes
#include <boost/shared_ptr.hpp>
#include <Interface/AppPreferences/ColorButton.h>
#include <Interface/AppPreferences/ColorPickerWidget.h>

#include <QtGui>



namespace Seg3D
{

class AppPreferencesPrivate;

class AppPreferences : public QDialog {
    Q_OBJECT
public:
    AppPreferences( QWidget *parent = 0 );
    virtual ~AppPreferences();
  
private:
  void setup_general_prefs();
  void setup_layer_prefs();
  void setup_viewer_prefs();
  void setup_sidebar_prefs();
  void setup_interface_controls_prefs();
  
  void hide_show_color_01( bool );
  
private Q_SLOTS:
  void hide_the_others( int index );
  void set_buttons_to_default_colors();
  
  

private:
  boost::shared_ptr< AppPreferencesPrivate > private_;
  QVector< ColorButton* > color_buttons_;
  QVector< ColorPickerWidget* > color_pickers_;

};

}

#endif // INTERFACE_APPPREFERENCES_APPPREFERENCES_H
