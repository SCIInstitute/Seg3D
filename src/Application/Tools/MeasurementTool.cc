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

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/MeasurementTool.h>
#include <Application/ViewerManager/ViewerManager.h>

// Core includes
#include <Core/State/Actions/ActionRemove.h>
#include <Core/State/Actions/ActionSetAt.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, MeasurementTool )

namespace Seg3D
{

class MeasurementToolPrivate 
{
public:
  void set_tool( MeasurementTool* tool );
  void update_active_index();
  void handle_units_selection_changed( std::string units );
  void handle_active_layer_changed( LayerHandle active_layer );
  void handle_opacity_changed();
  void update_viewers();

  MeasurementTool* tool_;
  std::string active_group_id_;
};

void MeasurementToolPrivate::set_tool( MeasurementTool* tool )
{
  this->tool_ = tool;
}

void MeasurementToolPrivate::update_active_index()
{
  ASSERT_IS_APPLICATION_THREAD();

  size_t num_measurements = this->tool_->get_measurements().size();
  
  if( num_measurements > 0 )
  {
    // If the active index isn't in the valid range, set to end of list
    int active_index = this->tool_->get_active_index();
    if( active_index == -1 || active_index >= static_cast< int >( num_measurements ) )
    {
      this->tool_->set_active_index( static_cast< int >( num_measurements ) - 1 );
    }
  }
  else
  {
    this->tool_->set_active_index( -1 );
  }
}

void MeasurementToolPrivate::handle_units_selection_changed( std::string units )
{
  ASSERT_IS_APPLICATION_THREAD();

  bool old_show_world_units_state = this->tool_->show_world_units_state_->get();

  if ( units == MeasurementTool::WORLD_UNITS_C )
  {
    this->tool_->show_world_units_state_->set( true );
  }
  else
  {
    this->tool_->show_world_units_state_->set( false );
  }

  // If units have changed, emit signal
  if( old_show_world_units_state != this->tool_->show_world_units_state_->get() )
  {
    this->tool_->units_changed_signal_();
  }
}

void MeasurementToolPrivate::handle_active_layer_changed( LayerHandle active_layer )
{
  ASSERT_IS_APPLICATION_THREAD();

  // To minimize measurement table updates, only emit units_changed_signal_ when 
  // the active group has changed AND the index units are selected.
  bool show_index_units = !this->tool_->show_world_units_state_->get();
  if ( active_layer )
  {
    // If the active group has changed
    std::string curr_active_group_id = active_layer->get_layer_group()->get_group_id();
    // Don't need to mutex-protect active_group_id_ because it will only be accessed
    // on the application thread.
    // Check to see if the group size is 1 to handle case where layer is deleted, then 
    // delete is undone but group id didn't get changed in process.
    if( this->active_group_id_ != curr_active_group_id || 
      active_layer->get_layer_group()->get_list_size() == 1 )  
    {
      this->active_group_id_ = curr_active_group_id;

      if( show_index_units )
      {
        // Signal that measurement units have changed
        this->tool_->units_changed_signal_();
      } 
    }
  }
  else
  {
    this->active_group_id_ = "";
  }
}

void MeasurementToolPrivate::handle_opacity_changed()
{
  this->update_viewers();
}

void MeasurementToolPrivate::update_viewers()
{
  /*if ( this->signal_block_count_ > 0 )
  {
    return;
  }*/

  ViewerManager::Instance()->update_2d_viewers_overlay();
}

//void create_test_data( std::vector< Core::Measurement >& measurements )
//{
//  // Populate measurements list with test data
//  measurements.push_back( 
//    Core::Measurement( true, "M1", "Knee", Core::Point(0, 0, 0), 
//    Core::Point(1, 1, 1), Core::AXIAL_E, 50, 1 ) );
//  measurements.push_back( 
//    Core::Measurement( true, "M2", "Heart", Core::Point(0, 0, 0), 
//    Core::Point(2, 2, 2), Core::AXIAL_E, 50, 1 ) );
//  measurements.push_back( 
//    Core::Measurement( true, "M3", "Head", Core::Point(0, 0, 0), 
//    Core::Point(3, 3, 3), Core::AXIAL_E, 50, 1 ) ); 
//  measurements.push_back( 
//    Core::Measurement( true, "M4", "Toe", Core::Point(0, 0, 0), 
//    Core::Point(4, 4, 4), Core::AXIAL_E, 50, 1 ) ); 
//  measurements.push_back( 
//    Core::Measurement( true, "M5", "Eye", Core::Point(0, 0, 0), 
//    Core::Point(5, 5, 5), Core::AXIAL_E, 50, 1 ) ); 
//  measurements.push_back( 
//    Core::Measurement( true, "M6", "Nose", Core::Point(0, 0, 0), 
//    Core::Point(6, 6, 6), Core::AXIAL_E, 50, 1 ) ); 
//  measurements.push_back( 
//    Core::Measurement( true, "M7", "Hand", Core::Point(0, 0, 0), 
//    Core::Point(7, 7, 7), Core::AXIAL_E, 50, 1 ) ); 
//  measurements.push_back( 
//    Core::Measurement( true, "M8", "Ear", Core::Point(0, 0, 0), 
//    Core::Point(8, 8, 8), Core::AXIAL_E, 50, 1 ) ); 
//}

//////////////////////////////////////////////////////////////////////////
// Class MeasurementTool
//////////////////////////////////////////////////////////////////////////

const std::string MeasurementTool::INDEX_UNITS_C( "index_units" );
const std::string MeasurementTool::WORLD_UNITS_C( "world_units" );

MeasurementTool::MeasurementTool( const std::string& toolid ) :
  Tool( toolid ),
  private_( new MeasurementToolPrivate )
{
  this->private_->set_tool( this );
  this->private_->active_group_id_ = "";

  // State variable gets allocated here
  this->add_state( "measurements", this->measurements_state_ );
  this->add_state( "active_index", this->active_index_state_, -1 );
  this->add_state( "units_selection", this->units_selection_state_, WORLD_UNITS_C, 
    INDEX_UNITS_C + "=Index Units|" +
    WORLD_UNITS_C + "=World Units" );
  this->add_state( "show_world_units", this->show_world_units_state_, true );
  this->add_state( "opacity", this->opacity_state_, 0.5, 0.0, 1.0, 0.1 );

  LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
  if( active_layer )
  {
    this->private_->active_group_id_ = active_layer->get_layer_group()->get_group_id();
  }

  this->add_connection( this->measurements_state_->state_changed_signal_.connect( 
    boost::bind( &MeasurementToolPrivate::update_active_index, this->private_ ) ) );
  this->add_connection( this->units_selection_state_->value_changed_signal_.connect(
    boost::bind( &MeasurementToolPrivate::handle_units_selection_changed, this->private_, _2 ) ) );
  this->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect(
    boost::bind( &MeasurementToolPrivate::handle_active_layer_changed, this->private_, _1 ) ) );
  this->add_connection( this->opacity_state_->state_changed_signal_.connect(
    boost::bind( &MeasurementToolPrivate::handle_opacity_changed, this->private_ ) ) );
}

MeasurementTool::~MeasurementTool()
{
  this->disconnect_all();
}

std::vector< Core::Measurement > MeasurementTool::get_measurements() const
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return this->measurements_state_->get();
}

void MeasurementTool::set_measurement( size_t index, const Core::Measurement& measurement )
{
  // Ensure that state is changed on application thread
  Core::ActionSetAt::Dispatch( Core::Interface::GetMouseActionContext(),
    this->measurements_state_, index, measurement );
}

bool MeasurementTool::remove_measurement( const Core::Measurement& measurement )
{
  // Set active index to the end of the list 
  size_t num_measurements = this->get_measurements().size();
  if( num_measurements > 1 )
  {
    this->set_active_index( static_cast< int >( num_measurements ) - 1 );
  }

  // Remove measurements
  // Ensure that state is changed on application thread
  Core::ActionRemove::Dispatch( Core::Interface::GetWidgetActionContext(), 
    this->measurements_state_, measurement );

  return true;
}

int MeasurementTool::get_active_index() const
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return this->active_index_state_->get();
}

void MeasurementTool::set_active_index( int active_index )
{
  // Ensure that state is changed on application thread
  Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
    this->active_index_state_, active_index );
}

bool MeasurementTool::get_show_world_units() const
{
  // NOTE: Need to lock state engine as this function may be run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return this->show_world_units_state_->get();
}

} // end namespace Seg3D

