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
#include <Application/Tools/Actions/ActionSetMeasurementNote.h>

// Core includes
#include <Core/Action/ActionFactory.h>

CORE_REGISTER_ACTION( Seg3D, SetMeasurementNote )

namespace Seg3D
{

class ActionSetMeasurementNotePrivate
{
public:
  // Required action parameters
  std::string measurements_stateid_;
  std::string measurement_id_;
  std::string note_;
  
  // This is an internal optimization to avoid the lookup in the state database
  Core::StateVectorBaseWeakHandle measurements_state_weak_handle_;
};

ActionSetMeasurementNote::ActionSetMeasurementNote() :
  private_( new ActionSetMeasurementNotePrivate )
{
  this->add_parameter( this->private_->measurements_stateid_ );
  this->add_parameter( this->private_->measurement_id_ );
  this->add_parameter( this->private_->note_ );
}

ActionSetMeasurementNote::~ActionSetMeasurementNote()
{
}

bool ActionSetMeasurementNote::validate( Core::ActionContextHandle& context )
{
  // Check for a valid measurement list
  // Check to see if the handle is still valid
  Core::StateBaseHandle vector_state( this->private_->measurements_state_weak_handle_.lock() );
  if ( !vector_state )
  {
    // Query the state engine
    if ( !Core::StateEngine::Instance()->get_state( 
      this->private_->measurements_stateid_, vector_state ) )
    {
      context->report_error( std::string( "Unknown state variable '" ) + 
        this->private_->measurements_stateid_ + "'" );
      return false;
    }
    // Check the type of the state
    Core::StateMeasurementVectorHandle measurment_state = 
      boost::dynamic_pointer_cast< Core::StateMeasurementVector >( vector_state );
    if ( !measurment_state )
    {
      context->report_error( std::string( "State variable '") + 
        this->private_->measurements_stateid_ + "' doesn't support ActionSetMeasurementNote" );
      return false;
    }
    this->private_->measurements_state_weak_handle_ = measurment_state;
  }

  return true;
}

bool ActionSetMeasurementNote::run( Core::ActionContextHandle& context, 
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
      if( measurements[ i ].get_id() == this->private_->measurement_id_ )
      {
        // Set note
        Core::Measurement m = measurements[ i ];
        m.set_note( this->private_->note_ );
        measurements_state->set_at( i, m, context->source() );
        return true;
      }
    }
  }

  return false;
} 


Core::ActionHandle ActionSetMeasurementNote::Create( 
  const Core::StateMeasurementVectorHandle& measurements_state, 
  const std::string& measurement_id, const std::string& note )
{
  ActionSetMeasurementNote* action = new ActionSetMeasurementNote;
  action->private_->measurements_stateid_ = measurements_state->get_stateid();
  action->private_->measurements_state_weak_handle_ = measurements_state;
  action->private_->measurement_id_ = measurement_id;
  action->private_->note_ = note;

  return Core::ActionHandle( action );
}

void ActionSetMeasurementNote::Dispatch( Core::ActionContextHandle context, 
  const Core::StateMeasurementVectorHandle& measurements_state, 
  const std::string& measurement_id, const std::string& note )
{
  Core::ActionDispatcher::PostAction( Create( measurements_state, measurement_id, note ), 
    context );
}

} // end namespace Seg3D

