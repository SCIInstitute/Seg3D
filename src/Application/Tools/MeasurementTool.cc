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
//#include <Application/Filters/Actions/ActionMeasure.h>
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/MeasurementTool.h>

// Core includes
#include <Core/State/Actions/ActionRemove.h>
#include <Core/State/Actions/ActionSetAt.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, MeasurementTool )

namespace Seg3D
{

class MeasurementToolPrivate {
public:
  void set_tool( MeasurementTool* tool );
  void update_active_index();

  MeasurementTool* tool_;
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
    if( active_index == MeasurementTool::INVALID_ACTIVE_INDEX_C || 
      active_index >= num_measurements )
    {
      this->tool_->set_active_index( static_cast< int >( num_measurements ) - 1 );
    }
  }
  else
  {
    this->tool_->set_active_index( MeasurementTool::INVALID_ACTIVE_INDEX_C );
  }
}

const int MeasurementTool::INVALID_ACTIVE_INDEX_C = -1;

void create_test_data( std::vector< Core::Measurement >& measurements )
{
  // Populate measurements list with test data
  measurements.push_back( 
    Core::Measurement( true, "M1", "Knee", Core::Point(0, 0, 0), 
    Core::Point(1, 1, 1), Core::AXIAL_E, 50, 1 ) );
  measurements.push_back( 
    Core::Measurement( true, "M2", "Heart", Core::Point(0, 0, 0), 
    Core::Point(2, 2, 2), Core::AXIAL_E, 50, 1 ) );
  measurements.push_back( 
    Core::Measurement( true, "M3", "Head", Core::Point(0, 0, 0), 
    Core::Point(3, 3, 3), Core::AXIAL_E, 50, 1 ) ); 
  measurements.push_back( 
    Core::Measurement( true, "M4", "Toe", Core::Point(0, 0, 0), 
    Core::Point(4, 4, 4), Core::AXIAL_E, 50, 1 ) ); 
  measurements.push_back( 
    Core::Measurement( true, "M5", "Eye", Core::Point(0, 0, 0), 
    Core::Point(5, 5, 5), Core::AXIAL_E, 50, 1 ) ); 
  measurements.push_back( 
    Core::Measurement( true, "M6", "Nose", Core::Point(0, 0, 0), 
    Core::Point(6, 6, 6), Core::AXIAL_E, 50, 1 ) ); 
  measurements.push_back( 
    Core::Measurement( true, "M7", "Hand", Core::Point(0, 0, 0), 
    Core::Point(7, 7, 7), Core::AXIAL_E, 50, 1 ) ); 
  measurements.push_back( 
    Core::Measurement( true, "M8", "Ear", Core::Point(0, 0, 0), 
    Core::Point(8, 8, 8), Core::AXIAL_E, 50, 1 ) ); 
}

MeasurementTool::MeasurementTool( const std::string& toolid ) :
  Tool( toolid ),
  private_( new MeasurementToolPrivate )
{
  this->private_->set_tool( this );

  // Test code
  std::vector< Core::Measurement > measurements;
  create_test_data( measurements );

  // State variable gets allocated here
  this->add_state( "measurements", this->measurements_state_, measurements );
  this->add_state( "active_index", this->active_index_state_, INVALID_ACTIVE_INDEX_C );

  this->add_connection( this->measurements_state_->state_changed_signal_.connect( 
    boost::bind( &MeasurementToolPrivate::update_active_index, this->private_ ) ) );
}

MeasurementTool::~MeasurementTool()
{
  this->disconnect_all();
}

void MeasurementTool::execute( Core::ActionContextHandle context )
{
  //ActionInvert::Dispatch( context );
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

} // end namespace Seg3D
