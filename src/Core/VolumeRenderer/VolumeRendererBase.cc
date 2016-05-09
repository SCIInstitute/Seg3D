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

#include <Core/Math/MathFunctions.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/VolumeRenderer/VolumeRendererBase.h>
#include <Core/Geometry/Algorithm.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace Core
{

//////////////////////////////////////////////////////////////////////////
// Marching Cubes Lookup Tables
//////////////////////////////////////////////////////////////////////////

// marching cubes edge table
static const unsigned short EDGE_TABLE_C[256] = 
{
  0x0, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
  0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
  0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
  0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
  0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
  0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
  0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
  0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
  0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
  0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
  0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
  0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
  0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
  0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
  0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
  0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
  0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
  0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
  0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
  0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
  0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
  0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
  0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
  0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
  0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
  0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
  0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
  0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
  0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
  0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
  0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
  0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0
};  

// marching cubes triangle table
static const char POLY_TABLE_C[256][7] = 
{
  {-1},
  {0, 8, 3, -1},
  {0, 1, 9, -1},
  {1, 9, 8, 3, -1},
  {1, 2, 10, -1},
  {-1},
  {9, 0, 2, 10, -1},
  {2, 10, 9, 8, 3, -1},
  {3, 11, 2, -1},
  {0, 8, 11, 2, -1},
  {-1},
  {1, 9, 8, 11, 2, -1},
  {3, 11, 10, 1, -1},
  {0, 8, 11, 10, 1, -1},
  {3, 11, 10, 9, 0, -1},
  {9, 8, 11, 10, -1},
  {4, 7, 8, -1},
  {4, 7, 3, 0, -1},
  {-1},
  {4, 7, 3, 1, 9, -1},
  {-1},
  {-1},
  {-1},
  {2, 10, 9, 4, 7, 3, -1},
  {-1},
  {11, 2, 0, 4, 7, -1},
  {-1},
  {4, 7, 11, 2, 1, 9, -1},
  {-1},
  {1, 0, 4, 7, 11, 10, -1},
  {-1},
  {4, 7, 11, 10, 9, -1},
  {9, 5, 4, -1},
  {-1},
  {0, 1, 5, 4, -1},
  {8, 3, 1, 5, 4, -1},
  {-1},
  {-1},
  {5, 4, 0, 2, 10, -1},
  {2, 10, 5, 4, 8, 3, -1},
  {-1},
  {-1},
  {-1},
  {2, 1, 5, 4, 8, 11, -1},
  {-1},
  {-1},
  {5, 4, 0, 3, 11, 10, -1},
  {5, 4, 8, 11, 10, -1},
  {9, 5, 7, 8, -1},
  {9, 5, 7, 3, 0, -1},
  {0, 1, 5, 7, 8, -1},
  {1, 5, 7, 3, -1},
  {-1},
  {-1},
  {8, 0, 2, 10, 5, 7, -1},
  {2, 10, 5, 7, 3, -1},
  {-1},
  {9, 5, 7, 11, 2, 0, -1},
  {-1},
  {11, 2, 1, 5, 7, -1},
  {-1},
  {-1},
  {-1},
  {11, 10, 5, 7, -1},
  {10, 6, 5, -1},
  {-1},
  {-1},
  {-1},
  {1, 2, 6, 5, -1},
  {-1},
  {9, 0, 2, 6, 5, -1},
  {5, 9, 8, 3, 2, 6, -1},
  {-1},
  {-1},
  {-1},
  {-1},
  {6, 5, 1, 3, 11, -1},
  {0, 8, 11, 6, 5, 1, -1},
  {3, 11, 6, 5, 9, 0, -1},
  {6, 5, 9, 8, 11, -1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {10, 6, 4, 9, -1},
  {-1},
  {10, 6, 4, 0, 1, -1},
  {8, 3, 1, 10, 6, 4, -1},
  {1, 2, 6, 4, 9, -1},
  {-1},
  {0, 2, 6, 4, -1},
  {8, 3, 2, 6, 4, -1},
  {-1},
  {-1},
  {-1},
  {-1},
  {9, 1, 3, 11, 6, 4, -1},
  {-1},
  {3, 11, 6, 4, 0, -1},
  {6, 4, 8, 11, -1},
  {7, 8, 9, 10, 6, -1},
  {0, 9, 10, 6, 7, 3, -1},
  {10, 6, 7, 8, 0, 1, -1},
  {10, 6, 7, 3, 1, -1},
  {1, 2, 6, 7, 8, 9, -1},
  {-1},
  {7, 8, 0, 2, 6, -1},
  {7, 3, 2, 6, -1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {7, 11, 6, -1},
  {7, 6, 11, -1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {7, 6, 2, 3, -1},
  {7, 6, 2, 0, 8, -1},
  {-1},
  {1, 9, 8, 7, 6, 2, -1},
  {10, 1, 3, 7, 6, -1},
  {10, 1, 0, 8, 7, 6, -1},
  {0, 3, 7, 6, 10, 9, -1},
  {7, 6, 10, 9, 8, -1},
  {6, 11, 8, 4, -1},
  {3, 0, 4, 6, 11, -1},
  {-1},
  {9, 4, 6, 11, 3, 1, -1},
  {-1},
  {-1},
  {-1},
  {-1},
  {8, 4, 6, 2, 3, -1},
  {0, 4, 6, 2, -1},
  {-1},
  {1, 9, 4, 6, 2, -1},
  {8, 4, 6, 10, 1, 3, -1},
  {10, 1, 0, 4, 6, -1},
  {-1},
  {10, 9, 4, 6, -1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {-1},
  {6, 11, 8, 9, 5, -1},
  {3, 0, 9, 5, 6, 11, -1},
  {0, 1, 5, 6, 11, 8, -1},
  {6, 11, 3, 1, 5, -1},
  {-1},
  {-1},
  {-1},
  {-1},
  {5, 6, 2, 3, 8, 9, -1},
  {9, 5, 6, 2, 0, -1},
  {-1},
  {1, 5, 6, 2, -1},
  {-1},
  {-1},
  {-1},
  {10, 5, 6, -1},
  {11, 7, 5, 10, -1},
  {-1},
  {-1},
  {-1},
  {11, 7, 5, 1, 2, -1},
  {-1},
  {9, 0, 2, 11, 7, 5, -1},
  {-1},
  {2, 3, 7, 5, 10, -1},
  {8, 7, 5, 10, 2, 0, -1},
  {-1},
  {-1},
  {1, 3, 7, 5, -1},
  {0, 8, 7, 5, 1, -1},
  {9, 0, 3, 7, 5, -1},
  {9, 8, 7, 5, -1},
  {5, 10, 11, 8, 4, -1},
  {5, 10, 11, 3, 0, 4, -1},
  {-1},
  {-1},
  {2, 11, 8, 4, 5, 1, -1},
  {-1},
  {-1},
  {-1},
  {2, 3, 8, 4, 5, 10, -1},
  {5, 10, 2, 0, 4, -1},
  {-1},
  {-1},
  {8, 4, 5, 1, 3, -1},
  {0, 4, 5, 1, -1},
  {-1},
  {9, 4, 5, -1},
  {4, 9, 10, 11, 7, -1},
  {-1},
  {1, 10, 11, 7, 4, 0, -1},
  {-1},
  {4, 9, 1, 2, 11, 7, -1},
  {-1},
  {11, 7, 4, 0, 2, -1},
  {-1},
  {2, 3, 7, 4, 9, 10, -1},
  {-1},
  {-1},
  {-1},
  {4, 9, 1, 3, 7, -1},
  {-1},
  {4, 0, 3, 7, -1},
  {4, 8, 7, -1},
  {9, 10, 11, 8, -1},
  {3, 0, 9, 10, 11, -1},
  {0, 1, 10, 11, 8, -1},
  {3, 1, 10, 11, -1},
  {1, 2, 11, 8, 9, -1},
  {-1},
  {0, 2, 11, 8, -1},
  {3, 2, 11, -1},
  {2, 3, 8, 9, 10, -1},
  {9, 10, 2, 0, -1},
  {-1},
  {1, 10, 2, -1},
  {1, 3, 8, 9, -1},
  {0, 9, 1, -1},
  {0, 3, 8, -1},
  {-1}
};  

static const unsigned char EDGE_CORNERS_C[12][2] =
{
  { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 4, 5 }, { 5, 6 },
  { 6, 7 }, { 7, 4 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
};

static const unsigned int CORNER_INDEX_C[24] = 
{
  0, 1, 2, 3, 
  4, 5, 6, 7,
  0, 1, 5, 4,
  1, 2, 6, 5,
  2, 3, 7, 6,
  3, 0, 4, 7
};

//////////////////////////////////////////////////////////////////////////
// Class VolumeRendererBasePrivate
//////////////////////////////////////////////////////////////////////////

// NOTE: The Ordering of Vertices
// Let zyx be the binary representation of the index of a vertex. Each bit is set to 1 if
// the corresponding coordinate component is at its maximum value, or 0 if at minimum.
// Then the vertices of a box can be numbered as
// 000, 001, 010, 011, 100, 101, 110, 111.
  
class VolumeRendererBasePrivate
{
public:
  // The viewing direction
  Vector view_dir_;
  // Index of the vertex closest to the eye
  int front_vertex_;
  // Index of the vertex farthest away from the eye
  int back_vertex_;
  // The distance between adjacent sampling slices
  double sample_distance_;
  // The start position for sampling
  double sample_start_;
  // The size of the voxel in world space
  Vector voxel_size_;
  // The normalized sampling distance
  double normalized_sample_distance_;
  // Whether to render the volume in front-to-back order.
  bool front_to_back_;
};

// EYETOBOXDISTANCE:
// Compute the distance from eye to the given box.
static double EyeToBoxDistance( const Point& eyep, const Vector& view_dir, const BBox& bbox )
{
  if ( bbox.inside( eyep ) )
  {
    return 0;
  }

  Point corners[] = { bbox.min(), bbox.max() };

  // Front facing surface in X-direction
  int front_face = view_dir.x() > 0 ? 0 : 1;
  Point A( corners[ front_face ].x(), corners[ 0 ].y(), corners[ 1 ].z() );
  Point B( corners[ front_face ].x(), corners[ 0 ].y(), corners[ 0 ].z() );
  Point C( corners[ front_face ].x(), corners[ 1 ].y(), corners[ 1 ].z() );
  Point closest_p;
  ClosestPointOnTriangle( closest_p, eyep, A, B, C );
  Vector pvec = closest_p - eyep;
  double dist = pvec.length();
  A = Point( corners[ front_face ].x(), corners[ 1 ].y(), corners[ 0 ].z() );
  ClosestPointOnTriangle( closest_p, eyep, A, B, C );
  pvec = closest_p - eyep;
  dist = Min( dist, pvec.length() );

  // Front facing surface in Y-direction
  front_face = view_dir.y() > 0 ? 0 : 1;
  A = Point( corners[ 0 ].x(), corners[ front_face ].y(), corners[ 1 ].z() );
  B = Point( corners[ 0 ].x(), corners[ front_face ].y(), corners[ 0 ].z() );
  C = Point( corners[ 1 ].x(), corners[ front_face ].y(), corners[ 1 ].z() );
  ClosestPointOnTriangle( closest_p, eyep, A, B, C );
  pvec = closest_p - eyep;
  dist = Min( dist, pvec.length() );
  A = Point( corners[ 1 ].x(), corners[ front_face ].y(), corners[ 0 ].z() );
  ClosestPointOnTriangle( closest_p, eyep, A, B, C );
  pvec = closest_p - eyep;
  dist = Min( dist, pvec.length() );

  // Front facing surface in Z-direction
  front_face = view_dir.z() > 0 ? 0 : 1;
  A = Point( corners[ 0 ].x(), corners[ 1 ].y(), corners[ front_face ].z() );
  B = Point( corners[ 0 ].x(), corners[ 0 ].y(), corners[ front_face ].z() );
  C = Point( corners[ 1 ].x(), corners[ 1 ].y(), corners[ front_face ].z() );
  ClosestPointOnTriangle( closest_p, eyep, A, B, C );
  pvec = closest_p - eyep;
  dist = Min( dist, pvec.length() );
  A = Point( corners[ 1 ].x(), corners[ 0 ].y(), corners[ front_face ].z() );
  ClosestPointOnTriangle( closest_p, eyep, A, B, C );
  pvec = closest_p - eyep;
  dist = Min( dist, pvec.length() );

  return dist;
}

//////////////////////////////////////////////////////////////////////////
// Class VolumeRendererBase
//////////////////////////////////////////////////////////////////////////

VolumeRendererBase::VolumeRendererBase() :
  private_( new VolumeRendererBasePrivate )
{
}

VolumeRendererBase::~VolumeRendererBase()
{
}

void VolumeRendererBase::initialize()
{
}

static bool CompareBrickEntryAscend( const BrickEntry& brick1, const BrickEntry& brick2 )
{
  return brick1.distance_ < brick2.distance_;
}

static bool CompareBrickEntryDescend( const BrickEntry& brick1, const BrickEntry& brick2 )
{
  return brick1.distance_ > brick2.distance_;
}

void VolumeRendererBase::process_volume( DataVolumeHandle volume, 
  double sample_rate, const View3D& view, bool orthographic, 
  bool front_to_back, std::vector< BrickEntry >& sorted_bricks )
{
  std::vector< DataVolumeBrickHandle > bricks;
  volume->get_bricks( bricks );
  size_t num_bricks = bricks.size();
  if ( num_bricks == 0 )
  {
    return;
  }

  this->private_->view_dir_ = view.lookat() - view.eyep();
  this->private_->view_dir_.normalize();
  this->private_->front_to_back_ = front_to_back;

  GridTransform grid_trans = volume->get_grid_transform();
  this->private_->voxel_size_ = grid_trans * Vector( 1.0, 1.0, 1.0 );
  Point voxel_min( 0.0, 0.0, 0.0 );
  Point voxel_max( this->private_->voxel_size_ );
  Point corners[ 2 ] = { voxel_min, voxel_max };
  double min_dist = std::numeric_limits< double >::max();
  double max_dist = std::numeric_limits< double >::min();

  // Computing the sampling distance and front and back vertices
  for ( int z = 0; z < 2; ++z )
  {
    for ( int y = 0; y < 2; ++y )
    {
      for ( int x = 0; x < 2; ++x )
      {
        Vector vertex( corners[ x ].x(), corners[ y ].y(), corners[ z ].z() );
        double dist = Dot( vertex, this->private_->view_dir_ );
        if ( dist < min_dist )
        {
          min_dist = dist;
          this->private_->front_vertex_ = ( z << 2 ) + ( y << 1 ) + x;
        }
        if ( dist > max_dist )
        {
          max_dist = dist;
        } 
      }
    }
  }
  // Make sure that the back vertex and the front vertex are on the opposite corners of the box
  this->private_->back_vertex_ = ( ~this->private_->front_vertex_ ) & 0x7;
  assert( this->private_->front_vertex_ >= 0 && this->private_->front_vertex_ <= 7 && 
    this->private_->back_vertex_ >= 0 && this->private_->back_vertex_ <= 7 );
  this->private_->sample_distance_ = ( max_dist - min_dist ) / sample_rate;

  // Compute the start sampling position for the volume
  Point vol_bbox_min( -0.5, -0.5, -0.5 );
  corners[ 0 ] = grid_trans * vol_bbox_min;
  double nx = static_cast< double >( grid_trans.get_nx() );
  double ny = static_cast< double >( grid_trans.get_ny() );
  double nz = static_cast< double >( grid_trans.get_nz() );
  Point vol_bbox_max( nx - 0.5, ny - 0.5, nz - 0.5 );
  corners[ 1 ] = grid_trans * vol_bbox_max;
  double unit_length = ( corners[ 1 ] - corners[ 0 ] ).length() / Sqrt( nx * nx + ny * ny + nz * nz );
  this->private_->normalized_sample_distance_ = this->private_->sample_distance_ / unit_length;
  int start_vertex_index = front_to_back ? this->private_->front_vertex_ : this->private_->back_vertex_;
  Vector start_vertex( corners[ ( start_vertex_index & 0x1 ) > 0 ? 1 : 0 ].x(),
    corners[ ( start_vertex_index & 0x2 ) > 0 ? 1 : 0 ].y(),
    corners[ ( start_vertex_index & 0x4 ) > 0 ? 1 : 0 ].z() );
  this->private_->sample_start_ = Dot( start_vertex, this->private_->view_dir_ ) 
    + ( front_to_back ? this->private_->sample_distance_ : -this->private_->sample_distance_ );
  
  // Sort the bricks in the specified order based on their distances to the eye
  for ( size_t i = 0; i < num_bricks; ++i )
  {
    BrickEntry brick_entry;
    brick_entry.brick_ = bricks[ i ];
    BBox brick_bbox = brick_entry.brick_->get_brick_bbox();
    Point corners[] = { brick_bbox.min(), brick_bbox.max() };

    // orthographic: sort bricks based on distance to the view plane
    if ( orthographic )
    {
      Vector vertex( corners[ ( this->private_->front_vertex_ & 0x1 ) > 0 ? 1 : 0 ].x(),
        corners[ ( this->private_->front_vertex_ & 0x2 ) > 0 ? 1 : 0 ].y(),
        corners[ ( this->private_->front_vertex_ & 0x4 ) > 0 ? 1 : 0 ].z() );
      brick_entry.distance_ = Dot( vertex, this->private_->view_dir_ );
    }
    // perspective: sort bricks based on distance to the eye point
    else
    {
      brick_entry.distance_ = EyeToBoxDistance( view.eyep(), this->private_->view_dir_, brick_bbox );
    }

    sorted_bricks.push_back( brick_entry );
  }

  if ( front_to_back )
  {
    std::sort( sorted_bricks.begin(), sorted_bricks.end(), CompareBrickEntryAscend );
  }
  else
  {
    std::sort( sorted_bricks.begin(), sorted_bricks.end(), CompareBrickEntryDescend );
  }
}

void VolumeRendererBase::slice_brick( DataVolumeBrickHandle brick, 
                   std::vector< PointF >& polygon_vertices, 
                   std::vector< int >& first_vec, std::vector< int >& count_vec )
{
  BBox brick_bbox = brick->get_brick_bbox();
  Point corners[] = { brick_bbox.min(), brick_bbox.max() };

  // Coordinates of 8 brick vertices
  Vector vertex_pos[ 8 ] =
  {
    Vector( corners[ 0 ] ),
    Vector( corners[ 1 ].x(), corners[ 0 ].y(), corners[ 0 ].z() ),
    Vector( corners[ 1 ].x(), corners[ 1 ].y(), corners[ 0 ].z() ),
    Vector( corners[ 0 ].x(), corners[ 1 ].y(), corners[ 0 ].z() ),
    Vector( corners[ 0 ].x(), corners[ 0 ].y(), corners[ 1 ].z() ),
    Vector( corners[ 1 ].x(), corners[ 0 ].y(), corners[ 1 ].z() ),
    Vector( corners[ 1 ].x(), corners[ 1 ].y(), corners[ 1 ].z() ),
    Vector( corners[ 0 ].x(), corners[ 1 ].y(), corners[ 1 ].z() ),
  };

  double vals[ 8 ];
  double near_distance = std::numeric_limits< double >::max();
  double far_distance = std::numeric_limits< double >::min();

  for ( int i = 0; i < 8; ++i )
  {
    vals[ i ] = Dot( vertex_pos[ i ], this->private_->view_dir_ );
    near_distance = Min( near_distance, vals[ i ] );
    far_distance = Max( far_distance, vals[ i ] );
  }

  double sample_pos;
  if ( this->private_->front_to_back_ )
  {
    sample_pos = this->private_->sample_start_ + this->private_->sample_distance_ * ( 
      near_distance <= this->private_->sample_start_ ? 0 : Ceil( 
      ( near_distance - this->private_->sample_start_ ) / this->private_->sample_distance_ ) );
  }
  else
  {
    sample_pos = this->private_->sample_start_ - this->private_->sample_distance_ * ( 
      far_distance >= this->private_->sample_start_ ? 0 : Ceil( 
      ( this->private_->sample_start_ - far_distance ) / this->private_->sample_distance_ ) );
  }
  double sample_offset = this->private_->front_to_back_ ? 
    this->private_->sample_distance_ : -this->private_->sample_distance_;

  // Use marching cubes algorithm to find the intersections of the slices and the brick
  Vector intersect_pos[ 12 ];
  while ( ( !this->private_->front_to_back_ && sample_pos > near_distance ) ||
    ( this->private_->front_to_back_ && sample_pos < far_distance ) )
  {
    // generate the code to look up edge table and triangle table
    int c_code = 0;
    for( int i = 0; i < 8; ++i )
    {
      c_code |= ( ( vals[ i ] > sample_pos ) << i );
    }

    // calculate the intersection points along the edges
    unsigned short e_code = EDGE_TABLE_C[ c_code ];
    unsigned short mask = 1;
    for( int i = 0; i < 12; ++i, mask <<= 1 )
    {
      if( e_code & mask )
      {
        unsigned char first = EDGE_CORNERS_C[ i ][ 0 ];
        unsigned char second = EDGE_CORNERS_C[ i ][ 1 ];
        double val = ( sample_pos - vals[ first ] ) / ( vals[ second ] - vals[ first ] );
        intersect_pos[ i ] = vertex_pos[ first ] * ( 1.0 - val ) + vertex_pos[ second ] * val;
      }
    }

    // add the polyhedrons
    int n = 0;
    first_vec.push_back( static_cast< int >( polygon_vertices.size() ) );
    const char* poly_code = POLY_TABLE_C[ c_code ];
    for ( ; poly_code[ n ] >= 0; ++n )
    {
      polygon_vertices.push_back( PointF( intersect_pos[ static_cast< int >( poly_code[ n ] ) ] ) );
    }
    assert( n < 7 );
    count_vec.push_back( n );

    sample_pos += sample_offset;
  }
}

Core::Vector VolumeRendererBase::get_voxel_size()
{
  return this->private_->voxel_size_;
}

double VolumeRendererBase::get_normalized_sample_distance()
{
  return this->private_->normalized_sample_distance_;
}

double VolumeRendererBase::get_sample_distance()
{
  return this->private_->sample_distance_;
}

} // end namespace Core
