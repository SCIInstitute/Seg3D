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

#ifndef CORE_GEOMETRY_PATH_H
#define CORE_GEOMETRY_PATH_H

// System includes
#include <string>
#include <vector>

// Core includes
#include <Core/Geometry/Point.h>

namespace Core
{

//=============================================================================
// Class: Path
//=============================================================================

class Path 
{
public:

  Path();
  ~Path();

  // GET_LENGTH:
  // Get number of Path
  size_t get_path_num() const;

  // GET_ONE_PATH:
  // Get the corresponding path
  std::vector< Point >& get_one_path( int index ) ;

  std::vector< std::vector< Core::Point > >& get_all_paths( ) ;

  // SET_ONE_PATH:
  // Set the corresponding path
  void set_one_path( int index, const std::vector< Point >& pt ) ;

  // ADD_ONE_PATH:
  // Add one path to paths
  void add_one_path( const std::vector< Point >& pt );

  inline bool operator==( const Path& ) const;
  inline bool operator!=( const Path& ) const;

private:
  std::vector< std::vector< Core::Point > > paths_; 

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
bool ImportFromString( const std::string& str, Path& value );

} // end namespace Core

#endif 
