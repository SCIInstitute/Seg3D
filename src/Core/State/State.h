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

#ifndef CORE_STATE_STATE_H
#define CORE_STATE_STATE_H 1

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>
#include <map>
#include <typeinfo>

#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>


// CLASS STATE:
// A simple system to organize the state of an object into a set 
// of key/value pairs. This class is templatized in such way that
// new types can easily be added


namespace Core {

// CLASS STATEITEM:
// Base class for state variable contained in the state container

class StateItem {
  public:
    // Virtual destructor needed for destroying state item from base.
    virtual ~StateItem(); 
};


// CLASSS STATEITEMT:
// Template containing the value of the state

template<class T>
class StateItemT : public StateItem {

  public:
    typedef T value_type;
    
    // Constructor needs a default value, as state always needs to be
    // initialized
    StateItemT(T default_value) :
      value_(default_value) {}
    // virtual destructor
    virtual ~StateItemT() {}

    // Accessor
    T& value() { return value_; }

  private:
    // Storage location of state value
    T value_;
};

// CLASS STATE:

// This class contains a list with state items that can be serialized into
// a file and a list of signals that will be notified when the state of the
// object changes

class State {

// -- typedefs --
  private:
      typedef std::map<std::string,StateItem*> state_map_type;

// -- constructor/destructor --
  public:
    State();
    virtual ~State();
  
  public:

    // add_state : add a state item to the list that is maintained by this
    // class. Each new state needs a default value.

    template<class T>
    void add_state(const std::string& key, const T& default_value)
    {
      state_map_type::iterator it = state_.find(key); 
      if (it != state_.end()) delete state_[key];
      state_[key] = new StateItemT<T>(default_value);
    }

    template<class T>
    T& state(const std::string& key)
    {
      StateItemT<T>* item = dynamic_cast<StateItemT<T>*>(state_[key]);
      if (item)
      {
        return item->value();
      }
      else
      {
        std::string error = std::string("Unknown state variable ")+
                            key+" of type "+ typeid(T).name();
        SCI_THROW_INVALIDARGUMENT(error);
      }
    }

    // Get the current state of the object
    template<class T>
    bool get_state(const std::string& key, T& value) const
    {
      StateItemT<T>* item = dynamic_cast<StateItemT<T>*>(state_[key]);
      if (item)
      {
        value = item->value();
        return (true);
      }
      return (false);
    }

    // Set the state of an object and signal that the state has changed
    template<class T>
    void set_state(const std::string& key, const T& value)
    {
      StateItemT<T>* item = dynamic_cast<StateItemT<T>*>(state_[key]);
      if (item)
      {
        if (value != item->value_)
        {
          item->value_ = value();
        }
      }
    }
    
  private:
  
    // Map that contains the state of the object
    state_map_type state_;
};

} // end namespace Core

#endif
