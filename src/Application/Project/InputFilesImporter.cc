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
#include <vector>

// Boost includes
#include <boost/function.hpp>
#include <boost/filesystem.hpp>

#include <Application/Project/InputFilesImporter.h>

namespace Seg3D
{

class InputFilesImporterPrivate
{
public:
  // Provenance_id of this transfer
  InputFilesID inputfiles_id_;

  // Names of the files that need to be copied
  std::vector<boost::filesystem::path> filenames_;
  
  // Function used to copy the file
  boost::function<bool( const boost::filesystem::path&, const boost::filesystem::path& )> copy_file_function_;
};

  
InputFilesImporter::InputFilesImporter( InputFilesID inputfiles_id ) :
  private_( new InputFilesImporterPrivate )
{
  this->private_->inputfiles_id_ = inputfiles_id;
}

InputFilesImporter::~InputFilesImporter()
{
}

void InputFilesImporter::add_filename( const boost::filesystem::path& filename )
{
  this->private_->filenames_.push_back( filename );
}

void InputFilesImporter::set_copy_file_function( boost::function
  < bool( const boost::filesystem::path&, const boost::filesystem::path& ) > copy_file_function )
{
  this->private_->copy_file_function_ = copy_file_function;
}


InputFilesID InputFilesImporter::get_inputfiles_id()
{
  return this->private_->inputfiles_id_;
}

bool InputFilesImporter::copy_files( boost::filesystem::path& project_cache_path )
{
  if ( this->private_->copy_file_function_ )
  {
    for( size_t j = 0; j < this->private_->filenames_.size(); j++ )
    {
      try
      {
        this->private_->copy_file_function_( this->private_->filenames_[ j ], 
          project_cache_path / this->private_->filenames_[ j ].filename() );
      }
      catch( ... )
      {
        return false;
      }
    } 
  
  }
  else
  {
    for( size_t j = 0; j < this->private_->filenames_.size(); j++ )
    {
      try
      {
        boost::filesystem::copy_file( this->private_->filenames_[ j ], 
          project_cache_path / this->private_->filenames_[ j ].filename() );
      }
      catch( ... )
      {
        return false;
      }
    } 
  }
  
  return true;
}

} // end namespace seg3D

