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
#include <Application/LayerIO/LayerImporterFileInfo.h>

namespace Seg3D
{

class LayerImporterFileInfoPrivate
{
public:
  LayerImporterFileInfoPrivate() :
    data_type_( Core::DataType::UNKNOWN_E ),
    mask_compatible_( false )
  {
  }
    
  // The transform of the data contained in the file
  Core::GridTransform grid_transform_;
  
  // The data type of the data contained in the file
  Core::DataType data_type_;

  // File type that is being imported
  std::string file_type_;
  
  // Whether the data can be converted to a mask
  bool mask_compatible_;
};


LayerImporterFileInfo::LayerImporterFileInfo() :
  private_( new LayerImporterFileInfoPrivate )
{ 
}

LayerImporterFileInfo::~LayerImporterFileInfo()
{
}

const Core::GridTransform& LayerImporterFileInfo::get_grid_transform() const
{
  return this->private_->grid_transform_;
}

void LayerImporterFileInfo::set_grid_transform( const Core::GridTransform& grid_transform )
{
  this->private_->grid_transform_ = grid_transform;
}

const Core::DataType& LayerImporterFileInfo::get_data_type() const
{
  return this->private_->data_type_;
}
  
void LayerImporterFileInfo::set_data_type( const Core::DataType& data_type )
{
  this->private_->data_type_ = data_type;
}
  
const std::string& LayerImporterFileInfo::get_file_type()
{
  return this->private_->file_type_;
}
  
void LayerImporterFileInfo::set_file_type( const std::string& file_type )
{
  this->private_->file_type_ = file_type;
}
  
bool LayerImporterFileInfo::get_mask_compatible() const
{
  // If the file format generally stores masks and if the data is integer
  return this->private_->mask_compatible_ && IsInteger( this->private_->data_type_ );
}

void LayerImporterFileInfo::set_mask_compatible( bool mask_compatible )
{
  this->private_->mask_compatible_ = mask_compatible;
}

} // end namespace seg3D
