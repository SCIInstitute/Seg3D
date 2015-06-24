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

#ifndef APPLICATION_FILTERS_THRESHOLDFILTER_H
#define APPLICATION_FILTERS_THRESHOLDFILTER_H

#include <Application/Filters/LayerFilter.h>

namespace Filter {

class ThresholdFilter : public Seg3D::LayerFilter
{
private:
  Seg3D::DataLayerHandle src_layer_;
  Seg3D::MaskLayerHandle dst_layer_;

  double lower_threshold_;
  double upper_threshold_;

public:
  ThresholdFilter( double lower_threshold, double upper_threshold ) :
    lower_threshold_(lower_threshold),
    upper_threshold_(upper_threshold)
  {
  }

  ~ThresholdFilter() {}

  template< class T >
  void threshold_data( Core::DataBlockHandle dst, double min_val, double max_val )
  {
    Core::DataBlockHandle src_data_block = this->src_layer_->
    get_data_volume()->get_data_block();
    T* src_data = reinterpret_cast< T* >( src_data_block->get_data() );
    unsigned char* dst_data = reinterpret_cast< unsigned char* >( dst->get_data() );
    size_t z_plane_size = src_data_block->get_nx() * src_data_block->get_ny();
    size_t nz = src_data_block->get_nz();
    size_t tenth_nz = nz / 10;

    // Lock the source data block
    Core::DataBlock::shared_lock_type lock( src_data_block->get_mutex() );
    size_t index = 0;
    for ( size_t z = 0; z < nz; ++z )
    {
      for ( size_t i = 0; i < z_plane_size; ++i, ++index )
      {
        dst_data[ index ] = ( src_data[ index ] >= min_val &&
                             src_data[ index ] <= max_val ) ? 1 : 0;
      }
      if ( this->check_abort() )
      {
        return;
      }
      if ( tenth_nz > 0 && z > 0 && z % tenth_nz == 0 )
      {
        this->dst_layer_->update_progress_signal_( ( z * 0.5 ) / nz );
      }
    }
    this->dst_layer_->update_progress_signal_( 0.5 );
  }

  inline void set_data_layer(Seg3D::DataLayerHandle data) { this->src_layer_ = data; }
  inline Seg3D::DataLayerHandle data_layer() { return this->src_layer_; }

  inline void set_mask_layer(Seg3D::MaskLayerHandle mask) { this->dst_layer_ = mask; }
  inline Seg3D::MaskLayerHandle mask_layer() { return this->dst_layer_; }

  inline double lower_threshold() { return this->lower_threshold_; }
  inline double upper_threshold() { return this->upper_threshold_; }

  // RUN_FILTER:
  // Implementation of run of the Runnable base class, this function is called when the thread
  // is launched.
  virtual void run_filter();

  // GET_FILTER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const;

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name,
  // when a new layer is generated.
  virtual std::string get_layer_prefix() const;
};

}

#endif
