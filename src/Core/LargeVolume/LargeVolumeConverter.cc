/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#include <string>
#include <vector>
#include <iomanip>

#include <Core/DataBlock/ITKDataBlock.h>
#include <Core/DataBlock/ITKImage2DData.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/Geometry/IndexVector.h>
#include <Core/Utils/FileUtil.h>
#include <Core/Utils/Parallel.h>

#include <itkPNGImageIO.h>
#include <itkTIFFImageIO.h>
#include <itkJPEGImageIO.h>
#include <itkGDCMImageIO.h>
#include <itkImageSeriesReader.h>

#include <Core/LargeVolume/LargeVolumeConverter.h>
#include <Core/LargeVolume/LargeVolumeSchema.h>


namespace Core
{

class LargeVolumeBrickLevel;
typedef boost::shared_ptr<LargeVolumeBrickLevel> LargeVolumeBrickLevelHandle;

class LargeVolumeBrickLevel
{
public:
  LargeVolumeBrickLevel( LargeVolumeSchemaHandle schema, size_t level ) :
    level_( level ),
    buffer_start_( 0 ),
    buffer_size_( 0 ),
    buffer_index_( 0 ),
    buffer_count_( 0 ),
    schema_( schema )
  {
    this->layout_ = schema_->get_level_layout( this->level_ );
    this->buffers_.resize( this->layout_.x() * this->layout_.y() );
  }

  size_t level_;
  size_t buffer_start_;
  size_t buffer_size_;
  size_t buffer_index_;
  size_t buffer_count_;

  IndexVector layout_;

  std::vector<DataBlockHandle> buffers_;
  LargeVolumeSchemaHandle schema_;

public:

  size_t get_num_buffers() const
  {
    return this->buffers_.size();
  }

  void allocate_buffers( size_t size );

  template<class T>
  bool insert_slice_internals( DataBlockHandle slice );
  bool insert_slice( DataBlockHandle slice );
  bool sync_buffers( bool done, std::string& error );
};

void LargeVolumeBrickLevel::allocate_buffers( size_t size )
{
  this->buffer_size_ = size;

  for ( size_t j = 0; j < this->buffers_.size(); j++ )
  {
    BrickInfo bi( j , this->level_ );
    IndexVector brick_size =  this->schema_->get_brick_size( bi );
    this->buffers_[ j ] = StdDataBlock::New( brick_size[0], brick_size[1], size, this->schema_->get_data_type() );
  }
}

template<class T>
bool LargeVolumeBrickLevel::insert_slice_internals( DataBlockHandle slice )
{
  const IndexVector::index_type overlap = static_cast<IndexVector::index_type>( this->schema_->get_overlap() );
  const IndexVector brick_size = this->schema_->get_brick_size();
  const IndexVector eff_brick_size =  this->schema_->get_effective_brick_size();

  IndexVector::index_type k = 0;

  if (slice)
  {
    T* sdata = reinterpret_cast<T*>( slice->get_data() );
    IndexVector::index_type snx = slice->get_nx();
    IndexVector::index_type sny = slice->get_ny();

    for ( IndexVector::index_type by = 0; by < this->layout_.y(); by++ )
    {
      for ( IndexVector::index_type bx = 0; bx < this->layout_.x(); bx++, k++ )
      {
        DataBlockHandle buffer = buffers_[k];

        IndexVector::index_type nx = buffer->get_nx();
        IndexVector::index_type ny = buffer->get_ny();

        T* data = reinterpret_cast<T*>( buffer->get_data() );
        data += ( nx * ny * this->buffer_index_ );

        IndexVector::index_type sy_begin = by * eff_brick_size.y() - overlap;
        IndexVector::index_type sy_begin2 = Max( by * eff_brick_size.y() - overlap , static_cast<IndexVector::index_type>( 0 ) );
        IndexVector::index_type sy_end2 = Min( ( by + 1 ) * eff_brick_size.y() + overlap, sny );
        IndexVector::index_type sy_end = Min( ( by + 1 ) * eff_brick_size.y() + overlap, sny + overlap ) ;
        
        IndexVector::index_type sx_begin = bx * eff_brick_size.x() - overlap;
        IndexVector::index_type sx_begin2 = Max(bx * eff_brick_size.x() - overlap, static_cast<IndexVector::index_type>( 0 ) );
        IndexVector::index_type sx_end2 = Min( ( bx + 1 ) * eff_brick_size.x() + overlap, snx );
        IndexVector::index_type sx_end = Min( ( bx + 1 ) * eff_brick_size.x() + overlap, snx + overlap );

        // Copy brick;

        for ( IndexVector::index_type p = sy_begin * nx ; p < sy_begin2 * nx ; p++, data++ )  
        { 
          *data = T(0);
        }

        for (  IndexVector::index_type y = sy_begin2; y < sy_end2; y++ )
        {
          // Add overlap
          for ( IndexVector::index_type p = sx_begin; p < sx_begin2 ; p++, data++ )
          { 
            *data = T(0);
          }

          for ( IndexVector::index_type x = sx_begin2; x < sx_end2; x++, data++  )
          {
            *data = sdata[ y * snx + x ];
          }

          // Add overlap
          for ( IndexVector::index_type p = sx_end2; p < sx_end ; p++, data++ )
          { 
            *data = T(0);
          }
        }

        for ( IndexVector::index_type p = sy_end2 * nx ; p < sy_end * nx ; p++, data++ )  
        { 
          *data = T(0);
        }

      } 
    }
  } 
  else
  {
    for ( IndexVector::index_type by = 0; by < this->layout_.y(); by++ )
    {
      for ( IndexVector::index_type bx = 0; bx < this->layout_.x(); bx++, k++ )
      {
        DataBlockHandle buffer = buffers_[k];
        IndexVector::index_type nx = buffer->get_nx();
        IndexVector::index_type ny = buffer->get_ny();
        IndexVector::index_type nxy = nx * ny;

        T* data = reinterpret_cast<T*>( buffer->get_data() );
        data += ( nxy * this->buffer_index_ );

        for ( IndexVector::index_type p = 0; p < nxy; p++, data++ )
        {
          *data = T(0);
        }
      }
    }
  }

  buffer_index_++;
  buffer_count_++;

  return true;
}

bool LargeVolumeBrickLevel::insert_slice( DataBlockHandle slice )
{
  switch( slice->get_data_type() )
  {
    case DataType::CHAR_E:
      return this->insert_slice_internals<signed char>( slice );
    case DataType::UCHAR_E:
      return this->insert_slice_internals<unsigned char>( slice );
    case DataType::SHORT_E:
      return this->insert_slice_internals<short>( slice );
    case DataType::USHORT_E:
      return this->insert_slice_internals<unsigned short>( slice );
    case DataType::INT_E:
      return this->insert_slice_internals<int>( slice );
    case DataType::UINT_E:
      return this->insert_slice_internals<unsigned int>( slice );
    case DataType::FLOAT_E:
      return this->insert_slice_internals<float>( slice );
    case DataType::DOUBLE_E:
      return this->insert_slice_internals<double>( slice );   
  }

  return false;
}

bool LargeVolumeBrickLevel::sync_buffers( bool done, std::string& error )
{

  if ( done )
  {
    std::cout << "done bricking level: " << this->level_ << std::endl;
  }


  if ( done || this->buffer_index_ == this->buffer_size_ )
  {
    IndexVector::index_type buffer_start = this->buffer_count_ - this->buffer_index_;
    IndexVector::index_type buffer_size = this->buffer_count_ - buffer_start;
        
    IndexVector brick_size = this->schema_->get_brick_size();
    IndexVector eff_brick_size = this->schema_->get_effective_brick_size();
    IndexVector::index_type overlap = this->schema_->get_overlap();
    IndexVector level_size = this->schema_->get_level_size( this->level_ );

    for (IndexVector::index_type z = 0; z < this->layout_.z(); z++)
    {
      IndexVector::index_type b_start = ( z * eff_brick_size.z() );
      IndexVector::index_type b_end = ( Min( ( z + 1 ) * eff_brick_size.z(), level_size.z() ) ) + 2 * overlap;

      IndexVector::index_type z_start = b_start - buffer_start;
      IndexVector::index_type z_end = b_end - buffer_start;

      IndexVector::index_type start = Max( IndexVector::index_type( 0 ), z_start );
      IndexVector::index_type end = Min( z_end , buffer_size );

            IndexVector::index_type offset = buffer_start + start - b_start;
            
      if ( end >= 0 && start < buffer_size && start < end)
      {

        std::cout << "saving buffers level " << this->level_ << ": 000000/000000";

        for (size_t k = 0; k < this->buffers_.size(); k++ )
        {
          

          IndexVector::index_type brick = k + z * (this->layout_.x() * this->layout_.y() );
          BrickInfo bi( brick, this->level_ );

          std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b" << std::setfill('0') << std::setw(6) << (k+1) << "/" << std::setfill('0') << std::setw(6) << this->buffers_.size();
          std::cout.flush();

          if (! this->schema_->append_brick_buffer( this->buffers_[ k ], start, end, offset, bi, error ) )
          {
            return false;
          }
        }
        std::cout << std::endl;
      }

    }


    // reset ring buffer count
    this->buffer_index_ = 0;

  }

  return true;
}



class LargeVolumeConverterPrivate {

public:
  LargeVolumeConverterPrivate() :
    data_type_( DataType::UNKNOWN_E )
  {}

  // -- input parameters --
public:

  boost::filesystem::path first_file_;

  std::vector< boost::filesystem::path > files_;

  DataType data_type_;
  IndexVector data_size_;
  Vector spacing_;
  Point origin_;
  IndexVector brick_size_;
  size_t overlap_;

  long long mem_limit_;

  LargeVolumeSchemaHandle schema_;

  // -- loaders --
public:
  /// LOAD_FILE
  /// Load a file from a stack into memory
  DataBlockHandle load_file( const boost::filesystem::path& filename, std::string& error );

  /// LOAD_FILE_INTERNALS
  /// Intrernals for dealing with data type 
  template<class T>
  DataBlockHandle load_file_internals( const boost::filesystem::path& filename, std::string& error );

  /// SCAN_FILE
  /// Scan file to determine type and size
  bool scan_file( const boost::filesystem::path& filename, std::string& error );
    
    // -- downsample --
public:
    /// DOWNSAMPLE
    /// Down sample a slice based on the level ratios
    bool downsample( DataBlockHandle input, DataBlockHandle output,
        const IndexVector& input_ratio, const IndexVector& output_ratio );
    
    /// DOWNSAMPLE_INTERNALS
    /// Templated version that does internal computation
    template<class T, class U>
    bool downsample_internals( DataBlockHandle input, DataBlockHandle output,
        const IndexVector& input_ratio, const IndexVector& output_ratio );

    /// DOWNSAMPLE_ADD
    /// Down sample a slice based on the level ratios and adds it to the existing slice
    bool downsample_add( DataBlockHandle input, DataBlockHandle output,
        const IndexVector& input_ratio, const IndexVector& output_ratio );
    
    /// DOWNSAMPLE_ADD_INTERNALS
    /// Templated version that does internal computation
    template<class T, class U>
    bool downsample_add_internals( DataBlockHandle input, DataBlockHandle output,
        const IndexVector& input_ratio, const IndexVector& output_ratio );

    // -- min and max --
public:

  /// COMPUTE_MIN_MAX_INTERNALS
  /// Compute min and max value
  template<class T>
  bool compute_min_max_internals( DataBlockHandle slice, double& min, double& max );

  /// COMPUTE_MIN_MAX
  /// Compute min and max value
  bool compute_min_max( DataBlockHandle slice, double& min, double& max );


    // -- slice processor --
public:
    bool process_slice( size_t level, std::string& error );
    

    // slices at different resolution levels
    std::vector<DataBlockHandle> slices_;

    std::vector<IndexVector::index_type> index_;
  std::vector<LargeVolumeBrickLevelHandle> brick_level_;

  void run_phase3_parallel( int num_threads, int thread_num, boost::barrier& barrier  );

  bool success_;

};

template<class T>
bool LargeVolumeConverterPrivate::compute_min_max_internals( DataBlockHandle slice, double& min, double& max )
{
  T* data = reinterpret_cast<T*>( slice->get_data() );
  size_t size = slice->get_size();

  T min_val = std::numeric_limits<T>::max();
  T max_val = std::numeric_limits<T>::min();

  for (size_t k = 0; k < size; k++, data++ )
  {
    if (*data < min_val) min_val = *data;
    if (*data > max_val) max_val = *data;
  }

  min = Min( min, static_cast<double>( min_val ) );
  max = Max( max, static_cast<double>( max_val ) );

  return true;
}

bool LargeVolumeConverterPrivate::compute_min_max( DataBlockHandle slice, double& min, double& max )
{
  switch( slice->get_data_type() )
  {
    case DataType::CHAR_E:
      return compute_min_max_internals<signed char>( slice, min, max);
    case DataType::UCHAR_E:
      return compute_min_max_internals<unsigned char>( slice, min, max);
    case DataType::SHORT_E:
      return compute_min_max_internals<short>( slice, min, max);
    case DataType::USHORT_E:
      return compute_min_max_internals<unsigned short>( slice, min, max);
    case DataType::INT_E:
      return compute_min_max_internals<int>( slice, min, max);
    case DataType::UINT_E:
      return compute_min_max_internals<unsigned int>( slice, min, max); 
    case DataType::FLOAT_E:
      return compute_min_max_internals<float>( slice, min, max);
    case DataType::DOUBLE_E:
      return compute_min_max_internals<double>( slice, min, max); 
  }

  return false;
}

bool LargeVolumeConverterPrivate::process_slice( size_t level, std::string& error )
{
  bool last_slice = ( this->files_.size() - 1 ) == this->index_[ 0 ];
  bool first_slice = ( this->index_[ level ] == 0);

    DataBlockHandle slice = slices_[ level ];
    
    // Brick the data
    if ( first_slice )
  {
    DataBlockHandle empty = Core::StdDataBlock::New( this->slices_[ level ]->get_nx(),
      this->slices_[ level ]->get_ny(), this->slices_[ level ]->get_nz(),
      this->slices_[ level ]->get_data_type() );
    empty->clear();

    size_t overlap = this->schema_->get_overlap();
    for (size_t k = 0; k < overlap; k++ )
    {
      this->brick_level_[ level ]->insert_slice( empty );
      this->brick_level_[ level ]->sync_buffers( false, error );
    }
  }

  this->brick_level_[ level ]->insert_slice( this->slices_[ level ] );

    if ( last_slice )
  {
    DataBlockHandle empty = Core::StdDataBlock::New( this->slices_[ level ]->get_nx(),
      this->slices_[ level ]->get_ny(), this->slices_[ level ]->get_nz(),
      this->slices_[ level ]->get_data_type() );
    empty->clear();

    size_t overlap = this->schema_->get_overlap();
    for (size_t k = 0; k < overlap; k++ )
    {
      this->brick_level_[ level ]->sync_buffers( false, error );
      this->brick_level_[ level ]->insert_slice( empty );
    }

    this->brick_level_[ level ]->sync_buffers( true, error );
  }
  else
  {
    this->brick_level_[ level ]->sync_buffers( false, error );
  }
    
    // Down sample data for next level
    if ( level < this->schema_->get_num_levels() - 1 )
    {
        IndexVector input_ratio = this->schema_->get_level_downsample_ratio( level );
        IndexVector output_ratio = this->schema_->get_level_downsample_ratio( level + 1);
        
        if ( output_ratio.z() / input_ratio.z() == 2 )
        {
            if ( index_[ level ] % 2 )
            {
                if (! this->downsample_add( slices_[ level ], slices_[ level + 1 ], input_ratio, output_ratio ) )
                {
                    error = "Failed to downsample slice.";
                    return false;
                }
                if (! this->process_slice( level + 1, error ) )
                {
                    return false;
                }
            }
            else
            {
                if (! this->downsample( slices_[ level ], slices_[ level + 1 ], input_ratio, output_ratio ) )
                {
                    error = "Failed to downsample slice.";
                    return false;
                }

        if ( last_slice )
        {
          if (! this->process_slice( level + 1, error ) )
          {
            return false;
          }       
        }
            }
        }
        else
        {
            if (! this->downsample( slices_[ level ], slices_[ level + 1 ], input_ratio, output_ratio ) )
            {
                error = "Failed to downsample slice.";
                return false;
            }
            
            if (! this->process_slice( level + 1, error ) )
            {
                return false;
            }
        }
        
    }

  index_[ level ]++;

  return true;
}


template<class T>
DataBlockHandle LargeVolumeConverterPrivate::load_file_internals( const boost::filesystem::path& filename, std::string& error )
{
  error = "";

  typedef itk::ImageFileReader< itk::Image< T, 2 > > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

  // Setup the importer
  reader->SetFileName( filename.string() );

  try
  {
    reader->Update();
  }
  catch( ... )
  {
    error = "ITK Crashed while reading file.";
    return DataBlockHandle();
  }

  // Grab the image from the output so we can read its transform
  typedef ITKImage2DDataT<T> ITKImageContainer;
  typename ITKImageContainer::Handle image_data;

  try
  {
    image_data = typename ITKImageContainer::Handle( 
        new ITKImageContainer( reader->GetOutput() ) ); 
  }
  catch ( ... )
  {
    error = "Importer could not read itk object.";
    return DataBlockHandle();
  } 

  return ITKDataBlock::New( image_data );
}


DataBlockHandle LargeVolumeConverterPrivate::load_file( const boost::filesystem::path& filename, std::string& error )
{

  switch (this->data_type_)
  {
    case DataType::UCHAR_E:
      return this->load_file_internals<unsigned char>( filename, error );
    case DataType::CHAR_E:
      return this->load_file_internals<signed char>( filename, error );
    case DataType::USHORT_E:
      return this->load_file_internals<unsigned short>( filename, error );
    case DataType::SHORT_E:
      return this->load_file_internals<short>( filename, error ); 
    case DataType::UINT_E:
      return this->load_file_internals<unsigned int>( filename, error );
    case DataType::INT_E:
      return this->load_file_internals<int>( filename, error );   
    case DataType::FLOAT_E:
      return this->load_file_internals<float>( filename, error );
    case DataType::DOUBLE_E:
      return this->load_file_internals<double>( filename, error );    
  }

  error = "Could not determine data type.";
  return DataBlockHandle();
}

bool LargeVolumeConverterPrivate::scan_file( const boost::filesystem::path& filename, std::string& error )
{
  error = "";

  typedef itk::ImageFileReader< itk::Image< unsigned char, 2 > > ReaderType;
  ReaderType::Pointer reader = ReaderType::New();

  // We explicitly spell out the importer to use
  if ( Core::FileUtil::CheckExtension( filename, ".png") )
  {
    reader->SetImageIO( itk::PNGImageIO::New() );
  } 
  else if ( Core::FileUtil::CheckExtension( filename, ".tif|.tiff") )
  {
    reader->SetImageIO( itk::TIFFImageIO::New() );  
  }
  else if ( Core::FileUtil::CheckExtension( filename, ".jpg|.jpeg") )
  {
    reader->SetImageIO( itk::JPEGImageIO::New() );  
  }
  else if ( Core::FileUtil::CheckExtension( filename, ".dcm|.dicom") )
  {
    // ITK defaults without additional checks in GDCM importer
    reader->SetImageIO( itk::GDCMImageIO::New() );
  }

  // Setup the importer
  reader->SetFileName( filename.string() );

  try
  {
    reader->Update();
  }
  catch( ... )
  {
    error = "ITK Crashed while reading file.";
    return false;
  }

  itk::ImageIOBase* IO = reader->GetImageIO();

  // Grab the information on the data type from the ITK image
  std::string type_string = IO->GetComponentTypeAsString( IO->GetComponentType() );
  
  if( type_string == "unsigned_char" ) this->data_type_ = Core::DataType::UCHAR_E;
  if( type_string == "char" ) this->data_type_ = Core::DataType::CHAR_E;
  if( type_string == "unsigned_short" ) this->data_type_ = Core::DataType::USHORT_E;
  if( type_string == "short" ) this->data_type_ = Core::DataType::SHORT_E;
  if( type_string == "unsigned_int" ) this->data_type_ = Core::DataType::UINT_E;
  if( type_string == "int" ) this->data_type_ = Core::DataType::INT_E;
  if( type_string == "float" ) this->data_type_ = Core::DataType::FLOAT_E;
  if( type_string == "double" ) this->data_type_ = Core::DataType::DOUBLE_E;

  if ( this->data_type_ == Core::DataType::UNKNOWN_E )
  {
    error = "Could not determine data type.";
    return false;
  }
  
  // Grab the image from the output so we can read its transform
  Core::ITKUCharImage2DDataHandle image_data;
  try
  {
    image_data =  Core::ITKUCharImage2DDataHandle( 
        new Core::ITKUCharImage2DData( reader->GetOutput() ) ); 
  }
  catch ( ... )
  {
    error = "Importer could not read itk object.";
    return false;
  } 
  
  this->data_size_.x( image_data->get_nx() );
  this->data_size_.y( image_data->get_ny() );

  return true;
}


template<class T, class U>
bool LargeVolumeConverterPrivate::downsample_internals( DataBlockHandle input, DataBlockHandle output,
    const IndexVector& input_ratio, const IndexVector& output_ratio )
{
    DataBlock::index_type ratio_x = output_ratio.x() / input_ratio.x();
    DataBlock::index_type ratio_y = output_ratio.y() / input_ratio.y();

    T* src = reinterpret_cast<T*>( input->get_data() );
    T* dst = reinterpret_cast<T*>( output->get_data() );

    DataBlock::index_type nx = input->get_nx();
    DataBlock::index_type ny = input->get_ny();

    if ( ratio_x == 2 && ratio_y == 2 )
    {
        for ( DataBlock::index_type y = 0; y < (ny-1); y += 2, src+= nx )
        {
            for ( DataBlock::index_type x = 0; x < (nx-1); x += 2, src += 2, dst++ )
            {
                *dst = static_cast<T>( ( static_cast<U>( src[0] ) + static_cast<U>( src[1] ) + static_cast<U>( src[nx] ) +  static_cast<U>( src[nx+1] ) ) / 4 );
            }
        
            if (nx % 2)
            {
                *dst = static_cast<T>( ( static_cast<U>( src[0] ) + static_cast<U>( src[nx] ) ) / 2 );
                dst++;
                src++;
            }
        }
        
        if ( ny % 2 )
        {
            for ( DataBlock::index_type x = 0; x < (nx-1); x += 2, src += 2, dst++ )
            {
                *dst = static_cast<T>( ( static_cast<U>( src[0] ) + static_cast<U>( src[1] ) ) / 2 );
            }
        
            if (nx % 2)
            {
                *dst = *src;
                dst++;
                src++;
            }
        }
    }
    else if ( ratio_x == 2 && ratio_y == 1 )
    {
        for ( DataBlock::index_type y = 0; y < ny; y ++)
        {
            for ( DataBlock::index_type x = 0; x < (nx-1); x += 2, src += 2, dst++ )
            {
                *dst = static_cast<T>( ( static_cast<U>( src[0] ) + static_cast<U>( src[1] ) ) / 2 );
            }
        
            if (nx % 2)
            {
                *dst = *src;
                dst++;
                src++;
            }
        }
    }
    else if ( ratio_x == 1 && ratio_y == 2 )
    {
        for ( DataBlock::index_type y = 0; y < (ny-1); y += 2, src+= nx )
        {
            for ( DataBlock::index_type x = 0; x < nx; x++, src ++, dst++ )
            {
                *dst = static_cast<T>( ( static_cast<U>( src[0] ) + static_cast<U>( src[nx] ) ) / 2 );
            }
        }
        
        if ( ny % 2 )
        {
            for ( DataBlock::index_type x = 0; x < nx; x ++, src ++, dst++ )
            {
                *dst = *src;
            }
        }
    }
    else
    {
        DataBlock::index_type nxy = nx * ny;
        for ( DataBlock::index_type k = 0; k < nxy; k++, src++, dst++  )
        {
            *dst = *src;
        }
    }
    
    return true;
}

bool LargeVolumeConverterPrivate::downsample( DataBlockHandle input, DataBlockHandle output,
    const IndexVector& input_ratio, const IndexVector& output_ratio )
{
    if ( input->get_data_type() != output->get_data_type() )
    {
        return false;
    }
    
    switch( input->get_data_type() )
    {
        case DataType::UCHAR_E:
            return this->downsample_internals<unsigned char, unsigned short>( input, output, input_ratio, output_ratio );
        case DataType::CHAR_E:
            return this->downsample_internals<signed char, short>( input, output, input_ratio, output_ratio );
        case DataType::USHORT_E:
            return this->downsample_internals<unsigned short, unsigned int>( input, output, input_ratio, output_ratio );
        case DataType::SHORT_E:
            return this->downsample_internals<short, int>( input, output, input_ratio, output_ratio );
        case DataType::UINT_E:
            return this->downsample_internals<unsigned int, unsigned long long>( input, output, input_ratio, output_ratio );
        case DataType::INT_E:
            return this->downsample_internals<int, long long>( input, output, input_ratio, output_ratio );
        case DataType::FLOAT_E:
            return this->downsample_internals<float, float>( input, output, input_ratio, output_ratio );
        case DataType::DOUBLE_E:
            return this->downsample_internals<double, double>( input, output, input_ratio, output_ratio );
    }
    
    return false;
}

template<class T, class U>
bool LargeVolumeConverterPrivate::downsample_add_internals( DataBlockHandle input, DataBlockHandle output,
    const IndexVector& input_ratio, const IndexVector& output_ratio )
{
    DataBlock::index_type ratio_x = output_ratio.x() / input_ratio.x();
    DataBlock::index_type ratio_y = output_ratio.y() / input_ratio.y();

    T* src = reinterpret_cast<T*>( input->get_data() );
    T* dst = reinterpret_cast<T*>( output->get_data() );

    DataBlock::index_type nx = input->get_nx();
    DataBlock::index_type ny = input->get_ny();

    if ( ratio_x == 2 && ratio_y == 2 )
    {
        for ( DataBlock::index_type y = 0; y < (ny-1); y += 2, src+= nx )
        {
            for ( DataBlock::index_type x = 0; x < (nx-1); x += 2, src += 2, dst++ )
            {
                *dst = static_cast<T>( ( static_cast<U>( dst[0] * 4 ) + static_cast<U>( src[0] ) + static_cast<U>( src[1] ) + static_cast<U>( src[nx] ) +  static_cast<U>( src[nx+1] ) ) / 8 );
            }
        
            if (nx % 2)
            {
                *dst = static_cast<T>( ( static_cast<U>( dst[0] * 2 ) + static_cast<U>( src[0] ) + static_cast<U>( src[nx] ) ) / 4 );
                dst++;
                src++;
            }
        }
        
        if ( ny % 2 )
        {
            for ( DataBlock::index_type x = 0; x < (nx-1); x += 2, src += 2, dst++ )
            {
                *dst = static_cast<T>( ( static_cast<U>( dst[0] * 2 ) +  static_cast<U>( src[0] ) + static_cast<U>( src[1] ) ) / 4 );
            }
        
            if (nx % 2)
            {
                *dst = static_cast<T>( ( static_cast<U>( *dst ) + static_cast<U>( *src ) ) / 2 );
                dst++;
                src++;
            }
        }
    }
    else if ( ratio_x == 2 && ratio_y == 1 )
    {
        for ( DataBlock::index_type y = 0; y < ny; y ++)
        {
            for ( DataBlock::index_type x = 0; x < (nx-1); x += 2, src += 2, dst++ )
            {
                *dst = static_cast<T>( ( static_cast<U>( dst[0] * 2 ) + static_cast<U>( src[0] ) + static_cast<U>( src[1] ) ) / 4 );
            }
        
            if (nx % 2)
            {
                *dst = static_cast<T>( ( static_cast<U>( *dst ) + static_cast<U>( *src ) ) / 2 );
                dst++;
                src++;
            }
        }
    }
    else if ( ratio_x == 1 && ratio_y == 2 )
    {
        for ( DataBlock::index_type y = 0; y < (ny-1); y += 2, src+= nx )
        {
            for ( DataBlock::index_type x = 0; x < nx; x++, src++, dst++ )
            {
                *dst = static_cast<T>( ( static_cast<U>( dst[0] * 2 ) + static_cast<U>( src[0] ) + static_cast<U>( src[nx] ) ) / 4 );
            }
        }
        
        if ( ny % 2 )
        {
            for ( DataBlock::index_type x = 0; x < nx; x ++, src++, dst++ )
            {
                *dst = static_cast<T>( ( static_cast<U>( *dst ) + static_cast<U>( *src ) ) / 2 );
            }
        }
    }
    else
    {
        DataBlock::index_type nxy = nx * ny;
        for ( DataBlock::index_type k = 0; k < nxy; k++, src++, dst++  )
        {
            *dst = static_cast<T>( ( static_cast<U>( *dst ) + static_cast<U>( *src ) ) / 2 );
        }
    }
    
    return true;
}


bool LargeVolumeConverterPrivate::downsample_add( DataBlockHandle input, DataBlockHandle output,
    const IndexVector& input_ratio, const IndexVector& output_ratio )
{
    if ( input->get_data_type() != output->get_data_type() )
    {
        return false;
    }
    
    switch( input->get_data_type() )
    {
        case DataType::UCHAR_E:
            return downsample_add_internals<unsigned char, unsigned short>( input, output, input_ratio, output_ratio );
        case DataType::CHAR_E:
            return downsample_add_internals<signed char, short>( input, output, input_ratio, output_ratio );
        case DataType::USHORT_E:
            return downsample_add_internals<unsigned short, unsigned int>( input, output, input_ratio, output_ratio );
        case DataType::SHORT_E:
            return downsample_add_internals<short, int>( input, output, input_ratio, output_ratio );
        case DataType::UINT_E:
            return downsample_add_internals<unsigned int, unsigned long long>( input, output, input_ratio, output_ratio );
        case DataType::INT_E:
            return downsample_add_internals<int, long long>( input, output, input_ratio, output_ratio );
        case DataType::FLOAT_E:
            return downsample_add_internals<float, float>( input, output, input_ratio, output_ratio );
        case DataType::DOUBLE_E:
            return downsample_add_internals<double, double>( input, output, input_ratio, output_ratio );
    }
    
    return false;
}


LargeVolumeConverter::LargeVolumeConverter() :
    private_( new LargeVolumeConverterPrivate )
{
  this->private_->schema_ = LargeVolumeSchemaHandle( new LargeVolumeSchema() );
}


void LargeVolumeConverter::set_output_dir( const boost::filesystem::path& dir )
{
  this->private_->schema_->set_dir( dir );
}

void LargeVolumeConverter::set_first_file( const boost::filesystem::path& first_file )
{
  this->private_->first_file_ = first_file;
}

LargeVolumeSchemaHandle LargeVolumeConverter::get_schema() const
{
  return this->private_->schema_;
}

void LargeVolumeConverter::set_schema_parameters( const Vector& spacing, const Point& origin, const IndexVector& brick_size, size_t overlap )
{
  this->private_->spacing_ = spacing;
  this->private_->origin_ = origin;
  this->private_->brick_size_ = brick_size;
  this->private_->overlap_ = overlap;
}

bool LargeVolumeConverter::run_phase1( std::string& error )
{
  error = "";

  // Find all the other files in the series
  if( ! FileUtil::FindFileSeries( this->private_->first_file_, this->private_->files_, error ) )
  {
    return false;
  }

  // Set the output size
  this->private_->data_size_.z( this->private_->files_.size() );

  // Now load a file to determine data type and size
  if (! this->private_->scan_file( this->private_->files_[ 0 ], error ) )
  {
    return false;
  }

  this->private_->schema_->set_parameters( this->private_->data_size_, this->private_->spacing_,
    this->private_->origin_, this->private_->brick_size_, this->private_->overlap_, this->private_->data_type_ );

  this->private_->schema_->set_compression( true );
  this->private_->schema_->compute_levels();

  return true;
}

void LargeVolumeConverter::set_mem_limit( long long mem_limit )
{
  this->private_->mem_limit_ = mem_limit;
}


bool LargeVolumeConverter::run_phase2( std::string& error )
{
  error = "";

  // Save schema file
  if (! this->private_->schema_->save(error) )
  {
    return false;
  }

  // Start creating bricks

  // Calculate number of slice buffers
  size_t num_levels = this->private_->schema_->get_num_levels();

    // Calculate size for down sample slices
    size_t slice_buffer_size = 0;
  size_t element_size = Core::GetSizeDataType( this->private_->schema_->get_data_type() );
    
    // Calculate size for each slice
  for ( size_t j = 0; j < num_levels; j++)
  {
        IndexVector level_size = this->private_->schema_->get_level_size( j );
        size_t slice_size = level_size.x() * level_size.y() * element_size;
        slice_buffer_size += slice_size;
        // First slice most likely needs space to decompress
        if (j == 0 ) slice_buffer_size += slice_size;
    }

    // Check total size
  if ( slice_buffer_size > this->private_->mem_limit_ )
  {
    error = "Please allocate more memory to conversion process.";
    return false;
  }

    // Initialize parameters for each level
    this->private_->slices_.resize( num_levels );
    this->private_->index_.resize( num_levels, 0 );
  
  this->private_->brick_level_.resize( num_levels );

    // Allocate resample buffers

  size_t num_buffers = 0;

    for ( size_t j = 0; j < num_levels; j++ )
    {
        IndexVector level_size = this->private_->schema_->get_level_size( j );
        size_t slice_size = level_size.x() * level_size.y();
        if ( j > 0 )
    {
        // NOTE: The first one will always be allocated by ITK
      this->private_->slices_[ j ] = StdDataBlock::New( level_size.x(), level_size.y(), 1, this->private_->schema_->get_data_type() );
    }
    this->private_->brick_level_[ j ] = LargeVolumeBrickLevelHandle( new LargeVolumeBrickLevel( this->private_->schema_, j ) ) ;

    num_buffers += this->private_->brick_level_[ j ]->get_num_buffers();
    }

  IndexVector brick_size = this->private_->schema_->get_brick_size();
  size_t buffer_size = Min( static_cast<size_t>( brick_size.z() ), static_cast<size_t>( (this->private_->mem_limit_ - slice_buffer_size ) / ( num_buffers * element_size * brick_size.x() * brick_size.y() ) ) );

  if ( buffer_size == 0 )
  {
    error = "Please allocate more memory to conversion process.";
    return false; 
  }

    for ( size_t j = 0; j < num_levels; j++ )
    {
    this->private_->brick_level_[ j ]->allocate_buffers( buffer_size );
  }

    // Main loading loop
    size_t num_files = this->private_->files_.size();
    double min = std::numeric_limits<double>::max();
  double max = std::numeric_limits<double>::min();

  IndexVector total_size = this->private_->schema_->get_size();

    for ( IndexVector::index_type slice_idx = 0; slice_idx < num_files; slice_idx++)
    {

        // indicate which slice is being processed
        std::cout << "Processing file: " << this->private_->files_[ slice_idx ].string() << std::endl;
    
        // load slice
        this->private_->slices_[ 0 ] = this->private_->load_file( this->private_->files_[ slice_idx ], error );
    
    if (! this->private_->slices_[ 0 ] ) 
    {
      return false;
    }

    if ( this->private_->slices_[ 0 ]->get_nx() != total_size.x() ||  this->private_->slices_[ 0 ]->get_ny() != total_size.y() )
    {
      std::cout << "WARNING: Dimensions of the slices are not equal, clipping/padding image to fit dimensions of first image." <<std::endl;
      DataBlock::Clip( this->private_->slices_[ 0 ], this->private_->slices_[ 0 ], total_size.x(), total_size.y(), 1, 0.0 );
    } 
  
    
    if (! this->private_->compute_min_max( this->private_->slices_[ 0 ], min, max ) )
    {
      error = "Could not compute min and max.";
      return false;
    }

    this->private_->schema_->set_min_max( min, max );

        if (! this->private_->process_slice( 0, error ) )
        {
            return false;
        }
    }

  // Save schema file to update min and max
  if (! this->private_->schema_->save( error ) )
  {
    return false;
  }

  this->private_->slices_.clear();
  this->private_->brick_level_.clear();
  this->private_->index_.clear();


  return true;
}

void LargeVolumeConverterPrivate::run_phase3_parallel( int thread_num, int num_threads, boost::barrier& barrier )
{
  std::string error;

  size_t num_levels = this->schema_->get_num_levels();

    for ( size_t j = 0; j < num_levels; j ++ )
    {
    if ( thread_num == 0 )
    {
      std::cout << "processing level: " << ExportToString( j ) << std::endl; 
    }

        IndexVector layout = this->schema_->get_level_layout( j );
        IndexVector::index_type num_bricks = layout[0] * layout[1] * layout[2];
        
    if ( thread_num == 0 )
    {
      std::cout << "processing brick: 000000/000000";
    }

        for ( size_t k = thread_num;  k < num_bricks; k += num_threads )
        {
      if ( thread_num == 0 )
      {
        std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b" << std::setfill('0') << std::setw(6) << (k+1) << "/" << std::setfill('0') << std::setw(6) << num_bricks;
        std::cout.flush();
      }
      
      BrickInfo bi( k ,j );
            if (! this->schema_->reprocess_brick( bi, error) )
      {
                std::cerr << error << std::endl;
                this->success_ = false;
        break;
      }
        }
    barrier.wait();

    if ( thread_num == 0 )
    {
      std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b" << std::setfill('0') << std::setw(6) << num_bricks << "/" << std::setfill('0') << std::setw(6) << num_bricks;
      std::cout << std::endl;
            
    }
  }

}

bool LargeVolumeConverter::run_phase3( std::string& error )
{
  error = "";
  this->private_->success_ = true;

  Parallel parallel( boost::bind( &LargeVolumeConverterPrivate::run_phase3_parallel, this->private_, _1, _2, _3 ) );

  parallel.run();
    if ( !this->private_->success_ )
    {
        error = "Could not compress bricks.";
    }

  return this->private_->success_;
}


} // end namespace
