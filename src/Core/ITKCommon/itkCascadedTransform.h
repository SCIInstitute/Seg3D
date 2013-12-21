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

// ITK includes:
#include <itkTransform.h>
#include <itkMacro.h>


//----------------------------------------------------------------
// itk::CascadedTransform
// 
namespace itk
{
  template <class TScalar, unsigned int Dimension>
  class CascadedTransform :
    public Transform<TScalar, Dimension, Dimension>
  {
  public:
    // standard typedefs:
    typedef CascadedTransform Self;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;
    
    typedef Transform<TScalar, Dimension, Dimension> Superclass;
    
    // Base inverse transform type:
//    typedef typename Superclass::InverseTransformType InverseTransformType;
    typedef Superclass InverseTransformType;
    typedef SmartPointer< InverseTransformType > InverseTransformPointer;
    
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
    typedef typename Superclass::ParametersType ParametersType;
    typedef typename Superclass::JacobianType JacobianType;
    
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
    
    // virtual:
    PointType TransformPoint(const PointType & x) const
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
    VectorType TransformVector(const VectorType & x) const
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
    VnlVectorType TransformVector(const VnlVectorType & x) const
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
    CovariantVectorType
    TransformCovariantVector(const CovariantVectorType & x) const
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
    
    // virtual:
    InverseTransformPointer GetInverse() const
    {
      const unsigned int cascade_length = transform_.size();
      
      Pointer inv = Self::New();
      inv->transform_.resize(cascade_length);
      
      for (unsigned int i = 0; i < cascade_length; i++)
      {
  unsigned int idx = (cascade_length - 1) - i;
  inv->transform_[i] = transform_[idx]->GetInverse();
      }
      
      inv->active_start_  = active_start_;
      inv->active_finish_ = active_finish_;
      inv->active_params_ = (cascade_length - 1) - active_params_;
      
      InverseTransformPointer tmp = inv.GetPointer();
      return tmp;
    }
    
    // virtual:
    void SetParameters(const ParametersType & params)
    { transform_[active_params_]->SetParameters(params); }
    
    // virtual:
    const ParametersType & GetParameters() const
    { return transform_[active_params_]->GetParameters(); }
    
    // virtual:
    unsigned int GetNumberOfParameters() const
    { return transform_[active_params_]->GetNumberOfParameters(); }
    
    // virtual:
    const JacobianType & GetJacobian(const PointType & pt) const
    { return transform_[active_params_]->GetJacobian(pt); }
    
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
      assert(index < transform_.size());
      active_params_ = index;
      assert(active_params_ <= active_finish_);
    }
    
    inline unsigned int GetActiveIndex() const
    { return active_finish_; }
    
    inline void SetActiveIndex(const unsigned int & index)
    {
      assert(index < transform_.size());
      active_finish_ = index;
      active_params_ = index;
    }
    
  protected:
    CascadedTransform():
      Superclass(Dimension, 0),
      transform_(0),
      active_params_(~0),
      active_start_(0),
      active_finish_(~0)
    {}
    
    // virtual:
    void PrintSelf(std::ostream & os, Indent indent) const
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
