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

// Boost includes
#include <boost/algorithm/string.hpp>

// STL includes
#include <string>

// Application includes
#include <Application/State/StateEngine.h>

namespace Seg3D
{

StateEngine::StateEngine()
{
}

StateEngine::~StateEngine()
{
}

bool StateEngine::add_state( const std::string& state_id, StateBaseHandle& state )
{
  lock_type lock( mutex_ );

  state_map_type::iterator it = state_map_.find( state_id );

  if ( it != state_map_.end() )
  {
    // State is already there
    state_map_.erase( it );
  }

  state_map_[ state_id ] = state;
  state_list_.push_back(state);
  
  state_changed_signal_();
  
  return ( true );
}

bool StateEngine::get_state( const std::string& state_id, StateBaseHandle& state )
{
  lock_type lock( mutex_ );

  state_map_type::const_iterator it;
  if ( state_id.size() > 0 && state_id[ 0 ] == '$' )
  {
    // This name is an alias
    std::string state_alias = state_id.substr( 1 );
    // Find the name in the alias list
    if ( statealias_list_.find( state_alias ) == statealias_list_.end() )
    {
      state.reset();
      return ( false );
    }

    // fill in the proper state id
    it = state_map_.find( statealias_list_[ state_alias ] );
  }
  else
  {
    it = state_map_.find( state_id );
  }
  if ( it == state_map_.end() )
  {
    // make the handle invalid
    state.reset();
    return ( false );
  }

  state = StateBaseHandle( ( *it ).second );
  return ( true );
}

void StateEngine::remove_state( const std::string& remove_state_id )
{
  lock_type lock( mutex_ );

  // ensure that we can change it
  std::string state_id = remove_state_id;

  if ( state_id.size() > 0 && state_id[ 0 ] == '$' )
  {
    // This name is an alias
    std::string state_alias = state_id.substr( 1 );
    // Find the name in the alias list
    if ( statealias_list_.find( state_alias ) == statealias_list_.end() )
    {
      return;
    }

    // fill in the proper state id
    state_id = statealias_list_[ state_alias ];
  }

  state_list_type::iterator it = state_list_.begin();
  state_list_type::iterator it_end = state_list_.end();

  while ( it != it_end )
  {
    std::string state_name = (*it)->stateid();
    
    if ( state_name.size() >= state_id.size() )
    {
      if ( state_name.compare( 0, state_id.size(), state_id, 0, state_id.size() ) == 0 )
      {
        if ( state_name.size() > state_id.size() )
        {
          if ( state_name[ state_id.size() ] == ':' )
          {
            state_map_.erase( state_map_.find( state_name ) );
            it = state_list_.erase(it);
            it_end = state_list_.end();
            continue;
          }
        }
        else
        {
          state_map_.erase( state_map_.find( state_name ) );
          it = state_list_.erase(it);
          it_end = state_list_.end();
          continue;
        }
      }
    }
    ++it;
  }
  
  state_changed_signal_();  
}

size_t StateEngine::num_states()
{
  lock_type lock( get_mutex() );
  return state_list_.size();
}

bool StateEngine::get_state( const size_t idx, StateBaseHandle& state)
{
  lock_type lock( get_mutex() );
  if (idx < state_list_.size())
  {
    state = state_list_[idx];
    return true;
  }
  else
  {
    state.reset();
    return false;
  }
}

void StateEngine::add_stateid( const std::string& stateid )
{
  lock_type lock( get_mutex() );
  if ( stateid_list_.find( stateid ) != stateid_list_.end() )
  {
    SCI_THROW_LOGICERROR( std::string("Trying to add stateid '") + stateid +
      std::string("' that already exists") );
  }
  stateid_list_.insert( stateid );
}

void StateEngine::remove_stateid( const std::string& stateid )
{
  lock_type lock( mutex_ );
  stateid_list_.erase( stateid );
}

bool StateEngine::is_stateid( const std::string& stateid )
{
  lock_type lock( get_mutex() );
  return ( stateid_list_.find( stateid ) != stateid_list_.end() );
}

std::string StateEngine::create_stateid( std::string baseid )
{
  // Check
  std::string::size_type loc = baseid.find_last_of( '_' );
  if ( loc != std::string::npos )
  {
    // there is an under score in the name
    // check whether the last part is a name
    bool is_number = true;
    if ( loc == baseid.size()-1 ) is_number = false;
  
    for ( std::string::size_type j = loc + 1; j < baseid.size(); j++ )
    {
      if ( baseid[j] < '0' || baseid[j] > '9' ) is_number = false;
    }
    
    if ( is_number ) baseid = baseid.substr( 0, loc );
  }

  lock_type lock( get_mutex() );
  
  int number = 1;
  std::string new_stateid;
  
  do
  {
    new_stateid = baseid + std::string( "_" ) + Utils::to_string( number );
    number++;
  }
  while ( stateid_list_.find( new_stateid ) != stateid_list_.end() );

  return new_stateid;
}


void StateEngine::add_statealias( const std::string& statealias, const std::string& stateid )
{
  lock_type lock( mutex_ );
  if ( statealias_list_.find( statealias ) != statealias_list_.end() )
  {
    SCI_THROW_LOGICERROR( std::string("Trying to add statealias '") +
      statealias + std::string("' that already exists") );
  }
  statealias_list_[ statealias ] = stateid;
}

void StateEngine::remove_statealias( const std::string& statealias )
{
  lock_type lock( mutex_ );
  statealias_list_.erase( statealias );
}

bool StateEngine::is_statealias( const std::string& statealias )
{
  lock_type lock( mutex_ );
  return ( statealias_list_.find( statealias ) != statealias_list_.end() );
}

std::string StateEngine::create_statealias( std::string basealias )
{
  // Check
  std::string::size_type loc = basealias.find_last_of( '_' );
  if ( loc != std::string::npos )
  {
    // there is an under score in the name
    // check whether the last part is a name
    bool is_number = true;
    if ( loc == basealias.size()-1 ) is_number = false;
  
    for ( std::string::size_type j = loc + 1; j < basealias.size(); j++ )
    {
      if ( basealias[j] < '0' || basealias[j] > '9' ) is_number = false;
    }
    
    if ( is_number ) basealias = basealias.substr( 0, loc );
  }

  lock_type lock( get_mutex() );
  
  int number = 1;
  std::string new_statealias;
  
  do
  {
    new_statealias = basealias + std::string( "_" ) + Utils::to_string( number );
    number++;
  }
  while ( statealias_list_.find( new_statealias ) != statealias_list_.end() );

  return new_statealias;
}

} // end namespace Seg3D
