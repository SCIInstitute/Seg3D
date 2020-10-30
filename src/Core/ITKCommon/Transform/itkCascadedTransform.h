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

// File         : itkCascadedTransform.h
// Author       : Pavel A. Koshevoy
// Created      : 2005/06/03 10:16
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A composite transform used to chain several conventional
//                ITK transforms together.

#ifndef __itkCascadedTransform_h
#define __itkCascadedTransform_h

// system includes:
#include <iostream>
#include <cassert>
#include <vector>
#include <sstream>

// ITK includes:
#include <itkTransform.h>
#include <itkMacro.h>

#include <Core/Utils/Log.h>


//----------------------------------------------------------------
// itk::CascadedTransform
//
namespace itk
{

template <class TScalar, unsigned int Dimension>
class CascadedTransform :
public ITK_EXPORT Transform<TScalar, Dimension, Dimension>
{
public:
  // standard typedefs:
  typedef CascadedTransform Self;
  typedef Transform<TScalar, Dimension, Dimension> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;


  /** Base inverse transform type. This type should not be changed to the
   * concrete inverse transform type or inheritance would be lost.*/
  typedef typename Superclass::InverseTransformBaseType InverseTransformBaseType;
  typedef typename InverseTransformBaseType::Pointer    InverseTransformBasePointer;

  typedef SmartPointer<Superclass> TransformPointer;
  typedef SmartPointer<const Superclass> ConstTransformPointer;

  // RTTI:
  itkTypeMacro(CascadedTransform, Transform);

  // macro for instantiation through the object factory:
  itkNewMacro(Self);

  /** Standard scalar type for this class. */
  typedef typename Superclass::ScalarType ScalarType;

  /** Dimension of the domain space. */
  itkStaticConstMacro(InputSpaceDimension, unsigned int, Dimension);
  itkStaticConstMacro(OutputSpaceDimension, unsigned int, Dimension);

  // shortcuts:
  typedef typename Superclass::FixedParametersType      FixedParametersType;
  typedef typename Superclass::FixedParametersValueType FixedParametersValueType;
  typedef typename Superclass::ParametersType           ParametersType;
  typedef typename Superclass::JacobianType             JacobianType;

  typedef typename Superclass::InputPointType PointType;
  typedef PointType InputPointType;
  typedef PointType OutputPointType;

  typedef typename Superclass::InputVectorType VectorType;
  typedef VectorType InputVectorType;
  typedef VectorType OutputVectorType;

  typedef typename Superclass::InputVnlVectorType VnlVectorType;
  typedef VnlVectorType InputVnlVectorType;
  typedef VnlVectorType OutputVnlVectorType;

  typedef typename Superclass::InputCovariantVectorType CovariantVectorType;
  typedef CovariantVectorType InputCovariantVectorType;
  typedef CovariantVectorType OutputCovariantVectorType;

  /** The number of parameters defininig this transform. */
  typedef typename Superclass::NumberOfParametersType NumberOfParametersType;

  // virtual:
  virtual PointType TransformPoint(const PointType & x) const override
  {
    PointType y = x;

    // apply the transforms:
    for (unsigned int i = active_start_; i <= active_finish_; i++)
    {
      PointType tx = transform_[i]->TransformPoint(y);
      if (tx[0] == std::numeric_limits<double>::max())
      {
        return tx;
      }

      y = tx;
    }

    return y;
  }

  // virtual:
  virtual VectorType TransformVector(const VectorType & x) const override
  {
    VectorType y(x);

    // apply the transforms:
    for (unsigned int i = active_start_; i <= active_finish_; i++)
    {
      y = transform_[i]->TransformVector(y);
    }

    return y;
  }

  // virtual:
  virtual VnlVectorType TransformVector(const VnlVectorType & x) const override
  {
    VnlVectorType y(x);

    // apply the transforms:
    for (unsigned int i = active_start_; i <= active_finish_; i++)
    {
      y = transform_[i]->TransformVector(y);
    }

    return y;
  }

  // virtual:
  virtual CovariantVectorType
  TransformCovariantVector(const CovariantVectorType & x) const override
  {
    CovariantVectorType y(x);

    // apply the transforms:
    for (unsigned int i = active_start_; i < active_finish_; i++)
    {
      y = transform_[i]->TransformCovariantVector(y);
    }

    return y;
  }

  // Inverse transformations:
  // If y = Transform(x), then x = BackTransform(y);
  // if no mapping from y to x exists, then an exception is thrown
  bool GetInverse(Self* inverse) const
  {
    if (! inverse) return false;

    const unsigned int cascade_length = transform_.size();

//    Pointer inv = Self::New();
    inverse->transform_.resize(cascade_length);

    for (unsigned int i = 0; i < cascade_length; i++)
    {
      unsigned int idx = (cascade_length - 1) - i;
//      inverse->transform_[i] = transform_[idx]->GetInverseTransform();
      TransformPointer inverseTransform =
        dynamic_cast<Superclass*>( transform_[idx]->GetInverseTransform().GetPointer() );
      if (inverseTransform)
      {
        inverse->transform_[i] = inverseTransform;
      }
      else
      {
        std::ostringstream oss;
        oss << "Could not convert inverse tranform obtained from transform "
            << i << " from itkTransformBase to itkTransform. Skipping transform.";
        CORE_LOG_DEBUG(oss.str());
      }
    }

    inverse->active_start_  = active_start_;
    inverse->active_finish_ = active_finish_;
    inverse->active_params_ = (cascade_length - 1) - active_params_;

//    InverseTransformPointer tmp = inverse.GetPointer();
//    return tmp;
    return true;
  }

  virtual InverseTransformBasePointer GetInverseTransform() const override
  {
    Pointer inv = Self::New();
    return this->GetInverse(inv) ? inv.GetPointer() : 0;
  }

  // virtual:
  virtual void SetParameters(const ParametersType & params) override
  { transform_[active_params_]->SetParameters(params); }

  virtual void SetFixedParameters(const FixedParametersType &) override
  {
    itkExceptionMacro(<< "SetFixedParameters is not implemented for CascadedTransform");
  }

  // virtual:
  virtual const ParametersType & GetParameters() const override
  { return transform_[active_params_]->GetParameters(); }

  // virtual:
  virtual NumberOfParametersType GetNumberOfParameters() const override
  { return transform_[active_params_]->GetNumberOfParameters(); }

  // virtual:
  virtual void
  ComputeJacobianWithRespectToPosition(const PointType & pt,
                                       JacobianType & jacobian) const override
  {
    transform_[active_params_]->ComputeJacobianWithRespectToPosition(pt, jacobian);
  }

  virtual void ComputeJacobianWithRespectToParameters(const InputPointType &, JacobianType &) const override
  {
    itkExceptionMacro(<< "mputeJacobianWithRespectToParameters is not implemented for CascadedTransform");
  }

  // add a transform to the stack:
  void append(TransformPointer transform)
  {
    const unsigned int old_sz = transform_.size();
    std::vector<TransformPointer> new_array(old_sz + 1);

    for (unsigned int i = 0; i < old_sz; i++)
    {
      new_array[i] = transform_[i];
    }

    new_array[old_sz] = transform;
    transform_ = new_array;
    active_params_ = transform_.size() - 1;
    active_finish_ = transform_.size() - 1;
  }

  // setup a cascaded transform:
  template <typename container_t>
  void setup(const container_t & transforms,
             const unsigned int & cascade_length)
  {
    transform_.resize(cascade_length);

    for (unsigned int i = 0; i < cascade_length; i++)
    {
      transform_[i] = transforms[i];
    }

    active_start_  = 0;
    active_finish_ = cascade_length - 1;
    active_params_ = active_finish_;
  }

  // accessor to the individual transforms on the stack:
  template <class transform_t> transform_t *
  GetTransform(const unsigned int & index) const
  { return dynamic_cast<transform_t *>(transform_[index].GetPointer()); }

  // accessor:
  inline const std::vector<TransformPointer> & GetTransforms() const
  { return transform_; }

  // accessors:
  inline unsigned int GetParamsIndex() const
  { return active_params_; }

  inline void SetParamsIndex(const unsigned int & index)
  {
//    assert(index < transform_.size());
    if (index >= transform_.size())
    {
      itkExceptionMacro(<< "parameter index out of bounds");
    }
    active_params_ = index;
//    assert(active_params_ <= active_finish_);
    if (active_params_ > active_finish_)
    {
      itkExceptionMacro(<< "active parameter index out of bounds");
    }
  }

  inline unsigned int GetActiveIndex() const
  { return active_finish_; }

  inline void SetActiveIndex(const unsigned int & index)
  {
//    assert(index < transform_.size());
    if (index >= transform_.size())
    {
      itkExceptionMacro(<< "parameter index out of bounds");
    }
    active_finish_ = index;
    active_params_ = index;
  }

protected:
  CascadedTransform():
  Superclass(/*Dimension,*/0),
  transform_(0),
  active_params_(~0),
  active_start_(0),
  active_finish_(~0)
  {}

  // virtual:
  virtual void PrintSelf(std::ostream & os, Indent indent) const override
  {
    Superclass::PrintSelf(os, indent);

    const unsigned int sz = transform_.size();
    for (unsigned int i = 0; i < sz; i++)
    {
      os << indent << "transform_[" << i << "] = "
      << transform_[i] << std::endl;
    }

    os << indent << "active_params_ = " << active_params_ << std::endl
      << indent << "active_start_  = " << active_start_ << std::endl
      << indent << "active_finish_ = " << active_finish_ << std::endl;
  }

private:
  // disable default copy constructor and assignment operator:
  CascadedTransform(const Self & other);
  const Self & operator = (const Self & t);

  // Cascaded transforms - input is first transformed by the first
  // transform, followed by the other transforms sequentially.
  // Only the top-most (last) transform in the array is accessible
  // for modification, all preciding transforms are fixed.
  std::vector<TransformPointer> transform_;

  // This index controls which transform on the stack can be manipulated
  // via GetParameters/SetParameters function. By default this is
  // the top transform on the stack, but it may be adjusted to suit
  // anyones needs:
  unsigned int active_params_;

  // Transforms which are outside of the [start, finish] range are
  // completely ignored. This feature is useful whenever it is necessary
  // to temporarily remove some of the transforms from the cascade:
  unsigned int active_start_;
  unsigned int active_finish_;

}; // class CascadedTransform

} // namespace itk

#endif // __itkCascadedTransform_h
