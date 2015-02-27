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

// File         : itkRadialDistortionTransform.h
// Author       : Pavel A. Koshevoy
// Created      : 2005/06/03 10:16
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A radial distortion transform.

#ifndef __itkRadialDistortionTransform_h
#define __itkRadialDistortionTransform_h

// system includes:
#include <iostream>

// ITK includes:
#include <itkTransform.h>
#include <itkExceptionObject.h>
#include <itkMacro.h>

// local includes:
#include <Core/ITKCommon/Transform/itkInverseTransform.h>


//----------------------------------------------------------------
// itk::RadialDistortionTransform
// 
// Let
//    A = (a + ta * Rmax - ac)
//    B = (b + tb * Rmax - bc)
// where ta, tb specify percentage of translation along the
// a and b axis respectively, and
//    ac = 0.5 * (a_min + a_max)
//    bc = 0.5 * (b_min + b_max)
// define the center of distortion (specified by a_min, a_max,
// b_min, b_max -- the bounding box of some image).
// 
// The transform is defined as
//    x(a, b) = ac + A * S
//    y(a, b) = bc + B * S
// 
// where
//    S = sum(n in [0, N - 1], k[n] * pow(R2 / Rmax2, n));
//    R2 = A^2 + B^2
//    Rmax2 = Rmax * Rmax
// 
namespace itk
{

template <class TScalar = double, unsigned int N = 2>
class RadialDistortionTransform :
public Transform<TScalar, 2, 2>
{
public:
  // standard typedefs:
  typedef RadialDistortionTransform Self;
  typedef Transform<TScalar, 2, 2> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;
  
  
  /** Base inverse transform type. This type should not be changed to the
   * concrete inverse transform type or inheritance would be lost.*/
  typedef typename Superclass::InverseTransformBaseType InverseTransformBaseType;
  typedef typename InverseTransformBaseType::Pointer    InverseTransformBasePointer;
  
  // static constant for the number of polynomial coefficients:
  itkStaticConstMacro(Nk, unsigned int, N);
  
  // RTTI:
  itkTypeMacro(RadialDistortionTransform, Transform);
  
  // macro for instantiation through the object factory:
  itkNewMacro(Self);
  
  /** Standard scalar type for this class. */
  typedef typename Superclass::ScalarType ScalarType;
  
  /** The number of parameters defininig this transform. */
  typedef typename Superclass::NumberOfParametersType NumberOfParametersType;
  
  // shortcuts:
  typedef typename Superclass::ParametersType ParametersType;
  typedef typename Superclass::JacobianType JacobianType;
  
  typedef typename Superclass::InputPointType  InputPointType;
  typedef typename Superclass::OutputPointType OutputPointType;
  
  // virtual:
  virtual
  OutputPointType TransformPoint(const InputPointType & p) const;
  
  // virtual: Inverse transformations:
  // If y = Transform(x), then x = BackTransform(y);
  // if no mapping from y to x exists, then an exception is thrown.
  InputPointType BackTransformPoint(const OutputPointType & y) const;
  
  // virtual:
  virtual
  void SetFixedParameters(const ParametersType & params)
  { this->m_FixedParameters = params; }
  
  // virtual:
  virtual
  const ParametersType & GetFixedParameters() const
  { return this->m_FixedParameters; }
  
  // virtual:
  virtual
  void SetParameters(const ParametersType & params)
  { this->m_Parameters = params; }
  
  // virtual:
  virtual
  const ParametersType & GetParameters() const
  { return this->m_Parameters; }
  
  // virtual: mumber of parameters that define this transform:
  virtual
  NumberOfParametersType GetNumberOfParameters() const
  { return N + 2; }
  
  // virtual:
//  virtual
//  const JacobianType & GetJacobian(const InputPointType & point) const;
  virtual void ComputeJacobianWithRespectToParameters( const InputPointType &, JacobianType & ) const;
  
  // virtual: return an inverse of this transform.
  virtual InverseTransformBasePointer GetInverseTransform() const
  {
    typedef InverseTransform<Self> InvTransformType;
    typename InvTransformType::Pointer inv = InvTransformType::New();
    inv->SetForwardTransform(this);
    return inv.GetPointer();
  }
  
  // setup the fixed transform parameters:
  void setup(// image bounding box expressed in the physical space:
             const double a_min,
             const double a_max,
             const double b_min,
             const double b_max,
             
             // normalization parameter (image radius in physical space):
             const double Rmax = 0.0)
  {
    double & ac_ = this->m_FixedParameters[0];
    double & bc_ = this->m_FixedParameters[1];
    double & rmax_ = this->m_FixedParameters[2];
    
    // center of distortion:
    ac_ = 0.5 * (a_min + a_max);
    bc_ = 0.5 * (b_min + b_max);
    
    // setup the normalization parameter:
    if (Rmax != 0.0)
    {
      rmax_ = Rmax;
    }
    else
    {
      const double w = a_max - a_min;
      const double h = b_max - b_min;
      rmax_ = sqrt(w * w + h * h) / 2.0;
    }
  }
  
  // setup the translation parameters:
  void setup_translation(// translation is expressed in the physical space:
                         const double ta_Rmax = 0.0,
                         const double tb_Rmax = 0.0)
  {
    const double & Rmax = this->m_FixedParameters[2];
//    assert(Rmax != 0.0);
    if (Rmax == 0.0)
    {
      itkExceptionMacro(<< "Rmax parameter is null");
    }
    
    // store ta,tb as translation relative to Rmax:
    double & ta = this->m_Parameters[N];
    double & tb = this->m_Parameters[N + 1];
    ta = ta_Rmax / Rmax;
    tb = tb_Rmax / Rmax;
  }
  
  // helper required by BackTransform:
  // evaluate F = T(x), J = dT/dx (another Jacobian):
  void eval(const std::vector<ScalarType> & x,
            std::vector<ScalarType> & F,
            std::vector<std::vector<ScalarType> > & J) const;
  
  // accessors to the fixed normalization parameter:
  inline const double & GetRmax() const
  { return this->m_FixedParameters[2]; }
  
  // generate a mask of shared parameters:
  static void setup_shared_params_mask(bool shared, std::vector<bool> & mask)
  {
    mask.assign(N + 2, false);
    for (unsigned int i = 0; i < N; i++)
    {
      mask[i] = shared;
    }
  }
  
protected:
  RadialDistortionTransform();      
  
  // virtual:
  virtual
  void PrintSelf(std::ostream & s, Indent indent) const;
  
private:
  // disable default copy constructor and assignment operator:
  RadialDistortionTransform(const Self & other);
  const Self & operator = (const Self & t);
  
}; // class RadialDistortionTransform

} // namespace itk


#ifndef ITK_MANUAL_INSTANTIATION
#include <Core/ITKCommon/Transform/itkRadialDistortionTransform.txx>
#endif

#endif // __itkRadialDistortionTransform_h
