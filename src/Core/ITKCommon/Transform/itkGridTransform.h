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

#ifndef __itkGridTransform_h
#define __itkGridTransform_h

// system includes:
#include <math.h>
#include <iostream>

// ITK includes:
#include <itkTransform.h>
#include <itkExceptionObject.h>
#include <itkIdentityTransform.h>
#include <itkMacro.h>
#include <itkImage.h>

// local includes:
#include <Core/ITKCommon/Transform/itkDiscontinuousTransform.h>


//----------------------------------------------------------------
// itk::GridTransform
//
namespace itk
{

class GridTransform : public Transform<double, 2, 2>
{
public:
  // standard typedefs:
  typedef GridTransform Self;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  typedef Transform<double, 2, 2> Superclass;

  /** Base inverse transform type. This type should not be changed to the
   * concrete inverse transform type or inheritance would be lost.*/
  typedef Superclass::InverseTransformBaseType InverseTransformBaseType;
  typedef InverseTransformBaseType::Pointer    InverseTransformBasePointer;
  
  // RTTI:
  itkTypeMacro(GridTransform, Transform);

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
  
  typedef Superclass::InputPointType InputPointType;
  typedef Superclass::OutputPointType OutputPointType;
  
  // virtual:
  virtual OutputPointType TransformPoint(const InputPointType & x) const
  {
    OutputPointType y;
    if (is_inverse())
    {
      pnt2d_t uv;
      uv[0] = (x[0] - transform_.tile_min_[0]) / transform_.tile_ext_[0];
      uv[1] = (x[1] - transform_.tile_min_[1]) / transform_.tile_ext_[1];
      transform_.transform_inv(uv, y);
    }
    else
    {
      transform_.transform(x, y);
      y[0] *= transform_.tile_ext_[0];
      y[1] *= transform_.tile_ext_[1];
      y[0] += transform_.tile_min_[0];
      y[1] += transform_.tile_min_[1];
    }
    
    // ITK does not handle NaN numbers:
    if (y[0] != y[0])
    {
      y[0] = std::numeric_limits<double>::max();
      y[1] = y[0];
    }
    
    return y;
  }
  
  // Inverse transformations:
  // If y = Transform(x), then x = BackTransform(y);
  // if no mapping from y to x exists, then an exception is thrown.
  InputPointType BackTransformPoint(const OutputPointType & y) const
  {
    InputPointType x;
    if (is_inverse())
    {
      transform_.transform(y, x);
      x[0] *= transform_.tile_ext_[0];
      x[1] *= transform_.tile_ext_[1];
      x[0] += transform_.tile_min_[0];
      x[1] += transform_.tile_min_[1];
    }
    else
    {
      pnt2d_t uv;
      uv[0] = (y[0] - transform_.tile_min_[0]) / transform_.tile_ext_[0];
      uv[1] = (y[1] - transform_.tile_min_[1]) / transform_.tile_ext_[1];
      transform_.transform_inv(uv, x);
    }
    
    // ITK does not handle NaN numbers:
    if (x[0] != x[0])
    {
      x[0] = std::numeric_limits<double>::max();
      x[1] = x[0];
    }
    
    return x;
  }
  
  // virtual:
  virtual void SetFixedParameters(const ParametersType & params)
  { this->m_FixedParameters = params; }
  
  // virtual:
  virtual const ParametersType & GetFixedParameters() const
  {
    ParametersType params = this->m_FixedParameters;
    
    params[1] = transform_.rows_;
    params[2] = transform_.cols_;
    params[3] = transform_.tile_min_[0];
    params[4] = transform_.tile_min_[1];
    params[5] = transform_.tile_ext_[0];
    params[6] = transform_.tile_ext_[1];
    
    // update the parameters vector:
    GridTransform * fake = const_cast<GridTransform *>(this);
    fake->m_FixedParameters = params;
    return this->m_FixedParameters;
  }

  // virtual:
  virtual void SetParameters(const ParametersType & params)
  {
    this->m_Parameters = params;
    
    size_t rows = int(this->m_FixedParameters[1]);
    size_t cols = int(this->m_FixedParameters[2]);

    pnt2d_t tile_min;
    tile_min[0] = this->m_FixedParameters[3];
    tile_min[1] = this->m_FixedParameters[4];

    pnt2d_t tile_max;
    tile_max[0] = tile_min[0] + this->m_FixedParameters[5];
    tile_max[1] = tile_min[1] + this->m_FixedParameters[6];

    std::vector<pnt2d_t> xy((rows + 1) * (cols + 1));

    unsigned int num_points = xy.size();
//    assert(2 * num_points == params.size());
    if (2 * num_points != params.size())
    {
      itkExceptionMacro(<< "xy size does not match number of parameters");
    }

    for (unsigned int i = 0; i < num_points; i++)
    {
      xy[i][0] = params[i * 2];
      xy[i][1] = params[i * 2 + 1];
    }

    transform_.setup(rows,
                     cols,
                     tile_min,
                     tile_max,
                     xy);
  }

  // virtual:
  virtual const ParametersType & GetParameters() const
  {
    ParametersType params(GetNumberOfParameters());
    unsigned int num_pts = params.size() / 2;
    unsigned int num_cols = (transform_.cols_ + 1);
    for (unsigned int i = 0; i < num_pts; i++)
    {
      unsigned int row = i / num_cols;
      unsigned int col = i % num_cols;

      const pnt2d_t & xy = transform_.vertex(row, col).xy_;
      unsigned int idx = 2 * i;
      params[idx] = xy[0];
      params[idx + 1] = xy[1];
    }

    // update the parameters vector:
    GridTransform * fake = const_cast<GridTransform *>(this);
    fake->m_Parameters = params;
    return this->m_Parameters;
  }

  // virtual:
  virtual NumberOfParametersType GetNumberOfParameters(void) const
  { return 2 * transform_.grid_.mesh_.size(); }

  // virtual: return an inverse of this transform.
  virtual InverseTransformBasePointer GetInverseTransform() const
  {
    GridTransform::Pointer inv = GridTransform::New();
    inv->setup(transform_, !is_inverse());
    return inv.GetPointer();
  }

  // setup the transform:
  void setup(const the_grid_transform_t & transform,
             const bool & is_inverse = false)
  {
    transform_ = transform;
    GetParameters();
    GetFixedParameters();
//    this->m_Jacobian.SetSize(2, GetNumberOfParameters());
    this->m_FixedParameters[0] = is_inverse ? 1.0 : 0.0;
  }

  // inverse transform flag check:
  inline bool is_inverse() const
  { return this->m_FixedParameters[0] != 0.0; }

//  // virtual:
//  virtual
//  const JacobianType &
//  GetJacobian(const InputPointType & point) const
//  {
//    // FIXME: 20061227 -- this function was written and not tested:
//
//    // these scales are necessary to account for the fact that
//    // the_grid_transform_t expects uv in the [0,1]x[0,1] range,
//    // where as we remap it into the image tile physical coordinates
//    // according to tile_min_ and tile_ext_:
//    double Su = transform_.tile_ext_[0];
//    double Sv = transform_.tile_ext_[1];
//
//    unsigned int idx[3];
//    double jac[12];
//    this->m_Jacobian.SetSize(2, GetNumberOfParameters());
//    this->m_Jacobian.Fill(0.0);
//    if (transform_.jacobian(point, idx, jac))
//    {
//      for (unsigned int i = 0; i < 3; i++)
//      {
//        unsigned int addr = idx[i] * 2;
//        this->m_Jacobian(0, addr) = Su * jac[i * 2];
//        this->m_Jacobian(0, addr + 1) = Su * jac[i * 2 + 1];
//        this->m_Jacobian(1, addr) = Sv * jac[i * 2 + 6];
//        this->m_Jacobian(1, addr + 1) = Sv * jac[i * 2 + 7];
//      }
//    }
//
//    return this->m_Jacobian;
//  }

  virtual
  void
  ComputeJacobianWithRespectToParameters( const InputPointType &point, JacobianType &jacobian ) const
  {
    // FIXME: 20061227 -- this function was written and not tested:
    
    // these scales are necessary to account for the fact that
    // the_grid_transform_t expects uv in the [0,1]x[0,1] range,
    // where as we remap it into the image tile physical coordinates
    // according to tile_min_ and tile_ext_:
    double Su = transform_.tile_ext_[0];
    double Sv = transform_.tile_ext_[1];
    
    unsigned int idx[3];
    double jac[12];
    jacobian.SetSize(2, GetNumberOfParameters());
    jacobian.Fill(0.0);
    if (transform_.jacobian(point, idx, jac))
    {
      for (unsigned int i = 0; i < 3; i++)
      {
        unsigned int addr = idx[i] * 2;
        jacobian(0, addr) = Su * jac[i * 2];
        jacobian(0, addr + 1) = Su * jac[i * 2 + 1];
        jacobian(1, addr) = Sv * jac[i * 2 + 6];
        jacobian(1, addr + 1) = Sv * jac[i * 2 + 7];
      }
    }
  }
  
protected:
  GridTransform():
//    Superclass(2, 0)
    Superclass(0)
  {
    this->m_FixedParameters.SetSize(7);

    // initialize the inverse flag:
    this->m_FixedParameters[0] = 0.0;

    // grid size:
    this->m_FixedParameters[1] = 0.0;
    this->m_FixedParameters[2] = 0.0;

    // tile bbox:
    this->m_FixedParameters[3] = std::numeric_limits<double>::max();
    this->m_FixedParameters[4] = this->m_FixedParameters[3];
    this->m_FixedParameters[5] = -(this->m_FixedParameters[3]);
    this->m_FixedParameters[6] = -(this->m_FixedParameters[3]);
  }

private:
  // disable default copy constructor and assignment operator:
  GridTransform(const Self & other);
  const Self & operator = (const Self & t);

public:
  // the actual transform:
  the_grid_transform_t transform_;

}; // class GridTransform

} // namespace itk


#endif //  __itkGridTransform_h
