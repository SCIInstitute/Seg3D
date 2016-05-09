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

#ifndef CORE_GEOMETRY_PATH_H
#define CORE_GEOMETRY_PATH_H

// System includes
#include <string>
#include <vector>
#include <set>
#include <float.h>

// Core includes
#include <Core/Geometry/Point.h>
#include <Core/Geometry/SinglePath.h>

namespace Core
{

//=============================================================================
// Class: Path
//=============================================================================
class SinglePath;

class Path 
{
public:

  Path();
  ~Path();

  // GET_LENGTH:
  /// Get number of Path
  size_t get_path_num() const;

  // GET_ONE_PATH:
  /// Get the corresponding path
  SinglePath& get_one_path( int index ) ;
  bool find_one_path( Point& p1, Point& p2, SinglePath& spath ) ;


  const std::vector< SinglePath >& get_all_paths( )  const;
  void set_all_paths(  std::vector< SinglePath >& paths );

  // SET_ONE_PATH:
  /// Set the corresponding path
  /// void set_one_path( int index, const std::vector< Point >& pt ) ;
  void set_one_path( int index, const SinglePath& pt ) ;

  // ADD_ONE_PATH:
  /// Add one path to paths
  /// void add_one_path( const std::vector< Point >& pt );
  void add_one_path( const SinglePath& pt );

  bool delete_one_path( Point& p1, Point& p2 ) ;

  void delete_all_paths( ) ;

  void set_start_point( const Point& pt );
  const Point& get_start_point () const; 

  void set_end_point( const Point& pt );
  const Point& get_end_point () const;


  inline bool operator==( const Path& ) const;
  inline bool operator!=( const Path& ) const;

private:
  const static size_t PATHS_SIZE_C = 100;
  std::vector< SinglePath > paths_;

  Point start_point_;
  Point end_point_;

public:

};

inline bool Path::operator==( const Path& m ) const
{
  return false;
}

inline bool Path::operator!=( const Path& m ) const
{
  return true;
}


std::string ExportToString( const Path& value );
std::string ExportToString( const SinglePath& value );
bool ImportFromString( const std::string& str, Path& value );
bool ImportFromString( const std::string& str, SinglePath& value );

} // end namespace Core

#endif 
