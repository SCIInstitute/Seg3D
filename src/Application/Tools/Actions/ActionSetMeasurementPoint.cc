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
#include <Application/Tools/Actions/ActionSetMeasurementPoint.h>

// Core includes
#include <Core/Action/ActionFactory.h>

CORE_REGISTER_ACTION( Seg3D, SetMeasurementPoint )

namespace Seg3D
{

class ActionSetMeasurementPointPrivate
{
public:
  // Required action parameters
  Core::ActionParameter< std::string > measurements_stateid_;
  Core::ActionParameter< std::string > measurement_id_;
  Core::ActionParameter< int > point_index_;
  Core::ActionParameter< Core::Point > world_point_;
  
  // This is an internal optimization to avoid the lookup in the state database
  Core::StateVectorBaseWeakHandle measurements_state_weak_handle_;
};

ActionSetMeasurementPoint::ActionSetMeasurementPoint() :
  private_( new ActionSetMeasurementPointPrivate )
{
  this->add_argument( this->private_->measurements_stateid_ );
  this->add_argument( this->private_->measurement_id_ );
  this->add_argument( this->private_->point_index_ );
  this->add_argument( this->private_->world_point_ );
}

ActionSetMeasurementPoint::~ActionSetMeasurementPoint()
{
}

bool ActionSetMeasurementPoint::validate( Core::ActionContextHandle& context )
{
  // Check for a valid measurement list
  // Check to see if the handle is still valid
  Core::StateBaseHandle vector_state( this->private_->measurements_state_weak_handle_.lock() );
  if ( !vector_state )
  {
    // Query the state engine
    if ( !Core::StateEngine::Instance()->get_state( 
      this->private_->measurements_stateid_.value(), vector_state ) )
    {
      context->report_error( std::string( "Unknown state variable '" ) + 
        this->private_->measurements_stateid_.value() + "'" );
      return false;
    }
    // Check the type of the state
    Core::StateMeasurementVectorHandle measurment_state = 
      boost::dynamic_pointer_cast< Core::StateMeasurementVector >( vector_state );
    if ( !measurment_state )
    {
      context->report_error( std::string( "State variable '") + 
        this->private_->measurements_stateid_.value() + "' doesn't support ActionSetMeasurementPoint" );
      return false;
    }
    this->private_->measurements_state_weak_handle_ = measurment_state;
  }

  if( !( this->private_->point_index_.value() == 0 || this->private_->point_index_.value() == 1 ) ) 
  {
    return false;
  }
  return true;
}

bool ActionSetMeasurementPoint::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  Core::StateVectorBaseHandle vector_state( 
    this->private_->measurements_state_weak_handle_.lock() );
  Core::StateMeasurementVectorHandle measurements_state = 
    boost::dynamic_pointer_cast< Core::StateMeasurementVector >( vector_state );
  
  if ( measurements_state )
  {
    // Get measurements
    const std::vector< Core::Measurement >& measurements = measurements_state->get();

    // Find one with matching id
    for( size_t i = 0; i < measurements.size(); i++ )
    {
      if( measurements[ i ].get_id() == this->private_->measurement_id_.value() )
      {
        // Set point
        Core::Measurement m = measurements[ i ];
        m.set_point( this->private_->point_index_.value(), 
          this->private_->world_point_.value() );
        measurements_state->set_at( i, m, context->source() );
        return true;
      }
    }
  }

  return false;
} 

Core::ActionHandle ActionSetMeasurementPoint::Create( 
  const Core::StateMeasurementVectorHandle& measurements_state, 
  const std::string& measurement_id, int point_index, const Core::Point& world_point )
{
  ActionSetMeasurementPoint* action = new ActionSetMeasurementPoint;
  action->private_->measurements_stateid_.set_value( measurements_state->get_stateid() );
  action->private_->measurements_state_weak_handle_ = measurements_state;
  action->private_->measurement_id_.value() = measurement_id;
  action->private_->point_index_.value() = point_index;
  action->private_->world_point_.value() = world_point;
  
  return Core::ActionHandle( action );
}

void ActionSetMeasurementPoint::Dispatch( Core::ActionContextHandle context, 
  const Core::StateMeasurementVectorHandle& measurements_state, 
  const std::string& measurement_id, int point_index, const Core::Point& world_point )
{
  Core::ActionDispatcher::PostAction( Create( measurements_state, measurement_id, point_index, 
    world_point ), context );
}

} // end namespace Seg3D

