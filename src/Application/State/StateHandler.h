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

#ifndef APPLICATION_STATE_STATEHANDLER_H
#define APPLICATION_STATE_STATEHANDLER_H

#include <vector>
#include <string>

// Boost includes
#include <boost/utility.hpp>

// State includes
#include <Application/State/State.h>
#include <Application/State/StateValue.h>
#include <Application/State/StateClampedValue.h>
#include <Application/State/StateOption.h>

namespace Seg3D {

class StateHandler : public boost::noncopyable {

// -- constructor/destructor --
  public:
    StateHandler(const std::string& stateid_prefix);    
    virtual ~StateHandler();

  private:
    std::string stateid_prefix_;

  public:
  
    // ADD_STATE:
    // Add a local state variable with default value
    
    template<class HANDLE, class T>
    bool add_state(const std::string& key, HANDLE& state, 
                   const T& default_value)
    { 
      // Step (1): Generate the state variable
      state = StateHandle(new typename HANDLE::element_type(default_value));

      // Step (2): Now handle the common part for each add_state function
      return (add_statebase(key,state));
    }  

    // ADD_STATE:
    // Add a local state variable with default value and min and max
    
    template<class HANDLE, class T>
    bool add_state(const std::string& key, HANDLE& state, 
                  const T& min, const T& max, 
                  const T& step, T& default_value)
    { 
      // Step (1): Generate the state variable
      state = StateHandle(new typename HANDLE::element_type(min,max,step,default_value));

      // Step (2): Now handle the common part for each add_state function
      return (add_statebase(key,state));
    }  

    // ADD_STATE:
    // Add a local state variable without default value

    template<class HANDLE>
    bool add_state(const std::string& key, HANDLE& state)
    { 
      // Step (1): Generate the state variable
      state = StateHandle(new typename HANDLE::element_type);

      // Step (2): Now handle the common part for each add_state function
      return (add_statebase(key,state));
    }  

    // ADD_STATE:
    // Add a local state variable with option list

    template<class HANDLE>
    bool add_state(const std::string& key, HANDLE& state, 
                   const std::vector<std::string>& option_list,
                   const std::string& default_option)
    { 
      // Step (1): Generate the state variable
      state = StateHandle(new typename HANDLE::element_type(option_list,default_option));

      // Step (2): Now handle the common part for each add_state function
      return (add_statebase(key,state));
    }  

  private:
    // Function that adds the state variable to the database
    bool add_statebase(const std::string& key, StateBaseHandle& state) const;

};

} // end namespace

#endif
