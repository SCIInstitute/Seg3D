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

#ifndef INTERFACE_APPLICATION_PREFERENCESINTERFACE_H
#define INTERFACE_APPLICATION_PREFERENCESINTERFACE_H

#ifndef Q_MOC_RUN

// Boost includes
#include <boost/shared_ptr.hpp>

// Qt includes
#include <QFileDialog>
#include <QPointer>

// Core includes
#include <Core/Utils/ConnectionHandler.h>

// Interface includes
#include <Interface/Application/ColorPickerWidget.h>

// QtUtils includes
#include <QtUtils/Widgets/QtCustomDialog.h>

#endif

namespace Seg3D
{

class PreferencesInterfacePrivate;

class PreferencesInterface : public QtUtils::QtCustomDialog, public Core::ConnectionHandler
{
    Q_OBJECT
public:
    PreferencesInterface( QWidget *parent = 0 );
    virtual ~PreferencesInterface();
  
private:
  /// SETUP_GENERAL_PREFS:
  /// This function will initialize all the preferences contained within the general tab
  void setup_general_prefs();

  /// SETUP_LAYER_PREFS:
  /// This function will initialize all the preferences contained within the layer tab
  void setup_layer_prefs();

  /// SETUP_VIEWER_PREFS:
  /// This function will initialize all the preferences contained within the viewer tab
  void setup_viewer_prefs();

  /// SETUP_SIDEBAR_PREFS:
  /// This function will initialize all the preferences contained within the sidebar tab
  void setup_sidebar_prefs();

  // SETUP_INTERFACE_CONTROLS_PREFS:
  // This function will initialize all the preferences contained within the interface control tab
  // (NOTE: not implemented)
//  void setup_interface_controls_prefs();

  void setup_large_volume_prefs();
  
  void set_autosave_checkbox( bool state );
  
  typedef QPointer< PreferencesInterface > qpointer_type;
  
  static void HandleAutosaveStateChanged( qpointer_type qpointer, bool state );
  
private Q_SLOTS:
  /// HIDE_THE_OTHERS
  /// This function will hide the inactive colorpickers
  void hide_the_others( int index );

  /// SET_BUTTONS_TO_DEFAULT_COLORS:
  /// this function will set all the buttons colors back to default
  void set_buttons_to_default_colors();

  /// CHANGE_PROJECT_DIRECTORY:
  /// this function will be called when the user clicks the button to change the default location
  /// of the project
  void change_project_directory();
  
  void set_autosave_checked_state( bool state );

private:
  boost::shared_ptr< PreferencesInterfacePrivate > private_;
  ColorPickerWidget* active_picker_;
  QDir project_directory_;
  
};

} // end namespace Seg3D

#endif
