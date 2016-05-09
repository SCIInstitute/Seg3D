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

// File         : itkRBFTransform.h
// Author       : Pavel A. Koshevoy
// Created      : 2007/01/23 10:15
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A Thin Plate Spline transform.

#ifndef __itkRBFTransform_h
#define __itkRBFTransform_h

// system includes:
#include <iostream>
#include <assert.h>

// ITK includes:
#include <itkTransform.h>
#include <itkExceptionObject.h>
#include <itkMacro.h>

// local includes:
#include <Core/ITKCommon/Transform/itkInverseTransform.h>


//----------------------------------------------------------------
// itk::RBFTransform
// 
// Radial Basis Function transform:
// 
// Let
//   A = (u - uc) / Xmax
//   B = (v - vc) / Ymax
//   Ci = (u - ui) / Xmax
//   Di = (v - vi) / Ymax
// 
// where Xmax, Ymax are normalization parameters (typically the width and
// height of the image), uc, vc corresponds to the center or rotation
// of the image expressed in the coordinate system of the mosaic,
// and ui, vi are the mosaic space coordinates of the control points.
// 
// The transform is defined as
//   x(u, v) = Xmax * (a0 + a1 * A + a2 * B + F)
//   y(u, v) = Ymax * (b0 + b1 * A + b2 * B + G)
// 
// where
//   F = sum(i in [0, k-1], fi * Q(Ci, Di));
//   G = sum(i in [0, k-1], gi * Q(Ci, Di));
//   Q  = r2 * ln(r2)
//   r2 = Ci^2 + Di^2
// 
namespace itk
{

class RBFTransform : public Transform<double, 2, 2>
{
public:
  // standard typedefs:
  typedef RBFTransform Self;
  typedef Transform<double, 2, 2> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  /** Base inverse transform type. This type should not be changed to the
   * concrete inverse transform type or inheritance would be lost.*/
  typedef Superclass::InverseTransformBaseType InverseTransformBaseType;
  typedef InverseTransformBaseType::Pointer    InverseTransformBasePointer;
  
  // RTTI:
  itkTypeMacro(RBFTransform, Transform);
  
  // macro for instantiation through the object factory:
  itkNewMacro(Self);
  
  /** Standard scalar type for this class. */
  typedef double ScalarType;
  
  /** Dimension of the domain space. */
  itkStaticConstMacro(InputSpaceDimension, unsigned int, 2);
  itkStaticConstMacro(OutputSpaceDimension, unsigned int, 2);
  
  // shortcuts:
  typedef Superclass::ParametersType ParametersType;
  typedef Superclass::JacobianType JacobianType;
  
  typedef Superclass::InputPointType  InputPointType;
  typedef Superclass::OutputPointType OutputPointType;
  
  // virtual:
  virtual OutputPointType TransformPoint(const InputPointType & uv) const;
  
  // Inverse transformations:
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
  
  // virtual:
  virtual
  NumberOfParametersType GetNumberOfParameters() const
  { return this->m_Parameters.size(); }

  bool GetInverse(Self* inverse) const;
  
  // virtual: return an inverse of this transform.
  virtual InverseTransformBasePointer GetInverseTransform() const;
  
  // setup the transform parameters:
  void setup(// image bounding box expressed in the image space,
       // defines transform normalization parameters:
       const OutputPointType & tile_min,  // tile space
       const OutputPointType & tile_max,  // tile space
       
       // landmark correspondences:
       const unsigned int num_pts,    // number of pairs
       const InputPointType * uv,   // mosaic space
       const OutputPointType * xy);   // tile space
  
  // virtual:
//  virtual
//  const JacobianType & GetJacobian(const InputPointType & point) const;

  virtual void ComputeJacobianWithRespectToParameters( const InputPointType &, JacobianType & ) const;
  
#if 0
//    // helper required for numeric inverse transform calculation;
//    // evaluate F = T(x), J = dT/dx (another Jacobian):
//    void eval(const std::vector<double> & x,
//        std::vector<double> & F,
//        std::vector<std::vector<double> > & J) const;
#endif
  
  // number of landmark correspondences:
  inline unsigned int num_points() const
  { return (this->m_FixedParameters.size() - 4) / 2; }
  
  // calculate parameter vector indeces for various transform parameters:
  inline static unsigned int index_a(const unsigned int & i)
  { return i * 2; }
  
  inline static unsigned int index_b(const unsigned int & i)
  { return i * 2 + 1; }
  
  inline static unsigned int index_f(const unsigned int & i)
  { return i * 2 + 6; }
  
  inline static unsigned int index_g(const unsigned int & i)
  { return i * 2 + 7; }
  
  inline static unsigned int index_uv(const unsigned int & i)
  { return i * 2 + 4; }
  
  // accessors to the normalization parameters Xmax, Ymax:
  inline const double & GetXmax() const
  { return this->m_FixedParameters[0]; }
  
  inline const double & GetYmax() const
  { return this->m_FixedParameters[1]; }
  
  // accessors to the warp origin expressed in the mosaic coordinate system:
  inline const double & GetUc() const
  { return this->m_FixedParameters[2]; }
  
  inline const double & GetVc() const
  { return this->m_FixedParameters[3]; }
  
  // mosaic space landmark accessor:
  inline const double * uv(const unsigned int & i) const
  { return &(this->m_FixedParameters[index_uv(i)]); }
  
  // polynomial coefficient accessors:
  inline const double & a(const unsigned int & i) const
  { return this->m_Parameters[index_a(i)]; }
  
  inline const double & b(const unsigned int & i) const
  { return this->m_Parameters[index_b(i)]; }
  
  // radial basis function accessors:
  inline const double & f(const unsigned int & i) const
  { return this->m_Parameters[index_f(i)]; }
  
  inline const double & g(const unsigned int & i) const
  { return this->m_Parameters[index_g(i)]; }
  
  // the Radial Basis Function kernel:
  inline static double kernel(const double * uv,
      const double * uv_i,
      const double & Xmax,
      const double & Ymax)
  {
    double U = (uv[0] - uv_i[0]) / Xmax;
    double V = (uv[1] - uv_i[1]) / Ymax;
    double R = U * U + V * V;
    return R == 0 ? 0 : R * log(R);
  }
  
protected:
  RBFTransform();      
  ~RBFTransform();      
  
  // virtual:
  virtual
  void PrintSelf(std::ostream & s, Indent indent) const;
  
private:
  // disable default copy constructor and assignment operator:
  RBFTransform(const Self & other);
  const Self & operator = (const Self & t);
  
}; // class RBFTransform

} // namespace itk


#endif // __itkRBFTransform_h
