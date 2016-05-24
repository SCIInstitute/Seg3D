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

// STL includes
#include <vector>
#include <limits>

// Boost includes
#include <boost/algorithm/minmax_element.hpp>

// Core includes
#include <Core/Utils/Parallel.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Math/MathFunctions.h>
#include <Core/DataBlock/Histogram.h>

namespace Core
{

Histogram::Histogram() 
{
  this->min_ = Core::Nan();
  this->max_ = Core::Nan();
  this->bin_start_ = Core::Nan();
  this->bin_size_ = Core::Nan();
  this->histogram_.resize( 0 );
}

Histogram::Histogram( const signed char* data, size_t size )
{
  this->compute( data, size );
}
  
Histogram::Histogram( const unsigned char* data, size_t size )
{
  this->compute( data, size );
}

Histogram::Histogram( const short* data, size_t size )
{
  this->compute( data, size );
}

Histogram::Histogram( const unsigned short* data, size_t size )
{
  this->compute( data, size );
}

Histogram::Histogram( const int* data, size_t size )
{
  this->compute( data, size );
}

Histogram::Histogram( const unsigned int* data, size_t size )
{
  this->compute( data, size );
}

Histogram::Histogram( const float* data, size_t size )
{
  this->compute( data, size );
}

Histogram::Histogram( const double* data, size_t size )
{
  this->compute( data, size );
}

Histogram::~Histogram()
{
}

// For char and short data we do a single pass over the data to speed up the computation. The
// first pass is used to generate the histogram. In the next step we use this data to compute
// min and max

bool Histogram::compute( const signed char* data, size_t size )
{
  this->min_ = Core::Nan();
  this->max_ = Core::Nan();
  this->bin_start_ = Core::Nan();
  this->bin_size_ = Core::Nan();
  this->histogram_.resize( 0 );
  if ( size == 0 ) return false;
  
  try
  {
    std::vector<size_t> histogram;
    histogram.resize( 0x100 , 0 );

    for ( size_t j = 0 ; j < size ; j++ )
    {
      histogram[ static_cast<int>( data[ j ] ) + 0x80 ]++;
    }
    
    size_t hist_begin = 0;
    size_t hist_end = 0;
    
    for ( size_t j = 0 ; j < histogram.size() ; j++ )
    {
      if ( histogram[ j ] > 0 ) hist_end = j;
    }

    size_t hsize = histogram.size();
    for ( size_t j = 0 ; j < hsize; j++ )
    {
      if ( histogram[ hsize - 1 - j ] > 0 ) hist_begin = hsize - 1 - j;
    }

    this->min_ = static_cast<double>( hist_begin ) - static_cast<double>( 0x80 );
    this->max_ = static_cast<double>( hist_end ) - static_cast<double>( 0x80 );

    this->bin_size_  = 1.0;
    this->bin_start_ = this->min_ - 0.5;

    this->histogram_.resize( hist_end + 1 - hist_begin );
    for ( size_t j = 0 ; j < histogram_.size() ; j++ )
    {
      this->histogram_[ j ] = histogram[ j + hist_begin ];
    }

    std::pair< std::vector<size_t>::iterator, std::vector<size_t>::iterator > min_max = 
      boost::minmax_element( this->histogram_.begin(), this->histogram_.end() );
    this->min_bin_ = (*min_max.first);
    this->max_bin_ = (*min_max.second);
  }
  catch( ... )
  {
    this->min_ = Core::Nan();
    this->max_ = Core::Nan();
    this->bin_start_ = Core::Nan();
    this->bin_size_ = Core::Nan();
    this->histogram_.resize( 0 );
    return false;
  }
  
  return true;
}

bool Histogram::compute( const unsigned char* data, size_t size )
{
  this->min_ = Core::Nan();
  this->max_ = Core::Nan();
  this->bin_start_ = Core::Nan();
  this->bin_size_ = Core::Nan();
  this->histogram_.resize( 0 );
  if ( size == 0 ) return false;

  try
  {
    std::vector<size_t> histogram;
    histogram.resize( 0x100, 0 );
    
    for ( size_t j = 0 ; j < size ; j++ )
    {
      histogram[ static_cast<int>( data[ j ] ) ]++;
    }
    
    size_t hist_begin = 0;
    size_t hist_end = 0;
    
    for ( size_t j = 0 ; j < histogram.size() ; j++ )
    {
      if ( histogram[ j ] > 0 ) hist_end = j;
    }

    size_t hsize = histogram.size();
    for ( size_t j = 0 ; j < hsize; j++ )
    {
      if ( histogram[ hsize - 1 - j ] > 0 ) hist_begin = hsize - 1 - j;
    }


    this->min_ = static_cast<double>( hist_begin );
    this->max_ = static_cast<double>( hist_end );

    this->bin_size_  = 1.0;
    this->bin_start_ = this->min_ - 0.5;

    this->histogram_.resize( hist_end + 1 - hist_begin );

    for ( size_t j = 0 ; j < histogram_.size() ; j++ )
    {
      this->histogram_[ j ] = histogram[ j + hist_begin ];
    }

    std::pair< std::vector<size_t>::iterator, std::vector<size_t>::iterator > min_max = 
      boost::minmax_element( this->histogram_.begin(), this->histogram_.end() );
    this->min_bin_ = (*min_max.first);
    this->max_bin_ = (*min_max.second);
  }
  catch( ... )
  {
    this->min_ = Core::Nan();
    this->max_ = Core::Nan();
    this->bin_start_ = Core::Nan();
    this->bin_size_ = Core::Nan();
    this->histogram_.resize( 0 );
    return false;
  }
  
  return true;  
}

bool Histogram::compute( const short* data, size_t size )
{
  this->min_ = Core::Nan();
  this->max_ = Core::Nan();
  this->bin_start_ = Core::Nan();
  this->bin_size_ = Core::Nan();
  this->histogram_.resize( 0 );
  if ( size == 0 ) return false;

  try
  {
    std::vector<size_t> histogram;
    histogram.resize( 0x10000 , 0 );
    
    for ( size_t j = 0 ; j < size ; j++ )
    {
      histogram[ static_cast<int>( data[ j ] ) + 0x8000 ]++;
    }
    
    size_t hist_begin = 0;
    size_t hist_end = 0;
    
    for ( size_t j = 0 ; j < histogram.size() ; j++ )
    {
      if ( histogram[ j ] > 0 ) hist_end = j;
    }

    size_t hsize = histogram.size();
    for ( size_t j = 0 ; j < hsize; j++ )
    {
      if ( histogram[ hsize - 1 - j ] > 0 ) hist_begin = hsize - 1 - j;
    }

    this->min_ = static_cast<double>( hist_begin ) - static_cast<double>( 0x8000 );
    this->max_ = static_cast<double>( hist_end ) - static_cast<double>( 0x8000 );

    size_t hist_length = hist_end + 1 - hist_begin;
    if ( hist_length > 0x100 ) hist_length = 0x100;
    
    this->histogram_.resize( hist_length, 0 );

    if ( hist_length == 1 )
    {
      this->bin_size_  = 1.0;
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );
    }
    else
    {
      this->bin_size_ = ( this->max_ - this->min_ ) / static_cast<double>( hist_length - 1 );
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );
    }
    
    for ( size_t j = 0 ; j < histogram_.size() ; j++ )
    {
      double min_value = this->bin_start_ + j * this->bin_size_;
      double max_value = this->bin_start_ + ( j + 1 ) * this->bin_size_;

      for ( int k = Ceil( min_value ) ; k < Ceil( max_value ); k++ )
      {
        int idx = static_cast<int>( k ) + 0x8000;
        if ( idx >= 0 && idx < static_cast<int>( histogram.size() ) ) 
        {
          this->histogram_[ j ] += histogram[ idx ];
        }
      }
    }

    std::pair< std::vector<size_t>::iterator, std::vector<size_t>::iterator > min_max = 
      boost::minmax_element( this->histogram_.begin(), this->histogram_.end() );
    this->min_bin_ = (*min_max.first);
    this->max_bin_ = (*min_max.second);
  }
  catch( ... )
  {
    this->min_ = Core::Nan();
    this->max_ = Core::Nan();
    this->bin_start_ = Core::Nan();
    this->bin_size_ = Core::Nan();
    this->histogram_.resize( 0 );
    return false;
  }
  
  return true;  
}


bool Histogram::compute( const unsigned short* data, size_t size )
{
  this->min_ = Core::Nan();
  this->max_ = Core::Nan();
  this->bin_start_ = Core::Nan();
  this->bin_size_ = Core::Nan();
  this->histogram_.resize( 0 );
  if ( size == 0 ) return false;

  try
  {
    std::vector<size_t> histogram;
    histogram.resize( 0x10000 , 0 );
    
    for ( size_t j = 0 ; j < size ; j++ )
    {
      histogram[ static_cast<int>( data[ j ] ) ]++;
    }
    
    size_t hist_begin = 0;
    size_t hist_end = 0;
    
    for ( size_t j = 0 ; j < histogram.size() ; j++ )
    {
      if ( histogram[ j ] > 0 ) hist_end = j;
    }

    size_t hsize = histogram.size();
    for ( size_t j = 0 ; j < hsize; j++ )
    {
      if ( histogram[ hsize - 1 - j ] > 0 ) hist_begin = hsize - 1 - j;
    }

    this->min_ = static_cast<double>( hist_begin );
    this->max_ = static_cast<double>( hist_end );

    size_t hist_length = hist_end + 1 - hist_begin;
    if ( hist_length > 0x100 ) hist_length = 0x100;
    
    this->histogram_.resize( hist_length, 0 );

    if ( hist_length == 1 )
    {
      this->bin_size_  = 1.0;
      this->bin_start_ = min_ - ( this->bin_size_ * 0.5 );
    }
    else
    {
      this->bin_size_ = ( this->max_ - this->min_ ) / static_cast<double>( hist_length - 1 );
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );
    }
    
    for ( size_t j = 0 ; j < histogram_.size() ; j++ )
    {
      double min_value = this->bin_start_ + j * this->bin_size_;
      double max_value = this->bin_start_ + ( j + 1 ) * this->bin_size_;

      for ( int k = Ceil( min_value ) ; k < Ceil( max_value ); k ++ )
      {
        int idx = static_cast<int>( k );
        if ( idx >= 0 && idx < static_cast<int>( histogram.size() ) ) 
        {
          this->histogram_[ j ] += histogram[ idx ];
        }
      }
    }

    std::pair< std::vector<size_t>::iterator, std::vector<size_t>::iterator > min_max = 
      boost::minmax_element( this->histogram_.begin(), this->histogram_.end() );
    this->min_bin_ = (*min_max.first);
    this->max_bin_ = (*min_max.second);
  }
  catch( ... )
  {
    this->min_ = Core::Nan();
    this->max_ = Core::Nan();
    this->bin_start_ = Core::Nan();
    this->bin_size_ = Core::Nan();
    this->histogram_.resize( 0 );
    return false;
  }
  
  return true;  
}


bool Histogram::compute( const int* data, size_t size )
{
  this->min_ = Core::Nan();
  this->max_ = Core::Nan();
  this->bin_start_ = Core::Nan();
  this->bin_size_ = Core::Nan();
  this->histogram_.resize( 0 );
  if ( size == 0 ) return false;

  try
  {
    int int_min = data[ 0 ];
    int int_max = data[ 0 ];

    for ( size_t j = 1 ; j < size ; j++ )
    {
      if ( data[ j ] < int_min ) int_min = data[ j ];
      if ( data[ j ] > int_max ) int_max = data[ j ];
    }
    
    this->min_ = static_cast<double>( int_min );
    this->max_ = static_cast<double>( int_max );

    if ( this->min_ == this->max_ )
    {
      this->bin_size_  = 1.0;
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );
      this->histogram_.resize( 1, 0 );
    }
    else if ( ( this->max_ - this->min_ ) < 256.0 )
    {
      size_t hist_size = static_cast<size_t>( this->max_ - this->min_ ) + 1;
      this->histogram_.resize( hist_size, 0 );
      this->bin_size_ = ( this->max_ - this->min_ ) / static_cast<double>( hist_size - 1 );
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );
    }
    else
    {
      size_t hist_size = 0x100;
      this->histogram_.resize( hist_size, 0 );
      this->bin_size_ = ( this->max_ - this->min_ ) / static_cast<double>( hist_size - 1 );
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );  
    }

    double inv_bin_size = 1.0 / this->bin_size_;
    for ( size_t j = 1 ; j < size ; j++ )
    {
      size_t idx = static_cast<size_t>( ( static_cast<double>( data[j] ) - 
        this->min_ ) * inv_bin_size );
      this->histogram_[ idx ]++;
    }

    std::pair< std::vector<size_t>::iterator, std::vector<size_t>::iterator > min_max = 
      boost::minmax_element( this->histogram_.begin(), this->histogram_.end() );
    this->min_bin_ = (*min_max.first);
    this->max_bin_ = (*min_max.second);
  }
  catch( ... )
  {
    this->min_ = Core::Nan();
    this->max_ = Core::Nan();
    this->bin_start_ = Core::Nan();
    this->bin_size_ = Core::Nan();
    this->histogram_.resize( 0 );
    return false;
  }
  
  return true;
}


bool Histogram::compute( const unsigned int* data, size_t size )
{
  this->min_ = Core::Nan();
  this->max_ = Core::Nan();
  this->bin_start_ = Core::Nan();
  this->bin_size_ = Core::Nan();
  this->histogram_.resize( 0 );
  if ( size == 0 ) return false;

  try
  {
    unsigned int int_min = data[ 0 ];
    unsigned int int_max = data[ 0 ];

    for ( size_t j = 1 ; j < size ; j++ )
    {
      if ( data[ j ] < int_min ) int_min = data[ j ];
      if ( data[ j ] > int_max ) int_max = data[ j ];
    }
    
    this->min_ = static_cast<double>( int_min );
    this->max_ = static_cast<double>( int_max );

    if ( this->min_ == this->max_ )
    {
      this->bin_size_  = 1.0;
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );
      this->histogram_.resize( 1, 0 );
    }
    else if ( ( this->max_ - this->min_ ) < 256.0 )
    {
      size_t hist_size = static_cast<size_t>( this->max_ - this->min_ ) + 1;
      this->histogram_.resize( hist_size, 0 );
      this->bin_size_ = ( this->max_ - this->min_ ) / static_cast<double>( hist_size - 1 );
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );
    }
    else
    {
      size_t hist_size = 0x100;
      this->histogram_.resize( hist_size, 0 );
      this->bin_size_ = ( this->max_ - this->min_ ) / static_cast<double>( hist_size - 1 );
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );  
    }

    double inv_bin_size = 1.0 / this->bin_size_;
    for ( size_t j = 1 ; j < size ; j++ )
    {
      size_t idx = static_cast<size_t>( ( static_cast<double>( data[j] ) - this->min_ ) * inv_bin_size );
      this->histogram_[ idx ]++;
    }

    std::pair< std::vector<size_t>::iterator, std::vector<size_t>::iterator > min_max = 
      boost::minmax_element( this->histogram_.begin(), this->histogram_.end() );
    this->min_bin_ = (*min_max.first);
    this->max_bin_ = (*min_max.second);
  }
  catch( ... )
  {
    this->min_ = Core::Nan();
    this->max_ = Core::Nan();
    this->bin_start_ = Core::Nan();
    this->bin_size_ = Core::Nan();
    this->histogram_.resize( 0 );
    return false;
  }
  
  return true;
}


bool Histogram::compute( const float* data, size_t size )
{
  this->min_ = Core::Nan();
  this->max_ = Core::Nan();
  this->bin_start_ = Core::Nan();
  this->bin_size_ = Core::Nan();
  this->histogram_.resize( 0 );
  if ( size == 0 ) return false;

  try
  {
    float float_min = std::numeric_limits<float>::max();
    float float_max = std::numeric_limits<float>::min();

    for ( size_t j = 0 ; j < size ; j++ )
    {
      float val = data[ j ];
      if ( ! Core::IsFinite( val ) ) continue;
      if ( val < float_min ) float_min = val;
      if ( val > float_max ) float_max = val;
    }
    
    if ( float_min > float_max )
    {
      // Most likely all the data is NaN
      this->min_ = Core::Nan();
      this->max_ = Core::Nan();
      this->bin_start_ = Core::Nan();
      this->bin_size_ = Core::Nan();
      this->histogram_.resize( 0 );
      return false;   
    }
    
    this->min_ = static_cast<double>( float_min );
    this->max_ = static_cast<double>( float_max );

    if ( this->min_ == this->max_ )
    {
      this->bin_size_  = 1.0;
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );
      this->histogram_.resize( 1, 0 );
    }
    else
    {
      size_t hist_size = 0x100;
      this->histogram_.resize( hist_size, 0 );
      this->bin_size_ = ( this->max_ - this->min_ ) / static_cast<double>( hist_size - 1 );
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );  
    }

    float inv_bin_size = static_cast< float >( 1.0f / this->bin_size_ );
    for ( size_t j = 1 ; j < size ; j++ )
    {
      float val = data[ j ];
      if ( Core::IsFinite( val ) )
      {
        size_t idx = static_cast<size_t>( ( val - float_min ) * inv_bin_size );
        this->histogram_[ idx ]++;
      }
    }

    std::pair< std::vector<size_t>::iterator, std::vector<size_t>::iterator > min_max = 
      boost::minmax_element( this->histogram_.begin(), this->histogram_.end() );
    this->min_bin_ = (*min_max.first);
    this->max_bin_ = (*min_max.second);
  }
  catch( ... )
  {
    this->min_ = Core::Nan();
    this->max_ = Core::Nan();
    this->bin_start_ = Core::Nan();
    this->bin_size_ = Core::Nan();
    this->histogram_.resize( 0 );
    return false;
  }
  
  return true;
}


bool Histogram::compute( const double* data, size_t size )
{
  this->min_ = Core::Nan();
  this->max_ = Core::Nan();
  this->bin_start_ = Core::Nan();
  this->bin_size_ = Core::Nan();
  this->histogram_.resize( 0 );
  if ( size == 0 ) return false;

  try
  {
    this->min_ = std::numeric_limits<double>::max();
    this->max_ = std::numeric_limits<double>::min();

    for ( size_t j = 0 ; j < size ; j++ )
    {
      double val = data[ j ];
      if ( ! Core::IsFinite( val ) ) continue;
      if ( val < this->min_ ) this->min_ = val;
      if ( val > this->max_ ) this->max_ = val;
    }
    
    if ( this->min_ > this->max_ )
    {
      // Most likely all the data is NaN
      this->min_ = Core::Nan();
      this->max_ = Core::Nan();
      this->bin_start_ = Core::Nan();
      this->bin_size_ = Core::Nan();
      this->histogram_.resize( 0 );
      return false;   
    }
      
    if ( this->min_ == this->max_ )
    {
      this->bin_size_  = 1.0;
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );
      this->histogram_.resize( 1, 0 );
    }
    else
    {
      size_t hist_size = 0x100;
      this->histogram_.resize( hist_size, 0 );
      this->bin_size_ = ( this->max_ - this->min_ ) / static_cast<double>( hist_size - 1 );
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );  
    }

    double inv_bin_size = 1.0 / bin_size_;


    for ( size_t j = 1 ; j < size ; j++ )
    {
      double val = data[ j ];
      if ( Core::IsFinite( val ) )
      {
        size_t idx = static_cast<size_t>( ( val - this->min_ ) * inv_bin_size );
        this->histogram_[ idx ]++;
      }   
    }
    
    std::pair< std::vector<size_t>::iterator, std::vector<size_t>::iterator > min_max = 
      boost::minmax_element( this->histogram_.begin(), this->histogram_.end() );
    this->min_bin_ = (*min_max.first);
    this->max_bin_ = (*min_max.second);
  }
  catch( ... )
  {
    this->min_ = Core::Nan();
    this->max_ = Core::Nan();
    this->bin_start_ = Core::Nan();
    this->bin_size_ = Core::Nan();
    this->histogram_.resize( 0 );
    return false;
  }
  
  return true;  
}

double Histogram::get_min() const
{ 
  return this->min_;
}

double Histogram::get_max() const
{ 
  return this->max_; 
}

double Histogram::get_cum_value( double fraction ) const
{
  size_t tot_hist = 0;
  for ( size_t j = 0; j < this->histogram_.size(); j++ )
  {
    tot_hist += this->histogram_[ j ];
  }

  double multiplier = 1.0 / static_cast<double>( tot_hist );
  double jj = 0.0;
  size_t cur_hist = 0;

  for ( size_t j = 0; j < this->histogram_.size(); j++ )
  {
    double frac_start = static_cast<double>( cur_hist ) * multiplier;
    cur_hist += this->histogram_[ j ];
    double frac_end = static_cast<double>( cur_hist ) * multiplier;
    if ( fraction > frac_start && fraction <= frac_end )
    {
      jj = static_cast<double>( j ) + ( frac_end - fraction )/( frac_end - frac_start );
      break;
    }
  }
  
  return this->bin_start_ + ( jj * this->bin_size_ );
}

size_t Histogram::get_max_bin() const
{ 
  return this->max_bin_;
}

size_t Histogram::get_min_bin() const
{ 
  return this->min_bin_; 
}

double Histogram::get_bin_size() const
{
  return this->bin_size_;
}

double Histogram::get_bin_start( size_t idx ) const
{
  return this->bin_start_ + idx * this->bin_size_;
}

double Histogram::get_bin_end( size_t idx ) const
{
  return this->bin_start_ + ( idx + 1 ) * this->bin_size_;
}

const std::vector<size_t>& Histogram::get_bins() const
{
  return this->histogram_;
}

size_t Histogram::get_size() const
{
  return this->histogram_.size();
}

bool Histogram::is_valid() const
{
  return !( IsNan( this->min_ ) || IsNan( this->max_ ) );
}

std::string ExportToString( const Histogram& value )
{
  std::string result = '[' + ExportToString( value.min_ ) + ',' +
    ExportToString( value.max_ ) + ',' +
    ExportToString( value.min_bin_ ) + ',' +
    ExportToString( value.max_bin_ ) + ',' +
    ExportToString( value.bin_start_ ) + ',' +
    ExportToString( value.bin_size_ ) + ',' +
    ExportToString( value.histogram_ ) + ']';

  return result;
}

bool ImportFromString( const std::string& str, Histogram& value )
{
  std::vector< double > values;
  ImportFromString( str, values );
  if ( values.size() > 7 )
  {
    value.min_ = values[ 0 ];
    value.max_ = values[ 1 ];
    value.min_bin_ = static_cast<size_t>( values[ 2 ] );
    value.max_bin_ = static_cast<size_t>( values[ 3 ] );
    value.bin_start_ = values[ 4 ];
    value.bin_size_ = values[ 5 ];
    value.histogram_.resize( values.size() - 6 );
    for ( size_t j = 0; j < value.histogram_.size(); j++ )
    {
      value.histogram_[ j ] = static_cast< size_t >( values[ j + 6 ] );
    }
    return true;
  }
  return false;
}

} // end namespace Core
