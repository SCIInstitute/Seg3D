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

#ifndef CORE_STATE_STATENAME_H
#define CORE_STATE_STATENAME_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <Core/State/StateBase.h>

namespace Core
{

// Forward declarations
class StateName;
class StateNamePrivate;
typedef boost::shared_ptr< StateName > StateNameHandle;
typedef boost::shared_ptr< StateNamePrivate > StateNamePrivateHandle;

class StateName : public StateBase
{
  // -- type definitions --
public:
  typedef std::string value_type;
  typedef StateNameHandle handle_type;
  
  // -- Constructor/Destructor --
public:
  StateName( const std::string& stateid, const std::string& value );
  virtual ~StateName();

  // -- Functions for accessing data --

public:
  // EXPORT_TO_STRING:
  /// Convert the contents of the State into a string
  virtual std::string export_to_string() const;

  // IMPORT_FROM_STRING:
  /// Set the State from a string
  virtual bool import_from_string( const std::string& str, ActionSource source =
    ActionSource::NONE_E );

protected:
  // EXPORT_TO_VARIANT
  /// Export the state data to a variant parameter
  virtual void export_to_variant( Variant& variant ) const;

  // IMPORT_FROM_VARIANT:
  /// Import the state data from a variant parameter.
  virtual bool import_from_variant( Variant& variant, 
    ActionSource source = ActionSource::NONE_E );

  // VALIDATE_VARIANT:
  /// Validate a variant parameter
  /// This function returns false if the parameter is invalid or cannot be
  /// converted and in that case error will describe the error.
  virtual bool validate_variant( Variant& variant, std::string& error );

  // -- signals describing the state --
public:

  // VALUE_CHANGED_SIGNAL:
  /// Signal when the data in the state is changed.
  /// The first two parameters contain the old and new values respectively.
  /// The third parameter indicates the source of the change
  typedef boost::signals2::signal< void( std::string, std::string, ActionSource ) > 
    value_changed_signal_type;
  value_changed_signal_type value_changed_signal_;

  // -- access value --
public:
  // GET:
  /// Get the value of the state variable
  const std::string& get() const;

  // SET:
  /// Set the value of the state variable
  /// NOTE: this function by passes the action mechanism and should only be used
  /// to enforce a constraint from another action.
  bool set( const std::string& value, ActionSource source = ActionSource::NONE_E );

protected:
  // INVALIDATE:
  /// Invalidate the state variable. This function is called by the "invalidate" function of 
  /// StateHandler to release any resource occupied by the state.
  /// It will put the name string back to the pool.
  virtual void invalidate();

public:
  /// The regular expression for validating a name string
  const static std::string REGEX_VALIDATOR_C;

private:
  StateNamePrivateHandle private_;

};

} // end namespace Core

#endif
