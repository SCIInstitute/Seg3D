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
#include <Application/Action/ActionVariantParameter.h>

namespace Seg3D {

class StateBase : public boost::noncopyable {

// -- destructor --
  public:
    virtual ~StateBase() = 0;
    
// -- functions for accessing data --

    // EXPORT_TO_STRING:
    // Convert the contents of the State into a string
    virtual std::string export_to_string() const = 0;

    // IMPORT_FROM_STRING:
    // Set the State from a string
    virtual bool import_from_string(const std::string& str) = 0;
    
// -- stateid handling --
  
  public:
    // GET_STATEID:
    // Get the unique id assigned to the state variable
    std::string get_stateid() const { return (stateid_); }
  
    // SET_STATEID:
    // Set the unique id to be used to locate this state variable
    void set_stateid(const std::string& stateid) { stateid_ = stateid; }
    
  private:
    std::string stateid_;
    
// -- import from ActionVariantParameter --
  protected:
    friend class ActionSet;
    friend class ActionGet;
    
    // VALIDATE_VARIANT:
    // Validate that the data contained in the variat parameter can actually
    // be used.
    virtual bool validate_and_compare_variant(ActionVariantParameter& variant, 
                                  bool& changed) const = 0;
    
    // IMPORT_FROM_VARIANT:
    // Import the state data from a variant parameter.
    virtual bool import_from_variant(ActionVariantParameter& variant) = 0;
    
    // EXPORT_TO_VARIANT
    // Export the state data to a variant parameter
    virtual void export_to_variant(ActionVariantParameter& variant) = 0;    
};

} // end namespace Seg3D

#endif

