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

#ifndef CORE_DATABLOCK_ITKDATA_H
#define CORE_DATABLOCK_ITKDATA_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// ITK includes
#include <itkImage.h>
#include <itkImageFileReader.h>

// Boost includes
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

// Core includes
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Transform.h>
#include <Core/Geometry/GridTransform.h>
#include <Core/Geometry/Vector.h>

#include <Core/DataBlock/DataType.h>
#include <Core/DataBlock/DataBlock.h>

namespace Core
{

// Forward declarations
class ITKImageData;
typedef boost::shared_ptr< ITKImageData > ITKImageDataHandle;

// Class definition
class ITKImageData : public boost::noncopyable
{
public:
  typedef itk::ImageBase<3> image_base_type;

  // -- Constructor/destructor --
public:
  ITKImageData();
  virtual ~ITKImageData();

  // -- Accessors --
public:
  // GET_DATA:
  /// Get the pointer to the data block within the itkImage
  virtual void* get_data() const = 0;

  // GET_BASE_IMAGE:
  /// Get the pointer to the base image class
  virtual image_base_type::Pointer get_base_image() const = 0;

  // GRID_TRANSFORM:
  /// Extract the transform from the nrrd
  virtual GridTransform get_grid_transform() const = 0;

  // TRANSFORM:
  /// Extract the transform from the nrrd
  virtual Transform get_transform() const = 0;

  // SET_TRANSFORM:
  /// Set the transfrom in the nrrd data
  virtual void set_transform( Transform& transform ) = 0;

  // GET_NX, GET_NY, GET_NZ:
  /// Get the dimensions of the itkImage
  virtual size_t get_nx() const = 0;
  virtual size_t get_ny() const = 0;
  virtual size_t get_nz() const = 0;

  // GET_SIZE
  /// Get the total size of the pixel buffer
  virtual size_t get_size() const = 0;

  // GET_DATA_TYPE:
  /// Get the data type of the nrrd
  virtual DataType get_data_type() const = 0;
  
};

// Forward declaration
template<class T>
class ITKImageDataT;

// Class definition
template<class T>
class ITKImageDataT : public ITKImageData
{
  // -- Handle support --
public:
  typedef typename boost::shared_ptr< ITKImageDataT< T > > handle_type;
  typedef handle_type Handle;
  typedef T value_type;
  typedef itk::Image<T,3> image_type;
  typedef ITKImageData::image_base_type image_base_type;

  // -- Constructor/destructor --
public:
  /// Create the wrapper class from an itkImage smart pointer
  ITKImageDataT( typename image_type::Pointer itk_image );
  
  /// Create the wrapper class from a data block
  ITKImageDataT( DataBlockHandle data_block );
  ITKImageDataT( DataBlockHandle data_block, Transform transform );

  virtual ~ITKImageDataT();

private:
  bool initialize( DataBlockHandle data_block, Transform transform ) ;

  // -- Accessors --
public:
  // GET_TYPED_IMAGE:
  /// Return the itkImage class template to the derived class
  typename image_type::Pointer get_image() const;

  // GET_BASE_IMAGE:
  /// Get the pointer to the base image class
  virtual image_base_type::Pointer get_base_image() const;

  // GET_DATA:
  /// Get the pointer to the data block within the itkImage
  virtual void* get_data() const;

  // GET_TYPED_DATA:
  /// Get the pointer to the data block within the itkImage
  T* get_typed_data() const;

  // GRID_TRANSFORM:
  /// Extract the transform from the nrrd
  virtual GridTransform get_grid_transform() const;

  // TRANSFORM:
  /// Extract the transform from the nrrd
  virtual Transform get_transform() const;

  // SET_TRANSFORM:
  /// Set the transfrom in the nrrd data
  virtual void set_transform( Transform& transform );

  // GET_NX, GET_NY, GET_NZ:
  /// Get the dimensions of the itkImage
  virtual size_t get_nx() const;
  virtual size_t get_ny() const;
  virtual size_t get_nz() const;

  // GET_SIZE
  /// Get the total size of the pixel buffer
  virtual size_t get_size() const;

  // GET_DATA_TYPE:
  /// Get the data type of the nrrd
  virtual DataType get_data_type() const;

  // -- Internals of this class --
private:
  /// Smart pointer to the itk object
  typename image_type::Pointer itk_image_;

  /// Handle to a data_block that was wrapped in an itk object
  DataBlockHandle data_block_;

public:

  static bool LoadImage( const std::string& filename, handle_type& image_data, 
    std::string& error );
    
  static bool SaveImage( const std::string& filename, handle_type& nrrddata, 
    std::string& error );
};


// -- Define typed versions of the main template --

typedef ITKImageDataT<signed char> ITKCharImageData;
typedef ITKImageDataT<unsigned char> ITKUCharImageData;
typedef ITKImageDataT<signed short> ITKShortImageData;
typedef ITKImageDataT<unsigned short> ITKUShortImageData;
typedef ITKImageDataT<signed int> ITKIntImageData;
typedef ITKImageDataT<unsigned int> ITKUIntImageData;
typedef ITKImageDataT<float> ITKFloatImageData;
typedef ITKImageDataT<double> ITKDoubleImageData;

// -- Define the associated handles --

typedef ITKCharImageData::handle_type ITKCharImageDataHandle;
typedef ITKUCharImageData::handle_type ITKUCharImageDataHandle;
typedef ITKShortImageData::handle_type ITKShortImageDataHandle;
typedef ITKUShortImageData::handle_type ITKUShortImageDataHandle;
typedef ITKIntImageData::handle_type ITKIntImageDataHandle;
typedef ITKUIntImageData::handle_type ITKUIntImageDataHandle;
typedef ITKFloatImageData::handle_type ITKFloatImageDataHandle;
typedef ITKDoubleImageData::handle_type ITKDoubleImageDataHandle;

// -- Template class implementation --

template<class T>
ITKImageDataT<T>::ITKImageDataT( typename image_type::Pointer itk_image ) :
  itk_image_( itk_image )
{
}

template<class T>
ITKImageDataT<T>::ITKImageDataT( DataBlockHandle data_block ) :
  itk_image_( 0 )
{
  Transform transform;
  initialize( data_block, transform );
}

template<class T>
ITKImageDataT<T>::ITKImageDataT( DataBlockHandle data_block, Transform transform ) :
  itk_image_( 0 )
{
  initialize( data_block, transform );
}

template<class T>
ITKImageDataT<T>::~ITKImageDataT()
{
  itk_image_ = 0;
  this->data_block_.reset();
}

template<class T>
bool ITKImageDataT<T>::initialize( DataBlockHandle data_block, Transform transform )
{
  // Step (1) : convert image data to the right type
  // NOTE: This conversion is done on the fly here
  
  DataType image_data_type = GetDataType( reinterpret_cast<T*>(0) );
  if ( data_block->get_data_type() != image_data_type )
  {
    if ( ! DataBlock::ConvertDataType( data_block, data_block_, image_data_type ) )
    {
      data_block_.reset();
    }
  }
  else
  {
    data_block_ = data_block;
  }
  
  // If the data block could not be generated bail out of the code
  if ( ! data_block_ ) return false;

  // Step (2) : Use itk factory method for building new itk image object
  itk_image_ = image_type::New();
  
  // Step (3) : copy the dimensions of the data block
  typename image_type::RegionType region;
  region.SetSize( 0, data_block->get_nx() );
  region.SetSize( 1, data_block->get_ny() );
  region.SetSize( 2, data_block->get_nz() );
  itk_image_->SetRegions( region );

  // Step (4) : Wrap our data into an ITK pixel container
  typename image_type::PixelContainerPointer pixel_container = 
    image_type::PixelContainer::New();
  
  pixel_container->SetImportPointer( reinterpret_cast<T*>( data_block->get_data() ),
    static_cast<typename image_type::PixelContainer::ElementIdentifier >
    ( data_block->get_size() ), false );

  itk_image_->SetPixelContainer( pixel_container );

  // Step (5) : Add the transformation to the itk data
  set_transform( transform );

  return true;
}

template<class T>
typename ITKImageDataT<T>::image_type::Pointer ITKImageDataT<T>::get_image() const
{
  return itk_image_;
}

template<class T>
ITKImageDataT<T>::image_base_type::Pointer ITKImageDataT<T>::get_base_image() const
{
  return static_cast< image_base_type::Pointer >( itk_image_ );
}


template<class T>
void* ITKImageDataT<T>::get_data() const
{
  return reinterpret_cast< void* >( itk_image_->GetBufferPointer() );
}

template<class T>
T* ITKImageDataT<T>::get_typed_data() const
{
  return reinterpret_cast< T* >( itk_image_->GetBufferPointer() );
}

template<class T>
GridTransform ITKImageDataT<T>::get_grid_transform() const
{
  typename image_type::RegionType::SizeType size = itk_image_->GetBufferedRegion().GetSize();
  GridTransform gt( size[ 0 ], size[ 1 ], size[ 2 ], get_transform() );
  gt.set_originally_node_centered( false );
  return gt;
}
  
template<class T>
Transform ITKImageDataT<T>::get_transform() const
{
  typename image_type::PointType point = itk_image_->GetOrigin();
  Point origin( point[ 0 ], point[ 1 ], point[ 2 ] );

  // ITK direction type is matrix of direction cosines
  // Seg3D uses same LPS (Left, Posterior, Superior) 3D basis
  typename image_type::DirectionType direction = itk_image_->GetDirection();
  typename image_type::SpacingType spacing = itk_image_->GetSpacing();
  Vector direction_x( direction[ 0 ][ 0 ], direction[ 0 ][ 1 ], direction[ 0 ][ 2 ] );
  Vector direction_y( direction[ 1 ][ 0 ], direction[ 1 ][ 1 ], direction[ 1 ][ 2 ] );
  Vector direction_z( direction[ 2 ][ 0 ], direction[ 2 ][ 1 ], direction[ 2 ][ 2 ] );

  direction_x *= spacing[ 0 ];
  direction_y *= spacing[ 1 ];
  direction_z *= spacing[ 2 ];

  return Transform( origin, direction_x, direction_y, direction_z );
}

template<class T>
void ITKImageDataT<T>::set_transform( Transform& transform )
{
  Point origin = transform.project( GridTransform::DEFAULT_ORIGIN );
  Vector direction_x = transform.project( GridTransform::X_AXIS );
  Vector direction_y = transform.project( GridTransform::Y_AXIS );
  Vector direction_z = transform.project( GridTransform::Z_AXIS );
  
  double spacing_x = direction_x.length();
  double spacing_y = direction_y.length();
  double spacing_z = direction_z.length();
  
  direction_x.normalize();
  direction_y.normalize();
  direction_z.normalize();
  
  typename image_type::PointType point;
  point[ 0 ] = origin[ 0 ];
  point[ 1 ] = origin[ 1 ];
  point[ 2 ] = origin[ 2 ];
  
  itk_image_->SetOrigin( point );
  
  typename image_type::DirectionType direction;

  direction[ 0 ][ 0 ] = direction_x[ 0 ];
  direction[ 0 ][ 1 ] = direction_x[ 1 ];
  direction[ 0 ][ 2 ] = direction_x[ 2 ];

  direction[ 1 ][ 0 ] = direction_y[ 0 ];
  direction[ 1 ][ 1 ] = direction_y[ 1 ];
  direction[ 1 ][ 2 ] = direction_y[ 2 ];

  direction[ 2 ][ 0 ] = direction_z[ 0 ];
  direction[ 2 ][ 1 ] = direction_z[ 1 ];
  direction[ 2 ][ 2 ] = direction_z[ 2 ];

  itk_image_->SetDirection( direction );
  
  typename image_type::SpacingType spacing;
  spacing[ 0 ] =  spacing_x;
  spacing[ 1 ] =  spacing_y;
  spacing[ 2 ] =  spacing_z;
  itk_image_->SetSpacing( spacing );
}

template<class T>
size_t ITKImageDataT<T>::get_nx() const
{
  typename image_type::RegionType::SizeType size = itk_image_->GetBufferedRegion().GetSize();
  return size[ 0 ];
}

template<class T>
size_t ITKImageDataT<T>::get_ny() const
{
  typename image_type::RegionType::SizeType size = itk_image_->GetBufferedRegion().GetSize();
  return size[ 1 ];
}

template<class T>
size_t ITKImageDataT<T>::get_nz() const
{
  typename image_type::RegionType::SizeType size = itk_image_->GetBufferedRegion().GetSize();
  return size[ 2 ];
}

template<class T>
size_t ITKImageDataT<T>::get_size() const
{
  typename image_type::RegionType::SizeType size = itk_image_->GetBufferedRegion().GetSize();
  return size[ 0 ] * size[ 1 ] * size[ 2 ]; 
}

template<class T>
DataType ITKImageDataT<T>::get_data_type() const
{
  return GetDataType( reinterpret_cast<T*>( 0 ) );
}

template<class T>
bool ITKImageDataT<T>::LoadImage( const std::string& filename, 
  typename ITKImageDataT<T>::handle_type& image_data, std::string& error )
{
  typedef itk::ImageFileReader< image_type > reader_type;
  
  reader_type reader = typename reader_type::New();
  reader->SetFileName( filename.c_str() );
  
  try
  {
    reader->Update();
  }
  catch ( itk::ExceptionObject& err )
  {
    std::ostringstream oss;
    oss << "Could not read file using ITK reader: " << err << std::endl;
    error = oss.str();
    
    return false;
  }
  
  image_data = ITKImageDataT<T>::handle_type( new ITKImageDataT<T>( reader->GetOutput() ) );

  error = "";
  return true;
}

template<class T>
bool ITKImageDataT<T>::SaveImage( const std::string& filename, 
  typename ITKImageDataT<T>::handle_type& nrrddata, std::string& error )
{
  return false;
}

} // end namespace Core

#endif
