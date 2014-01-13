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

// File         : itkRadialDistortionTransform.txx
// Author       : Pavel A. Koshevoy
// Created      : 2005/06/03 10:16
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A radial distortion transform.

#ifndef _itkRadialDistortionTransform_txx
#define _itkRadialDistortionTransform_txx

// local includes:
#include <Core/ITKCommon/itkRadialDistortionTransform.h>
#include <Core/ITKCommon/itkNumericInverse.h>

// system includes:
#include <vector>
#include <assert.h>


namespace itk
{
  //----------------------------------------------------------------
  // RadialDistortionTransform
  // 
  template <class TScalar, unsigned int N>
  RadialDistortionTransform<TScalar, N>::
  RadialDistortionTransform():
    Superclass(2, N + 2) // there are k0, k1,.. kN-1, tx, ty parameters:
  {
    // by default, the parameters are initialized for an identity transform:
    this->m_Parameters[0] = 1.0;
    for (unsigned int i = 1; i < N + 2; i++)
    {
      this->m_Parameters[i] = 0.0;
    }
    
    // allocate some space for the fixed parameters (ac, bc, Rmax):
    this->m_FixedParameters.SetSize(3);
    double & ac = this->m_FixedParameters[0];
    double & bc = this->m_FixedParameters[1];
    double & Rmax = this->m_FixedParameters[2];
    ac = 0.0;
    bc = 0.0;
    Rmax = 0.0;
  }
  
  //----------------------------------------------------------------
  // TransformPoint
  // 
  template <class TScalar, unsigned int N>
  typename RadialDistortionTransform<TScalar, N>::OutputPointType
  RadialDistortionTransform<TScalar, N>::
  TransformPoint(const InputPointType & x) const
  {
    const double & ac = this->m_FixedParameters[0];
    const double & bc = this->m_FixedParameters[1];
    const double & Rmax = this->m_FixedParameters[2];
    
    const double & ta = this->m_Parameters[N];
    const double & tb = this->m_Parameters[N + 1];
    
    const ScalarType & a = x[0];
    const ScalarType & b = x[1];
    
    const double A = (a + ta * Rmax - ac);
    const double B = (b + tb * Rmax - bc);
    const double A2 = A * A;
    const double B2 = B * B;
    const double R2 = A2 + B2;
    const double Rmax2 = Rmax * Rmax;
    const double RRmax2 = R2 / Rmax2;
    
    // n = 0:
    double S = this->m_Parameters[0];
    
    // n = 1 ... N-1:
    double RRmax2n = RRmax2;
    for (unsigned int n = 1; n < N; n++)
    {
      const double knRRmax2n = this->m_Parameters[n] * RRmax2n;
      S += knRRmax2n;
      RRmax2n *= RRmax2;
    }
    
    OutputPointType y;
    y[0] = ac + A * S;
    y[1] = bc + B * S;
    
    return y;
  }
  
  //----------------------------------------------------------------
  // BackTransformPoint
  // 
  template <class TScalar, unsigned int N>
  typename RadialDistortionTransform<TScalar, N>::InputPointType
  RadialDistortionTransform<TScalar, N>::
  BackTransformPoint(const OutputPointType & y) const
  {
    NumericInverse<RadialDistortionTransform<TScalar, N> > inverse(*this);
    
    std::vector<ScalarType> vy(2);
    std::vector<ScalarType> vx(2);
    vy[0] = y[0];
    vy[1] = y[1];
    
    // initialize x: first guess - x is close to y:
    vx = vy;
    const double & Rmax = this->m_FixedParameters[2];
    const double & ta = this->m_Parameters[N];
    const double & tb = this->m_Parameters[N + 1];
    vx[0] -= ta * Rmax;
    vx[1] -= tb * Rmax;
    bool ok = inverse.transform(vy, vx, true);

    // TODO: replace with exception!!!
    if (!ok) assert(false);
    
    OutputPointType x;
    x[0] = vx[0];
    x[1] = vx[1];
    
    return x;
  }
  
  //----------------------------------------------------------------
  // GetJacobian
  // 
  template <class TScalar, unsigned int N>
  const typename RadialDistortionTransform<TScalar, N>::JacobianType &
  RadialDistortionTransform<TScalar, N>::
  GetJacobian(const InputPointType & x) const
  {
    const double & ac = this->m_FixedParameters[0];
    const double & bc = this->m_FixedParameters[1];
    const double & Rmax = this->m_FixedParameters[2];
    
    const double & ta = this->m_Parameters[N];
    const double & tb = this->m_Parameters[N + 1];
    
    const ScalarType & a = x[0];
    const ScalarType & b = x[1];
    
    const double A = (a + ta * Rmax - ac);
    const double B = (b + tb * Rmax - bc);
    const double A2 = A * A;
    const double B2 = B * B;
    const double R2 = A2 + B2;
    const double Rmax2 = Rmax * Rmax;
    const double RRmax2 = R2 / Rmax2;
    
    // derivatives with respect to kn:
    double RRmax2n = double(1);
    for (unsigned int n = 0; n < N; n++)
    {
      this->m_Jacobian(0, n) = A * RRmax2n;
      this->m_Jacobian(1, n) = B * RRmax2n;
      RRmax2n *= RRmax2;
    }
    
    // derivatives with respect to ta, tb:
    
    // n = 0:
    double P = double(0);
    double Q = double(0);
    
    // n = 1 ... N-1:
    double RRmax2n1 = double(1); // (R^2 / Rmax^2)^(n - 1)
    for (unsigned int n = 1; n < N; n++)
    {
      const double & k = this->m_Parameters[n];
      const double scale_n = k * RRmax2n1; // kn * (R^2/Rmax^2)^n
      P += scale_n;
      Q += scale_n * double(n);
      RRmax2n1 *= RRmax2;
    }
    
    double S = this->m_Parameters[0] + RRmax2 * P;
    
    this->m_Jacobian(0, N) =
      Rmax * S + (double(2) * A2 / Rmax) * Q;// dx/dta
    this->m_Jacobian(1, N + 1) =
      Rmax * S + (double(2) * B2 / Rmax) * Q;// dy/dtb
    
    this->m_Jacobian(0, N + 1) =
      ((double(2) * A * B) / Rmax) * Q;       // dx/dtb
    this->m_Jacobian(1, N) =
      this->m_Jacobian(0, N + 1);                // dy/dta
    
    return this->m_Jacobian;
  }
  
  //----------------------------------------------------------------
  // eval
  // 
  template <class TScalar, unsigned int N>
  void
  RadialDistortionTransform<TScalar, N>::
  eval(const std::vector<ScalarType> & x,
       std::vector<ScalarType> & F,
       std::vector<std::vector<ScalarType> > & J) const
  {
    const double & ac = this->m_FixedParameters[0];
    const double & bc = this->m_FixedParameters[1];
    const double & Rmax = this->m_FixedParameters[2];
    
    const double & ta = this->m_Parameters[N];
    const double & tb = this->m_Parameters[N + 1];
    
    const ScalarType & a = x[0];
    const ScalarType & b = x[1];
    
    const double A = (a + ta * Rmax - ac);
    const double B = (b + tb * Rmax - bc);
    const double A2 = A * A;
    const double B2 = B * B;
    const double R2 = A2 + B2;
    const double Rmax2 = Rmax * Rmax;
    const double RRmax2 = R2 / Rmax2;
    
    // n = 0:
    double P = double(0);
    double Q = double(0);
    
    // n = 1 ... N-1:
    double RRmax2n1 = double(1); // (R^2 / Rmax^2)^(n - 1)
    for (unsigned int n = 1; n < N; n++)
    {
      const double & k = this->m_Parameters[n];
      const double scale_n = k * RRmax2n1; // kn * (R^2/Rmax^2)^n
      P += scale_n;
      Q += scale_n * double(n);
      RRmax2n1 *= RRmax2;
    }
    
    double S = this->m_Parameters[0] + RRmax2 * P;
    F[0] = ac + A * S;
    F[1] = bc + B * S;
    
    // calc dF/dx:
    J[0][0] = Rmax * S + (double(2) * A2 / Rmax) * Q;
    J[1][1] = Rmax * S + (double(2) * B2 / Rmax) * Q;
    
    J[0][1] = ((double(2) * A * B) / Rmax) * Q;
    J[1][0] = J[0][1];
  }
  
  //----------------------------------------------------------------
  // PrintSelf
  // 
  template <class TScalar, unsigned int N>
  void
  RadialDistortionTransform<TScalar, N>::
  PrintSelf(std::ostream & os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
    
    for (unsigned int i = 0; i < N; i++)
    {
      os << indent << "k[" << i << "] = " << this->m_Parameters[i]
	 << std::endl;
    }
    
    os << indent << "ta = " << this->m_Parameters[N] << std::endl
       << indent << "tb = " << this->m_Parameters[N + 1] << std::endl
       << indent << "ac = " << this->m_FixedParameters[0] << std::endl
       << indent << "bc = " << this->m_FixedParameters[1] << std::endl
       << indent << "Rmax = " << this->m_FixedParameters[2] << std::endl;
  }
  
} // namespace itk


#endif // _itkRadialDistortionTransform_txx
