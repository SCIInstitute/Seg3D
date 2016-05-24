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
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>

#include <sstream>

// Core includes
#include <Core/Utils/FilesystemUtil.h>

namespace Core
{

bool CreateOrIgnoreDirectory( const boost::filesystem::path& dir_path )
{
  if ( ! boost::filesystem::exists( dir_path ) )
  {
    try
    {
      boost::filesystem::create_directories( dir_path );
    }
    catch ( ... )
    {
      return false;
    }
  }
  else if ( ! boost::filesystem::is_directory( dir_path ) )
  {
    return false;
  }
  
  return true;
}

bool RecursiveCopyDirectory( const boost::filesystem::path& from, const boost::filesystem::path& to )
{
  using namespace boost::filesystem;
  
  // If the source path doesn't exist or it's not a directory
  if ( ! exists( from ) || !is_directory( from ) ) return false;
  
  // Create the destination directory
  if ( ! CreateOrIgnoreDirectory( to ) ) return false;
  
  try
  {
    recursive_directory_iterator dir_it( from );
    recursive_directory_iterator dir_end;
    while ( dir_it != dir_end )
    {
      int level = dir_it.level();
      path source_path = dir_it->path();
      path file_path = source_path.filename();
      path::iterator path_it = source_path.end();
      --path_it;
      while ( level > 0 )
      {
        --path_it;
        --level;
        file_path = ( *path_it ) / file_path;
      }
      path dst_path = to / file_path;

      if ( is_regular_file( dir_it->symlink_status() ) )
      {
        copy_file( source_path, dst_path );
      }
      else if ( is_directory( dir_it->symlink_status() ) )
      {
        copy_directory( source_path, dst_path );
      }

      ++dir_it; // Increment the directory iterator
    }
    
  }
  catch ( ... )
  {
    return false;
  }

  return true;
}

std::tuple< std::string, std::string > GetFullExtension( const boost::filesystem::path& filename )
{
  // NOTE: extension includes the dot
  std::string extension = boost::to_lower_copy( boost::filesystem::extension( filename ) );
  std::string filename_base = filename.stem().string();
  
  // Special case if gzipped (ITK, possibly others...)
  // TODO: check to see if there's another extension
  if (extension == ".gz")
  {
    std::ostringstream oss;
    oss << boost::to_lower_copy( boost::filesystem::extension( filename.stem() ) ) << ".gz";
    extension = oss.str();
    filename_base = filename.stem().stem().string();
  }

  return std::make_tuple( extension, filename_base);
}


} // end namespace
