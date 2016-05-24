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

#ifndef CORE_DATABLOCK_ITKIMAGE2DDATA_H
#define CORE_DATABLOCK_ITKIMAGE2DDATA_H

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
class ITKImage2DData;
typedef boost::shared_ptr< ITKImage2DData > ITKImage2DDataHandle;

// Class definition   
class ITKImage2DData : public boost::noncopyable
{
public:
  typedef itk::ImageBase<2> image_base_type;

  // -- Constructor/destructor --
public:
  ITKImage2DData();
  virtual ~ITKImage2DData();

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
  virtual void set_transform( Transform& transform, SliceType slice = SliceType::AXIAL_E  ) = 0;

  // GET_NX, GET_NY:
  /// Get the dimensions of the itkImage
  virtual size_t get_nx() const = 0;
  virtual size_t get_ny() const = 0;

  // GET_SIZE
  /// Get the total size of the pixel buffer
  virtual size_t get_size() const = 0;

  // GET_DATA_TYPE:
  /// Get the data type of the nrrd
  virtual DataType get_data_type() const = 0;
  
};

// Forward declaration
template<class T>
class ITKImage2DDataT;

// Class definition
template<class T>
class ITKImage2DDataT : public ITKImage2DData
{
  // -- Handle support --
public:
  typedef typename boost::shared_ptr< ITKImage2DDataT< T > > handle_type;
  typedef handle_type Handle;
  typedef T value_type;
  typedef itk::Image<T,2> image_type;
  typedef ITKImage2DData::image_base_type image_base_type;

  // -- Constructor/destructor --
public:
  /// Create the wrapper class from an itkImage smart pointer
  ITKImage2DDataT( typename image_type::Pointer itk_image );
  
  /// Create the wrapper class from a data block
  ITKImage2DDataT( DataSliceHandle data_block );
  ITKImage2DDataT( DataSliceHandle data_block, Transform transform );

  virtual ~ITKImage2DDataT();

private:
  bool initialize( DataBlockHandle data_block, Transform transform, SliceType slice ) ;

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
  virtual void set_transform( Transform& transform, SliceType slice = SliceType::AXIAL_E );

  // GET_NX, GET_NY:
  /// Get the dimensions of the itkImage
  virtual size_t get_nx() const;
  virtual size_t get_ny() const;

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
};


// -- Define typed versions of the main template --

typedef ITKImage2DDataT<signed char> ITKCharImage2DData;
typedef ITKImage2DDataT<unsigned char> ITKUCharImage2DData;
typedef ITKImage2DDataT<signed short> ITKShortImage2DData;
typedef ITKImage2DDataT<unsigned short> ITKUShortImage2DData;
typedef ITKImage2DDataT<signed int> ITKIntImage2DData;
typedef ITKImage2DDataT<unsigned int> ITKUIntImage2DData;
typedef ITKImage2DDataT<float> ITKFloatImage2DData;
typedef ITKImage2DDataT<double> ITKDoubleImage2DData;

// -- Define the associated handles --

typedef ITKCharImage2DData::handle_type ITKCharImage2DDataHandle;
typedef ITKUCharImage2DData::handle_type ITKUCharImage2DDataHandle;
typedef ITKShortImage2DData::handle_type ITKShortImage2DDataHandle;
typedef ITKUShortImage2DData::handle_type ITKUShortImage2DDataHandle;
typedef ITKIntImage2DData::handle_type ITKIntImage2DDataHandle;
typedef ITKUIntImage2DData::handle_type ITKUIntImage2DDataHandle;
typedef ITKFloatImage2DData::handle_type ITKFloatImage2DDataHandle;
typedef ITKDoubleImage2DData::handle_type ITKDoubleImage2DDataHandle;

// -- Template class implementation --

template<class T>
ITKImage2DDataT<T>::ITKImage2DDataT( typename image_type::Pointer itk_image ) :
  itk_image_(itk_image)
{
}

template<class T>
ITKImage2DDataT<T>::ITKImage2DDataT( DataSliceHandle slice ) :
  itk_image_( 0 )
{
  Transform transform;
  initialize( slice->get_data_block(), transform, slice->get_slice_type() );
}

template<class T>
ITKImage2DDataT<T>::ITKImage2DDataT( DataSliceHandle slice, Transform transform ) :
  itk_image_( 0 )
{
  initialize( slice->get_data_block(), transform, slice->get_slice_type() );
}

template<class T>
ITKImage2DDataT<T>::~ITKImage2DDataT()
{
  itk_image_ = 0;
  this->data_block_.reset();
}

template<class T>
bool ITKImage2DDataT<T>::initialize( DataBlockHandle data_block, 
  Transform transform, SliceType slice )
{
  // Step (1) : convert image data to the right type
  // NOTE: This conversion is done on the fly here
  
  DataType image_data_type = GetDataType( reinterpret_cast<T*>(0) );
  if ( data_block->get_data_type() != image_data_type )
  {
    if ( !( DataBlock::ConvertDataType( data_block, data_block_, image_data_type ) ) )
    {
      data_block_.reset();
    }
  }
  else
  {
    data_block_ = data_block;
  }
  
  // If the data block could not be generated bail out of the code
  if ( !data_block_ ) return false;

  // Step (2) : Use itk factory method for building new itk image object
  itk_image_ = image_type::New();
  
  // Step (3) : copy the dimensions of the data block
  typename image_type::RegionType region;
  
  switch ( slice )
  {
    case SliceType::SAGITTAL_E:
      region.SetSize( 0, data_block->get_ny() );
      region.SetSize( 1, data_block->get_nz() );
      break;
    case SliceType::CORONAL_E:
      region.SetSize( 0, data_block->get_nx() );
      region.SetSize( 1, data_block->get_nz() );
      break;
    case SliceType::AXIAL_E:
      region.SetSize( 0, data_block->get_nx() );
      region.SetSize( 1, data_block->get_ny() );
      break;
  }
  itk_image_->SetRegions( region );

  // Step (4) : Wrap our data into an ITK pixel container
  typename image_type::PixelContainerPointer pixel_container = 
    image_type::PixelContainer::New();
  
  pixel_container->SetImportPointer( reinterpret_cast<T*>( data_block->get_data() ),
    static_cast<typename image_type::PixelContainer::ElementIdentifier >
    ( data_block->get_size() ), false );

  itk_image_->SetPixelContainer( pixel_container );

  // Step (5) : Add the transformation to the itk data
  set_transform( transform, slice );

  return true;
}

template<class T>
typename ITKImage2DDataT<T>::image_type::Pointer ITKImage2DDataT<T>::get_image() const
{
  return itk_image_;
}

template<class T>
ITKImage2DDataT<T>::image_base_type::Pointer ITKImage2DDataT<T>::get_base_image() const
{
  return static_cast< image_base_type::Pointer >( itk_image_ );
}


template<class T>
void* ITKImage2DDataT<T>::get_data() const
{
  return reinterpret_cast< void* >( itk_image_->GetBufferPointer() );
}

template<class T>
T* ITKImage2DDataT<T>::get_typed_data() const
{
  return reinterpret_cast< T* >( itk_image_->GetBufferPointer() );
}

template<class T>
GridTransform ITKImage2DDataT<T>::get_grid_transform() const
{
  typename image_type::RegionType::SizeType size = itk_image_->GetBufferedRegion().GetSize();
  return GridTransform( size[ 0 ], size[ 1 ], 1, get_transform() );
}
  
template<class T>
Transform ITKImage2DDataT<T>::get_transform() const
{
  typename image_type::PointType point = itk_image_->GetOrigin();
  Point origin( point[ 0 ], point[ 1 ], 0.0 );
  
  typename image_type::DirectionType direction = itk_image_->GetDirection();
  typename image_type::SpacingType spacing = itk_image_->GetSpacing();
  Vector direction_x( direction[ 0 ][ 0 ], direction[ 0 ][ 1 ], 0.0 );
  Vector direction_y( direction[ 1 ][ 0 ], direction[ 1 ][ 1 ], 0.0 );
  Vector direction_z( 0.0, 0.0, 1.0 );

  direction_x *= spacing[ 0 ];
  direction_y *= spacing[ 1 ];

  return Transform( origin, direction_x, direction_y, direction_z );
}

template<class T>
void ITKImage2DDataT<T>::set_transform( Transform& transform, SliceType slice )
{
  switch ( slice )
  {
    case SliceType::SAGITTAL_E :
    {
      Point origin = transform.project( Point( 0.0, 0.0, 0.0 ) );
      Vector direction_x = transform.project( Vector( 0.0, 1.0, 0.0 ) );
      Vector direction_y = transform.project( Vector( 0.0, 0.0, 1.0 ) );
      
      double spacing_x = direction_x.length();
      double spacing_y = direction_y.length();
      
      direction_x.normalize();
      direction_y.normalize();
      
      typename image_type::PointType point;
      point[ 0 ] = origin[ 1 ];
      point[ 1 ] = origin[ 2 ];
      
      itk_image_->SetOrigin( point );
      
      typename image_type::DirectionType direction;

      direction[ 0 ][ 0 ] = direction_x[ 1 ];
      direction[ 0 ][ 1 ] = direction_x[ 2 ];

      direction[ 1 ][ 0 ] = direction_y[ 1 ];
      direction[ 1 ][ 1 ] = direction_y[ 2 ];

      itk_image_->SetDirection( direction );
      
      typename image_type::SpacingType spacing;
      spacing[ 0 ] =  spacing_x;
      spacing[ 1 ] =  spacing_y;
      itk_image_->SetSpacing( spacing );
    }
    break;
    
    case SliceType::CORONAL_E :
    {
      Point origin = transform.project( Point( 0.0, 0.0, 0.0 ) );
      Vector direction_x = transform.project( Vector( 1.0, 0.0, 0.0 ) );
      Vector direction_y = transform.project( Vector( 0.0, 0.0, 1.0 ) );
      
      double spacing_x = direction_x.length();
      double spacing_y = direction_y.length();
      
      direction_x.normalize();
      direction_y.normalize();
      
      typename image_type::PointType point;
      point[ 0 ] = origin[ 0 ];
      point[ 1 ] = origin[ 2 ];
      
      itk_image_->SetOrigin( point );
      
      typename image_type::DirectionType direction;

      direction[ 0 ][ 0 ] = direction_x[ 0 ];
      direction[ 0 ][ 1 ] = direction_x[ 2 ];

      direction[ 1 ][ 0 ] = direction_y[ 0 ];
      direction[ 1 ][ 1 ] = direction_y[ 2 ];

      itk_image_->SetDirection( direction );
      
      typename image_type::SpacingType spacing;
      spacing[ 0 ] =  spacing_x;
      spacing[ 1 ] =  spacing_y;
      itk_image_->SetSpacing( spacing );
    }
    break;    
    
    case SliceType::AXIAL_E :
    {
      Point origin = transform.project( Point( 0.0, 0.0, 0.0 ) );
      Vector direction_x = transform.project( Vector( 1.0, 0.0, 0.0 ) );
      Vector direction_y = transform.project( Vector( 0.0, 1.0, 0.0 ) );
      
      double spacing_x = direction_x.length();
      double spacing_y = direction_y.length();
      
      direction_x.normalize();
      direction_y.normalize();
      
      typename image_type::PointType point;
      point[ 0 ] = origin[ 0 ];
      point[ 1 ] = origin[ 1 ];
      
      itk_image_->SetOrigin( point );
      
      typename image_type::DirectionType direction;

      direction[ 0 ][ 0 ] = direction_x[ 0 ];
      direction[ 0 ][ 1 ] = direction_x[ 1 ];

      direction[ 1 ][ 0 ] = direction_y[ 0 ];
      direction[ 1 ][ 1 ] = direction_y[ 1 ];

      itk_image_->SetDirection( direction );
      
      typename image_type::SpacingType spacing;
      spacing[ 0 ] =  spacing_x;
      spacing[ 1 ] =  spacing_y;
      itk_image_->SetSpacing( spacing );
    }
    break;
  }
}

template<class T>
size_t ITKImage2DDataT<T>::get_nx() const
{
  typename image_type::RegionType::SizeType size = itk_image_->GetBufferedRegion().GetSize();
  return size[ 0 ];
}

template<class T>
size_t ITKImage2DDataT<T>::get_ny() const
{
  typename image_type::RegionType::SizeType size = itk_image_->GetBufferedRegion().GetSize();
  return size[ 1 ];
}

template<class T>
size_t ITKImage2DDataT<T>::get_size() const
{
  typename image_type::RegionType::SizeType size = itk_image_->GetBufferedRegion().GetSize();
  return size[ 0 ] * size[ 1 ]; 
}

template<class T>
DataType ITKImage2DDataT<T>::get_data_type() const
{
  return GetDataType( reinterpret_cast<T*>( 0 ) );
}

} // end namespace Core

#endif
