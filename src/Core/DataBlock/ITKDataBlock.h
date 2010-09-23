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

#ifndef CORE_DATABLOCK_ITKDATABLOCK_H
#define CORE_DATABLOCK_ITKDATABLOCK_H

// Core includes
#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/ITKImageData.h>

namespace Core
{

// Forward Declaration
class ITKDataBlock;
typedef boost::shared_ptr< ITKDataBlock > ITKDataBlockHandle;

class ITKDataBlockPrivate;
typedef boost::shared_ptr< ITKDataBlockPrivate > ITKDataBlockPrivateHandle;

// Class definition
class ITKDataBlock : public DataBlock
{
  // -- Constructor/destructor --
private:
  ITKDataBlock( ITKImageDataHandle itk_data );
public: 
  virtual ~ITKDataBlock();

  // -- Internal implementation of this class --
private:
  ITKDataBlockPrivateHandle private_;
  
public: 
  // NEW:
  // Constructor of a new data block using the ITKImageData wrapper class.
  static DataBlockHandle New( ITKImageDataHandle itk_data, generation_type generation = -1 );

  // NEW:
  // Constructor of a new data block using an itk image pointer.
  template< class T >
  static DataBlockHandle New( typename itk::Image<T,3>::Pointer itk_image, generation_type generation = -1 )
  {
    // Create a wrapper class.
    typename ITKImageDataT<T>::Handle itk_data = 
      typename ITKImageDataT<T>::Handle( new ITKImageDataT<T>( itk_image) );
    // Use the wrapper class to generate the data block.
    return New( itk_data, generation );
  }

  // NEW:
  // Constructor of a new data block using an itk image pointer.
  template< class T >
  static DataBlockHandle New( typename itk::Image<T,3>* itk_image, generation_type generation = -1 )
  {
    // Create a wrapper class.
    typename ITKImageDataT<T>::Handle itk_data = 
      typename ITKImageDataT<T>::Handle( 
        new ITKImageDataT<T>( 
          typename itk::Image<T,3>::Pointer( itk_image) ) );
    // Use the wrapper class to generate the data block.
    return New( itk_data, generation );
  }
};

} // end namespace Core

#endif
