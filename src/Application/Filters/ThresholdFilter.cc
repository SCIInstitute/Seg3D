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

#include <Application/Filters/ThresholdFilter.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/DataBlock/StdDataBlock.h>

#include <sstream>

using namespace Filter;
using namespace Seg3D;
using namespace Core;

std::string ThresholdFilter::get_filter_name() const
{
  return "Threshold Tool";
}

// GET_LAYER_PREFIX:
// This function returns the name of the filter. The latter is prepended to the new layer name,
// when a new layer is generated.
std::string ThresholdFilter::get_layer_prefix() const
{
  std::ostringstream oss;
  oss << "Threshold" << ExportToString( this->lower_threshold_ )
      << "TO" << ExportToString( this->upper_threshold_ );
  return oss.str();
}

void ThresholdFilter::run_filter()
{
  DataBlockHandle threshold_result =
    StdDataBlock::New(this->src_layer_->get_grid_transform(), DataType::UCHAR_E );

  switch ( this->src_layer_->get_data_type() )
  {
    case DataType::CHAR_E:
      this->threshold_data< signed char >( threshold_result,
                                           this->lower_threshold_, this->upper_threshold_ );
      break;
    case DataType::UCHAR_E:
      this->threshold_data< unsigned char >( threshold_result,
                                             this->lower_threshold_, this->upper_threshold_ );
      break;
    case DataType::SHORT_E:
      this->threshold_data< short >( threshold_result,
                                     this->lower_threshold_, this->upper_threshold_ );
      break;
    case DataType::USHORT_E:
      this->threshold_data< unsigned short >( threshold_result,
                                              this->lower_threshold_, this->upper_threshold_ );
      break;
    case DataType::INT_E:
      this->threshold_data< int >( threshold_result,
                                   this->lower_threshold_, this->upper_threshold_ );
      break;
    case DataType::UINT_E:
      this->threshold_data< unsigned int >( threshold_result,
                                            this->lower_threshold_, this->upper_threshold_ );
      break;
    case DataType::FLOAT_E:
      this->threshold_data< float >( threshold_result,
                                     this->lower_threshold_, this->upper_threshold_ );
      break;
    case DataType::DOUBLE_E:
      this->threshold_data< double >( threshold_result,
                                      this->lower_threshold_, this->upper_threshold_ );
      break;
  }

  if ( this->check_abort() )
  {
    return;
  }

  MaskDataBlockHandle threshold_mask;
  MaskDataBlockManager::Convert( threshold_result,
                                 this->dst_layer_->get_grid_transform(),
                                 threshold_mask );

  if ( this->check_abort() )
  {
    return;
  }

  this->dst_layer_->update_progress_signal_( 1.0 );

  this->dispatch_insert_mask_volume_into_layer( this->dst_layer_,
    MaskVolumeHandle( new MaskVolume( this->dst_layer_->get_grid_transform(),
                                      threshold_mask ) ) );
}
