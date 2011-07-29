#ifndef CORE_GEOMETRY_SINGLEPATH_H
#define CORE_GEOMETRY_SINGLEPATH_H

// System includes
#include <vector>
#include <float.h>

// Core includes
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Algorithm.h>

namespace Core
{

class SinglePath
{
private:
  Point points_[ 2 ]; // 3D world coordinates of end points
  std::vector< Point > path_;
  const static size_t POINTS_NUM_ON_PATH_C = 1000;

public:
  SinglePath() 
  {
    //this->path_.reserve( POINTS_NUM_ON_PATH_C );
  }
  SinglePath( Point& p1, Point& p2 )
  {
    points_[ 0 ] = p1 ;
    points_[ 1 ] = p2 ; 
    //this->path_.reserve( POINTS_NUM_ON_PATH_C );
  }


  ~SinglePath()
  {
    this->path_.clear();
  }

  void add_a_point( const Core::Point& p )
  {
    this->path_.push_back( p );
  }

  Point& get_a_point( int index )  { return this->path_[ index ]; }

  size_t get_points_num_on_path() const { return this->path_.size(); }

  bool get_point_on_ends( int index, Point& pt ) const
  {  
    if( !( index == 0 || index == 1 ) ) return false;

    pt = this->points_[ index ];
    return true;
  }

  // SET_POINT:
  // Set 3D world coordinate of point at given index (0 or 1)
  bool set_point_on_ends( int index, const Point& pt )
  {
    if( !( index == 0 || index == 1 ) ) return false;

    this->points_[ index ] = pt;
    return true;
  }

  // Compute the distance to every piece of path
  // Return the smallest distance

  double get_smallest_distance2( const Point& pt )
  {
    double min_dist = DBL_MAX;
    size_t num_points_on_path = this->path_.size();
    const double epsilon = 1e-12;

    for ( unsigned int i = 0; i < num_points_on_path - 1; ++i )
    {
      double dist = DistanceToLine2( pt, this->path_[ i ], this->path_[ i + 1 ], epsilon );
      if ( dist < min_dist )
      {
        min_dist = dist;
      }
    }

    return min_dist;
    
  }

  inline bool operator==( const SinglePath& m ) const 
  {
    return ( this->points_[ 0 ] == m.points_[ 0 ] && this->points_[ 1 ] == m.points_[ 1 ] );
  }

  inline bool operator!=( const SinglePath& m ) const
  {
    return ( this->points_[ 0 ] != m.points_[ 0 ] || this->points_[ 1 ] != m.points_[ 1 ] ) ;
  }
};

} // end namespace Core
#endif