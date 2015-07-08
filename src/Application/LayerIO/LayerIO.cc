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

// STL includes
#include <algorithm>

// Boost includes
#include <boost/algorithm/minmax_element.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Utils/FilesystemUtil.h>

// Application includes
#include <Application/LayerIO/LayerIO.h>

namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( LayerIO );

const std::string LayerIO::DATA_MODE_C = "data";
const std::string LayerIO::SINGLE_MASK_MODE_C = "single_mask";
const std::string LayerIO::LABEL_MASK_MODE_C = "label_mask";
const std::string LayerIO::BITPLANE_MASK_MODE_C = "bitplane_mask";

class LayerIOPrivate
{
public:
  // The internal lists of importers
  std::vector< LayerImporterInfoHandle > single_file_importer_list_;
  std::vector< LayerImporterInfoHandle > file_series_importer_list_;
  
  // The internal list of exporters
  std::vector< LayerExporterInfoHandle > exporter_list_;
};

LayerIO::LayerIO() :
  private_( new LayerIOPrivate )
{
}

LayerIO::~LayerIO()
{
}

void LayerIO::register_importer_internal( LayerImporterInfoHandle info )
{
  // Lock the factory
  lock_type lock( this->get_mutex() );

  // Insert the information block into the correct importer list
  if ( info->get_type() == LayerImporterType::SINGLE_FILE_E )
  {
    this->private_->single_file_importer_list_.push_back( info );
  }
  else
  {
    this->private_->file_series_importer_list_.push_back( info );
  } 
}

void LayerIO::register_exporter_internal( LayerExporterInfoHandle info )
{
  // Lock the factory
  lock_type lock( this->get_mutex() );

  this->private_->exporter_list_.push_back( info );
}

std::vector< std::string > LayerIO::get_single_file_importer_types()
{
  // Lock the factory
  lock_type lock( this->get_mutex() );  

  std::vector<std::string> importer_types;
  importer_types.push_back( "All Importers (*)" );

  for ( size_t j = 0; j< this->private_->single_file_importer_list_.size(); j++ )
  {
    importer_types.push_back( 
      this->private_->single_file_importer_list_[ j ]->get_file_type_string() );
  }
  
  return importer_types;
}

std::vector< std::string > LayerIO::get_file_series_importer_types()
{
  // Lock the factory
  lock_type lock( this->get_mutex() );  

  std::vector<std::string> importer_types;
  importer_types.push_back( "All Importers (*)" );

  for ( size_t j = 0; j< this->private_->file_series_importer_list_.size(); j++ )
  {
    importer_types.push_back( 
      this->private_->file_series_importer_list_[ j ]->get_file_type_string() );
  }
  
  return importer_types;
}

std::vector< std::string > LayerIO::get_exporter_types()
{
  // Lock the factory
  lock_type lock( this->get_mutex() );  

  std::vector<std::string> exporter_types;
  exporter_types.push_back( "All Importers (*)" );

  for ( size_t j = 0; j< this->private_->exporter_list_.size(); j++ )
  {
    exporter_types.push_back( 
      this->private_->exporter_list_[ j ]->get_file_type_string() );
  }

  return exporter_types;
}

bool LayerIO::create_single_file_importer( const std::string& filename, 
    LayerImporterHandle& importer, std::string& error, 
    const std::string& full_importername )
{
  // Clear out old results
  importer.reset();
  error = "";

  // Check whether file exists
  boost::filesystem::path full_filename( filename );
  if ( ! boost::filesystem::exists( full_filename.parent_path() ) ) 
  {
    error = std::string( "File '" ) + full_filename.string() + "' does not exist.";
    return false;
  }

  // Determine the file extension  
  // NOTE: extension includes the dot
  std::string extension = Core::GetFullExtension( full_filename );

  // Lock the factory
  lock_type lock( this->get_mutex() );

  // Clean up the importer name.
  // TODO: Need to move this clean up to the Interface layer
  std::string importername = full_importername.substr( 0, full_importername.find( " (" ) );
  
  // If no name was given, the most appriopriate is used by
  // searching the list for the importer that deals with this
  // file type and has the highest priority
  if ( importername == "" || importername == "All Importers" )
  {
    int priority = 0;
    
    // Search the list for an approriate importer
    for (size_t j = 0; j < this->private_->single_file_importer_list_.size(); j++ )
    {
      if ( this->private_->single_file_importer_list_[ j ]->converts_file_type( extension, true ) &&  
         this->private_->single_file_importer_list_[ j ]->get_priority() >= priority )
      {
        try
        {
          // Create a new importer for this type
          LayerImporterHandle new_importer( 
            this->private_->single_file_importer_list_[ j ]->build() );
          // Upcast it to the right type so we can set the filename
          LayerSingleFileImporterHandle single_file_importer = 
            boost::dynamic_pointer_cast<LayerSingleFileImporter>( new_importer ); 

          // Test whether upcast succeeded
          if ( single_file_importer )
          {
            single_file_importer->set_filename( filename );

            if ( single_file_importer->check_files() )
            {
              importer = new_importer;
              priority = this->private_->single_file_importer_list_[ j ]->get_priority();
            }
          }
        }
        catch ( ... )
        {
          // Continue to look for another importer
        }
      }
    }   
    
    // If none with this extension is found, we look for one that accepts any extension.
    if ( !importer )
    {
      for (size_t j = 0; j < this->private_->single_file_importer_list_.size(); j++ )
      {
        if ( this->private_->single_file_importer_list_[ j ]->converts_file_type( extension, false ) &&  
           this->private_->single_file_importer_list_[ j ]->get_priority() >= priority )
        {
          try
          {
            // Create a new importer based on the information in the info classes that we
            // registered
            LayerImporterHandle new_importer( 
              this->private_->single_file_importer_list_[ j ]->build() );
            
            // Upcast handle so we can insert the filename
            LayerSingleFileImporterHandle single_file_importer = 
              boost::dynamic_pointer_cast<LayerSingleFileImporter>( new_importer ); 

            // Test whether upcast succeeded
            if ( single_file_importer )
            {
              single_file_importer->set_filename( filename );

              if ( single_file_importer->check_files() )
              {
                importer = new_importer;
                priority = this->private_->single_file_importer_list_[ j ]->get_priority();
              }
            }
          }
          catch ( ... )
          {
            // Continue to look for another importer
          }
        }
      }   
    }
  }
  // Else find the specific importer, this can be either through the name (using the
  // script) or through the file type string ( from the GUI)  
  else
  {
    for ( size_t j = 0; j < this->private_->single_file_importer_list_.size(); j++ )
    {
      if ( this->private_->single_file_importer_list_[ j ]->get_name() == importername &&
        this->private_->single_file_importer_list_[ j ]->converts_file_type( extension ) )
      {
        // Build the new importer
        importer = this->private_->single_file_importer_list_[ j ]->build();
        
        try
        {
          LayerSingleFileImporterHandle single_file_importer = 
            boost::dynamic_pointer_cast<LayerSingleFileImporter>( importer ); 

          // Check upcast, if correct set the file name
          if ( single_file_importer )
          {
            single_file_importer->set_filename( filename );
            break;
          }
        }
        catch ( ... )
        {
          importer.reset();
          // Continue to find another importer
        }
      }
    }
  }

  if ( importer )
  {
    return true;
  }
  else
  {
    error = std::string( "Could not find importer '" ) + importername + "'.";
    return false;
  }
}

bool LayerIO::create_file_series_importer( const std::vector< std::string >& filenames, 
    LayerImporterHandle& importer, std::string& error, const std::string& full_importername )
{
  // Clear out old results
  importer.reset();
  error = "";

  // Clean up the importer name.
  // TODO: Need to move this clean up to the Interface layer
  std::string importername = full_importername.substr( 0, full_importername.find( " (" ) );

  if ( filenames.size() == 0 )
  {
    error = "No filenames were given.";
    return false;
  }

  // Check whether file exists
  for ( size_t j = 0; j < filenames.size(); j++ )
  {
    boost::filesystem::path full_filename( filenames[ j ] );
    if ( ! ( boost::filesystem::exists( full_filename.parent_path() ) ) ) 
    {
      error = std::string( "File '" ) + full_filename.string() + "' does not exist.";
      return false;
    }
  }
  
  // Determine the file extension  
  // NOTE: extension includes the dot
  boost::filesystem::path full_filename( filenames[ 0 ] );
  std::string extension = boost::to_lower_copy( boost::filesystem::extension( full_filename ) );

  // Lock the factory
  lock_type lock( this->get_mutex() );
  
  // If no name was given, the most appriopriate is used by
  // searching the list for the importer that deals with this
  // file type and has the highest priority
  if ( importername == "" || importername == "All Importers" )
  {
    int priority = 0;
    
    // Search the list for an approriate importer
    for (size_t j = 0; j < this->private_->file_series_importer_list_.size(); j++ )
    {
      if ( this->private_->file_series_importer_list_[ j ]->converts_file_type( extension, true ) &&  
         this->private_->file_series_importer_list_[ j ]->get_priority() >= priority )
      {
        try
        {
          // Create a new importer
          LayerImporterHandle new_importer = 
            this->private_->file_series_importer_list_[ j ]->build();
          
          // Upcast it to the right type so we can insert the filenames
          LayerFileSeriesImporterHandle series_importer = 
            boost::dynamic_pointer_cast<LayerFileSeriesImporter>( new_importer );

          // Check upcast
          if ( series_importer )
          {
            series_importer->set_filenames( filenames );

            if ( series_importer->check_files() )
            {
              importer = new_importer;
              priority = this->private_->file_series_importer_list_[ j ]->get_priority();
            }
          }
        }
        catch ( ... )
        {
          // Continue to find an importer
        }
      }
    }   
    // If none with this extension is found, we look for one that accepts any extension.
    if ( ! importer )
    {
      for (size_t j = 0; j < this->private_->file_series_importer_list_.size(); j++ )
      {
        if ( this->private_->file_series_importer_list_[ j ]->converts_file_type( extension, false ) &&  
           this->private_->file_series_importer_list_[ j ]->get_priority() >= priority )
        {
          try
          {
            // Create a new importer
            LayerImporterHandle new_importer = 
              this->private_->file_series_importer_list_[ j ]->build();

            // Upcast it to the right type so we can insert the filenames
            LayerFileSeriesImporterHandle series_importer = 
              boost::dynamic_pointer_cast<LayerFileSeriesImporter>( new_importer );
              
            // Check upcast
            if ( series_importer )
            { 
              series_importer->set_filenames( filenames );

              if ( new_importer->check_files() )
              {
                importer = new_importer;
                priority = this->private_->file_series_importer_list_[ j ]->get_priority();
              }
            }
          }
          catch ( ... )
          {
            // Continue to find an importer
          }
        }
      }   
    }
  }
  // Else find the specific importer, this can be either through the name (using the
  // script) or through the file type string ( from the GUI)  
  else
  {
    for (size_t j = 0; j < this->private_->file_series_importer_list_.size(); j++ )
    {
      if ( this->private_->file_series_importer_list_[ j ]->get_name() == importername &&
        this->private_->file_series_importer_list_[ j ]->converts_file_type( extension ) )
      {
        try
        {
          // Create new importer
          importer = this->private_->file_series_importer_list_[ j ]->build();
          LayerFileSeriesImporterHandle series_importer = 
            boost::dynamic_pointer_cast<LayerFileSeriesImporter>( importer );
          // Insert the filenames
          if ( series_importer )
          {
            series_importer->set_filenames( filenames );
            break;
          }
        }
        catch ( ... )
        {
          // Continue to find another importer
          importer.reset();
        }
      }
    }
  }

  if ( importer )
  {
    return true;
  }
  else
  {
    error = std::string( "Could not find importer '" ) + importername + "'.";
    return false;
  }
}


bool LayerIO::create_exporter( LayerExporterHandle& exporter, std::vector< LayerHandle >& layers, 
  const std::string full_exportername, const std::string extension )
{
  // Clean up the importer name.
  // TODO: Need to move this clean up to the Interface layer
  std::string exportername = full_exportername.substr( 0, full_exportername.find( " (" ) );

  for ( size_t j = 0; j < this->private_->exporter_list_.size(); j++ )
  {
    if ( this->private_->exporter_list_[ j ]->get_name() == exportername &&
      this->private_->exporter_list_[ j ]->converts_file_type( extension ) )
    {
      exporter = this->private_->exporter_list_[ j ]->build( layers );
      exporter->set_extension( extension );
      return true;
    }
  }
  return false;
}

bool LayerIO::FindFileSeries( std::vector<std::string >& filenames )
{ 
  if ( filenames.size() == 1 )
  {
    // If we are able to create an importer, we then need to figure out which files are part
    // of the series.

    // Step 1: we get the get a boost::filesystem::path version of the file name so we can 
    // take advantage of the filesystem functionality
    boost::filesystem::path full_filename( filenames[ 0 ] );
    try
    {
      full_filename = boost::filesystem::absolute( full_filename );
    }
    catch( ... )
    {
      // In case we are not able to complete the name
      return false;
    }
    
    if ( ! boost::filesystem::exists( full_filename ) ) return false;
    
    // Step 3: now we want to see if we can figure out the file name pattern.  We will start by
    // checking to see if the sequence numbers are at the end of the file name.  
    std::string filename = boost::filesystem::basename( full_filename );
    std::string extension = Core::GetFullExtension( full_filename );
    
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
    std::vector< boost::filesystem::path > dir_full_filenames;
    if ( boost::filesystem::exists( full_filename.parent_path() ) )
    {
      boost::filesystem::directory_iterator dir_end;
      for( boost::filesystem::directory_iterator dir_itr( full_filename.parent_path() ); 
        dir_itr != dir_end; ++dir_itr )
      {
        dir_full_filenames.push_back( *dir_itr );
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
        std::string filename_extension = Core::GetFullExtension( dir_full_filenames[ k ] );

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
        std::string filename_extension = Core::GetFullExtension( dir_full_filenames[ k ] );
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
            filenames.push_back( dir_full_filenames[ k ].string() );
          }
        }
      }
    
      // NOTE: Sort will use the first and then the second item of the pair to sort
      std::sort( order.begin(), order.end() );
      
      std::vector<std::string> old_files = filenames;
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
  else
  {
    std::vector<std::string>::iterator it = filenames.begin();
    std::vector<std::string>::iterator it_end = filenames.end();
    
    std::vector<std::vector<size_t> > file_numbers;
    while ( it != it_end )
    {
      std::string filename = (*it);
      
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
        
        std::vector<std::string> old_files = filenames;
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
}

} // end namespace seg3D
