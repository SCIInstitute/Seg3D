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

#ifndef OPTIMIZER_OBSERVER_HXX_
#define OPTIMIZER_OBSERVER_HXX_

#include <sstream>

// ITK includes:
#include <itkCommand.h>

#include <Core/Utils/Log.h>


//----------------------------------------------------------------
// optimizer_observer_t
//
template <typename TOptimizer>
class optimizer_observer_t : public itk::Command
{
public:
  typedef optimizer_observer_t Self;
  typedef itk::Command Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  
  itkNewMacro(Self);
  
  void Execute(itk::Object *caller, const itk::EventObject & event)
  { Execute((const itk::Object *)(caller), event); }
  
  void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    if (typeid(event) != typeid(itk::IterationEvent)) return;
    
    const TOptimizer * optimizer = dynamic_cast<const TOptimizer *>(object);
    std::ostringstream oss;
    oss << static_cast<unsigned int>(optimizer->GetCurrentIteration()) << '\t'
    << optimizer->GetValue() << '\t'
    << optimizer->GetCurrentPosition() << std::endl;
    CORE_LOG_MESSAGE(oss.str());
  }
  
protected:
  optimizer_observer_t() {}
};


#endif // OPTIMIZER_OBSERVER_HXX_
