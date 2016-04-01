/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
#include <Application/LayerIO/LayerSingleFileImporter.h>

// Core includes
#include <Core/Utils/FilesystemUtil.h>

#include <tuple>

namespace Seg3D {

class LayerSingleFileImporterPrivate
{
public:
  std::string filename_;
};

LayerSingleFileImporter::LayerSingleFileImporter() :
  private_( new LayerSingleFileImporterPrivate )
{
}

LayerSingleFileImporter::~LayerSingleFileImporter()
{
}

std::string LayerSingleFileImporter::get_filename() const
{
  return this->private_->filename_;
}

std::vector<std::string> LayerSingleFileImporter::get_filenames() const
{
  std::vector<std::string> filenames( 1, this->private_->filename_ );
  return filenames;
}

std::string LayerSingleFileImporter::get_file_tag() const
{
  if ( this->private_->filename_.size() == 0 ) return "";

  boost::filesystem::path full_filename( this->private_->filename_ );
  std::string ext, base;
  std::tie( ext, base ) = Core::GetFullExtension( full_filename );
  if ( boost::filesystem::extension(ext) != "" )
  {
    return full_filename.stem().stem().string();
  }

  return base;
}

void LayerSingleFileImporter::set_filename( const std::string& filename )
{
  try
  {
    boost::filesystem::path full_filename( filename );
    full_filename = boost::filesystem::absolute( full_filename );
    this->private_->filename_ = full_filename.string();
  }
  catch( ... )
  {
    this->private_->filename_ = filename;
  }
}

bool LayerSingleFileImporter::check_files()
{
  boost::filesystem::path full_filename( this->private_->filename_ );
  
  if ( ! boost::filesystem::exists( full_filename ) )
  {
    this->set_error( std::string( "File '" ) + full_filename.string() + "' does not exist." );
    return false;
  }
  
  if ( ! boost::filesystem::is_regular_file( full_filename ) )
  {
    this->set_error( std::string( "File '" ) + full_filename.string() + "' is not a regular file." );
    return false; 
  }
  
  return true;
}


InputFilesImporterHandle LayerSingleFileImporter::get_inputfiles_importer()
{
  InputFilesImporterHandle importer( new InputFilesImporter( this->get_inputfiles_id() ) );
  try
  {
    boost::filesystem::path full_filename( this->private_->filename_ );
    importer->add_filename( full_filename );
  }
  catch ( ... )
  {
    this->set_error( std::string( "Could not resolve filename '" ) + 
      this->private_->filename_ + "'." );
  }
  
  return importer;
}


LayerImporterType LayerSingleFileImporter::GetType() 
{ 
  return LayerImporterType::SINGLE_FILE_E; 
}

} // end namespace Seg3D
