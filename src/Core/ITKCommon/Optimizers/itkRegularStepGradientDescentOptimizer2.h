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

// File         : itkRegularStepGradientDescentOptimizer2.h
// Author       : Pavel A. Koshevoy, Tolga Tasdizen
// Created      : 2005/11/11 14:54
// Copyright    : (C) 2004-2008 University of Utah
// Description  : An enhanced version of the
//                itk::RegularStepGradientDescentOptimizer
//                fixing a bug with relaxation and adding support for
//                step size increase (pick up pace), back tracking and
//                keeping track of the best metric value
//                and associated parameters.

#ifndef __itkRegularStepGradientDescentOptimizer2_h
#define __itkRegularStepGradientDescentOptimizer2_h

// ITK includes:
#include <itkSingleValuedNonLinearOptimizer.h>

namespace itk
{

//----------------------------------------------------------------
// RegularStepGradientDescentOptimizer2
// 
class ITK_EXPORT RegularStepGradientDescentOptimizer2 : 
  public SingleValuedNonLinearOptimizer
{
public:
  /** Standard "Self" typedef. */
  typedef RegularStepGradientDescentOptimizer2      Self;
  typedef SingleValuedNonLinearOptimizer               Superclass;
  typedef SmartPointer<Self>                           Pointer;
  typedef SmartPointer<const Self>                     ConstPointer;
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(RegularStepGradientDescentOptimizer2, 
         SingleValuedNonLinearOptimizer);
  
  //----------------------------------------------------------------
  // StopConditionType
  // 
  // Codes of stopping conditions:
  // 
  typedef enum {
    GradientMagnitudeTolerance = 1,
    StepTooSmall = 2,
    ImageNotAvailable = 3,
    SamplesNotAvailable = 4,
    MaximumNumberOfIterations = 5
  } StopConditionType;

  /** Specify whether to minimize or maximize the cost function. */
  itkSetMacro(Maximize, bool);
  itkGetConstReferenceMacro(Maximize, bool);
  itkBooleanMacro(Maximize);
  
  inline bool GetMinimize() const { return !m_Maximize; }
  inline void SetMinimize(bool v) { this->SetMaximize(!v); }
  inline void MinimizeOn()    { SetMaximize(false); }
  inline void MinimizeOff()   { SetMaximize(true); }
  
  // virtual:
  void StartOptimization();
  
  // virtual:
  void ResumeOptimization();
  
  // virtual:
  void StopOptimization();
  
  // Set/Get parameters to control the optimization process:
  itkSetMacro(MaximumStepLength, double);
  itkSetMacro(MinimumStepLength, double);
  itkGetConstReferenceMacro(MaximumStepLength, double);
  itkGetConstReferenceMacro(MinimumStepLength, double);
  
  itkSetMacro(RelaxationFactor, double);
  itkGetConstReferenceMacro(RelaxationFactor, double);
  
  itkSetMacro(NumberOfIterations, unsigned long);
  itkGetConstReferenceMacro(NumberOfIterations, unsigned long);
  
  itkSetMacro(GradientMagnitudeTolerance, double);
  itkGetConstReferenceMacro(GradientMagnitudeTolerance, double);
  
  itkSetMacro(BackTracking, bool);
  itkGetConstReferenceMacro(BackTracking, bool);
  
  itkSetMacro(PickUpPaceSteps, unsigned int);
  itkGetConstReferenceMacro(PickUpPaceSteps, unsigned int);
  
  itkGetConstReferenceMacro(CurrentStepLength, double);
  itkGetConstMacro(CurrentIteration, unsigned int);
  itkGetConstReferenceMacro(StopCondition, StopConditionType);
  itkGetConstReferenceMacro(Value, MeasureType);
  itkGetConstReferenceMacro(Gradient, DerivativeType);
  itkGetConstReferenceMacro(BestParams, ParametersType);
  itkGetConstReferenceMacro(BestValue, MeasureType);
  
protected:
  RegularStepGradientDescentOptimizer2();
  virtual ~RegularStepGradientDescentOptimizer2() {}
  
  // advance one step following the gradient direction:
  virtual void AdvanceOneStep();
  
  // advance one step along the given direction vector
  // scaled by the step length scale; this method is
  // called by AdvanceOneStep:
  virtual void StepAlongGradient(double step_length_scale,
         const DerivativeType & step_direction);
  
  // virtual:
  void PrintSelf(std::ostream& os, Indent indent) const;
  
  // data members:
  DerivativeType  m_Gradient; 
  DerivativeType  m_PreviousGradient; 
  
  bool      m_Stop;
  bool      m_Maximize;
  MeasureType   m_Value;
  MeasureType   m_PreviousValue;
  double    m_GradientMagnitudeTolerance;
  double    m_MaximumStepLength;
  double    m_MinimumStepLength;
  double    m_CurrentStepLength;
  double    m_RelaxationFactor;
  StopConditionType m_StopCondition;
  unsigned long   m_NumberOfIterations;
  unsigned long   m_CurrentIteration;
  
  // this keeps track of the best function value and corresponding parameters:
  MeasureType   m_BestValue;
  ParametersType  m_BestParams;
  
  // this flag controls whether the algorithm will step back
  // to a previous position as well as relax every time it
  // detects a worsening of the metric;
  // default is false:
  bool      m_BackTracking;
  
  // this is the number of successful iterations that must occur
  // after which the algorithm will increase the step size;
  // default is 1000000:
  unsigned int    m_PickUpPaceSteps;
  
private:
  // disable the copy constructor and assignment operator:
  RegularStepGradientDescentOptimizer2(const Self &);
  void operator = (const Self &);
};

} // end namespace itk


#endif // __itkRegularStepGradientDescentOptimizer2_h
