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

// Core includes
#include <Core/DataBlock/ITKDataBlock.h>
#include <Core/DataBlock/DataBlockManager.h>

namespace Core
{

class ITKDataBlockPrivate : public boost::noncopyable
{
public:
  // Location where the original nrrd is stored
  ITKImageDataHandle itk_image_data_;
  ITKImage2DDataHandle itk_image2d_data_;
};


ITKDataBlock::ITKDataBlock( ITKImageDataHandle itk_image_data ) :
  private_( new ITKDataBlockPrivate )
{
  this->private_->itk_image_data_ = itk_image_data;

  set_nx( itk_image_data->get_nx() );
  set_ny( itk_image_data->get_ny() );
  set_nz( itk_image_data->get_nz() );
  set_type( itk_image_data->get_data_type() );
  set_data( itk_image_data->get_data() );
}

ITKDataBlock::ITKDataBlock( ITKImage2DDataHandle itk_image_data, SliceType slice ) :
  private_( new ITKDataBlockPrivate )
{
  this->private_->itk_image2d_data_ = itk_image_data;

  if ( slice == SliceType::AXIAL_E )
  {
    set_nx( itk_image_data->get_nx() );
    set_ny( itk_image_data->get_ny() );
    set_nz( 1 );
  }
  else if ( slice == SliceType::CORONAL_E )
  {
    set_nx( 1 );
    set_ny( itk_image_data->get_nx() );
    set_nz( itk_image_data->get_ny() ); 
  }
  else
  {
    set_nx( itk_image_data->get_nx() );
    set_ny( 1 );
    set_nz( itk_image_data->get_ny() );   
  }
  
  set_type( itk_image_data->get_data_type() );
  set_data( itk_image_data->get_data() );
}

ITKDataBlock::~ITKDataBlock()
{
}

DataBlockHandle ITKDataBlock::New( ITKImageDataHandle itk_data )
{
  try
  {
    DataBlockHandle data_block( new ITKDataBlock( itk_data ) );
    return data_block;
  }
  catch ( ... )
  {
    // Return an empty handle
    DataBlockHandle data_block;
    return data_block;
  }
}

DataBlockHandle ITKDataBlock::New( ITKImage2DDataHandle itk_data, SliceType slice )
{
  try
  {
    DataBlockHandle data_block( new ITKDataBlock( itk_data, slice ) );
    return data_block;
  }
  catch ( ... )
  {
    // Return an empty handle
    DataBlockHandle data_block;
    return data_block;
  }
}


} // end namespace Core
