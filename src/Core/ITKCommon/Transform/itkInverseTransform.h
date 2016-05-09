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


// File         : itkInverseTransform.h
// Author       : Pavel A. Koshevoy
// Created      : 2005/06/03 10:16
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A inverse transform class.

#ifndef __itkInverseTransform_h
#define __itkInverseTransform_h

// ITK includes:
#include <itkTransform.h>
#include <itkMacro.h>


//----------------------------------------------------------------
// itk::InverseTransform
// 
namespace itk
{

//----------------------------------------------------------------
// InverseTransform
//
template <class ForwardTransform>
class InverseTransform :
public Transform< typename ForwardTransform::ScalarType,
ForwardTransform::OutputSpaceDimension,
ForwardTransform::InputSpaceDimension >
{
public:
  /** Standard class typedefs. */
  typedef InverseTransform Self;
  
  typedef Transform< typename ForwardTransform::ScalarType,
  ForwardTransform::OutputSpaceDimension,
  ForwardTransform::InputSpaceDimension > Superclass;
  
  typedef SmartPointer< Self >  Pointer;
  typedef SmartPointer< const Self >  ConstPointer;
  
  /** Base inverse transform type. This type should not be changed to the
   * concrete inverse transform type or inheritance would be lost.*/
  typedef typename Superclass::InverseTransformBaseType InverseTransformBaseType;
  typedef typename InverseTransformBaseType::Pointer    InverseTransformBasePointer;
  
  /** New method for creating an object using a factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro( InverseTransform, Transform );
  
  /** Standard scalar type for this class. */
  typedef typename Superclass::ScalarType ScalarType;
  
  /** Type of the input parameters. */
  typedef typename Superclass::ParametersType ParametersType;
  
  /** Type of the Jacobian matrix. */
  typedef typename Superclass::JacobianType JacobianType;
  
  /** Standard coordinate point type for this class. */
  typedef typename Superclass::InputPointType InputPointType;
  typedef typename Superclass::OutputPointType OutputPointType;
  
  /** The number of parameters defininig this transform. */
  typedef typename Superclass::NumberOfParametersType NumberOfParametersType;
  
  
  /** Dimension of the domain space. */
  itkStaticConstMacro(InputSpaceDimension,
                      unsigned int,
                      ForwardTransform::OutputSpaceDimension);
  itkStaticConstMacro(OutputSpaceDimension,
                      unsigned int,
                      ForwardTransform::InputSpaceDimension);

  /** Set the forward transform pointer. */
  void SetForwardTransform(const ForwardTransform * forward)
  { forward_ = forward; }
  
  /**  Method to transform a point. */
  virtual OutputPointType TransformPoint(const InputPointType & y) const
  {
    // TODO: replace with exception
//    assert(forward_ != NULL);
    if (forward_ == NULL)
    {
      itkExceptionMacro(<< "Forward transform is null");
    }
    return forward_->BackTransformPoint(y);
  }

  virtual void ComputeJacobianWithRespectToParameters( const InputPointType &, JacobianType & ) const
  {
    itkExceptionMacro(<< "ComputeJacobianWithRespectToParameters is not implemented for InverseTransform");
  }

  virtual void SetFixedParameters(const ParametersType &)
  {
    itkExceptionMacro(<< "SetFixedParameters is not implemented for InverseTransform");
  }
  
  virtual void SetParameters(const ParametersType &)
  {
    itkExceptionMacro(<< "SetParameters is not implemented for InverseTransform");
  }

  virtual NumberOfParametersType GetNumberOfParameters() const 
  { return 0; }

  virtual InverseTransformBasePointer GetInverseTransform() const
  { return const_cast<ForwardTransform *>(forward_); }

protected:
  InverseTransform(): Superclass(0) {}
  
private:
  // disable default copy constructor and assignment operator:
  InverseTransform(const Self & other);
  const Self & operator = (const Self & t);
  
  // the transform whose inverse we are trying to evaluate:
  const ForwardTransform * forward_;
};

} // namespace itk

#endif // __itkInverseTransform_h
