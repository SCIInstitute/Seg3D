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

#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>

#include <Application/InterfaceManager/InterfaceManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/Project/Project.h>

namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( InterfaceManager );

InterfaceManager::InterfaceManager() :
  StateHandler( "interface", false )
{
  // set up state variables
  this->initialize_states();

}

InterfaceManager::~InterfaceManager()
{
}

void InterfaceManager::initialize_states()
{
  // set up state variables
  this->add_state( "fullscreen", full_screen_state_, 
    PreferencesManager::Instance()->full_screen_on_startup_state_->get() );

  this->add_state( "project_dockwidget_visibility", this->project_dockwidget_visibility_state_, 
    PreferencesManager::Instance()->show_projectmanager_bar_state_->get() );
  this->add_state( "toolmanager_dockwidget_visibility", this->toolmanager_dockwidget_visibility_state_, 
    PreferencesManager::Instance()->show_tools_bar_state_->get() );
  this->add_state( "layermanager_dockwidget_visibility", this->layermanager_dockwidget_visibility_state_, 
    PreferencesManager::Instance()->show_layermanager_bar_state_->get() );
  this->add_state( "rendering_dockwidget_visible", this->rendering_dockwidget_visibility_state_, 
    PreferencesManager::Instance()->show_rendering_bar_state_->get() );
  this->add_state( "provenance_dockwidget_visibility", this->provenance_dockwidget_visibility_state_, 
    PreferencesManager::Instance()->show_provenance_bar_state_->get() );
  
  this->add_state( "preferences_manager_visibility", this->preferences_manager_visibility_state_, false );
  this->add_state( "controller_visibility", this->controller_visibility_state_, false );
  this->add_state( "message_window_visibility", this->message_window_visibility_state_, false );
  this->add_state( "splash_screen_visibility", this->splash_screen_visibility_state_, true );
  this->add_state( "keyboard_shortcut_visibility", this->keyboard_shortcut_visibility_state_, false );
  this->add_state( "python_console_visibility", this->python_console_visibility_state_, false );

  this->add_state( "enable_project_creation", enable_project_creation_state_, true );
  this->enable_project_creation_state_->set_locked( true );
  this->add_state( "enable_file_import", enable_file_import_state_, true );
  this->enable_file_import_state_->set_locked( true );

  this->add_state( "enable_controller", this->enable_controller_state_, true );
  this->enable_controller_state_->set_locked( true );
  this->add_state( "enable_tool_help", this->enable_tool_help_state_, true );
  this->enable_tool_help_state_->set_locked( true );

}

int InterfaceManager::get_session_priority()
{
  return SessionPriority::DEFAULT_PRIORITY_E;
}

} // end namespace Seg3D
