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
 
#ifndef APPLICATION_FILTERS_ITKFILTER_H 
#define APPLICATION_FILTERS_ITKFILTER_H
 
// Boost includes
#include <boost/function.hpp>
#include <boost/smart_ptr.hpp> 
#include <boost/noncopyable.hpp> 
 
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
#include <Application/Filters/LayerFilter.h>
 
namespace Seg3D
{

class ITKFilter;
class ITKFilterPrivate;
typedef boost::shared_ptr<ITKFilterPrivate> ITKFilterPrivateHandle;


class ITKFilter : public LayerFilter
{

public:
  typedef itk::Image< unsigned short, 3> USHORT_IMAGE_TYPE;
  typedef itk::Image< unsigned int, 3> UINT_IMAGE_TYPE;
  typedef itk::Image< float, 3> FLOAT_IMAGE_TYPE;
  typedef itk::Image< unsigned char, 3> UCHAR_IMAGE_TYPE;
  typedef Core::ITKImageDataT<float> FLOAT_CONTAINER_TYPE;
  typedef Core::ITKImageDataT<unsigned int> UINT_CONTAINER_TYPE;
  typedef Core::ITKImageDataT<unsigned short> USHORT_CONTAINER_TYPE;
  typedef Core::ITKImageDataT<unsigned char> UCHAR_CONTAINER_TYPE;

public:
  ITKFilter();
  virtual ~ITKFilter();
    
protected:

  /// GET_ITK_IMAGE_FROM_LAYER:
  /// Retrieve an itk image from a data or mask layer
  template <class T>
  bool get_itk_image_from_layer( const LayerHandle& layer, 
    typename Core::ITKImageDataT<T>::Handle& image, bool invert = false )
  {
    // Clear the handle
    image.reset();

    Core::DataBlockHandle data_block;
    Core::Transform transform;
    
    // If the layer is a data layer
    if ( layer->get_type() == Core::VolumeType::DATA_E )
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
          this->report_error( "Could not allocate enough memory." );
          return false;
        }
      }
      
      transform = volume->get_transform();
    }
    // If the layer is a mask layer
    else if ( layer->get_type() == Core::VolumeType::MASK_E )
    {
      MaskLayerHandle mask = boost::dynamic_pointer_cast<MaskLayer>( layer );
      Core::MaskVolumeHandle volume = mask->get_mask_volume();

      // We always need to convert the data, ITK does not support the compressed way of
      // dealing with bitplanes
      if ( ! ( Core::MaskDataBlockManager::Convert( volume->get_mask_data_block(), 
        data_block, Core::GetDataType( reinterpret_cast<T*>( 0 ) ), invert ) ) )
      {
        this->report_error( "Could not allocate enough memory." );
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
      
    if ( !image )
    {
      this->report_error( "Could not allocate enough memory." );
      return false;
    }
    
    // Success
    return true;
  }


  /// GET_ITK_IMAGE_FROM_MASK_LAYER:
  /// Retrieve an itk image from a data or mask layer
  template <class T>
  bool get_itk_image_from_mask_layer( const LayerHandle& layer, 
    typename Core::ITKImageDataT<T>::Handle& image, double label = 1.0 )
  {
    // Clear the handle
    image.reset();

    Core::DataBlockHandle data_block;
    Core::Transform transform;
    
    
    // If the layer is a mask layer
    if ( layer->get_type() != Core::VolumeType::MASK_E )
    {
      return false;
    }

    MaskLayerHandle mask = boost::dynamic_pointer_cast<MaskLayer>( layer );
    Core::MaskVolumeHandle volume = mask->get_mask_volume();

    // We always need to convert the data, ITK does not support the compressed way of
    // dealing with bitplanes
    if ( ! ( Core::MaskDataBlockManager::ConvertLabel( volume->get_mask_data_block(), 
      data_block, Core::GetDataType( reinterpret_cast<T*>( 0 ) ), label ) ) )
    {
      this->report_error( "Could not allocate enough memory." );
      return false;
    }
      
    transform = volume->get_transform();

    // Create a new ITK Image wrapper
    image = typename Core::ITKImageDataT<T>::Handle( 
      new Core::ITKImageDataT<T>( data_block, transform ) );
      
    if ( !image )
    {
      this->report_error( "Could not allocate enough memory." );
      return false;
    }
    
    // Success
    return true;
  }


  /// INSERT_ITK_IMAGE_INTO_LAYER:
  /// Insert an itk image back into a layer
  template< class T >
  bool insert_itk_image_into_layer( const LayerHandle& layer, 
    typename itk::Image<T,3>* itk_image )
  {
    typename itk::Image<T,3>::Pointer pointer = itk_image;
    return insert_itk_image_pointer_into_layer<T>( layer, pointer );
  }

  /// INSERT_ITK_IMAGE_POINTER_INTO_LAYER:
  /// Insert an itk image back into a layer
  template< class T >
  bool insert_itk_image_pointer_into_layer( const LayerHandle& layer, 
    typename itk::Image<T,3>::Pointer itk_image )
  {
  
    // If the layer is a data layer
    if ( layer->get_type() == Core::VolumeType::DATA_E )
    {
      DataLayerHandle data_layer = boost::dynamic_pointer_cast<DataLayer>( layer );

      // Wrap an ITKImageData object around the itk object
      Core::DataVolumeHandle data_volume( new Core::DataVolume( 
        data_layer->get_grid_transform(), Core::ITKDataBlock::New<T>( itk_image ) ) );
        
      // NOTE: Do not need an update of the generation number as this volume 
      // was generated from scratch. But it will need a new histogram
      this->dispatch_insert_data_volume_into_layer( data_layer, data_volume, true );
      return true;
    }
    // If the layer is a mask layer
    else if ( layer->get_type() == Core::VolumeType::MASK_E )
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
        this->report_error( "Could not allocate enough memory." );
        return false;
      }
      
      Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
        mask_layer->get_grid_transform(), mask ) );
        
      this->dispatch_insert_mask_volume_into_layer( mask_layer, mask_volume );
      return true;
    }

    this->report_error( "Encountered unknown data type." );
    return false;
  }

  /// INSERT_ITK_POSITIVE_LABELS_INTO_MASK_LAYER:
  /// Insert an itk image back into a layer
  template< class T >
  bool insert_itk_positive_labels_into_mask_layer( const LayerHandle& layer, 
    typename itk::Image<T,3>* itk_image, bool invert = false )
  {
    typename itk::Image<T,3>::Pointer pointer = itk_image;
    return insert_itk_positive_labels_pointer_into_mask_layer<T>( layer, pointer, invert );
  }

  /// INSERT_ITK_POSITIVE_LABELS_POINTER_INTO_MASK_LAYER:
  /// Insert an itk image back into a layer
  template< class T >
  bool insert_itk_positive_labels_pointer_into_mask_layer( const LayerHandle& layer, 
    typename itk::Image<T,3>::Pointer itk_image, bool invert = false )
  {
    if ( layer->get_type() == Core::VolumeType::MASK_E )
    {
      MaskLayerHandle mask_layer = boost::dynamic_pointer_cast<MaskLayer>( layer );
          
      // Need to make an intermediate representation as a datablock of the data
      // before it can be uploaded into the mask.
      // NOTE: This only generates a wrapper, no new data is generated
      Core::DataBlockHandle data_block = Core::ITKDataBlock::New<T>( itk_image );
      
      // NOTE: The only reason we need the transform here, is that data blocks are
      // sorted by grid transform so we can use the nrrd library to save them, the
      // latter only allows storing one transform per data block, and since 8 masks
      // share one datablock, they are required for now to have the same transform.
      Core::MaskDataBlockHandle mask;

      if (!( Core::MaskDataBlockManager::ConvertLargerThan( data_block, 
        mask_layer->get_grid_transform(), mask, invert ) ) )
      {
        this->report_error( "Could not allocate enough memory." );
        return false;
      }
      
      Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
        mask_layer->get_grid_transform(), mask ) );
        
      this->dispatch_insert_mask_volume_into_layer( mask_layer, mask_volume );
      return true;
    }
    return false;
  }


  /// INSERT_ITK_LABEL_INTO_MASK_LAYER:
  /// Insert an itk image back into a layer
  template< class T >
  bool insert_itk_label_into_mask_layer( const LayerHandle& layer, 
    typename itk::Image<T,3>* itk_image, T label )
  {
    typename itk::Image<T,3>::Pointer pointer = itk_image;
    return insert_itk_label_pointer_into_mask_layer<T>( layer, pointer, label );
  }

  /// INSERT_ITK_LABEL_POINTER_INTO_MASK_LAYER:
  /// Insert an itk image back into a layer
  template< class T >
  bool insert_itk_label_pointer_into_mask_layer( const LayerHandle& layer, 
    typename itk::Image<T,3>::Pointer itk_image, T label )
  {
    if ( layer->get_type() == Core::VolumeType::MASK_E )
    {
      MaskLayerHandle mask_layer = boost::dynamic_pointer_cast<MaskLayer>( layer );
          
      // Need to make an intermediate representation as a datablock of the data
      // before it can be uploaded into the mask.
      // NOTE: This only generates a wrapper, no new data is generated
      Core::DataBlockHandle data_block = Core::ITKDataBlock::New<T>( itk_image );
      
      // NOTE: The only reason we need the transform here, is that data blocks are
      // sorted by grid transform so we can use the nrrd library to save them, the
      // latter only allows storing one transform per data block, and since 8 masks
      // share one datablock, they are required for now to have the same transform.
      Core::MaskDataBlockHandle mask;

      if (!( Core::MaskDataBlockManager::ConvertLabel( data_block, 
        mask_layer->get_grid_transform(), mask, static_cast<double>( label ) ) ) )
      {
        this->report_error( "Could not allocate enough memory." );
        return false;
      }
      
      Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
        mask_layer->get_grid_transform(), mask ) );
        
      this->dispatch_insert_mask_volume_into_layer( mask_layer, mask_volume );
      return true;
    }
    return false;
  }

  /// CONVERT_AND_INSERT_ITK_IMAGE_INTO_LAYER:
  /// Insert an itk image back into a layer
  template< class T >
  bool convert_and_insert_itk_image_into_layer( const LayerHandle& layer, 
    typename itk::Image<T,3>* itk_image, Core::DataType data_type )
  {
    typename itk::Image<T,3>::Pointer pointer = itk_image;
    return convert_and_insert_itk_image_pointer_into_layer<T>( layer, 
      pointer, data_type );
  }

  /// CONVERT_AND_INSERT_ITK_IMAGE_POINTER_INTO_LAYER:
  /// Insert an itk image back into a layer
  template< class T >
  bool convert_and_insert_itk_image_pointer_into_layer( const LayerHandle& layer, 
    typename itk::Image<T,3>::Pointer itk_image, Core::DataType data_type )
  {
  
    // If the layer is a data layer
    if ( layer->get_type() == Core::VolumeType::DATA_E )
    {
      DataLayerHandle data_layer = boost::dynamic_pointer_cast<DataLayer>( layer );

      // Wrap an ITKImageData object around the itk object
      Core::DataBlockHandle data_block_src = Core::ITKDataBlock::New<T>( itk_image ) ;
      if ( ! data_block_src )
      {
        this->report_error( "Could not allocate enough memory." );
        return false;     
      }
      
      Core::DataBlockHandle data_block_dst = data_block_src;
      if ( data_block_src->get_data_type() != data_type )
      {
        if ( !( Core::DataBlock::ConvertDataType( data_block_src, 
          data_block_dst, data_type ) ) )
        {
          this->report_error( "Could not allocate enough memory." );
          return false;             
        }
      }
      
      Core::DataVolumeHandle data_volume( new Core::DataVolume( 
        data_layer->get_grid_transform(), data_block_dst ) );
        
      // NOTE: Do not need an update of the generation number as this volume 
      // was generated from scratch. But it will need a new histogram
      this->dispatch_insert_data_volume_into_layer( data_layer, data_volume, true );
      return true;
    }
    // If the layer is a mask layer
    else if ( layer->get_type() == Core::VolumeType::MASK_E )
    {
      MaskLayerHandle mask_layer = boost::dynamic_pointer_cast<MaskLayer>( layer );
          
      // Need to make an intermediate representation as a datablock of the data
      // before it can be uploaded into the mask.
      // NOTE: This only generates a wrapper, no new data is generated
      Core::DataBlockHandle data_block = Core::ITKDataBlock::New<T>( 
        typename itk::Image<T,3>::Pointer( itk_image ) );

      if ( ! data_block )
      {
        this->report_error( "Could not allocate enough memory." );
        return false;     
      }
      
      // NOTE: The only reason we need the transform here, is that data blocks are
      // sorted by grid transform so we can use the nrrd library to save them, the
      // latter only allows storing one transform per data block, and since 8 masks
      // share one datablock, they are required for now to have the same transform.
      Core::MaskDataBlockHandle mask;

      if (!( Core::MaskDataBlockManager::Convert( data_block, 
        mask_layer->get_grid_transform(), mask ) ) )
      {
        this->report_error( "Could not allocate enough memory." );
        return false;
      }
      
      Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
        mask_layer->get_grid_transform(), mask ) );
        
      this->dispatch_insert_mask_volume_into_layer( mask_layer, mask_volume );
      return true;
    }
    
    return false;
  }

  /// FORWARD_ABORT_TO_FILTER:
  /// Forward a Seg3D abort to an itk filter
  template< class T >
  void forward_abort_to_filter( T filter_pointer, LayerHandle layer )
  {
    this->forward_abort_to_filter_internal( itk::ProcessObject::Pointer( filter_pointer ), 
      layer );
  }

  /// OBSERVE_ITK_PROGRESS:
  /// Forward the progress an itk filter is making and check for the abort status of the layer
  template< class T >
  void observe_itk_progress( T filter_pointer, const LayerHandle& layer, 
    float progress_start = 0.0, float progress_amount = 1.0 )
  {
    this->observe_itk_progress_internal( itk::ProcessObject::Pointer( filter_pointer ), layer,
      progress_start, progress_amount );
  }

  /// OBSERVE_ITK_ITERATIONS:
  /// Forward progess on iterations to a user specified function
  ///template< class T >
  ///void observe_itk_iterations( T filter_pointer, boost::function< void( itk::Object* ) > iteration_fcn )
  ///{
  /// this->observe_itk_iterations_internal( itk::ProcessObject::Pointer( filter_pointer ),
  ///   iteration_fcn );
  ///}


  template< class T >
  void observe_itk_iterations( T filter_pointer, boost::function< void( itk::Object* ) > iteration_fcn )
  {
    this->observe_itk_iterations_internal( itk::Object::Pointer( filter_pointer ),
      iteration_fcn );
  }


  /// LIMIT_NUMBER_OF_ITK_THREADS:
  /// Limit the number of itk threads so that at least one thread can be used to allow for
  /// interaction in the program.
  /// NOTE: If the host machine does not have multiple cores this function will not do anything
  template< class T>
  void limit_number_of_itk_threads( T filter_pointer )
  {
    this->limit_number_of_itk_threads_internal( itk::ProcessObject::Pointer( filter_pointer ) );
  }
  
protected:      
  /// HANDLE_ABORT:
  /// A virtual function that can be overloaded
  virtual void handle_abort();    

  /// HANDLE_STOP:
  /// A virtual function that can be overloaded
  virtual void handle_stop();   
  
private:
  // Internal function for setting up itk progress forwarding
  void observe_itk_progress_internal( itk::ProcessObject::Pointer filter, 
    const LayerHandle& layer, float progress_start, float progress_amount );

  /// Internal function for setting up itk iteration forwarding
  ///void observe_itk_iterations_internal( itk::ProcessObject::Pointer filter, 
  /// boost::function< void( itk::Object* ) > iteration_fcn );

  void observe_itk_iterations_internal( itk::Object::Pointer filter, 
    boost::function< void( itk::Object* ) > iteration_fcn );
    
  /// Internal  function for setting up abort handling
  void forward_abort_to_filter_internal( itk::ProcessObject::Pointer filter, LayerHandle layer );
    
  /// Internal function for limiting the number of threads  
  void limit_number_of_itk_threads_internal( itk::ProcessObject::Pointer filter );  
    
  ITKFilterPrivateHandle private_;
};

#define SCI_BEGIN_TYPED_ITK_RUN( DATATYPE ) \
public:\
  virtual void run_filter()\
  {\
    switch ( DATATYPE )\
    {\
      case Core::DataType::CHAR_E: this->typed_run_filter<signed char>(); break;\
      case Core::DataType::UCHAR_E: this->typed_run_filter<unsigned char>(); break;\
      case Core::DataType::SHORT_E: this->typed_run_filter<short>(); break;\
      case Core::DataType::USHORT_E: this->typed_run_filter<unsigned short>(); break;\
      case Core::DataType::INT_E: this->typed_run_filter<int>(); break;\
      case Core::DataType::UINT_E: this->typed_run_filter<unsigned int>(); break;\
      case Core::DataType::FLOAT_E: this->typed_run_filter<float>(); break;\
      case Core::DataType::DOUBLE_E: this->typed_run_filter<double>(); break;\
    };\
  }\
\
  template< class VALUE_TYPE>\
  void typed_run_filter()\
  {\
    typedef itk::Image< VALUE_TYPE, 3> TYPED_IMAGE_TYPE; \
    typedef Core::ITKImageDataT<VALUE_TYPE> TYPED_CONTAINER_TYPE; \
  

#define SCI_END_TYPED_ITK_RUN() \
  }

#define SCI_TYPED_ITK_RUN_DECL( DATATYPE ) \
public:\
  virtual void run_filter()\
  {\
    switch ( DATATYPE )\
    {\
      case Core::DataType::CHAR_E: this->typed_run_filter<signed char>(); break;\
      case Core::DataType::UCHAR_E: this->typed_run_filter<unsigned char>(); break;\
      case Core::DataType::SHORT_E: this->typed_run_filter<short>(); break;\
      case Core::DataType::USHORT_E: this->typed_run_filter<unsigned short>(); break;\
      case Core::DataType::INT_E: this->typed_run_filter<int>(); break;\
      case Core::DataType::UINT_E: this->typed_run_filter<unsigned int>(); break;\
      case Core::DataType::FLOAT_E: this->typed_run_filter<float>(); break;\
      case Core::DataType::DOUBLE_E: this->typed_run_filter<double>(); break;\
    };\
  }\
\
  template< class VALUE_TYPE>\
  void typed_run_filter();\


#define SCI_BEGIN_ITK_RUN( ) \
public:\
  virtual void run_filter()\
  {\


#define SCI_END_ITK_RUN() \
  }
  
} // end namespace Seg3D

#endif
