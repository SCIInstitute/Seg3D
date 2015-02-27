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

// File         : itkLegendrePolynomialTransform.txx
// Author       : Pavel A. Koshevoy
// Created      : 2006/02/22 15:55
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A bivariate centered/normalized Legendre polynomial
//                transform and helper functions.

#ifndef _itkLegendrePolynomialTransform_txx
#define _itkLegendrePolynomialTransform_txx

// local includes:
#include <Core/ITKCommon/Transform/itkLegendrePolynomialTransform.h>
#include <Core/ITKCommon/Transform/itkNumericInverse.h>

// system includes:
#include <vector>

// ITK includes:
#include <itkExceptionObject.h>


namespace itk
{
  namespace Legendre
  {
    // Legendre polynomials:
    static double P0(const double & /* x */)
    { return 1.0; }
    
    static double P1(const double & x)
    { return x; }
    
    static double P2(const double & x)
    {
      const double xx = x * x;
      return (3.0 * xx - 1.0) / 2.0;
    }
    
    static double P3(const double & x)
    {
      const double xx = x * x;
      return ((5.0 * xx - 3.0) * x) / 2.0;
    }
    
    static double P4(const double & x)
    {
      const double xx = x * x;
      return ((35.0 * xx - 30.0) * xx + 3.0) / 8.0;
    }
    
    static double P5(const double & x)
    {
      const double xx = x * x;
      return (((63.0 * xx - 70.0) * xx + 15.0) * x) / 8.0;
    }
    
    static double P6(const double & x)
    {
      const double xx = x * x;
      return (((231.0 * xx - 315.0) * xx + 105.0) * xx - 5.0) / 16.0;
    }
    
    //----------------------------------------------------------------
    // polynomial_t
    // 
    typedef double(*polynomial_t)(const double & x);
    
    //----------------------------------------------------------------
    // A partial table of the Legendre polynomials
    // 
    static polynomial_t P[] = { P0, P1, P2, P3, P4, P5, P6 };
    
    // first derivatives of the Legendre polynomials:
    static double dP0(const double & /* x */)
    { return 0.0; }
    
    static double dP1(const double & /* x */)
    { return 1.0; }
    
    static double dP2(const double & x)
    { return 3.0 * x; }
    
    static double dP3(const double & x)
    {
      const double xx = x * x;
      return (15.0 * xx - 3.0) / 2.0;
    }
    
    static double dP4(const double & x)
    {
      const double xx = x * x;
      return ((35.0 * xx - 15.0) * x) / 2.0;
    }
    
    static double dP5(const double & x)
    {
      const double xx = x * x;
      return ((315.0 * xx - 210.0) * xx + 15.0) / 8.0;
    }
    
    static double dP6(const double & x)
    {
      const double xx = x * x;
      return (((693.0 * xx - 630.0) * xx + 105.0) * x) / 8.0;
    }
    
    //----------------------------------------------------------------
    // A partial table of the derivatives of Legendre polynomials
    // 
    static polynomial_t dP[] = { dP0, dP1, dP2, dP3, dP4, dP5, dP6 };
  }
  
  //----------------------------------------------------------------
  // LegendrePolynomialTransform
  // 
  template <class TScalarType, unsigned int N>
  LegendrePolynomialTransform<TScalarType, N>::
  LegendrePolynomialTransform():
    Superclass(ParameterVectorLength)
  {
    // by default, the parameters are initialized for an identity transform:
    // initialize the parameters for an identity transform:
    for (unsigned int i = 0; i < ParameterVectorLength; i++)
    {
      this->m_Parameters[i] = 0.0;
    }
    
    this->m_Parameters[index_a(1, 0)] = 1.0;
    this->m_Parameters[index_b(0, 1)] = 1.0;
    
    // allocate some space for the fixed parameters:
    this->m_FixedParameters.SetSize(4);
    setup(-1, 1, -1, 1);
    
    this->m_Parameters[index_a(0, 0)] = GetUc() / GetXmax();
    this->m_Parameters[index_b(0, 0)] = GetVc() / GetYmax();
    
//    // zero-out the Jacobian:
//    for (unsigned int i = 0; i < ParameterVectorLength; i++)
//    {
//      this->m_Jacobian(0, i) = 0.0;
//      this->m_Jacobian(1, i) = 0.0;
//    }
  }
  
  //----------------------------------------------------------------
  // TransformPoint
  // 
  template <class TScalarType, unsigned int N>
  typename LegendrePolynomialTransform<TScalarType, N>::OutputPointType
  LegendrePolynomialTransform<TScalarType, N>::
  TransformPoint(const InputPointType & x) const
  {
    const double & uc = this->GetUc();
    const double & vc = this->GetVc();
    const double & Xmax = this->GetXmax();
    const double & Ymax = this->GetYmax();
    
    const ScalarType & u = x[0];
    const ScalarType & v = x[1];
    
    const double A = (u - uc) / Xmax;
    const double B = (v - vc) / Ymax;
    
    double P[N + 1];
    double Q[N + 1];
    for (unsigned int i = 0; i <= N; i++)
    {
      P[i] = Legendre::P[i](A);
      Q[i] = Legendre::P[i](B);
    }
    
    double Sa = 0.0;
    double Sb = 0.0;
    
    for (unsigned int i = 0; i <= N; i++)
    {
      for (unsigned int j = 0; j <= i; j++)
      {
        unsigned int k = i - j;
        double PjQk = P[j] * Q[k];
        Sa += a(j, k) * PjQk;
        Sb += b(j, k) * PjQk;
      }
    }
    
    OutputPointType y;
    y[0] = Xmax * Sa;
    y[1] = Ymax * Sb;
    
    return y;
  }
  
  //----------------------------------------------------------------
  // BackTransformPoint
  // 
  template <class TScalarType, unsigned int N>
  typename LegendrePolynomialTransform<TScalarType, N>::InputPointType
  LegendrePolynomialTransform<TScalarType, N>::
  BackTransformPoint(const OutputPointType & y) const
  {
    NumericInverse<LegendrePolynomialTransform<ScalarType, N> > inverse(*this);
    
    std::vector<ScalarType> vy(2);
    std::vector<ScalarType> vx(2);
    vy[0] = y[0];
    vy[1] = y[1];
    
    // initialize x: first guess - x is close to y:
    vx = vy;
    const double & uc = this->GetUc();
    const double & vc = this->GetVc();
    
    vx[0] += uc;
    vx[1] += vc;
    bool ok = inverse.transform(vy, vx, true);
    if (!ok)
    {
      itk::ExceptionObject e(__FILE__, __LINE__);
      e.SetDescription("could not perform a numeric inverse transformation "
		       "for a Legendre polynomial transform");
      throw e;
    }
    
    OutputPointType x;
    x[0] = vx[0];
    x[1] = vx[1];
    
    return x;
  }
  
//  //----------------------------------------------------------------
//  // GetJacobian
//  // 
//  template <class TScalarType, unsigned int N>
//  const typename LegendrePolynomialTransform<TScalarType, N>::JacobianType & 
//  LegendrePolynomialTransform<TScalarType, N>::
//  GetJacobian(const InputPointType & x) const
//  {
//    const double & uc = this->GetUc();
//    const double & vc = this->GetVc();
//    const double & Xmax = this->GetXmax();
//    const double & Ymax = this->GetYmax();
//    
//    const ScalarType & u = x[0];
//    const ScalarType & v = x[1];
//    
//    const double A = (u - uc) / Xmax;
//    const double B = (v - vc) / Ymax;
//    
//    double P[N + 1];
//    double Q[N + 1];
//    for (unsigned int i = 0; i <= N; i++)
//    {
//      P[i] = Legendre::P[i](A);
//      Q[i] = Legendre::P[i](B);
//    }
//    
//    // derivatives with respect to a_jk, b_jk:
//    for (unsigned int i = 0; i <= N; i++)
//    {
//      for (unsigned int j = 0; j <= i; j++)
//      {
//	unsigned int k = i - j;
//	
//	double PjQk = P[j] * Q[k];
//	this->m_Jacobian(0, index_a(j, k)) = Xmax * PjQk;
//	this->m_Jacobian(1, index_b(j, k)) = Ymax * PjQk;
//      }
//    }
//    
//    // done:
//    return this->m_Jacobian;
//  }

  template <class TScalarType, unsigned int N>
  void 
  LegendrePolynomialTransform<TScalarType, N>::
  ComputeJacobianWithRespectToParameters( const InputPointType &x, JacobianType &jacobian ) const
  {
    const double & uc = this->GetUc();
    const double & vc = this->GetVc();
    const double & Xmax = this->GetXmax();
    const double & Ymax = this->GetYmax();
    
    const ScalarType & u = x[0];
    const ScalarType & v = x[1];
    
    const double A = (u - uc) / Xmax;
    const double B = (v - vc) / Ymax;
    
    double P[N + 1];
    double Q[N + 1];
    for (unsigned int i = 0; i <= N; i++)
    {
      P[i] = Legendre::P[i](A);
      Q[i] = Legendre::P[i](B);
    }
    
    // derivatives with respect to a_jk, b_jk:
    for (unsigned int i = 0; i <= N; i++)
    {
      for (unsigned int j = 0; j <= i; j++)
      {
        unsigned int k = i - j;
        
        double PjQk = P[j] * Q[k];
        jacobian(0, index_a(j, k)) = Xmax * PjQk;
        jacobian(1, index_b(j, k)) = Ymax * PjQk;
      }
    }    
  }
  
  //----------------------------------------------------------------
  // eval
  // 
  template <class TScalarType, unsigned int N>
  void
  LegendrePolynomialTransform<TScalarType, N>::
  eval(const std::vector<ScalarType> & x,
       std::vector<ScalarType> & F,
       std::vector<std::vector<ScalarType> > & J) const
  {
    const double & uc = this->GetUc();
    const double & vc = this->GetVc();
    const double & Xmax = this->GetXmax();
    const double & Ymax = this->GetYmax();

    const ScalarType & u = x[0];
    const ScalarType & v = x[1];
    
    const double A = (u - uc) / Xmax;
    const double B = (v - vc) / Ymax;
    
    double P[N + 1];
    double Q[N + 1];
    for (unsigned int i = 0; i <= N; i++)
    {
      P[i] = Legendre::P[i](A);
      Q[i] = Legendre::P[i](B);
    }
    
    double Sa = 0.0;
    double Sb = 0.0;
    
    // derivatives with respect to a_jk, b_jk:
    for (unsigned int i = 0; i <= N; i++)
    {
      for (unsigned int j = 0; j <= i; j++)
      {
        unsigned int k = i - j;
        double PjQk = P[j] * Q[k];
        Sa += a(j, k) * PjQk;
        Sb += b(j, k) * PjQk;
      }
    }
    
    F[0] = Xmax * Sa;
    F[1] = Ymax * Sb;
    
    // derivatives with respect to u:
    double dSa_du = 0.0;
    double dSb_du = 0.0;
    
    for (unsigned int i = 1; i <= N; i++)
    {
      for (unsigned int j = 0; j <= i; j++)
      {
        unsigned int k = i - j;
        double dPjQk = Legendre::dP[j](A) * Q[k];
        dSa_du += a(j, k) * dPjQk;
        dSb_du += b(j, k) * dPjQk;
      }
    }
    
    // derivatives with respect to v:
    double dSa_dv = 0.0;
    double dSb_dv = 0.0;
    
    for (unsigned int i = 1; i <= N; i++)
    {
      for (unsigned int j = 0; j <= i; j++)
      {
        unsigned int k = i - j;
        double dQkPj = Legendre::dP[k](B) * P[j];
        dSa_dv += a(j, k) * dQkPj;
        dSb_dv += b(j, k) * dQkPj;
      }
    }
    
    // dx/du:
    J[0][0] = dSa_du;
    
    // dx/dv:
    J[0][1] = Xmax / Ymax * dSa_dv;
    
    // dy/du:
    J[1][0] = Ymax / Xmax * dSb_du;
    
    // dy/dv:
    J[1][1] = dSb_dv;
  }
  
  //----------------------------------------------------------------
  // setup_linear_system
  // 
  template <class TScalarType, unsigned int N>
  void
  LegendrePolynomialTransform<TScalarType, N>::
  setup_linear_system(const unsigned int start_with_degree,
		      const unsigned int degrees_covered,
		      const std::vector<InputPointType> & uv,
		      const std::vector<OutputPointType> & xy,
		      vnl_matrix<double> & M,
		      vnl_vector<double> & bx,
		      vnl_vector<double> & by) const
  {
    if (degrees_covered == 0) return;
    
    const double & uc = this->GetUc();
    const double & vc = this->GetVc();
    const double & Xmax = this->GetXmax();
    const double & Ymax = this->GetYmax();
    
    static double P[N + 1];
    static double Q[N + 1];
    
    const unsigned int & num_points = uv.size();
//    assert(num_points == xy.size());
    if (num_points != xy.size())
    {
      itkExceptionMacro(<< "xy size does not match number of parameters");
    }
    
    const unsigned int offset =
      index_a(0, start_with_degree);
    
    const unsigned int extent =
      index_a(0, start_with_degree + degrees_covered) - offset;
    
    M.set_size(num_points, extent);
    bx.set_size(num_points);
    by.set_size(num_points);
    
    for (unsigned int row = 0; row < num_points; row++)
    {
      const ScalarType & u = uv[row][0];
      const ScalarType & v = uv[row][1];
      const ScalarType & x = xy[row][0];
      const ScalarType & y = xy[row][1];
      
      const double A = (u - uc) / Xmax;
      const double B = (v - vc) / Ymax;
      
      bx[row] = x / Xmax;
      by[row] = y / Ymax;
      
      for (unsigned int i = 0; i < start_with_degree + degrees_covered; i++)
      {
        P[i] = Legendre::P[i](A);
        Q[i] = Legendre::P[i](B);
      }
      
      for (unsigned int i = 0; i < start_with_degree; i++)
      {
        for (unsigned int j = 0; j <= i; j++)
        {
          const unsigned int k = i - j;
          double PjQk = P[j] * Q[k];
          bx[row] -= a(j, k) * PjQk;
          by[row] -= b(j, k) * PjQk;
        }
      }
      
      for (unsigned int i = start_with_degree;
       i < start_with_degree + degrees_covered;
       i++)
      {
        for (unsigned int j = 0; j <= i; j++)
        {
          const unsigned int k = i - j;
          const unsigned int col = index_a(j, k) - offset;
          
          M[row][col] = P[j] * Q[k];
        }
      }
    }
  }
  
  //----------------------------------------------------------------
  // solve_for_parameters
  // 
  template <class TScalarType, unsigned int N>
  void
  LegendrePolynomialTransform<TScalarType, N>::
  solve_for_parameters(const unsigned int start_with_degree,
		       const unsigned int degrees_covered,
		       const std::vector<InputPointType> & uv,
		       const std::vector<OutputPointType> & xy,
		       ParametersType & params) const
  {
    if (degrees_covered == 0) return;

    vnl_matrix<double> M;
    vnl_vector<double> bx;
    vnl_vector<double> by;

    setup_linear_system(start_with_degree, degrees_covered, uv, xy, M, bx, by);

    // use SVD to find the inverse of M:
    vnl_svd<double> svd(M);
    vnl_vector<double> xa = svd.solve(bx);
    vnl_vector<double> xb = svd.solve(by);

    unsigned int offset = index_a(0, start_with_degree);
    for (unsigned int i = start_with_degree;
      i < start_with_degree + degrees_covered;
      i++)
    {
      for (unsigned int j = 0; j <= i; j++)
      {
        const unsigned int k = i - j;
        const unsigned int a_jk = index_a(j, k);
        const unsigned int b_jk = index_b(j, k);

        params[a_jk] = xa[a_jk - offset];
        params[b_jk] = xb[a_jk - offset];
      }
    }
  }
  
  //----------------------------------------------------------------
  // PrintSelf
  // 
  template <class TScalarType, unsigned int N>
  void
  LegendrePolynomialTransform<TScalarType, N>::
  PrintSelf(std::ostream & os, Indent indent) const
  {
    Superclass::PrintSelf(os,indent);
    
    for (unsigned int i = 0; i <= N; i++)
    {
      for (unsigned int j = 0; j <= i; j++)
      {
        unsigned int k = i - j;
        os << indent << "a(" << j << ", " << k << ") = " << a(j, k)
           << std::endl;
      }
    }
    for (unsigned int i = 0; i <= N; i++)
    {
      for (unsigned int j = 0; j <= i; j++)
      {
        unsigned int k = i - j;
        os << indent << "b(" << j << ", " << k << ") = " << b(j, k)
           << std::endl;
      }
    }
    os << indent << "uc = " << GetUc() << std::endl
       << indent << "vc = " << GetVc() << std::endl
       << indent << "Xmax = " << GetXmax() << std::endl
       << indent << "Ymax = " << GetYmax() << std::endl;
//#if 0
//    for (unsigned int i = 0; i < ParameterVectorLength; i++)
//    {
//      os << indent << "params[" << i << "] = " << this->m_Parameters[i] << ';'
//	 << std::endl;
//    }
//#endif
  }
  
} // namespace itk


#endif // _itkLegendrePolynomialTransform_txx
