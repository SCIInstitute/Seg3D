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

#ifndef APPLICATION_STATE_STATEBASE_H
#define APPLICATION_STATE_STATEBASE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// STL includes
#include <string>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Action includes
#include <Application/Action/ActionParameter.h>

namespace Seg3D {

// -- STATEBASE --

class StateBase;
typedef boost::shared_ptr<StateBase> StateBaseHandle;

class StateBase : public boost::noncopyable {

// -- constructor / destructor --
  public:
    StateBase() {}
    virtual ~StateBase() {}
    
// -- functions for accessing data --

    // EXPORT_TO_STRING:
    // Convert the contents of the State into a string
    virtual std::string export_to_string() const = 0;

    // IMPORT_FROM_STRING:
    // Set the State from a string
    virtual bool import_from_string(const std::string& str,
                                    bool from_interface = false) = 0;
    
  protected:
    friend class ActionSet;
    friend class ActionGet;

    // EXPORT_TO_VARIANT
    // Export the state data to a variant parameter
    virtual void export_to_variant(ActionParameterVariant& variant) const = 0;  

    // IMPORT_FROM_VARIANT:
    // Import the state data from a variant parameter.
    virtual bool import_from_variant(ActionParameterVariant& variant, 
                                     bool from_interface = false) = 0;    

    // VALIDATE_VARIANT:
    // Validate a variant parameter
    // This function returns false if the parameter is invalid or cannot be 
    // converted and in that case error will describe the error.
    virtual bool validate_variant(ActionParameterVariant& variant, std::string& error) = 0;
    
    // COMPARE_VARIANT:
    // Compare with variant parameter
    virtual bool compare_variant(ActionParameterVariant& variant) = 0;
    
// -- stateid handling --
  public:
    // GET_STATEID:
    // Get the unique id assigned to the state variable
    std::string stateid() const { return (stateid_); }
  
    // SET_STATEID:
    // Set the unique id to be used to locate this state variable
    void set_stateid(const std::string& stateid) { stateid_ = stateid; }
    
  protected:
    std::string stateid_;
     
};


} // end namespace Seg3D

#endif

