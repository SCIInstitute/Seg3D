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

#include <Core/Interface/Interface.h>
#include <Core/Utils/ScopedCounter.h>

#include <Application/Layer/LayerFWD.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionActivateLayer.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Tool/Tool.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/ToolManager/Actions/ActionActivateTool.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Implementation of class DataPointInfo
//////////////////////////////////////////////////////////////////////////

DataPointInfo::DataPointInfo() :
  index_coord_( 0, 0, 0 ),
  world_coord_( 0, 0, 0 ),
  value_( 0 )
{
}

DataPointInfo::DataPointInfo( const Core::Point& index_coord, 
  const Core::Point& world_coord, double value ) :
  index_coord_( index_coord ),
  world_coord_( world_coord ),
  value_( value )
{
}

DataPointInfo::DataPointInfo( const DataPointInfo& copy ) :
  index_coord_( copy.index_coord_ ),
  world_coord_( copy.world_coord_ ),
  value_( copy.value_ )
{
}

DataPointInfo& DataPointInfo::operator=( const DataPointInfo& copy )
{
  this->index_coord_ = copy.index_coord_;
  this->world_coord_ = copy.world_coord_;
  this->value_ = copy.value_;

  return ( *this );
}

//////////////////////////////////////////////////////////////////////////
// Implementation of class StatusBarPrivate
//////////////////////////////////////////////////////////////////////////

class StatusBarPrivate
{
public:
  void update_layer_list();
  void set_active_layer_by_layer_manager( LayerHandle active_layer );
  void set_active_layer_by_status_bar( std::string layer_id );

  void update_tool_list();
  void set_active_tool_by_tool_manager( ToolHandle active_tool );
  void set_active_tool_by_status_bar( std::string tool_id );

  StatusBar* status_bar_;
  size_t signal_block_count_;

  const static std::string EMPTY_OPTION_C;
};

const std::string StatusBarPrivate::EMPTY_OPTION_C( " " );

void StatusBarPrivate::update_layer_list()
{
  std::vector< LayerIDNamePair > layers;
  LayerManager::Instance()->get_layer_names( layers );
  if ( layers.size() == 0 )
  {
    layers.push_back( std::make_pair( EMPTY_OPTION_C, EMPTY_OPTION_C ) );
  }
  
  {
    Core::ScopedCounter counter( this->signal_block_count_ );
    this->status_bar_->active_layer_state_->set_option_list( layers );
  }
}

void StatusBarPrivate::set_active_layer_by_layer_manager( LayerHandle active_layer )
{
  std::string layer_id = active_layer->get_layer_id();
  if ( layer_id == this->status_bar_->active_layer_state_->get() )
  {
    return;
  }
  
  {
    Core::ScopedCounter counter( this->signal_block_count_ );
    this->status_bar_->active_layer_state_->set( layer_id );
  }
}

void StatusBarPrivate::set_active_layer_by_status_bar( std::string layer_id )
{
  if ( this->signal_block_count_ > 0 )
  {
    return;
  }
  
  if ( layer_id != EMPTY_OPTION_C )
  {
    ActionActivateLayer::Dispatch( Core::Interface::GetWidgetActionContext(), layer_id );
  }
}

void StatusBarPrivate::update_tool_list()
{
  std::vector< ToolIDNamePair > tool_names;
  ToolManager::Instance()->get_tool_names( tool_names );
  if ( tool_names.size() == 0 )
  {
    tool_names.push_back( std::make_pair( EMPTY_OPTION_C, EMPTY_OPTION_C ) );
  }
  
  {
    Core::ScopedCounter counter( this->signal_block_count_ );
    this->status_bar_->active_tool_state_->set_option_list( tool_names );
  }
}

void StatusBarPrivate::set_active_tool_by_tool_manager( ToolHandle active_tool )
{
  std::string tool_id = active_tool->toolid();
  if ( tool_id == this->status_bar_->active_tool_state_->get() )
  {
    return;
  }

  {
    Core::ScopedCounter counter( this->signal_block_count_ );
    this->status_bar_->active_tool_state_->set( tool_id );
  }
}

void StatusBarPrivate::set_active_tool_by_status_bar( std::string tool_id )
{
  if ( this->signal_block_count_ > 0 )
  {
    return;
  }

  if ( tool_id != EMPTY_OPTION_C )
  {
    ActionActivateTool::Dispatch( Core::Interface::GetWidgetActionContext(), tool_id );
  }
}

//////////////////////////////////////////////////////////////////////////
// Implementation of class StatusBar
//////////////////////////////////////////////////////////////////////////

CORE_SINGLETON_IMPLEMENTATION( StatusBar );
const size_t StatusBar::VERSION_NUMBER_C = 0;

StatusBar::StatusBar() :
  StateHandler( "statusbar", VERSION_NUMBER_C, false ),
  private_( new StatusBarPrivate )
{
  this->private_->status_bar_ = this;
  this->private_->signal_block_count_ = 0;

  std::vector< LayerIDNamePair > empty_names( 1, std::make_pair( 
    StatusBarPrivate::EMPTY_OPTION_C, StatusBarPrivate::EMPTY_OPTION_C ) );

  this->add_state( "active_layer", this->active_layer_state_, 
    StatusBarPrivate::EMPTY_OPTION_C, empty_names );
  this->add_state( "active_tool", this->active_tool_state_,
    StatusBarPrivate::EMPTY_OPTION_C, empty_names );

  this->add_connection( LayerManager::Instance()->layers_changed_signal_.connect(
    boost::bind( &StatusBarPrivate::update_layer_list, this->private_ ) ) );
  this->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect(
    boost::bind( &StatusBarPrivate::set_active_layer_by_layer_manager, this->private_, _1 ) ) );
  this->add_connection( this->active_layer_state_->value_changed_signal_.connect(
    boost::bind( &StatusBarPrivate::set_active_layer_by_status_bar, this->private_, _2 ) ) );

  this->add_connection( ToolManager::Instance()->open_tool_signal_.connect(
    boost::bind( &StatusBarPrivate::update_tool_list, this->private_ ) ) );
  this->add_connection( ToolManager::Instance()->close_tool_signal_.connect(
    boost::bind( &StatusBarPrivate::update_tool_list, this->private_ ) ) );
  this->add_connection( ToolManager::Instance()->activate_tool_signal_.connect(
    boost::bind( &StatusBarPrivate::set_active_tool_by_tool_manager, this->private_, _1 ) ) );
  this->add_connection( this->active_tool_state_->value_changed_signal_.connect(
    boost::bind( &StatusBarPrivate::set_active_tool_by_status_bar, this->private_, _2 ) ) );

  this->add_connection( Core::Interface::GetWidgetActionContext()->
    action_message_signal_.connect( boost::bind( &StatusBar::set_message, this, _1, _2 ) ) );
  this->add_connection( Core::Interface::GetMouseActionContext()->
    action_message_signal_.connect( boost::bind( &StatusBar::set_message, this, _1, _2 ) ) );
  this->add_connection( Core::Interface::GetKeyboardActionContext()->
    action_message_signal_.connect( boost::bind( &StatusBar::set_message, this, _1, _2 ) ) );

  this->add_connection( Core::Log::Instance()->post_log_signal_.connect( 
    boost::bind( &StatusBar::set_message, this, _1, _2 ) ) );
}

StatusBar::~StatusBar()
{
  this->disconnect_all();
}

void StatusBar::set_data_point_info( DataPointInfoHandle data_point )
{
  this->data_point_info_updated_signal_( data_point );
}

void StatusBar::set_message( int msg_type, std::string message )
{
  if ( msg_type != Core::LogMessageType::DEBUG_E )
  {
    this->message_updated_signal_( msg_type, message );
  }
}

} // end namespace Seg3D