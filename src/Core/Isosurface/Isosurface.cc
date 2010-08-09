/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Core/Isosurface/Isosurface.h>

namespace Core
{

class IsosurfacePrivate
{
public:
  void compute_normals();

  // Input to isosurface computation
  MaskVolumeHandle mask_volume_; 

  // Output mesh
  std::vector< PointF > points_; 
  std::vector< VectorF > normals_; 
  std::vector< unsigned int > faces_; 
};

void IsosurfacePrivate::compute_normals()
{
  // TODO Parallelize this algorithm as much as possible
  
  // Reset the normals vector
  this->normals_.clear();
  this->normals_.resize( this->points_.size(), VectorF( 0, 0, 0 ) );

  // Keep track of the number of faces per vertex
  std::vector< int > faces_per_vertex( this->points_.size(), 0 );

  // For each face
  for( size_t i = 0; i + 2 < this->faces_.size(); i += 3 )
  {
    int vertex_index1 = this->faces_[ i ];
    int vertex_index2 = this->faces_[ i + 1 ];
    int vertex_index3 = this->faces_[ i + 2 ];

    // Get vertices of face
    PointF p1 = this->points_[ vertex_index1 ];
    PointF p2 = this->points_[ vertex_index2 ];
    PointF p3 = this->points_[ vertex_index3 ];

    // Calculate cross product of edges
    VectorF v0 = p3 - p2;
    VectorF v1 = p1 - p2;
    VectorF n = Cross(v0, v1);

    // Add to normal for each included vertex
    this->normals_[ vertex_index1 ] += n;
    this->normals_[ vertex_index2 ] += n;
    this->normals_[ vertex_index3 ] += n;

    // Add to count of faces per vertex
    faces_per_vertex[ vertex_index1 ]++;
    faces_per_vertex[ vertex_index2 ]++;
    faces_per_vertex[ vertex_index3 ]++;
  }

  // For each vertex
  for( size_t i = 0; i < this->normals_.size(); i++ )
  {
    // Average normal (divide by number of faces)
    this->normals_[ i ] /= static_cast< float >( faces_per_vertex[ i ] );

    // Normalize normal
    this->normals_[ i ].normalize();
  }
}

Isosurface::Isosurface( const MaskVolumeHandle& mask_volume ) :
  private_( new IsosurfacePrivate )
{
  this->private_->mask_volume_ = mask_volume;

  this->compute();
}

void Isosurface::compute()
{
  this->private_->points_.clear();
  this->private_->normals_.clear();
  this->private_->faces_.clear();

  // Create simple test geometry -- cube
  this->private_->points_.push_back( PointF( 0, 0, 0 ) );
  this->private_->points_.push_back( PointF( 1, 0, 0 ) );
  this->private_->points_.push_back( PointF( 0, 1, 0 ) );
  this->private_->points_.push_back( PointF( 1, 1, 0 ) );
  this->private_->points_.push_back( PointF( 0, 0, 1 ) );
  this->private_->points_.push_back( PointF( 1, 0, 1 ) );
  this->private_->points_.push_back( PointF( 0, 1, 1 ) );
  this->private_->points_.push_back( PointF( 1, 1, 1 ) );

  // Front 
  this->private_->faces_.push_back( 0 );
  this->private_->faces_.push_back( 1 );
  this->private_->faces_.push_back( 2 );
  this->private_->faces_.push_back( 2 );
  this->private_->faces_.push_back( 1 );
  this->private_->faces_.push_back( 3 );

  // Right side
  this->private_->faces_.push_back( 1 );
  this->private_->faces_.push_back( 5 );
  this->private_->faces_.push_back( 3 );
  this->private_->faces_.push_back( 3 );
  this->private_->faces_.push_back( 5 );
  this->private_->faces_.push_back( 7 );

  // Left side
  this->private_->faces_.push_back( 4 );
  this->private_->faces_.push_back( 0 );
  this->private_->faces_.push_back( 6 );
  this->private_->faces_.push_back( 6 );
  this->private_->faces_.push_back( 0 );
  this->private_->faces_.push_back( 2 );

  // Back
  this->private_->faces_.push_back( 5);
  this->private_->faces_.push_back( 4 );
  this->private_->faces_.push_back( 7 );
  this->private_->faces_.push_back( 7 );
  this->private_->faces_.push_back( 4 );
  this->private_->faces_.push_back( 6 );

  // Top
  this->private_->faces_.push_back( 2 );
  this->private_->faces_.push_back( 3 );
  this->private_->faces_.push_back( 6 );
  this->private_->faces_.push_back( 6 );
  this->private_->faces_.push_back( 3 );
  this->private_->faces_.push_back( 7 );

  // Bottom
  this->private_->faces_.push_back( 4 );
  this->private_->faces_.push_back( 5 );
  this->private_->faces_.push_back( 0 );
  this->private_->faces_.push_back( 0 );
  this->private_->faces_.push_back( 5 );
  this->private_->faces_.push_back( 1 );

  // TODO Port SCIRun code to here

  this->private_->compute_normals();
}

const std::vector< PointF >& Isosurface::get_points() const
{
  return this->private_->points_;
}

const std::vector< VectorF >& Isosurface::get_normals() const
{ 
  return this->private_->normals_;
}

const std::vector< unsigned int >& Isosurface::get_faces() const
{
  return this->private_->faces_;
}


} // end namespace Core