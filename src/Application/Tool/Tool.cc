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

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>
#include <Core/State/State.h>
#include <Core/State/StateEngine.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Tool/Tool.h>
#include <Application/Tool/ToolFactory.h>

namespace Seg3D
{

class ToolPrivate
{
public:
  std::string name_;
};


const std::string Tool::NONE_OPTION_C( "<none>" );

Tool::Tool( const std::string& tool_type ) :
  StateHandler( tool_type, true ),
  private_( new ToolPrivate )
{
  this->mark_as_project_data();
}

Tool::~Tool()
{
}

void Tool::close()
{
  this->disconnect_all();
}

void Tool::activate()
{
  // Defaults to doing nothing
}

void Tool::deactivate()
{
  // Defaults to doing nothing
}

std::string Tool::get_name() const
{
  return get_tool_info()->get_name();
}

std::string Tool::get_menu() const
{
  return get_tool_info()->get_menu();
}

std::string Tool::get_menu_label() const
{
  return get_tool_info()->get_menu_label();
}

std::string Tool::get_shortcut_key() const
{
  return get_tool_info()->get_shortcut_key();
}

std::string Tool::get_hotkeys_and_descriptions() const
{
  return get_tool_info()->get_hotkeys();
}

std::string Tool::get_url() const
{
  return get_tool_info()->get_url();
}

std::string Tool::get_definition() const
{
  return get_tool_info()->get_definition();
}

const std::string& Tool::toolid() const
{
  return this->get_statehandler_id();
}

std::string Tool::tool_name() const
{
  std::string tool_id = this->toolid();
  return this->get_menu_label() + " " + tool_id.substr( tool_id.find( '_' ) + 1 );
}

bool Tool::handle_mouse_enter( ViewerHandle viewer, int x, int y )
{
  // Default implementation doesn't handle this event
  return false;
}

bool Tool::handle_mouse_leave( ViewerHandle viewer )
{
  // Default implementation doesn't handle this event
  return false;
}

bool Tool::handle_mouse_move( ViewerHandle viewer, 
               const Core::MouseHistory& mouse_history, 
               int button, int buttons, int modifiers )
{
  // Default implementation doesn't handle this event
  return false;
}

bool Tool::handle_mouse_press( ViewerHandle viewer, 
                const Core::MouseHistory& mouse_history, 
                int button, int buttons, int modifiers )
{
  // Default implementation doesn't handle this event
  return false;
}

bool Tool::handle_mouse_release( ViewerHandle viewer, 
                const Core::MouseHistory& mouse_history, 
                int button, int buttons, int modifiers )
{
  // Default implementation doesn't handle this event
  return false;
}

bool Tool::handle_wheel( ViewerHandle viewer, int delta, 
            int x, int y, int buttons, int modifiers )
{
  // Default implementation doesn't handle this event
  return false;
}

bool Tool::handle_key_press( ViewerHandle viewer, int key, int modifiers )
{
  // Default implementation doesn't handle this event
  return false;
}

bool Tool::handle_key_release( ViewerHandle viewer, int key, int modifiers )
{
  // Default implementation doesn't handle this event
  return false;
}

bool Tool::handle_update_cursor( ViewerHandle viewer )
{
  // Default implementation doesn't handle this event
  return false;
}

void Tool::redraw( size_t viewer_id, const Core::Matrix& proj_mat, 
  int viewer_width, int viewer_height )
{
}

bool Tool::has_2d_visual()
{
  return false;
}

bool Tool::has_3d_visual()
{
  return false;
}

void Tool::execute( Core::ActionContextHandle context )
{
}

} // end namespace Seg3D
