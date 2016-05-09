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

// Application Includes
#include <Application/LayerIO/LayerFileSeriesImporter.h>

namespace Seg3D {

class LayerFileSeriesImporterPrivate
{
public:
  std::vector< std::string > filenames_;
};

LayerFileSeriesImporter::LayerFileSeriesImporter() :
  private_( new LayerFileSeriesImporterPrivate )
{
}

LayerFileSeriesImporter::~LayerFileSeriesImporter()
{
}

std::string LayerFileSeriesImporter::get_filename() const
{
  if ( this->private_->filenames_.size() == 0) return "";
  else return this->private_->filenames_[ 0 ];
}

std::vector< std::string > LayerFileSeriesImporter::get_filenames() const
{
  // Return the vector with file names
  return this->private_->filenames_;
}

std::string LayerFileSeriesImporter::get_file_tag() const
{
  if ( this->private_->filenames_.size() == 0 ) return "";
  boost::filesystem::path full_filename( this->private_->filenames_[ 0 ] );
  return full_filename.parent_path().stem().string();
}


void LayerFileSeriesImporter::set_filenames( const std::vector< std::string >& filenames )
{
  try
  {
    for ( size_t j = 0 ; j < filenames.size(); j++ )
    {
      boost::filesystem::path full_filename( filenames[ j ] );
      full_filename = boost::filesystem::absolute( full_filename );
      this->private_->filenames_.push_back( full_filename.string() );
    }
  }
  catch( ... )
  {
    this->private_->filenames_.clear();
  }
}

bool LayerFileSeriesImporter::check_files()
{
  for ( size_t j = 0; j < this->private_->filenames_.size(); j++ )
  {
    boost::filesystem::path full_filename( this->private_->filenames_[ j ] ); 
    if ( ! boost::filesystem::exists( full_filename ) )
    {
      this->set_error( std::string( "File '" ) + this->private_->filenames_[ j ]
        + "' does not exist." );
      return false;
    }
    
    if ( ! boost::filesystem::is_regular_file( full_filename ) )
    {
      this->set_error( std::string( "File '" ) + this->private_->filenames_[ j ]
        + "' is not a regular file." );
      return false; 
    }
  }
  return true;
}

InputFilesImporterHandle LayerFileSeriesImporter::get_inputfiles_importer()
{
  InputFilesImporterHandle importer( new InputFilesImporter( this->get_inputfiles_id() ) );
  for ( size_t j = 0; j < this->private_->filenames_.size(); j++ )
  {
    try
    {
      boost::filesystem::path full_filename( this->private_->filenames_[ j ] );
      importer->add_filename( full_filename );
    }
    catch ( ... )
    {
      this->set_error( std::string( "Could not resolve filename '" ) + 
        this->private_->filenames_[ j ] + "'." );
    }
  }
  
  return importer;
}


LayerImporterType LayerFileSeriesImporter::GetType()
{ 
  return LayerImporterType::FILE_SERIES_E; 
}

} // end namespace Seg3D
