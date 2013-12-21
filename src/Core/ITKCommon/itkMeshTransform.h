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

// File         : itkMeshTransform.h
// Author       : Pavel A. Koshevoy
// Created      : 2009/05/06 20:13
// Copyright    : (C) 2009 University of Utah
// Description  : A discontinuous transform -- a Delaunay triangle mesh
//                mapped to an image. At each vertex, in addition to image
//                space coordinates a second set of coordinates is stored.
//                This is similar to texture mapped OpenGL triangle meshes,
//                where the texture coordinates correspond to the image space
//                vertex coordinates.

#ifndef __itkMeshTransform_h
#define __itkMeshTransform_h

// system includes:
#include <math.h>
#include <iostream>
#include <assert.h>

// ITK includes:
#include <itkTransform.h>
#include <itkExceptionObject.h>
#include <itkIdentityTransform.h>
#include <itkMacro.h>
#include <itkImage.h>

// local includes:
#include <Core/ITKCommon/itkDiscontinuousTransform.h>


//----------------------------------------------------------------
// itk::MeshTransform
//
namespace itk
{
  class MeshTransform : public Transform<double, 2, 2>
  {
  public:
    // standard typedefs:
    typedef MeshTransform Self;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;

    typedef Transform<double, 2, 2> Superclass;

    // Base inverse transform type:
//    typedef Superclass::InverseTransformType InverseTransformType;
    typedef Superclass InverseTransformType;
    typedef SmartPointer< InverseTransformType > InverseTransformPointer;

    // RTTI:
    itkTypeMacro(MeshTransform, Transform);

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
    OutputPointType TransformPoint(const InputPointType & x) const
    {
      OutputPointType y;
      if ( transform_.grid_.cols_ == 0 || transform_.grid_.cols_ == 0 )
      {
        // No grid has been setup.  Use the identity.
        y[0] = x[0];
        y[1] = x[1];
      }
      else if (is_inverse())
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
      
      // ITK is not forgiving to NaNs:
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
      
      // ITK does not handle NaNs well:
      if (x[0] != x[0])
      {
        x[0] = std::numeric_limits<double>::max();
        x[1] = x[0];
      }
      
      return x;
    }
    
    // virtual:
    void SetFixedParameters(const ParametersType & params)
    { this->m_FixedParameters = params; }
    
    // virtual:
    const ParametersType & GetFixedParameters() const
    {
      ParametersType params = this->m_FixedParameters;

      // acceleration grid size:
      params[1] = transform_.grid_.rows_;
      params[2] = transform_.grid_.cols_;
      
      // bounding box if the image associated with this transform:
      params[3] = transform_.tile_min_[0];
      params[4] = transform_.tile_min_[1];
      params[5] = transform_.tile_ext_[0];
      params[6] = transform_.tile_ext_[1];
      
      // number of vertices in the Delaunay triangle mesh:
      params[7] = transform_.grid_.mesh_.size();
      
      // update the parameters vector:
      MeshTransform * fake = const_cast<MeshTransform *>(this);
      fake->m_FixedParameters = params;
      return this->m_FixedParameters;
    }

    // virtual:
    void SetParameters(const ParametersType & params)
    {
      this->m_Parameters = params;
      
      std::size_t accel_grid_rows = std::size_t(this->m_FixedParameters[1]);
      std::size_t accel_grid_cols = std::size_t(this->m_FixedParameters[2]);

      pnt2d_t tile_min;
      tile_min[0] = this->m_FixedParameters[3];
      tile_min[1] = this->m_FixedParameters[4];

      pnt2d_t tile_max;
      tile_max[0] = tile_min[0] + this->m_FixedParameters[5];
      tile_max[1] = tile_min[1] + this->m_FixedParameters[6];

      const std::size_t num_points = std::size_t(this->m_FixedParameters[7]);
      std::vector<pnt2d_t> uv(num_points);
      std::vector<pnt2d_t> xy(num_points);

      for (unsigned int i = 0; i < num_points; i++)
      {
        const std::size_t idx = i * 4;
        
        uv[i][0] = params[idx + 0];
        uv[i][1] = params[idx + 1];
        xy[i][0] = params[idx + 2];
        xy[i][1] = params[idx + 3];
      }

      transform_.setup(tile_min,
                       tile_max,
                       uv,
                       xy,
                       accel_grid_rows,
                       accel_grid_cols);
    }

    // virtual:
    const ParametersType & GetParameters() const
    {
      ParametersType params(GetNumberOfParameters());

      const std::size_t num_points = transform_.grid_.mesh_.size();
      for (std::size_t i = 0; i < num_points; i++)
      {
        const vertex_t & vx = transform_.grid_.mesh_[i];
        const std::size_t idx = i * 4;
        
        params[idx + 0] = vx.uv_[0];
        params[idx + 1] = vx.uv_[1];
        params[idx + 2] = vx.xy_[0];
        params[idx + 3] = vx.xy_[1];
      }

      // update the parameters vector:
      MeshTransform * fake = const_cast<MeshTransform *>(this);
      fake->m_Parameters = params;
      return this->m_Parameters;
    }

    // virtual:
    unsigned int GetNumberOfParameters() const
    { return 4 * transform_.grid_.mesh_.size(); }

    // virtual: return an inverse of this transform.
    InverseTransformPointer GetInverse() const
    {
      MeshTransform::Pointer inv = MeshTransform::New();
      inv->setup(transform_, !is_inverse());
      return inv.GetPointer();
    }

    // setup the transform:
    void setup(const the_mesh_transform_t & transform,
               const bool & is_inverse = false)
    {
      transform_ = transform;
      GetParameters();
      GetFixedParameters();
      this->m_Jacobian.SetSize(2, GetNumberOfParameters());
      this->m_FixedParameters[0] = is_inverse ? 1.0 : 0.0;
    }

    // inverse transform flag check:
    inline bool is_inverse() const
    { return this->m_FixedParameters[0] != 0.0; }

    // virtual:
    const JacobianType &
    GetJacobian(const InputPointType & point) const
    {
      // FIXME: 20061227 -- this function was written and not tested:

      // these scales are necessary to account for the fact that
      // the_mesh_transform_t expects uv in the [0,1]x[0,1] range,
      // where as we remap it into the image tile physical coordinates
      // according to tile_min_ and tile_ext_:
      double Su = transform_.tile_ext_[0];
      double Sv = transform_.tile_ext_[1];

      unsigned int idx[3];
      double jac[12];
      this->m_Jacobian.SetSize(2, GetNumberOfParameters());
      this->m_Jacobian.Fill(0.0);
      if (transform_.jacobian(point, idx, jac))
      {
        for (unsigned int i = 0; i < 3; i++)
        {
          unsigned int addr = idx[i] * 2;
          this->m_Jacobian(0, addr) = Su * jac[i * 2];
          this->m_Jacobian(0, addr + 1) = Su * jac[i * 2 + 1];
          this->m_Jacobian(1, addr) = Sv * jac[i * 2 + 6];
          this->m_Jacobian(1, addr + 1) = Sv * jac[i * 2 + 7];
        }
      }

      return this->m_Jacobian;
    }

  protected:
    MeshTransform():
      Superclass(2, 0)
    {
      this->m_FixedParameters.SetSize(8);

      // initialize the inverse flag:
      this->m_FixedParameters[0] = 0.0;

      // acceleration grid size:
      this->m_FixedParameters[1] = 0.0;
      this->m_FixedParameters[2] = 0.0;

      // bounding box if the image associated with this transform:
      this->m_FixedParameters[3] = std::numeric_limits<double>::max();
      this->m_FixedParameters[4] = this->m_FixedParameters[3];
      this->m_FixedParameters[5] = -(this->m_FixedParameters[3]);
      this->m_FixedParameters[6] = -(this->m_FixedParameters[3]);

      // number of vertices in the Delaunay triangle mesh:
      this->m_FixedParameters[7] = 0.0;
    }

  private:
    // disable default copy constructor and assignment operator:
    MeshTransform(const Self & other);
    const Self & operator = (const Self & t);

  public:
    // the actual transform:
    the_mesh_transform_t transform_;

  }; // class MeshTransform

} // namespace itk


#endif //  __itkMeshTransform_h
