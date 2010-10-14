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
  // SETUP_GENERAL_PREFS:
  // This function will initialize all the preferences contained within the general tab
  void setup_general_prefs();

  // SETUP_LAYER_PREFS:
  // This function will initialize all the preferences contained within the layer tab
  void setup_layer_prefs();

  // SETUP_VIEWER_PREFS:
  // This function will initialize all the preferences contained within the viewer tab
  void setup_viewer_prefs();

  // SETUP_SIDEBAR_PREFS:
  // This function will initialize all the preferences contained within the sidebar tab
  void setup_sidebar_prefs();

  // SETUP_INTERFACE_CONTROLS_PREFS:
  // This function will initialize all the preferences contained within the interface control tab
  void setup_interface_controls_prefs();
  
  
private Q_SLOTS:
  // HIDE_THE_OTHERS
  // This function will hide the inactive colorpickers
  void hide_the_others( int index );

  // SET_BUTTONS_TO_DEFAULT_COLORS:
  // this function will set all the buttons colors back to default
  void set_buttons_to_default_colors();

  // CHANGE_PROJECT_DIRECTORY:
  // this function will be called when the user clicks the button to change the default location
  // of the project
  void change_project_directory();

  // SAVE_DEFAULTS:
  // this function will cause Seg3D2 to save all the user preferences to a file which will be 
  // loaded next time the program starts
  void save_settings();
  
private:
  boost::shared_ptr< AppPreferencesPrivate > private_;
  ColorPickerWidget* active_picker_;
  QDir project_directory_;
  
};

}

#endif // INTERFACE_APPPREFERENCES_APPPREFERENCES_H
