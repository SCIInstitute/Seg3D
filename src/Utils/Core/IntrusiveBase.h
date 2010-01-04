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

#ifndef UTILS_CORE_INTRUSIVEBASE_H
#define UTILS_CORE_INTRUSIVEBASE_H

// Boost includes
#include <boost/smart_ptr/detail/atomic_count.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/utility.hpp>

namespace Utils {

// Forward declaration
class IntrusiveBase;

// Class definition
class IntrusiveBase : public boost::noncopyable {

  public:
    IntrusiveBase() : count_(0) {}
    virtual ~IntrusiveBase() {}
  
  private:
    friend void intrusive_ptr_add_ref(IntrusiveBase*);
    friend void intrusive_ptr_release(IntrusiveBase*);

    boost::detail::atomic_count count_;
};

void intrusive_ptr_add_ref(IntrusiveBase* object);
void intrusive_ptr_release(IntrusiveBase* object);

}

#endif
