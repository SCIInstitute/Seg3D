/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

// File         : itkRegularStepGradientDescentOptimizer2.cxx
// Author       : Pavel A. Koshevoy, Tolga Tasdizen
// Created      : 2005/11/11 14:54
// Copyright    : (C) 2004-2008 University of Utah
// Description  : An enhanced version of the
//                itk::RegularStepGradientDescentOptimizer
//                fixing a bug with relaxation and adding support for
//                step size increase (pick_up_pace), back tracking and
//                keeping track of the best metric value
//                and associated parameters.

#ifndef _itkRegularStepGradientDescentOptimizer2_txx
#define _itkRegularStepGradientDescentOptimizer2_txx

// local includes:
#include <Core/ITKCommon/itkRegularStepGradientDescentOptimizer2.h>

// ITK includes:
#include <itkCommand.h>
#include <itkEventObject.h>
#include <vnl/vnl_math.h>

namespace itk
{

//----------------------------------------------------------------
// RegularStepGradientDescentOptimizer2::RegularStepGradientDescentOptimizer2
// 
RegularStepGradientDescentOptimizer2::RegularStepGradientDescentOptimizer2():
  m_Maximize(false),
  m_Value(0.0),
  m_PreviousValue(0.0),
  m_GradientMagnitudeTolerance(1e-4),
  m_MaximumStepLength(1.0),
  m_MinimumStepLength(1e-3),
  m_CurrentStepLength(0.0),
  m_RelaxationFactor(0.5),
  m_StopCondition(MaximumNumberOfIterations),
  m_NumberOfIterations(100),
  m_CurrentIteration(0),
  m_BestValue(0.0),
  m_BackTracking(false),
  m_PickUpPaceSteps(1000000)
{
  itkDebugMacro("Constructor");
  m_CostFunction = NULL;
}

//----------------------------------------------------------------
// RegularStepGradientDescentOptimizer2::StartOptimization
// 
// Start the optimization
// 
void
RegularStepGradientDescentOptimizer2::StartOptimization()
{
  itkDebugMacro("StartOptimization");
  const unsigned int spaceDimension = m_CostFunction->GetNumberOfParameters();
  
  if (m_RelaxationFactor < 0.0)
  {
    itkExceptionMacro
      (<< "Relaxation factor must be positive. Current value is "
       << m_RelaxationFactor);
    return;
  }
  
  if (m_RelaxationFactor >= 1.0)
  {
    itkExceptionMacro
      (<< "Relaxation factor must less than 1.0. Current value is "
       << m_RelaxationFactor);
    return;
  }
  
  // Make sure the scales have been set properly
  const unsigned int scalesSize = this->GetScales().size();
  if (scalesSize != spaceDimension)
  {
    itkExceptionMacro
      (<< "The size of Scales is "
       << scalesSize
       << ", but the NumberOfParameters for the CostFunction is "
       << spaceDimension
       << ".");
  }
  
  m_CurrentStepLength = m_MaximumStepLength;
  m_CurrentIteration = 0;
  
  m_BestValue =
    this->m_Maximize
    ? -std::numeric_limits<MeasureType>::max()
    : std::numeric_limits<MeasureType>::max();
  m_Value = m_BestValue;
  m_PreviousValue = m_BestValue;
  
  m_Gradient = DerivativeType(spaceDimension);
  m_Gradient.Fill(0.0f);
  
  m_PreviousGradient = DerivativeType(spaceDimension);
  m_PreviousGradient.Fill(0.0f);
  
  this->SetCurrentPosition(GetInitialPosition());
  this->ResumeOptimization();
}

//----------------------------------------------------------------
// RegularStepGradientDescentOptimizer2::ResumeOptimization
// 
// Resume the optimization
// 
void
RegularStepGradientDescentOptimizer2::ResumeOptimization()
{
  // keep track of the number of sequential steps that
  // resulted in function optimization:
  unsigned int successful_steps = 0;
  
  itkDebugMacro("ResumeOptimization");
  this->InvokeEvent(StartEvent());
  
  m_Stop = false;
  while (!m_Stop)
  {
    m_PreviousGradient = m_Gradient;
    m_PreviousValue = m_Value;
    
    ParametersType currentPosition = this->GetCurrentPosition();
    m_CostFunction->GetValueAndDerivative(currentPosition,
					  m_Value,
					  m_Gradient);
    
    if ((this->m_Maximize && m_Value < m_PreviousValue) ||
	(!this->m_Maximize && m_Value > m_PreviousValue))
    {
      // relax the step size:
      m_CurrentStepLength *= m_RelaxationFactor;
      
      if (m_BackTracking)
      {
//	// FIXME:
//	*log_ << m_Value << " vs " << m_BestValue
//	      << " -- relaxing and backtracking, new step length: "
//	      << m_CurrentStepLength << std::endl;
	
	// backtrack to the previous position:
	this->SetCurrentPosition(m_BestParams);
	currentPosition = this->GetCurrentPosition();
	m_CostFunction->GetValueAndDerivative(currentPosition,
					      m_Value,
					      m_Gradient);
      }
      else
      {
//	// FIXME:
//	*log_ << m_Value << " vs " << m_PreviousValue
//	      << " -- relaxing, new step length: "
//	      << m_CurrentStepLength << std::endl;
      }
      
      successful_steps = 0;
    }
    else
    {
      successful_steps++;
      if (successful_steps % m_PickUpPaceSteps == 0)
      {
	// pick up the pace after N successful steps:
	m_CurrentStepLength =
	  std::min(m_MaximumStepLength,
		   m_CurrentStepLength / m_RelaxationFactor);
	
//	// FIXME:
//	*log_ << successful_steps
//	      << " successful steps -- increasing pace, new step length: "
//	      << m_CurrentStepLength << std::endl;
      }
    }
    
    if (m_CurrentIteration == m_NumberOfIterations)
    {
      m_Stop = true;
      m_StopCondition = MaximumNumberOfIterations;
      this->StopOptimization();
    }
    
    if ((this->m_Maximize && m_Value > m_BestValue) ||
	(!this->m_Maximize && m_Value < m_BestValue))
    {
      m_BestValue = m_Value;
      m_BestParams = currentPosition;
    }
    
    if (!m_Stop)
    {
      this->AdvanceOneStep();
      m_CurrentIteration++;
    }
  }
}

//----------------------------------------------------------------
// RegularStepGradientDescentOptimizer2::StopOptimization
// 
// Stop optimization
// 
void
RegularStepGradientDescentOptimizer2::StopOptimization()
{
  itkDebugMacro("StopOptimization");
  m_Stop = true;
  this->InvokeEvent(EndEvent());
}

//----------------------------------------------------------------
// RegularStepGradientDescentOptimizer2::AdvanceOneStep
// 
// Advance one Step following the gradient direction
// 
void
RegularStepGradientDescentOptimizer2::AdvanceOneStep()
{ 
  itkDebugMacro("AdvanceOneStep");
  
  const unsigned int spaceDimension =
    m_CostFunction->GetNumberOfParameters();

  DerivativeType transformedGradient(spaceDimension);
  DerivativeType previousTransformedGradient(spaceDimension);
  const ScalesType & scales = this->GetScales();
  
  for (unsigned int i = 0; i < spaceDimension; i++)
  {
    transformedGradient[i] = m_Gradient[i] / scales[i];
    previousTransformedGradient[i] = m_PreviousGradient[i] / scales[i];
  }
  
  double magnitudeSquared = 0.0;
  for (unsigned int dim = 0; dim < spaceDimension; dim++)
  {
    const double weighted = transformedGradient[dim];
    magnitudeSquared += weighted * weighted;
  }
  
  const double gradientMagnitude = vcl_sqrt(magnitudeSquared);
  if (gradientMagnitude < m_GradientMagnitudeTolerance)
  {
    m_StopCondition = GradientMagnitudeTolerance;
    this->StopOptimization();
    return;
  }
  
  if (m_CurrentStepLength < m_MinimumStepLength)
  {
    m_StopCondition = StepTooSmall;
    this->StopOptimization();
    return;
  }
  
  const double direction = this->m_Maximize ? 1.0 : -1.0;
  const double step_scale = direction * m_CurrentStepLength;
  
  // FIXME:
  // *log_ << "gradientMagnitude: " << gradientMagnitude << std::endl;
  
  this->StepAlongGradient(step_scale, transformedGradient);
  this->InvokeEvent(IterationEvent());
}

//----------------------------------------------------------------
// RegularStepGradientDescentOptimizer2::StepAlongGradient
// 
// Advance one Step following the gradient direction
// This method will be overridden in non-vector spaces
// 
void
RegularStepGradientDescentOptimizer2::
StepAlongGradient(double factor, const DerivativeType & transformedGradient)
{ 
  itkDebugMacro(<< "factor = " << factor
		<< " transformedGradient = " << transformedGradient);
  
  const unsigned int spaceDimension =
    m_CostFunction->GetNumberOfParameters();
  
  ParametersType newPosition(spaceDimension);
  ParametersType currentPosition = this->GetCurrentPosition();

  for (unsigned int j = 0; j < spaceDimension; j++)
  {
    newPosition[j] = currentPosition[j] + transformedGradient[j] * factor;
  }
  
  itkDebugMacro(<< "new position = " << newPosition);
  this->SetCurrentPosition(newPosition);
}

//----------------------------------------------------------------
// RegularStepGradientDescentOptimizer2::PrintSelf
// 
void
RegularStepGradientDescentOptimizer2::
PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  os << indent << "MaximumStepLength: "
     << m_MaximumStepLength << std::endl;
  os << indent << "MinimumStepLength: "
     << m_MinimumStepLength << std::endl;
  os << indent << "RelaxationFactor: "
     << m_RelaxationFactor << std::endl;
  os << indent << "GradientMagnitudeTolerance: "
     << m_GradientMagnitudeTolerance << std::endl;
  os << indent << "NumberOfIterations: "
     << m_NumberOfIterations << std::endl;
  os << indent << "CurrentIteration: "
     << m_CurrentIteration   << std::endl;
  os << indent << "Value: "
     << m_Value << std::endl;
  os << indent << "Maximize: "
     << m_Maximize << std::endl;
  
  if (m_CostFunction)
  {
    os << indent << "CostFunction: "
       << &m_CostFunction << std::endl;
  }
  else
  {
    os << indent << "CostFunction: "
       << "(None)" << std::endl;
  }
  
  os << indent << "CurrentStepLength: "
     << m_CurrentStepLength << std::endl;
  os << indent << "StopCondition: "
     << m_StopCondition << std::endl;
  os << indent << "Gradient: "
     << m_Gradient << std::endl;
}

} // end namespace itk


#endif // _itkRegularStepGradientDescentOptimizer2_txx
