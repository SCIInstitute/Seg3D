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

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>
#include <Core/State/State.h>
#include <Core/State/StateEngine.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Tool/Tool.h>
#include <Application/Tool/ToolFactory.h>

namespace Seg3D
{

const std::string Tool::NONE_OPTION_C( "<none>" );

Tool::Tool( const std::string& tool_type, size_t version_number, bool auto_number ) :
  StateHandler( tool_type, version_number, auto_number )
{
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

const std::string& Tool::toolid() const
{
  return this->get_statehandler_id();
}

bool Tool::handle_mouse_enter( size_t viewer_id, int x, int y )
{
  // Default implementation doesn't handle this event
  return false;
}

bool Tool::handle_mouse_leave( size_t viewer_id )
{
  // Default implementation doesn't handle this event
  return false;
}

bool Tool::handle_mouse_move( const Core::MouseHistory& mouse_history, int button, int buttons, int modifiers )
{
  // Default implementation doesn't handle this event
  return false;
}

bool Tool::handle_mouse_press( const Core::MouseHistory& mouse_history, int button, int buttons, int modifiers )
{
  // Default implementation doesn't handle this event
  return false;
}

bool Tool::handle_mouse_release( const Core::MouseHistory& mouse_history, int button, int buttons, int modifiers )
{
  // Default implementation doesn't handle this event
  return false;
}

bool Tool::handle_wheel( int delta, int x, int y, int buttons, int modifiers )
{
  // Default implementation doesn't handle this event
  return false;
}

void Tool::repaint( size_t viewer_id, const Core::Matrix& proj_mat )
{
}


} // end namespace Seg3D
