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


const std::vector< SinglePath >& Path::get_all_paths( ) const
{
  return this->paths_;
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
  this->paths_.reserve( PATHS_SIZE_C );
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
  //std::vector< SinglePath > paths = value.get_all_paths();

  //Point p0 = value.get_start_point();
  //Point p1 = value.get_end_point();

  //std::string export_string = std::string( 1, '[' ) ;

  //BOOST_FOREACH( SinglePath spath, paths )
  //{
  //  export_string += ExportToString( spath );
  //  export_string += ";";
  //}

  //export_string += ExportToString( SinglePath( p0, p1 ) );
  //export_string += std::string( 1, ']') ;

  std::string export_string =  "";
  return export_string;
}

bool ImportFromString( const std::string& str, SinglePath& value )
{
  
  //std::string open_bracket_reg = "(\\[)";
  //std::string closed_bracket_reg = "(\\])";
  ////std::string word_reg = "(\\w*)";
  ////std::string visible_reg = word_reg;
  //std::string space_reg = "(\\s)";
  ////std::string name_reg = word_reg;
  ////std::string point_reg = "(\\[[^\\]]*])";
  //std::string point_reg = "(\\[*])";
  ////std::string color_reg = point_reg;
  ////std::string comment_reg = "(.*?)";
  ////std::string comment_end_reg = "(\\sCOMMENT_END\\]\\])";
  ////std::string full_reg = open_bracket_reg + visible_reg + space_reg + name_reg + space_reg + 
  ////  point_reg + space_reg + point_reg + space_reg + color_reg + space_reg + open_bracket_reg + 
  ////  comment_reg + comment_end_reg;

  //std::string full_reg = open_bracket_reg + point_reg + space_reg + point_reg + closed_bracket_reg;
  //boost::regex reg( full_reg );
  //boost::smatch m;
  //if( boost::regex_match( str, m, reg ) ) 
  //{
  //  Point p0, p1;
  //  ImportFromString( m[ 0 ].str(), p0 );
  //  value.set_point_on_ends( 0, p0 );
  //  
  //  ImportFromString( m[ 1 ].str(), p1 );
  //  value.set_point_on_ends( 1, p1 );
  //  return true;
  //}
  //return false;

  return true;
}

bool ImportFromString( const std::string& str, Path& value )
{
  //// Example string: [true M1 [0 0 0] [1 1 1] [Knee NOTE_END]]
  //// Example string: [[0 0 0] [[0 0 0] [1 1 1]] [1 1 1]]

  ////std::vector< std::vector< Point > > paths; //only contains start and end point
  ////str[ 0 ] = ' ';
  ////str[ str.length() - 1 ] = ' ';
  //std::vector< std::string > parts = SplitString( str, ";" );

  ////size_t paths_num = paths.size();

  //// The last part is ""
  //for( size_t i = 0; i < parts.size() - 1; i++ )
  //{
  //  std::string single_path_string = parts[ i ];
  //  if ( i == 0 )
  //  {
  //    single_path_string[ 0 ] = ' ';
  //  }
  //  SinglePath spath;
  //  if( ImportFromString( single_path_string, spath ) )
  //  {
  //    value.add_one_path( spath );
  //  }
  //  else
  //  {
  //    return false;
  //  }
  //}
  //
  ////for ( unsigned int i = 0; i < paths_num - 1; ++i )
  ////{
  ////  std::vector< Point > path = paths[ i ];
  ////  SinglePath spath( path[ 0 ], path[ 1 ] );
  ////  value.add_one_path( spath );
  ////}

  ////std::vector< Point > path = paths[ paths_num - 1 ];
  ////value.set_start_point( path[ 0 ] );
  ////value.set_end_point( path[ 1 ] );

  //SinglePath spath;

  //if( ImportFromString( parts[ parts.size() - 1 ], spath ) )
  //{
  //  Point p0, p1;
  //  spath.get_point_on_ends( 0, p0 );
  //  spath.get_point_on_ends( 1, p1 );

  //  value.set_start_point( p0 );
  //  value.set_end_point( p1 );
  //}
  //else
  //{
  //  return false;
  //}


  return true;
}


} // end namespace Core