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

#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>

#include <Application/Tool/ToolFactory.h>
#include <Application/ToolManager/ToolManager.h>

#include <Application/ToolManager/Actions/ActionOpenTool.h>
#include <Application/ToolManager/Actions/ActionCloseTool.h>
#include <Application/ToolManager/Actions/ActionActivateTool.h>

#include <Application/ViewerManager/ViewerManager.h>

namespace Seg3D
{

const size_t ToolManager::VERSION_NUMBER_C = 1;

CORE_SINGLETON_IMPLEMENTATION( ToolManager );

class ToolManagerPrivate
{
public:
  bool handle_mouse_enter( size_t viewer_id );
  bool handle_mouse_leave( size_t viewer_id );
  bool handle_mouse_move( const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );
  bool handle_mouse_press( const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );
  bool handle_mouse_release( const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );
  bool handle_wheel( int delta, int x, int y, int buttons, int modifiers );

  // All the open tools are stored in this hash map
  ToolManager::tool_list_type tool_list_;
  ToolHandle active_tool_;

  Core::StateStringVectorHandle tools_state_;
};

bool ToolManagerPrivate::handle_mouse_enter( size_t viewer_id )
{
  if ( this->active_tool_ )
  {
    return this->active_tool_->handle_mouse_enter( viewer_id );
  }
  return false;
}

bool ToolManagerPrivate::handle_mouse_leave( size_t viewer_id )
{
  if ( this->active_tool_ )
  {
    return this->active_tool_->handle_mouse_leave( viewer_id );
  }
  return false;
}

bool ToolManagerPrivate::handle_mouse_move( const Core::MouseHistory& mouse_history, 
                       int button, int buttons, int modifiers )
{
  if ( this->active_tool_ )
  {
    return this->active_tool_->handle_mouse_move( mouse_history, button, buttons, modifiers );
  }
  return false;
}

bool ToolManagerPrivate::handle_mouse_press( const Core::MouseHistory& mouse_history, 
                      int button, int buttons, int modifiers )
{
  if ( this->active_tool_ )
  {
    return this->active_tool_->handle_mouse_press( mouse_history, button, buttons, modifiers );
  }
  return false;
}

bool ToolManagerPrivate::handle_mouse_release( const Core::MouseHistory& mouse_history, 
                        int button, int buttons, int modifiers )
{
  if ( this->active_tool_ )
  {
    return this->active_tool_->handle_mouse_release( mouse_history, button, buttons, modifiers );
  }
  return false;
}

bool ToolManagerPrivate::handle_wheel( int delta, int x, int y, int buttons, int modifiers )
{
  if ( this->active_tool_ )
  {
    return this->active_tool_->handle_wheel( delta, x, y, buttons, modifiers );
  }
  return false;
}

ToolManager::ToolManager() :
  StateHandler( "ToolManager", VERSION_NUMBER_C, false, 2 ),
  private_( new ToolManagerPrivate )
{
  std::vector< std::string> tools;
  this->add_state( "tools", this->private_->tools_state_, tools );

  // Register mouse event handlers for all the viewers
  size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
  for ( size_t i = 0; i < num_of_viewers; i++ )
  {
    ViewerHandle viewer = ViewerManager::Instance()->get_viewer( i );
    viewer->set_mouse_enter_handler( boost::bind( &ToolManagerPrivate::handle_mouse_enter,
      this->private_, _1 ) );
    viewer->set_mouse_leave_handler( boost::bind( &ToolManagerPrivate::handle_mouse_leave,
      this->private_, _1 ) );
    viewer->set_mouse_move_handler( boost::bind( &ToolManagerPrivate::handle_mouse_move,
      this->private_, _1, _2, _3, _4 ) );
    viewer->set_mouse_press_handler( boost::bind( &ToolManagerPrivate::handle_mouse_press,
      this->private_, _1, _2, _3, _4 ) );
    viewer->set_mouse_release_handler( boost::bind( &ToolManagerPrivate::handle_mouse_release,
      this->private_, _1, _2, _3, _4 ) );
    viewer->set_wheel_event_handler( boost::bind( &ToolManagerPrivate::handle_wheel,
      this->private_, _1, _2, _3, _4, _5 ) );
  }
}

ToolManager::~ToolManager()
{
  this->disconnect_all();
}

// THREAD-SAFETY:
// Only ActionOpenTool calls this function and this action is only run on the
// application thread. Hence the function is always executed by the same thread.

bool ToolManager::open_tool( const std::string& tool_type, std::string& new_toolid, 
  bool create_tool_id /*= true */ )
{
  // Step (1): Make the function thread safe
  lock_type lock( this->get_mutex() );

  // Step (2): Add an entry in the debug log
  CORE_LOG_DEBUG( std::string( "Open tool: " ) + tool_type );

  // Step (4): Build the tool using the factory. This will generate the default
  // settings.
  ToolHandle tool;

  if ( !( ToolFactory::Instance()->create_tool( tool_type, tool ) ) )
  {
    CORE_LOG_ERROR( std::string( "Could not create tool of type: '" ) + tool_type + "'" );
    return false;
  }

  // Step (5): Add the tool id to the tool and add the tool to the list
  new_toolid = tool->toolid();
  this->private_->tool_list_[ new_toolid ] = tool;

  // Step (6): Signal any observers (UIs) that the tool has been opened
  open_tool_signal_( tool );

  // All done
  return true;
}

// THREAD-SAFETY:
// Only ActionCloseTool calls this function and this action is only run on the
// application thread. Hence the function is always executed by the same thread.

void ToolManager::close_tool( const std::string& toolid )
{
  // Step (1): Make the function thread safe
  lock_type lock( this->get_mutex() );

  // Step (2): Add an entry in the debug log
  CORE_LOG_DEBUG( std::string( "Close tool: " ) + toolid );

  // Step (3): Find the tool in the list.
  tool_list_type::iterator it = this->private_->tool_list_.find( toolid );
  if ( it == this->private_->tool_list_.end() )
  {
    CORE_LOG_ERROR( std::string( "Toolid '" + toolid + "' does not exist" ) );
    return;
  }

  // Step (4): Get the tool from the iterator
  ToolHandle tool = ( *it ).second;
  if ( tool == this->private_->active_tool_ )
  {
    // Call the tool deactivate function that will unregister the current
    // tool bindings
    tool->deactivate();

    // Set no tool as active
    this->private_->active_tool_.reset();
  }

  // Step (5): Move the tool from the list. The tool handle still persists
  // and will be removed after the signal has been posted.
  this->private_->tool_list_.erase( it );

  // Step (6): Run the function in the tool that cleans up the parts that
  // need to be cleaned up on the interface thread.
  tool->close();

  // Step (7): Signal that the tool will be closed.
  close_tool_signal_( tool );
}

// THREAD-SAFETY:
// Only ActionActivateTool calls this function and this action is only run on the
// application thread. Hence the function is always executed by the same thread.

void ToolManager::activate_tool( const std::string& toolid )
{
  // Step (1): Make the function thread safe
  lock_type lock( this->get_mutex() );

  // Step (2): Add an entry in the debug log
  CORE_LOG_DEBUG( std::string( "Activate tool: " ) + toolid );

  // Step (3): Check if anything needs to be done
  if ( this->private_->active_tool_ && 
    this->private_->active_tool_->toolid() == toolid )
  {
    return;
  }
  
  // Step (4): Deactivate the current active tool if it exists
  if ( this->private_->active_tool_ )
  {
    this->private_->active_tool_->deactivate();
  }
  
  // Step (5): Find new active tool and activate it
  tool_list_type::iterator it = this->private_->tool_list_.find( toolid );
  if ( it != this->private_->tool_list_.end() )
  {
    ( *it ).second->activate();
    this->private_->active_tool_ = ( *it ).second;
  }
  else
  {
    this->private_->active_tool_.reset();
    return;
  }

  // Step (4): signal for interface
  activate_tool_signal_( ( *it ).second );
}

ToolManager::tool_list_type ToolManager::tool_list()
{
  lock_type lock( this->get_mutex() );
  return this->private_->tool_list_;
}

std::string ToolManager::active_toolid()
{
  lock_type lock( this->get_mutex() );
  if ( this->private_->active_tool_ )
  {
    return this->private_->active_tool_->toolid();
  }
  return "";
}

ToolHandle ToolManager::get_active_tool()
{
  lock_type lock( this->get_mutex() );
  return this->private_->active_tool_;
}

bool ToolManager::pre_save_states()
{
  lock_type lock( this->get_mutex() );

  std::vector< std::string > tools_vector;
  this->private_->tools_state_->set( tools_vector );

  for( tool_list_type::iterator it = this->private_->tool_list_.begin(); 
    it != this->private_->tool_list_.end(); ++it )
  {
    tools_vector.push_back( ( *it ).first );
  }

  this->private_->tools_state_->set( tools_vector );

  return true;
}

bool ToolManager::post_save_states()
{
  lock_type lock( this->get_mutex() );

  for( tool_list_type::iterator it = this->private_->tool_list_.begin(); 
    it != this->private_->tool_list_.end(); ++it )
  {
    if( !( ( *it ).second )->populate_session_states() )
    {
      return false;
    }
  }
  return true;
}

} // end namespace Seg3D
