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

// boost includes
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <tinyxml.h>

#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>
#include <Core/State/StateIO.h>
#include <Core/Utils/ScopedCounter.h>

#include <Application/Project/Project.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/ToolManager/ToolManager.h>

#include <Application/ToolManager/Actions/ActionOpenTool.h>
#include <Application/ToolManager/Actions/ActionCloseTool.h>
#include <Application/ToolManager/Actions/ActionActivateTool.h>

#include <Application/ViewerManager/ViewerManager.h>

namespace Seg3D
{


//////////////////////////////////////////////////////////////////////////
// Class ToolManagerPrivate
//////////////////////////////////////////////////////////////////////////

class ToolManagerPrivate
{
public:
  bool handle_mouse_enter( ViewerHandle viewer, int x, int y );
  bool handle_mouse_leave( ViewerHandle viewer );
  bool handle_mouse_move( ViewerHandle viewer, const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );
  bool handle_mouse_press( ViewerHandle viewer, const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );
  bool handle_mouse_release( ViewerHandle viewer, const Core::MouseHistory& mouse_history,
    int button, int buttons, int modifiers );
  bool handle_wheel( ViewerHandle viewer, int delta, int x, int y, int buttons, int modifiers );
  bool handle_key_press( ViewerHandle viewer, int key, int modifiers );
  bool handle_key_release( ViewerHandle viewer, int key, int modifiers );
  bool handle_update_cursor( ViewerHandle viewer );

  void update_viewers( bool redraw_2d, bool redraw_3d );

  void update_tool_list();
  void handle_active_tool_changed();
  void handle_active_tool_state_changed( std::string tool_id );

  // DELETE_ALL:
  // This function closes and deletes all the current tools. 
  // It's called when the application is being reset.
  void reset();

  // All the open tools are stored in this hash map
  ToolManager::tool_list_type tool_list_;
  ToolHandle active_tool_;
  ViewerHandle focus_viewer_;
  ToolManager* tool_manager_;
  size_t signal_block_count_;
};

bool ToolManagerPrivate::handle_mouse_enter( ViewerHandle viewer, int x, int y )
{
  this->focus_viewer_ = viewer;

  ToolHandle active_tool;
  {
    ToolManager::lock_type lock( ToolManager::Instance()->get_mutex() );
    active_tool = this->active_tool_;
  }
  if ( active_tool )
  {
    return active_tool->handle_mouse_enter( viewer, x, y );
  }
  return false;
}

bool ToolManagerPrivate::handle_mouse_leave( ViewerHandle viewer )
{
  this->focus_viewer_.reset();

  ToolHandle active_tool;
  {
    ToolManager::lock_type lock( ToolManager::Instance()->get_mutex() );
    active_tool = this->active_tool_;
  }
  if ( active_tool )
  {
    return active_tool->handle_mouse_leave( viewer );
  }
  return false;
}

bool ToolManagerPrivate::handle_mouse_move( ViewerHandle viewer, 
                       const Core::MouseHistory& mouse_history, 
                       int button, int buttons, int modifiers )
{
  // If there is no focus viewer, simulate a mouse enter event.
  // NOTE: It is not safe to pass this mouse move event to the active tool, because
  // the mouse history information might not be correct.
  if ( !this->focus_viewer_ )
  {
    return this->handle_mouse_enter( viewer, mouse_history.current_.x_, 
      mouse_history.current_.y_ );
  }
  
  ToolHandle active_tool;
  {
    ToolManager::lock_type lock( ToolManager::Instance()->get_mutex() );
    active_tool = this->active_tool_;
  }
  if ( active_tool )
  {
    return active_tool->handle_mouse_move( viewer, mouse_history, 
      button, buttons, modifiers );
  }
  return false;
}

bool ToolManagerPrivate::handle_mouse_press( ViewerHandle viewer, 
                      const Core::MouseHistory& mouse_history, 
                      int button, int buttons, int modifiers )
{
  this->focus_viewer_ = viewer;

  ToolHandle active_tool;
  {
    ToolManager::lock_type lock( ToolManager::Instance()->get_mutex() );
    active_tool = this->active_tool_;
  }
  if ( active_tool )
  {
    return active_tool->handle_mouse_press( viewer, mouse_history, 
      button, buttons, modifiers );
  }
  return false;
}

bool ToolManagerPrivate::handle_mouse_release( ViewerHandle viewer,
                        const Core::MouseHistory& mouse_history, 
                        int button, int buttons, int modifiers )
{
  this->focus_viewer_ = viewer;

  ToolHandle active_tool;
  {
    ToolManager::lock_type lock( ToolManager::Instance()->get_mutex() );
    active_tool = this->active_tool_;
  }
  if ( active_tool )
  {
    return active_tool->handle_mouse_release( viewer, mouse_history, 
      button, buttons, modifiers );
  }
  return false;
}

bool ToolManagerPrivate::handle_wheel( ViewerHandle viewer, int delta, 
                    int x, int y, int buttons, int modifiers )
{
  this->focus_viewer_ = viewer;

  ToolHandle active_tool;
  {
    ToolManager::lock_type lock( ToolManager::Instance()->get_mutex() );
    active_tool = this->active_tool_;
  }
  if ( active_tool )
  {
    return active_tool->handle_wheel( viewer, delta, x, y, buttons, modifiers );
  }
  return false;
}

bool ToolManagerPrivate::handle_key_press( ViewerHandle viewer, int key, int modifiers )
{
  this->focus_viewer_ = viewer;

  ToolHandle active_tool;
  {
    ToolManager::lock_type lock( ToolManager::Instance()->get_mutex() );
    active_tool = this->active_tool_;
  }
  if ( active_tool )
  {
    return active_tool->handle_key_press( viewer, key, modifiers );
  }
  return false;
}

bool ToolManagerPrivate::handle_key_release( ViewerHandle viewer, int key, int modifiers )
{
  this->focus_viewer_ = viewer;

  ToolHandle active_tool;
  {
    ToolManager::lock_type lock( ToolManager::Instance()->get_mutex() );
    active_tool = this->active_tool_;
  }
  if ( active_tool )
  {
    return active_tool->handle_key_release( viewer, key, modifiers );
  }
  return false;
}

bool ToolManagerPrivate::handle_update_cursor( ViewerHandle viewer )
{
  ToolHandle active_tool;
  {
    ToolManager::lock_type lock( ToolManager::Instance()->get_mutex() );
    active_tool = this->active_tool_;
  }
  if ( active_tool )
  {
    return active_tool->handle_update_cursor( viewer );
  }
  return false;
}

void ToolManagerPrivate::update_viewers( bool redraw_2d, bool redraw_3d )
{
  size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
  for ( size_t i = 0; i < num_of_viewers; i++ )
  {
    ViewerHandle viewer = ViewerManager::Instance()->get_viewer( i );
    if ( viewer->is_volume_view() )
    {
      if ( redraw_3d )
      {
        viewer->redraw_scene();
      }
    }
    else if ( redraw_2d )
    {
      viewer->redraw_overlay();
    }
  }
}

void ToolManagerPrivate::update_tool_list()
{
  std::vector< ToolIDNamePair > tool_names;
  this->tool_manager_->get_tool_names( tool_names );

  {
    Core::ScopedCounter counter( this->signal_block_count_ );
    this->tool_manager_->active_tool_state_->set_option_list( tool_names );
  }
}

void ToolManagerPrivate::handle_active_tool_changed()
{
  if ( this->signal_block_count_ > 0 )
  {
    return;
  }
  
  Core::ScopedCounter signal_block( this->signal_block_count_ );
  if ( this->active_tool_ )
  {
    this->tool_manager_->active_tool_state_->set( this->active_tool_->toolid() );
  }
}

void ToolManagerPrivate::handle_active_tool_state_changed( std::string tool_id )
{
  if ( this->signal_block_count_ > 0 || tool_id == "" )
  {
    return;
  }
  
  Core::ScopedCounter signal_block( this->signal_block_count_ );
  this->tool_manager_->activate_tool( tool_id );
}

void ToolManagerPrivate::reset()
{
  ASSERT_IS_APPLICATION_THREAD();

  ToolManager::tool_list_type::iterator it = this->tool_list_.begin();
  while( it != this->tool_list_.end() )
  {
    ( *it ).second->invalidate();
    ( *it ).second->close();
    ++it;
  }
  this->active_tool_.reset();
  this->tool_list_.clear();
  this->update_tool_list();
  this->tool_manager_->disable_tools_state_->set( false );
  ViewerManager::Instance()->reset_cursor();
}

//////////////////////////////////////////////////////////////////////////
// Class ToolManager
//////////////////////////////////////////////////////////////////////////

CORE_SINGLETON_IMPLEMENTATION( ToolManager );

ToolManager::ToolManager() :
  StateHandler( "toolmanager", false ),
  private_( new ToolManagerPrivate )
{
  // Mask the data contained in this manager as session data.
  this->mark_as_project_data();

  this->add_state( "active_tool", this->active_tool_state_ );
  
  // this state variable is currently not being used.
  this->add_state( "disable_tools", this->disable_tools_state_, false );
  
  this->private_->tool_manager_ = this;
  this->private_->signal_block_count_ = 0;

  // Register mouse event handlers for all the viewers
  size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
  for ( size_t i = 0; i < num_of_viewers; i++ )
  {
    ViewerHandle viewer = ViewerManager::Instance()->get_viewer( i );
    viewer->set_mouse_enter_handler( boost::bind( &ToolManagerPrivate::handle_mouse_enter,
      this->private_, _1, _2, _3 ) );
    viewer->set_mouse_leave_handler( boost::bind( &ToolManagerPrivate::handle_mouse_leave,
      this->private_, _1 ) );
    viewer->set_mouse_move_handler( boost::bind( &ToolManagerPrivate::handle_mouse_move,
      this->private_, _1, _2, _3, _4, _5 ) );
    viewer->set_mouse_press_handler( boost::bind( &ToolManagerPrivate::handle_mouse_press,
      this->private_, _1, _2, _3, _4, _5 ) );
    viewer->set_mouse_release_handler( boost::bind( &ToolManagerPrivate::handle_mouse_release,
      this->private_, _1, _2, _3, _4, _5 ) );
    viewer->set_wheel_event_handler( boost::bind( &ToolManagerPrivate::handle_wheel,
      this->private_, _1, _2, _3, _4, _5, _6 ) );
    viewer->set_key_press_event_handler( boost::bind( &ToolManagerPrivate::handle_key_press,
      this->private_, _1, _2, _3 ) );
    viewer->set_key_release_event_handler( boost::bind( &ToolManagerPrivate::handle_key_release,
      this->private_, _1, _2, _3 ) );
    viewer->set_cursor_handler( boost::bind( &ToolManagerPrivate::handle_update_cursor,
      this->private_, _1 ) );
  }

  this->add_connection( this->open_tool_signal_.connect( boost::bind( 
    &ToolManagerPrivate::update_tool_list, this->private_ ) ) );
  this->add_connection( this->close_tool_signal_.connect( boost::bind( 
    &ToolManagerPrivate::update_tool_list, this->private_ ) ) );
  this->add_connection( this->activate_tool_signal_.connect( boost::bind( 
    &ToolManagerPrivate::handle_active_tool_changed, this->private_ ) ) );
  this->add_connection( this->active_tool_state_->value_changed_signal_.connect( boost::bind( 
    &ToolManagerPrivate::handle_active_tool_state_changed, this->private_, _2 ) ) );
  this->add_connection( Core::Application::Instance()->reset_signal_.connect( boost::bind(
    &ToolManagerPrivate::reset, this->private_ ) ) );
}

ToolManager::~ToolManager()
{
  this->disconnect_all();
}

// THREAD-SAFETY:
// Only ActionOpenTool calls this function and this action is only run on the
// application thread. Hence the function is always executed by the same thread.

bool ToolManager::open_tool( const std::string& tool_type, std::string& new_toolid )
{
  // Step (1): Make the function thread safe
  lock_type lock( this->get_mutex() );

  // Step (2): Add an entry in the debug log
  CORE_LOG_MESSAGE( std::string( "Open tool: " ) + tool_type );

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

  // Set the tool to be active
  this->activate_tool( new_toolid );

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
  CORE_LOG_MESSAGE( std::string( "Close tool: " ) + toolid );

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

  if ( !this->private_->active_tool_ )
  {
    if ( !this->private_->tool_list_.empty() )
    {
      this->private_->active_tool_ = ( *this->private_->tool_list_.begin() ).second;
      this->private_->active_tool_->activate();
      this->activate_tool_signal_( this->private_->active_tool_ );
    }

    bool redraw_2d = tool->has_2d_visual() || ( this->private_->active_tool_ && 
      this->private_->active_tool_->has_2d_visual() );
    bool redraw_3d = tool->has_3d_visual() || ( this->private_->active_tool_ && 
      this->private_->active_tool_->has_3d_visual() );

    if ( redraw_2d || redraw_3d )
    {
      this->private_->update_viewers( redraw_2d, redraw_3d );
    }
  }
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

  // Step (4): Find new active tool
  tool_list_type::iterator it = this->private_->tool_list_.find( toolid );
  if ( it == this->private_->tool_list_.end() )
  {
    return;
  }

  // Step (4): Deactivate the current active tool if it exists, and activate the new one
  ToolHandle old_tool = this->private_->active_tool_;
  this->private_->active_tool_ = ( *it ).second;
  if ( old_tool )
  {
    old_tool->deactivate();
  } 
  this->private_->active_tool_->activate();

  // Step (5): Update viewers if necessary.
  bool redraw_2d = this->private_->active_tool_->has_2d_visual() ||
    ( old_tool && old_tool->has_2d_visual() );
  bool redraw_3d = this->private_->active_tool_->has_3d_visual() ||
    ( old_tool && old_tool->has_3d_visual() );

  if ( redraw_2d || redraw_3d )
  {
    this->private_->update_viewers( redraw_2d, redraw_3d );
  }
  
  // Step (6): signal for interface
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
  return Tool::NONE_OPTION_C;
}

ToolHandle ToolManager::get_active_tool()
{
  lock_type lock( this->get_mutex() );
  return this->private_->active_tool_;
}

bool ToolManager::pre_save_states( Core::StateIO& state_io )
{
  return true;
}

bool ToolManager::post_save_states( Core::StateIO& state_io )
{
  TiXmlElement* tm_element = state_io.get_current_element();
  assert( this->get_statehandler_id() == tm_element->Value() );
  TiXmlElement* tools_element = new TiXmlElement( "tools" );
  tm_element->LinkEndChild( tools_element );

  state_io.push_current_element();
  state_io.set_current_element( tools_element );

  tool_list_type::iterator it = this->private_->tool_list_.begin();
  tool_list_type::iterator it_end = this->private_->tool_list_.end();
  while ( it != it_end )
  {
    ( *it ).second->save_states( state_io );
    ++it;
  }

  state_io.pop_current_element();

  return true;
}

bool ToolManager::pre_load_states( const Core::StateIO& state_io )
{
  // Make sure that the tool list is empty.
  // NOTE: The application should have been properly reset before loading a session.
  assert( this->private_->tool_list_.empty() );

  const TiXmlElement* tools_element = state_io.get_current_element()->
    FirstChildElement( "tools" );
  if ( tools_element == 0 )
  {
    CORE_LOG_ERROR( "Could not find tools element." );
    return false;
  }

  state_io.push_current_element();
  state_io.set_current_element( tools_element );

  bool success = true;
  const TiXmlElement* tool_element = tools_element->FirstChildElement();
  while ( tool_element != 0 )
  {
    std::string toolid( tool_element->Value() );
    if ( this->open_tool( toolid, toolid ) )
    {
      ToolHandle tool = this->get_tool( toolid );
      if ( ! tool->load_states( state_io ) )
      {
        std::string error = std::string( "Could not load states for tool '" ) + toolid + "'.";
        CORE_LOG_ERROR( error );
        // NOTE: We should not fail the entire session if a tool fails to load
        //success = false;
      } 
    }
    tool_element = tool_element->NextSiblingElement();
  }

  state_io.pop_current_element();

  return success;
}

bool ToolManager::post_load_states( const Core::StateIO& state_io )
{
  if( this->active_tool_state_->get() != "" )
  {
    this->activate_tool( this->active_tool_state_->get() );
  }
  else if ( !this->private_->tool_list_.empty() )
  {
    this->activate_tool( ( *this->private_->tool_list_.begin() ).first );
  }

  return true;
}

ToolHandle ToolManager::get_tool( const std::string& toolid )
{
  tool_list_type::iterator it = this->private_->tool_list_.find( toolid );
  if ( it != this->private_->tool_list_.end() )
  {
    return ( *it ).second;
  }
  
  return ToolHandle();
}

void ToolManager::get_tool_names( std::vector< ToolIDNamePair >& tool_names )
{
  lock_type lock( this->get_mutex() );
  tool_list_type::iterator it = this->private_->tool_list_.begin();
  tool_list_type::iterator it_end = this->private_->tool_list_.end();
  while ( it != it_end )
  {
    ToolHandle tool = ( *it ).second;
    tool_names.push_back( std::make_pair( tool->toolid(), tool->tool_name() ) );
    it++;
  }
}

int ToolManager::get_session_priority()
{
  return SessionPriority::TOOL_MANAGER_PRIORITY_E;
}

void ToolManager::open_default_tools()
{
  ToolFactory::startup_tools_map_type::const_iterator it = 
    ToolFactory::Instance()->startup_tools_state_.begin();
  while ( it != ToolFactory::Instance()->startup_tools_state_.end() )
  {
    const std::vector< std::string >& tools = ( *it ).second->get();
    std::string tool_id;
    std::for_each( tools.begin(), tools.end(), boost::lambda::bind( &ToolManager::open_tool,
      this, boost::lambda::_1, boost::lambda::var( tool_id ) ) );
    ++it;
  }
}

} // end namespace Seg3D
