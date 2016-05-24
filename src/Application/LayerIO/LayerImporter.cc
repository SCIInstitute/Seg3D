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

// Application includes
#include <Application/LayerIO/LayerImporter.h>
#include <Application/ProjectManager/ProjectManager.h>

namespace Seg3D
{

class LayerImporterPrivate
{
public:
  std::string error_; 
  std::string warning_; 
  
  InputFilesID inputfiles_id_;
};

LayerImporter::LayerImporter() :
  private_( new LayerImporterPrivate )
{
  // Set id to an invalid id.
  this->private_->inputfiles_id_ = -1;
}

LayerImporter::~LayerImporter()
{
}

void LayerImporter::set_error( const std::string& error )
{
  this->private_->error_ = error;
}

std::string LayerImporter::get_error() const
{
  return this->private_->error_;
}

void LayerImporter::set_warning( const std::string& warning )
{
  this->private_->warning_ = warning;
}

std::string LayerImporter::get_warning() const
{
  return this->private_->warning_;
}

void LayerImporter::set_dicom_swap_xyspacing_hint( bool )
{
}

InputFilesID LayerImporter::get_inputfiles_id()
{
  if ( this->private_->inputfiles_id_ == -1 )
  {
    ProjectHandle current_project = ProjectManager::Instance()->get_current_project();

    this->private_->inputfiles_id_ = current_project->inputfiles_count_state_->get() + 1;
    current_project->inputfiles_count_state_->set( this->private_->inputfiles_id_ );  

  }

  return this->private_->inputfiles_id_;
}


} // end namespace Seg3D
