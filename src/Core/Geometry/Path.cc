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

// Boost includes
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

// Core includes
#include <Core/Geometry/Path.h>
#include <Core/Utils/StringUtil.h>

namespace Core
{


Path::Path() 
{
  //this->paths_.reserve( PATHS_SIZE_C );
}

Path::~Path()
{ 
  //this->paths_.clear();
}

size_t Path::get_path_num() const
{
  return paths_.size();
}


void Path::add_one_path( const SinglePath& pt )
{
  this->paths_.push_back( pt );
}


SinglePath& Path::get_one_path( int index ) 
{
  return this->paths_[index];
}


const std::vector< SinglePath >& Path::get_all_paths( ) const
{
  return this->paths_;
}

void Path::set_all_paths(  std::vector< SinglePath >& paths ) 
{
  this->paths_.swap( paths );
}

void Path::set_one_path( int index, const SinglePath& pt ) 
{
  this->paths_[index] = pt;
}

bool Path::delete_one_path( Point& p1, Point& p2 ) 
{
  SinglePath path1( p1, p2 );
  for ( std::vector< SinglePath >::iterator it = this->paths_.begin(); it != this->paths_.end(); ++it )
  {
    SinglePath element = *it;
    if ( element == path1 )
    {
      this->paths_.erase( it );
      return true;
    }
  }
  
  return false;
}

bool Path::find_one_path( Point& p1, Point& p2, SinglePath& spath ) 
{
  SinglePath path1( p1, p2 );
  for ( std::vector< SinglePath >::iterator it = this->paths_.begin(); it != this->paths_.end(); ++it )
  {
    SinglePath element = *it;
    if ( element == path1 )
    {
      spath = *it;
      return true;
    }
  }

  return false;
}

void Path::set_start_point( const Point& pt ) 
{
  this->start_point_ = pt;
}

void Path::set_end_point( const Point& pt )
{
  this->end_point_ = pt;
}

const Point& Path::get_start_point()  const
{
  return this->start_point_;
}

const Point& Path::get_end_point () const
{
  return this->end_point_;
}

void Path::delete_all_paths( )
{
  this->paths_.clear();
  //this->paths_.reserve( PATHS_SIZE_C );
}

std::string ExportToString( const SinglePath& value )
{
  Point p0, p1;
  value.get_point_on_ends( 0, p0 );
  value.get_point_on_ends( 1, p1 );

  return ( std::string( 1, '[' ) + ExportToString( p0 ) + ' ' + 
            ExportToString( p1 ) + std::string( 1, ']') );
}

std::string ExportToString( const Path& value )
{
  std::string export_string =  "";
  return export_string;
}

bool ImportFromString( const std::string& str, SinglePath& value )
{
  return true;
}

bool ImportFromString( const std::string& str, Path& value )
{

  return true;
}


} // end namespace Core
