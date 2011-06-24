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
  this->paths_.reserve( PATHS_SIZE_C );
}

Path::~Path()
{ 
  this->paths_.clear();
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


std::vector< SinglePath >& Path::get_all_paths( ) 
{
  return this->paths_;
}

//void Path::set_one_path( int index, const std::vector< Point >& pt ) 
//{
//
//  this->paths_[index].clear();
//  for ( std::vector< Point >::const_iterator it = pt.begin(); it != pt.end(); ++it )
//  {
//    this->paths_[index].push_back( *it );
//  }
//}

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

Point& Path::get_start_point()  
{
  return this->start_point_;
}

Point& Path::get_end_point () 
{
  return this->end_point_;
}

void Path::delete_all_paths( )
{
  this->paths_.clear();
  this->paths_.reserve( PATHS_SIZE_C );
}

std::string ExportToString( const Path& value )
{

  //return ( std::string( 1, '[' ) + ExportToString( value.get_visible() ) + ' ' + value.get_id() 
  //  + ' ' + ExportToString( p0 ) + ' ' + ExportToString( p1 ) + 
  //  ' ' + '[' + value.get_note() + Path::NOTE_DELIMITER_C );

  std::string str;
  str = "[" ;
  //for ( std::vector< Point >::iterator it = value->get_path().begin(); it != value->get_path().end(); ++it )
  //{
  //  Point p0 = *it;
  //  str += ExportToString( p0 );
  //  str += ' ';
  //}

  str = "]";

  return str;
}


bool ImportFromString( const std::string& str, Path& value )
{
//  // Example string: [true M1 [0 0 0] [1 1 1] [Knee NOTE_END]]
//  std::string open_bracket_reg = "(\\[)";
//  std::string word_reg = "(\\w*)";
//  std::string visible_reg = word_reg;
//  std::string space_reg = "(\\s)";
//  std::string label_reg = word_reg;
//  std::string point_reg = "(\\[[^\\]]*])";
//  std::string note_reg = "(.*?)";
//  std::string note_end_reg = "(\\sNOTE_END\\]\\])";
//  std::string full_reg = open_bracket_reg + visible_reg + space_reg + label_reg + space_reg + 
//    point_reg + space_reg + point_reg + space_reg + open_bracket_reg + note_reg + 
//    note_end_reg;
//  boost::regex reg( full_reg );
//  boost::smatch m;
//  if( boost::regex_match( str, m, reg ) ) 
//  {
//    bool visible = false;
//    ImportFromString( m[ 2 ].str(), visible );
//    value.set_visible( visible );
//    value.set_id( m[ 4 ].str() );
//    Point p0;
//    ImportFromString( m[ 6 ].str(), p0 );
//    value.set_point( 0, p0 );
//    Point p1;
//    ImportFromString( m[ 8 ].str(), p1 );
//    value.set_point( 1, p1 );
//    value.set_note( m[ 11 ].str() );
//  return true;
//}
  return false;
}


} // end namespace Core