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
#include <Application/Tools/Actions/ActionSaveActiveNote.h>

// Core includes
#include <Core/Action/ActionFactory.h>

CORE_REGISTER_ACTION( Seg3D, SaveActiveNote )

namespace Seg3D
{

class ActionSaveActiveNotePrivate
{
public:
  // Required action parameters
  Core::ActionParameter< std::string > measurements_stateid_;
  Core::ActionParameter< std::string > active_index_stateid_;
  Core::ActionParameter< std::string > note_;
  
  // This is an internal optimization to avoid the lookup in the state database
  Core::StateBaseWeakHandle active_index_state_weak_handle_;
  Core::StateVectorBaseWeakHandle measurements_state_weak_handle_;
};

ActionSaveActiveNote::ActionSaveActiveNote() :
  private_( new ActionSaveActiveNotePrivate )
{
  this->add_argument( this->private_->measurements_stateid_ );
  this->add_argument( this->private_->active_index_stateid_ );
  this->add_argument( this->private_->note_ );
}

ActionSaveActiveNote::~ActionSaveActiveNote()
{
}

bool ActionSaveActiveNote::validate( Core::ActionContextHandle& context )
{
  // TODO: Implement
  // Make sure the measurements list exists and that the active index exists and is valid

  // Check for a valid measurement list
  // Check to see if the handle is still valid
  Core::StateBaseHandle vector_state( this->private_->measurements_state_weak_handle_.lock() );
  if ( !vector_state )
  {
    // Query the state engine
    if ( !Core::StateEngine::Instance()->get_state( this->private_->measurements_stateid_.value(), vector_state ) )
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
        this->private_->measurements_stateid_.value() + "' doesn't support ActionSaveActiveNote" );
      return false;
    }
    this->private_->measurements_state_weak_handle_ = measurment_state;
  }

  // Check for a valid active index state
  // Check for a valid measurement list
  Core::StateBaseHandle state( this->private_->active_index_state_weak_handle_.lock() );
  if ( !state )
  {
    if ( !Core::StateEngine::Instance()->get_state( this->private_->active_index_stateid_.value(), 
      state ) )
    {
      context->report_error( std::string( "Unknown state variable '" ) + 
        this->private_->active_index_stateid_.value() + "'" );
      return false;
    }
    Core::StateIntHandle int_state = 
      boost::dynamic_pointer_cast< Core::StateInt >( state );
    if ( !int_state )
    {
      context->report_error( std::string( "State variable '") + 
        this->private_->active_index_stateid_.value() + "' doesn't support ActionSaveActiveNote" );
      return false;
    }
    this->private_->active_index_state_weak_handle_ = int_state;
  }

  return true;
}

bool ActionSaveActiveNote::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  Core::StateVectorBaseHandle vector_state( 
    this->private_->measurements_state_weak_handle_.lock() );
  Core::StateMeasurementVectorHandle measurements_state = 
    boost::dynamic_pointer_cast< Core::StateMeasurementVector >( vector_state );
  
  Core::StateBaseHandle state( this->private_->active_index_state_weak_handle_.lock() );
  Core::StateIntHandle active_index_state = 
    boost::dynamic_pointer_cast< Core::StateInt >( state );

  if ( measurements_state && active_index_state )
  {
    int active_index = active_index_state->get();
    std::vector< Core::Measurement > measurements = measurements_state->get();
    if( active_index != -1 && active_index < static_cast< int >( measurements.size() ) )
    {
      Core::Measurement measurement = measurements[ active_index ];
      measurement.set_note( this->private_->note_.value() );
      measurements_state->set_at( active_index, measurement, context->source() );
      return true;
    }
  }

  return false;
} 

Core::ActionHandle ActionSaveActiveNote::Create( 
  const Core::StateMeasurementVectorHandle& measurements_state, 
  const Core::StateIntHandle& active_index_state, const std::string& note )
{
  ActionSaveActiveNote* action = new ActionSaveActiveNote;
  action->private_->measurements_stateid_.set_value( measurements_state->get_stateid() );
  action->private_->measurements_state_weak_handle_ = measurements_state;
  action->private_->active_index_stateid_.set_value( active_index_state->get_stateid() );
  action->private_->active_index_state_weak_handle_ = active_index_state;
  action->private_->note_.value() = note;

  return Core::ActionHandle( action );
}

void ActionSaveActiveNote::Dispatch( Core::ActionContextHandle context, 
  const Core::StateMeasurementVectorHandle& measurements_state, 
  const Core::StateIntHandle& active_index_state, const std::string& note )
{
  Core::ActionDispatcher::PostAction( Create( measurements_state, active_index_state, note ), 
    context );
}

} // end namespace Seg3D

