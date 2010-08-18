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
 
#ifndef APPLICATION_TOOL_ITKFILTER_H 
#define APPLICATION_TOOL_ITKFILTER_H 
 
// Boost includes
#include <boost/smart_ptr.hpp> 
#include <boost/utility.hpp> 
 
// Core includes
#include <Core/DataBlock/ITKImageData.h>
#include <Core/DataBlock/ITKDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/Utils/Runnable.h>
#include <Core/Volume/DataVolume.h>
#include <Core/Volume/MaskVolume.h>
 
// Application includes
#include <Application/Layer/DataLayer.h> 
#include <Application/Layer/MaskLayer.h> 

#include <Application/Tool/BaseFilter.h>
 
namespace Seg3D
{

class ITKFilter : public BaseFilter
{

public:
  ITKFilter();
  virtual ~ITKFilter();
    
protected:

  // GET_ITK_IMAGE_FROM_LAYER:
  // Retrieve an itk image from a data or mask layer
  template <class T>
  bool get_itk_image_from_layer( const LayerHandle& layer, 
    typename Core::ITKImageDataT<T>::Handle& image )
  {
    // Clear the handle
    image.reset();

    Core::DataBlockHandle data_block;
    Core::Transform transform;
    
    // If the layer is a data layer
    if ( layer->type() == Core::VolumeType::DATA_E )
    {
      DataLayerHandle data = boost::dynamic_pointer_cast<DataLayer>( layer );
      Core::DataVolumeHandle volume = data->get_data_volume();
      
      // If the data is in the right format: do not convert the data, just copy the
      // data block pointer
      if ( volume->get_data_type() == Core::GetDataType( reinterpret_cast<T*>( 0 ) ) )
      {
        data_block = volume->get_data_block();
      }
      else
      {
        // TODO: In future we may want to add quantization here --JS
        // Data is not in requested format, hence we need to cast the data
        if ( ! ( Core::DataBlock::ConvertDataType( volume->get_data_block(), 
          data_block, Core::GetDataType( reinterpret_cast<T*>( 0 ) ) ) ) )
        {
          return false;
        }
      }
      
      transform = volume->get_transform();
    }
    // If the layer is a mask layer
    else if ( layer->type() == Core::VolumeType::MASK_E )
    {
      MaskLayerHandle mask = boost::dynamic_pointer_cast<MaskLayer>( layer );
      Core::MaskVolumeHandle volume = mask->get_mask_volume();

      // We always need to convert the data, ITK does not support the compressed way of
      // dealing with bitplanes
      if ( ! ( Core::MaskDataBlockManager::Convert( volume->get_mask_data_block(), 
        data_block, Core::GetDataType( reinterpret_cast<T*>( 0 ) ) ) ) )
      {
        return false;
      }   
      transform = volume->get_transform();
    }
    else
    {
      // Unknown data type
      return false;
    }

    // Create a new ITK Image wrapper
    image = typename Core::ITKImageDataT<T>::Handle( 
      new Core::ITKImageDataT<T>( data_block, transform ) );
      
    if ( !image ) return false;
    // Success
    return true;
  }

  // INSERT_ITK_IMAGE_INTO_LAYER:
  // Insert an itk image back into a layer
  template< class T >
  bool insert_itk_image_into_layer( const LayerHandle& layer, 
    typename itk::Image<T,3>* itk_image )
  {
  
    // If the layer is a data layer
    if ( layer->type() == Core::VolumeType::DATA_E )
    {
      DataLayerHandle data_layer = boost::dynamic_pointer_cast<DataLayer>( layer );

      // Wrap an ITKImageData object around the itk object
      Core::DataVolumeHandle data_volume( new Core::DataVolume( 
        data_layer->get_grid_transform(), Core::ITKDataBlock::New<T>( 
          typename itk::Image<T,3>::Pointer( itk_image ) ) ) );
        
      // NOTE: Do not need an update of the generation number as this volume 
      // was generated from scratch. But it will need a new histogram
      this->dispatch_insert_data_volume_into_layer( data_layer, data_volume, false, true );
      return true;
    }
    // If the layer is a mask layer
    else if ( layer->type() == Core::VolumeType::MASK_E )
    {
      MaskLayerHandle mask_layer = boost::dynamic_pointer_cast<MaskLayer>( layer );
          
      // Need to make an intermediate representation as a datablock of the data
      // before it can be uploaded into the mask.
      // NOTE: This only generates a wrapper, no new data is generated
      Core::DataBlockHandle data_block = Core::ITKDataBlock::New<T>( 
        typename itk::Image<T,3>::Pointer( itk_image ) );
      
      // NOTE: The only reason we need the transform here, is that data blocks are
      // sorted by grid transform so we can use the nrrd library to save them, the
      // latter only allows storing one transform per data block, and since 8 masks
      // share one datablock, they are required for now to have the same transform.
      Core::MaskDataBlockHandle mask;

      if (!( Core::MaskDataBlockManager::Convert( data_block, 
        mask_layer->get_grid_transform(), mask ) ) )
      {
        return false;
      }
      
      Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
        mask_layer->get_grid_transform(), mask ) );
        
      // NOTE:Do not need an update of the generation number as this volume 
      // was generated from scratch.
      this->dispatch_insert_mask_volume_into_layer( mask_layer, mask_volume, false );
      return true;
    }
    return false;
  }


  // CONVERT_AND_INSERT_ITK_IMAGE_INTO_LAYER:
  // Insert an itk image back into a layer
  template< class T >
  bool convert_and_insert_itk_image_into_layer( const LayerHandle& layer, 
    typename itk::Image<T,3>* itk_image, Core::DataType data_type )
  {
  
    // If the layer is a data layer
    if ( layer->type() == Core::VolumeType::DATA_E )
    {
      DataLayerHandle data_layer = boost::dynamic_pointer_cast<DataLayer>( layer );

      // Wrap an ITKImageData object around the itk object
      Core::DataBlockHandle data_block_src = Core::ITKDataBlock::New<T>( 
          typename itk::Image<T,3>::Pointer( itk_image ) ) ;
      Core::DataBlockHandle data_block_dst = data_block_src;
      if ( data_block_src->get_data_type() != data_type )
      {
        Core::DataBlock::ConvertDataType( data_block_src, data_block_dst, data_type );
      }
      
      Core::DataVolumeHandle data_volume( new Core::DataVolume( 
        data_layer->get_grid_transform(), data_block_dst ) );
        
      // NOTE: Do not need an update of the generation number as this volume 
      // was generated from scratch. But it will need a new histogram
      this->dispatch_insert_data_volume_into_layer( data_layer, data_volume, false, true );
      return true;
    }
    // If the layer is a mask layer
    else if ( layer->type() == Core::VolumeType::MASK_E )
    {
      MaskLayerHandle mask_layer = boost::dynamic_pointer_cast<MaskLayer>( layer );
          
      // Need to make an intermediate representation as a datablock of the data
      // before it can be uploaded into the mask.
      // NOTE: This only generates a wrapper, no new data is generated
      Core::DataBlockHandle data_block = Core::ITKDataBlock::New<T>( 
        typename itk::Image<T,3>::Pointer( itk_image ) );
      
      // NOTE: The only reason we need the transform here, is that data blocks are
      // sorted by grid transform so we can use the nrrd library to save them, the
      // latter only allows storing one transform per data block, and since 8 masks
      // share one datablock, they are required for now to have the same transform.
      Core::MaskDataBlockHandle mask;

      if (!( Core::MaskDataBlockManager::Convert( data_block, 
        mask_layer->get_grid_transform(), mask ) ) )
      {
        return false;
      }
      
      Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
        mask_layer->get_grid_transform(), mask ) );
        
      // NOTE:Do not need an update of the generation number as this volume 
      // was generated from scratch.
      this->dispatch_insert_mask_volume_into_layer( mask_layer, mask_volume, false );
      return true;
    }
    
    return false;
  }


  // FORWARD_PROGRESS:
  // Forward the progress an itk filter is making
  template< class T>
  void forward_progress( T filter_pointer, const LayerHandle& layer )
  {
    this->forward_progress_internal( itk::ProcessObject::Pointer( filter_pointer ), layer );
  }
  
private:  
  // Internal function for setting up itk progress forwarding
  void forward_progress_internal( itk::ProcessObject::Pointer filter, const LayerHandle& layer );   
};

#define SWITCH_DATATYPE( DATATYPE, ... ) \
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
