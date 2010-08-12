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
 
#ifndef APPLICATION_TOOL_ITKFILTERTOOL_H 
#define APPLICATION_TOOL_ITKFILTERTOOL_H 
 
// Boost includes
#include <boost/smart_ptr.hpp> 
#include <boost/utility.hpp> 
 
// Core includes
#include <Core/Volume/DataVolume.h>
#include <Core/Volume/MaskVolume.h>
#include <Core/DataBlock/ITKImageData.h>
#include <Core/DataBlock/ITKDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
 
// Application includes
#include <Application/Layer/DataLayer.h> 
#include <Application/Layer/MaskLayer.h> 
 
namespace Seg3D
{

class ITKFilterAlgo : public boost::noncopyable
{

public:
  ITKFilterAlgo();
  virtual ~ITKFilterAlgo();
    
protected:

  // RETRIEVE_UCHAR_ITK_IMAGE:
  // Retrieve an itk image from a data layer
  bool retrieve_uchar_itk_image( const MaskLayerHandle& mask, Core::ITKUCharImageDataHandle& image )
  {
    Core::MaskVolumeHandle volume = mask->get_mask_volume();

    Core::DataBlockHandle data_block;
    if ( ! ( Core::MaskDataBlockManager::Convert( volume->get_mask_data_block(), data_block ) ) )
    {
      return false;
    }

    // Create a new ITK image
    image = Core::ITKUCharImageDataHandle( new Core::ITKUCharImageData( data_block, 
      volume->get_transform() ) );

    if ( !image ) return false;
    return true;  
  }

  // RETRIEVE_TYPED_ITK_IMAGE:
  // Retrieve an itk image from a data layer
  template <class T>
  bool retrieve_typed_itk_image( const DataLayerHandle& data, 
    typename Core::ITKImageDataT<T>::Handle& image )
  {
    Core::DataVolumeHandle volume = data->get_data_volume();
    
    Core::DataBlockHandle data_block;
    if ( volume->get_data_type() == Core::GetDataType( reinterpret_cast<T*>( 0 ) ) )
    {
      data_block = volume->get_data_block();
    }
    else
    {
      if ( ! ( Core::DataBlock::ConvertDataType( volume->get_data_block(), 
        data_block, Core::GetDataType( reinterpret_cast<T*>( 0 ) ) ) ) )
      {
        return false;
      }
    }
    
    image = typename Core::ITKImageDataT<T>::Handle( 
      new Core::ITKImageDataT<T>( data_block, volume->get_transform() ) );
      
    if ( !image ) return false;
    return true;
  }

  // RETRIEVE_UCHAR_ITK_IMAGE:
  // Retrieve an unsigned char itk image from a data layer
  bool retrieve_uchar_itk_image( const DataLayerHandle& data, Core::ITKUCharImageData::Handle& image )
  {
    return this->retrieve_typed_itk_image<unsigned char>( data, image );
  }

  // RETRIEVE_FOAT_ITK_IMAGE:
  // Retrieve a float itk image from a data layer
  bool retrieve_float_itk_image( const DataLayerHandle& data, Core::ITKFloatImageDataHandle& image )
  {
    return this->retrieve_typed_itk_image<float>( data, image );
  }

  // INSERT_MASK_INTO_LAYER:
  // Convert the itk image back into a mask volume
  template< class T >
  bool insert_mask_into_layer( typename Core::ITKImageDataT<T>::Handle image, MaskLayerHandle& layer )
  {
    Core::MaskDataBlockHandle mask;
    // Need to make an intermediate representation as a datablock of the data
    // before it can be uploaded into the mask.
    // NOTE: This only generates a wrapper, no new data is generated
    Core::DataBlockHandle data = Core::ITKDataBlock::New( Core::ITKImageDataHandle( image ) );
    
    // NOTE: The only reason we need the transform here, is that data blocks are
    // sorted grid transform so we can use the nrrd library to save them, the
    // latter only allows storing one transform per data block, and since 8 masks
    // share one datablock, they are required for now to have the same transform.
    if (!( Core::MaskDataBlockManager::Convert( data, image->get_grid_transform(), mask ) ) )
    {
      return false;
    }
    
    layer->set_mask_volume( Core::MaskVolumeHandle( 
      new Core::MaskVolume( image->get_grid_transform(), mask ) ) );
    return true;  
  }


  // INSERT_MASK_INTO_LAYER:
  // Convert the itk image back into a mask volume
  template< class T >
  bool insert_mask_into_layer( itk::Image<T,3>* itk_image, MaskLayerHandle& layer )
  {
    Core::MaskDataBlockHandle mask;
    // Need to make an intermediate representation as a datablock of the data
    // before it can be uploaded into the mask.
    // NOTE: This only generates a wrapper, no new data is generated
    typename Core::ITKImageDataT<T>::Handle image = new typename Core::ITKImageDataT<T>( itk_image );
    Core::DataBlockHandle data = Core::ITKDataBlock::New( image );
    
    // NOTE: The only reason we need the transform here, is that data blocks are
    // sorted grid transform so we can use the nrrd library to save them, the
    // latter only allows storing one transform per data block, and since 8 masks
    // share one datablock, they are required for now to have the same transform.
    if (!( Core::MaskDataBlockManager::Convert( data, image->get_grid_transform(), mask ) ) )
    {
      return false;
    }
    
    layer->set_mask_volume( Core::MaskVolumeHandle( 
      new Core::MaskVolume( image->get_grid_transform(), mask ) ) );
    return true;  
  }

  // INSERT_DATA_INTO_LAYER:
  /// Convert the itk image back into a data volume
  template< class T >
  bool insert_data_into_layer( typename Core::ITKImageDataT<T>::Handle image, DataLayerHandle& layer )
  {
    layer->set_data_volume( Core::DataVolumeHandle( 
      new Core::DataVolume( image->get_grid_transform(), 
      Core::ITKDataBlock::New( Core::ITKImageDataHandle( image ) ) ) ) );
    return true;
  } 


  // INSERT_DATA_INTO_LAYER:
  /// Convert the itk image back into a data volume
  template< class T >
  bool insert_data_into_layer( itk::Image<T,3>* itk_image, DataLayerHandle& layer )
  {
    // Wrap an ITKImageData object around the itk object
    typename Core::ITKImageDataT<T>::Handle image = typename
      Core::ITKImageDataT<T>::Handle(new typename Core::ITKImageDataT<T>( itk_image ) );

    layer->set_data_volume( Core::DataVolumeHandle( 
      new Core::DataVolume( image->get_grid_transform(), 
      Core::ITKDataBlock::New( Core::ITKImageDataHandle( image ) ) ) ) );
    return true;
  } 

  // FORWARD_PROGRESS:
  // Forward the progress an itk filter is making
  template< class T>
  void forward_progress( T filter, const DataLayerHandle& layer )
  {
    this->forward_progress_internal( itk::ProcessObject::Pointer( filter ), LayerHandle( layer ) );
  }

  // FORWARD_PROGRESS:
  // Forward the progress an itk filter is making
  template< class T>
  void forward_progress( T filter, const MaskLayerHandle& layer )
  {
    this->forward_progress_internal( itk::ProcessObject::Pointer( filter ), LayerHandle( layer ) );
  }
  
private:  
  void forward_progress_internal( itk::ProcessObject::Pointer filter, LayerHandle layer );  
  
};

#define FOREACH_DATATYPE( DATATYPE, ... ) \
{ \
  typedef itk::Image< float, 3> FLOAT_IMAGE_TYPE; \
  typedef itk::Image< float, 3> UCHAR_IMAGE_TYPE; \
  switch ( DATATYPE )\
  {\
    case Core::DataType::CHAR_E:\
    {\
      typedef signed char VALUE_TYPE; \
      typedef itk::Image< VALUE_TYPE, 3> TYPED_IMAGE_TYPE; \
      { __VA_ARGS__ } break; \
    }\
    case Core::DataType::UCHAR_E:\
    {\
      typedef unsigned char VALUE_TYPE; \
      typedef itk::Image< VALUE_TYPE, 3> TYPED_IMAGE_TYPE; \
      { __VA_ARGS__ } break; \
    }\
    case Core::DataType::SHORT_E:\
    {\
      typedef short VALUE_TYPE; \
      typedef itk::Image< VALUE_TYPE, 3> TYPED_IMAGE_TYPE; \
      { __VA_ARGS__ } break; \
    }\
    case Core::DataType::USHORT_E:\
    {\
      typedef unsigned short VALUE_TYPE; \
      typedef itk::Image< VALUE_TYPE, 3> TYPED_IMAGE_TYPE; \
      { __VA_ARGS__ } break; \
    }\
    case Core::DataType::INT_E:\
    {\
      typedef int VALUE_TYPE; \
      typedef itk::Image< VALUE_TYPE, 3> TYPED_IMAGE_TYPE; \
      { __VA_ARGS__ } break; \
    }\
    case Core::DataType::UINT_E:\
    {\
      typedef unsigned int VALUE_TYPE; \
      typedef itk::Image< VALUE_TYPE, 3> TYPED_IMAGE_TYPE; \
      { __VA_ARGS__ } break; \
    }\
    case Core::DataType::FLOAT_E:\
    {\
      typedef float VALUE_TYPE; \
      typedef itk::Image< VALUE_TYPE, 3> TYPED_IMAGE_TYPE; \
      { __VA_ARGS__ } break; \
    }\
    case Core::DataType::DOUBLE_E:\
    {\
      typedef double VALUE_TYPE; \
      typedef itk::Image< VALUE_TYPE, 3> TYPED_IMAGE_TYPE; \
      { __VA_ARGS__ } break; \
    }\
  }\
} \

} // end namespace Seg3D

#endif
