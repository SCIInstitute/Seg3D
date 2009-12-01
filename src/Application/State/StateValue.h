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

#ifndef APPLICATION_STATE_STATEVALUE_H
#define APPLICATION_STATE_STATEVALUE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <Application/State/State.h>

namespace Seg3D {

// STATEVALUE:
// This class is a specification of State that is use to hold a single unbound
// instance of a value.

template<class T>
class StateValue;

template<class T>
class StateValue : public State<T> {
  public:
    // One cannot define a templated typedef of StateHandle<>,
    // Hence we settle for StateValue<T>::Handle
    typedef boost::shared_ptr<StateValue<T> > Handle;

// -- constructor/destructor --
  public:

    // It is always good to initialize a state value before
    // using it.
    StateValue(const T& default_value) :
      State<T>(default_value)
    {
    }

    virtual ~StateValue()
    {
    }
};

} // end namespace Seg3D

#endif
