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

// STL includes
#include <algorithm>

// Boost includes
#include <boost/algorithm/minmax_element.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>


// Core includes
#include <Core/Utils/FileUtil.h>
#include <Core/Utils/StringUtil.h>

namespace Core
{


bool FileUtil::FindFileSeries( const boost::filesystem::path& first_file, std::vector<boost::filesystem::path >& filenames, std::string& error )
{
  filenames.clear();
  error = "";

  // If we are able to create an importer, we then need to figure out which files are part
  // of the series.

  // Step 1: we get the get a boost::filesystem::path version of the file name so we can 
  // take advantage of the filesystem functionality

  boost::filesystem::path full_filename;
  try
  {
    full_filename = boost::filesystem::absolute( first_file );
  }
  catch( ... )
  {
    // In case we are not able to complete the name
    error = "File '" + full_filename.string() + "' does not exist.";
    return false;
  }
    
  if ( ! boost::filesystem::exists( full_filename ) )
  {
    error = "File '" + full_filename.string() + "' does not exist.";
    return false;
  }
    
  // Step 3: now we want to see if we can figure out the file name pattern.  We will start by
  // checking to see if the sequence numbers are at the end of the file name.  
  std::string filename = boost::filesystem::basename( full_filename );
  std::string extension = boost::to_lower_copy( boost::filesystem::extension( full_filename ) );
    
  // Create a vector for finding numbers in the filename, for each one we will scan if there
  // is a sequential list available. The next vector denotes from where to where numbers can
  // be found.
  std::vector<std::pair<size_t,size_t> > numbers;
    
  // Scan through the filename
  size_t j = 0;
  while ( j < filename.size() )
  {
    size_t start, end;
    while ( j < filename.size() && ( filename[ j ] < '0' || filename[ j ] > '9' ) ) j++;
    if ( j == filename.size() ) break;
    start = j;
    while ( j < filename.size() && ( filename[ j ] >= '0' && filename[ j ] <= '9' ) ) j++;
    end = j;      
    if ( start != end ) numbers.push_back( std::make_pair( start, end ) );
  }
    
  std::vector<std::string> dir_files;
  std::vector< boost::filesystem::path > dir_first_filename;
  if ( boost::filesystem::exists( first_file.parent_path() ) )
  {
    boost::filesystem::directory_iterator dir_end;
    for( boost::filesystem::directory_iterator dir_itr( first_file.parent_path() ); 
      dir_itr != dir_end; ++dir_itr )
    {
      dir_first_filename.push_back( *dir_itr );
      dir_files.push_back( boost::filesystem::basename( dir_itr->path().filename().string() ) );
    }
  }

  // Now scan through the complete list of files that were found at the same location as the
  // first file.
  size_t index = 0;
  size_t num_files = 0;

  // For each number location find how many other files match. We then we choose the largest
  // consecutive numbering.
  for ( size_t j = 0; j < numbers.size(); j++ )
  {
    size_t start = numbers[ j ].first;
    size_t end = filename.size() - numbers[ j ].second;

    std::string filename_prefix = filename.substr( 0, start );
    std::string filename_postfix = filename.substr( filename.size() - end );
      
    std::set<size_t> indices;
      
    for ( size_t k = 0; k < dir_files.size(); k++ )
    {
      std::string filename_extension = boost::to_lower_copy( boost::filesystem::extension( dir_first_filename[ k ]  ) );

      if ( filename_extension == extension &&
        dir_files[ k ].substr( 0, start ) == filename_prefix &&
        ( dir_files[ k ].size() >= end ) &&
        dir_files[ k ].substr( dir_files[ k ].size() - end ) == filename_postfix )
      {
        size_t val;
        if ( dir_files[ k ].size() >= end + start  && 
          Core::ImportFromString( dir_files[ k ].substr( start, 
            dir_files[ k ].size() - end - start ), val ) )
        {
          indices.insert( val );
        }
      }
    }
      
    if ( num_files < indices.size() )
    {
      index = j;
      num_files = indices.size();
    }
  }

  // In case we found matching files, try to order them correctly
  if ( numbers.size() && num_files ) 
  {
    // order contains the remapping [ actual value in series, rank in vector ]
    std::vector<std::pair<size_t,size_t> > order( num_files );
      
    size_t start = numbers[ index ].first;
    size_t end = filename.size() - numbers[ index ].second;
      
    std::string filename_prefix = filename.substr( 0, start );
    std::string filename_postfix = filename.substr( filename.size() - end );
      
    size_t  j = 0;
      
    filenames.clear();
      
    for ( size_t k = 0; k < dir_files.size(); k++ )
    {
      std::string filename_extension = boost::to_lower_copy( boost::filesystem::extension( dir_first_filename[ k ]  ) );
      if ( filename_extension == extension &&
        dir_files[ k ].substr( 0, start ) == filename_prefix &&
        dir_files[ k ].size() >= end &&
        dir_files[ k ].substr( dir_files[ k ].size() - end ) == filename_postfix )
      {
        size_t val;
        if ( dir_files[ k ].size() >= end + start &&  
          Core::ImportFromString( dir_files[ k ].substr( start, 
          dir_files[ k ].size() - end - start ), val ) )
        {
          order[ j ].first = val;
          order[ j ].second = j;
          j++;
          filenames.push_back( dir_first_filename[ k ].string() );
        }
      }
    }
    
    // NOTE: Sort will use the first and then the second item of the pair to sort
    std::sort( order.begin(), order.end() );
      
    std::vector<boost::filesystem::path> old_files = filenames;
    // Remap the files in the right order
    for ( size_t j = 0; j < order.size(); j++ )
    {
      filenames[ j ] = old_files[ order[ j ].second ];
    }
      
    return  true;
  }
  else
  {
    // The filename that is there is the only one, so use that one
    return true;
  }
}

bool FileUtil::OrderFileSeries( const std::vector<boost::filesystem::path>& files, std::vector<boost::filesystem::path >& filenames, std::string& error )
{
  error = "";
  filenames = files;

  std::vector<boost::filesystem::path>::iterator it = filenames.begin();
  std::vector<boost::filesystem::path>::iterator it_end = filenames.end();
    
  std::vector<std::vector<size_t> > file_numbers;
  while ( it != it_end )
  {
    std::string filename = (*it).string();
      
    for ( size_t j = 0; j < filename.size(); j++ )
    {
      if ( filename[ j ] < '0' || filename[ j ] > '9' ) filename[ j ] = ' ';
    }

    std::vector<size_t> filename_numbers;
    Core::ImportFromString( filename, filename_numbers );
    file_numbers.push_back( filename_numbers );
    ++it;
  }
    
  size_t max_size = 0;
  for ( size_t j = 0; j < file_numbers.size(); j++ )
  {
    if ( file_numbers[ j ].size() > max_size ) max_size = file_numbers[ j ].size();
  }

  if ( max_size )
  {
    std::vector<std::set<size_t> > numbers( 2*max_size );
      
    for ( size_t j = 0; j < file_numbers.size(); j++ )
    {
      for ( size_t k = 0; k < file_numbers[ j ].size(); k++ )
      {
        numbers[ k ].insert( file_numbers[ j ][ k ] );
        numbers[ max_size + file_numbers[ j ].size() - 1 - k ].insert( 
          file_numbers[ j ][ k ] );
      }
    }

    size_t index = 0;
    std::pair< std::set<size_t>::iterator, std::set<size_t>::iterator > min_max = 
      boost::minmax_element( numbers[ 0 ].begin(), numbers[ 0 ].end() );
    size_t index_range = (*min_max.second) - (*min_max.first);
    size_t index_size = numbers[ 0 ].size();
      
    for ( size_t j = 0; j < numbers.size(); j++ )
    {
      min_max = boost::minmax_element( numbers[ j ].begin(), numbers[ j ].end() );
      size_t range = (*min_max.second) - (*min_max.first);
      size_t size = numbers[ j ].size();
        
      if ( ( size > index_size ) || ( size == index_size && range < index_range ) )
      {
        index = j;
        index_size = size;
        index_range = range;
      }
    }
      
    if ( index_size != filenames.size() )
    {
      // Not enough for sorting, use alphabetical order
      std::sort( filenames.begin(), filenames.end() );
      return true;
    }
    else
    {
      std::vector<std::pair<size_t,size_t> > order( filenames.size() );
      if ( index < max_size )
      {
        for ( size_t j = 0; j < order.size(); j++ )
        {
          order[ j ].second = j;
          order[ j ].first = file_numbers[ j ][ index ];
        }
      }
      else
      {
        for ( size_t j = 0; j < order.size(); j++ )
        {
          order[ j ].second = j;
          order[ j ].first = file_numbers[ j ][ file_numbers[ j ].size() - 1 - 
            ( index - max_size ) ];
        }
      }
        
      std::sort( order.begin(), order.end() );
        
      std::vector<boost::filesystem::path> old_files = filenames;
      for ( size_t j = 0; j < order.size(); j++ )
      {
        filenames[ j ] = old_files[ order[ j ].second ];
      }
      return true;
    }
  }
  else
  {
    // no numbers for sorting use alphabetical order
    std::sort( filenames.begin(), filenames.end() );
    return true;
  }
}


bool FileUtil::CheckExtension ( const boost::filesystem::path& file, const std::string& extensions )
{
  std::string ext = file.extension().string();
  boost::to_lower( ext );
  
  std::vector<std::string> ext_vec = SplitString( boost::to_lower_copy( extensions ), "|" );

  for ( size_t j = 0; j < ext_vec.size(); j++ )
  {
    if ( ext_vec[ j ] == ext ) return true;
  }

  return false;
}

} // end namespace
