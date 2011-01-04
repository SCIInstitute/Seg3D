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

// MatlabIO includes
#include <MatlabIO.h>

// Core includes
#include <Core/DataBlock/StdDataBlock.h>

// Application includes
#include <Application/LayerIO/MatlabLayerImporter.h>

SCI_REGISTER_IMPORTER( Seg3D, MatlabLayerImporter );

namespace Seg3D
{


class MatlabLayerImporterPrivate
{
public:
  Core::GridTransform grid_transform_;
  Core::DataBlockHandle data_block_;
  
  Core::DataType data_type_;

public:
  MatlabLayerImporterPrivate() :
    grid_transform_( 1, 1, 1 ),
    data_type_( Core::DataType::UNKNOWN_E ),
    read_header_( false )
  {}

  // IS_COMPATIBLE:
  // Check whether an array can be imported
  bool is_compatible( MatlabIO::matlabarray &mlarray );
  
  // IMPORT_MAT_ARRAY:
  // Import the matlab array into the program.
  bool import_mat_array( MatlabIO::matlabarray &mlarray );
  
  // SCAN_MAT_FILE:
  // Scan whether there is a compatible object in the mat file
  bool scan_mat_file( const std::string& filename );
  
  // IMPORT_MAT_FILE:
  // Scan through the file and now import the data
  bool import_mat_file( const std::string& filename );
  
  // CONVERT_TYPE:
  // Convert the matlabio types to Seg3D types
  Core::DataType convert_type( MatlabIO::matlabarray::mitype type );
  
  bool read_header_;
};

Core::DataType MatlabLayerImporterPrivate::convert_type( MatlabIO::matlabarray::mitype type )
{
  switch (type)
    {
    case MatlabIO::matlabarray::miINT8:   return Core::DataType::CHAR_E;
    case MatlabIO::matlabarray::miUINT8:  return Core::DataType::UCHAR_E;
    case MatlabIO::matlabarray::miINT16:  return Core::DataType::SHORT_E;
    case MatlabIO::matlabarray::miUINT16: return Core::DataType::USHORT_E;
    case MatlabIO::matlabarray::miINT32:  return Core::DataType::INT_E;
    case MatlabIO::matlabarray::miUINT32: return Core::DataType::UINT_E;
    case MatlabIO::matlabarray::miSINGLE: return Core::DataType::FLOAT_E;
    case MatlabIO::matlabarray::miDOUBLE: return Core::DataType::DOUBLE_E;
    default: return Core::DataType::UNKNOWN_E;
    }
}

bool MatlabLayerImporterPrivate::is_compatible( MatlabIO::matlabarray &mlarray )
{
  if ( mlarray.isempty() )
  {
    // empty array cannot contain any data
    return false ;
  }
  
  if ( mlarray.getnumelements() == 0 ) 
  {
    return false ;
  }
  
  MatlabIO::matlabarray::mlclass mclass;
  mclass = mlarray.getclass();
        
  // parse matrices are dealt with in a separate 
  // pr as the the data needs to be divided over
  // three separate Nrrds
        
  if ( mclass == MatlabIO::matlabarray::mlSTRUCT || mclass == MatlabIO::matlabarray::mlOBJECT )
  {
    int fieldnameindex;
    MatlabIO::matlabarray subarray;
              
    fieldnameindex = mlarray.getfieldnameindexCI( "data" );
    if (fieldnameindex == -1) fieldnameindex = mlarray.getfieldnameindexCI( "potvals" );
    if (fieldnameindex == -1) fieldnameindex = mlarray.getfieldnameindexCI( "field" );
    if (fieldnameindex == -1) fieldnameindex = mlarray.getfieldnameindexCI( "scalarfield" );
    if (fieldnameindex == -1) 
    {
      return false;
    }
    
    subarray = mlarray.getfield( 0, fieldnameindex );      
    if ( subarray.isempty() ) 
    {
      return false;
    }

    MatlabIO::matlabarray::mlclass mclass;
    mclass = subarray.getclass();
              
    if ( mclass != MatlabIO::matlabarray::mlDENSE && mclass != MatlabIO::matlabarray::mlSPARSE ) 
    {
      return false;
    }
              
    if ( subarray.getnumdims() > 3 )    
    {
      return false;    
    }
    
    if ( subarray.getnumelements() == 0 )
    {   
      return false;
    }      
              
    this->data_type_ = this->convert_type( subarray.gettype() );    
    return true;  
  }
        
  if ( mclass != MatlabIO::matlabarray::mlDENSE )
  {
    return false;
  }

  if ( mlarray.isempty() ) 
  {
    return false;
  }
    
  if ( mlarray.getnumelements() == 0 )
  {   
    return false;
  }      

  this->data_type_ = this->convert_type( mlarray.gettype() );   

  return true;
}


bool MatlabLayerImporterPrivate::scan_mat_file( const std::string& filename )
{
  MatlabIO::matlabfile matlab_file;
  MatlabIO::matlabarray matlab_array;
  
  try
  {
    matlab_file.open( filename, "r" );
    
    int numarrays = matlab_file.getnummatlabarrays();

    for (int j = 0; j < numarrays; j++ )
    {
      matlab_array = matlab_file.getmatlabarrayinfo( j );
      if ( this->is_compatible( matlab_array ) )
      {
        read_header_ = true;
        return true;
      }
    }
  }
  catch( ... )
  {
  }

  return false;
}

bool MatlabLayerImporterPrivate::import_mat_file( const std::string& filename )
{
  MatlabIO::matlabfile matlab_file;
  MatlabIO::matlabarray matlab_array;
  
  try
  {
    matlab_file.open( filename, "r" );
    
    int numarrays = matlab_file.getnummatlabarrays();

    for (int j = 0; j < numarrays; j++ )
    {
      matlab_array = matlab_file.getmatlabarray( j );
      if ( this->is_compatible( matlab_array ) )
      {
        return this->import_mat_array( matlab_array );
      }
    }
  }
  catch( ... )
  {
  }

  return false;
}


bool MatlabLayerImporterPrivate::import_mat_array( MatlabIO::matlabarray &mlarray )
{
  try
  {
    MatlabIO::matlabarray::mlclass mclass;
    mclass = mlarray.getclass();
                      
    switch( mclass )
    {
      case MatlabIO::matlabarray::mlDENSE:
      {   
        this->data_type_ = convert_type( mlarray.gettype() );
            
        std::vector<int> dims = mlarray.getdims();
        while ( dims.size() < 3 ) dims.push_back( 1 );

        this->data_block_ = Core::StdDataBlock::New( static_cast<size_t>( dims[ 0 ] ),
          static_cast<size_t>( dims[ 1 ] ), static_cast<size_t>( dims[ 2 ] ),
          this->data_type_ );
          
        switch( this->data_type_ )
        {
          case Core::DataType::CHAR_E:
            mlarray.getnumericarray( 
              static_cast<signed char *>( this->data_block_->get_data() ),
              static_cast<int>( this->data_block_->get_size() ) );
            break;
          case Core::DataType::UCHAR_E:
            mlarray.getnumericarray( 
              static_cast<unsigned char *>( this->data_block_->get_data() ),
              static_cast<int>( this->data_block_->get_size() ) );
            break;          
          case Core::DataType::SHORT_E:
            mlarray.getnumericarray( 
              static_cast<signed short *>( this->data_block_->get_data() ),
              static_cast<int>( this->data_block_->get_size() ) );
            break;
          case Core::DataType::USHORT_E:
            mlarray.getnumericarray( 
              static_cast<unsigned short *>( this->data_block_->get_data() ),
              static_cast<int>( this->data_block_->get_size() ) );
            break;  
          case Core::DataType::INT_E:
            mlarray.getnumericarray( 
              static_cast<signed int *>( this->data_block_->get_data() ),
              static_cast<int>( this->data_block_->get_size() ) );
            break;
          case Core::DataType::UINT_E:
            mlarray.getnumericarray( 
              static_cast<unsigned int *>( this->data_block_->get_data() ),
              static_cast<int>( this->data_block_->get_size() ) );
            break;  
          case Core::DataType::FLOAT_E:
            mlarray.getnumericarray( 
              static_cast<float *>( this->data_block_->get_data() ),
              static_cast<int>( this->data_block_->get_size() ) );
            break;
          case Core::DataType::DOUBLE_E:
            mlarray.getnumericarray( 
              static_cast<double *>( this->data_block_->get_data() ),
              static_cast<int>( this->data_block_->get_size() ) );
            break;
          default:
            return false;
        }
        
        this->grid_transform_ = Core::GridTransform( this->data_block_->get_nx(),
          this->data_block_->get_ny(), this->data_block_->get_nz() );
        
        return true;
      }
      case MatlabIO::matlabarray::mlSTRUCT:
      case MatlabIO::matlabarray::mlOBJECT:
      {
        int dataindex;
        dataindex = mlarray.getfieldnameindexCI("data");
        if ( dataindex == -1 ) dataindex = mlarray.getfieldnameindexCI( "potvals" );
        if ( dataindex == -1 ) dataindex = mlarray.getfieldnameindexCI( "field" );
        if ( dataindex == -1 ) dataindex = mlarray.getfieldnameindexCI( "scalarfield" );             
        if (dataindex == -1)
        {
           return false;
        }
                
        MatlabIO::matlabarray subarray;
        subarray = mlarray.getfield( 0, dataindex );
                  
        MatlabIO::matlabarray::mlclass subclass;
        subclass = subarray.getclass();
                  
        if ( subclass != MatlabIO::matlabarray::mlDENSE )
        {
          return false;
        }
            
        if ( !( this->import_mat_array( subarray ) ) ) return false;
                  
        // Add axes properties if they are specified
                  
        int axisindex;
        axisindex = mlarray.getfieldnameindexCI( "axis" );
                  
        if (axisindex != -1)
        {             
          float nx = static_cast<float>( this->data_block_->get_nx() );
          float ny = static_cast<float>( this->data_block_->get_ny() );
          float nz = static_cast<float>( this->data_block_->get_nz() );
          
          if ( mlarray.isfieldCI(  "axis" ) )
          {
            MatlabIO::matlabarray axisarray = mlarray.getfieldCI( 0, "axis" );
                        
            if ( ! axisarray.isstruct() ) return false;

            int numaxis = axisarray.getm();
            if ( numaxis > 3 ) numaxis = 3;
            
            bool read_spacing = false;
            bool read_min = false;
            bool read_max = false;
            
            Core::Vector spacing;
            Core::Point min;
            Core::Point max;

            for ( int p = 0 ; p < numaxis; p++ )
            { 
              if ( axisarray.isfield( "spacing" ) )
              {
                MatlabIO::matlabarray farray = axisarray.getfield( p, "spacing" );
                if ( farray.isdense() && farray.getnumelements() > 0 )
                {
                  farray.getnumericarray( &spacing[ p ], 1 );
                  read_spacing = true;
                }
              }

              if ( axisarray.isfield( "min" ) )
              {
                MatlabIO::matlabarray farray = axisarray.getfield( p, "min" );
                if ( farray.isdense() && farray.getnumelements() > 0 )
                {
                  farray.getnumericarray( &min[ p ], 1 );
                  read_min = true;
                }
              }

              if ( axisarray.isfield( "max" ) )
              {
                MatlabIO::matlabarray farray = axisarray.getfield( p, "max" );
                if ( farray.isdense() && farray.getnumelements() > 0 )
                {
                  farray.getnumericarray( &max[ p ], 1 );
                  read_max = true;
                }
              }           
            }
            
            Core::Point Origin;
            if ( ! read_spacing )
            {
              if ( read_min && read_max )
              {
                spacing = Core::Vector( 
                  ( max.x() - min.x() ) / Core::Max( 1.0f, nx ),
                  ( max.y() - min.y() ) / Core::Max( 1.0f, ny ),
                  ( max.z() - min.z() ) / Core::Max( 1.0f, nz ) );
              }
              else
              {
                spacing = Core::Vector( 1.0, 1.0, 1.0 );
              }
            }
            
            if ( read_min )
            {
              Origin = min + 0.5 * spacing;
            }
            else if ( read_max )
            {
              Origin = max - Core::Vector( spacing.x() * ( nx - 0.5 ), 
                spacing.y() * ( ny - 0.5 ), spacing.z() * ( nz - 0.5 ) );
            }
            
            this->grid_transform_ = Core::GridTransform( this->data_block_->get_nx(),
              this->data_block_->get_ny(), this->data_block_->get_nz(),
              Origin, spacing.x() * Core::Vector( 1.0, 0.0, 0.0 ), 
              spacing.y() * Core::Vector( 0.0, 1.0, 0.0 ),
              spacing.z() * Core::Vector( 0.0, 0.0, 1.0 ) );
          }
        }
        return true;        
      }
      default:
      return false;
    }
  }
  catch ( ... )
  {
  }
  
  return false;
}


MatlabLayerImporter::MatlabLayerImporter(const std::string& filename) :
  LayerImporter(filename),
  private_( new MatlabLayerImporterPrivate )
{
}

MatlabLayerImporter::~MatlabLayerImporter()
{
}

bool MatlabLayerImporter::import_header()
{
  if ( this->private_->read_header_ ) return true;
  return this->private_->scan_mat_file( this->get_filename() );
}

Core::GridTransform MatlabLayerImporter::get_grid_transform()
{
  return this->private_->grid_transform_;
}

Core::DataType MatlabLayerImporter::get_data_type()
{
  return this->private_->data_type_;
}

int MatlabLayerImporter::get_importer_modes()
{
  Core::DataType data_type = this->private_->data_type_;
  
  int importer_modes = 0;
  if ( Core::IsReal( data_type ) )
  {
    importer_modes |= LayerImporterMode::DATA_E;
  }
  
  if ( Core::IsInteger( data_type ) ) 
  {
    importer_modes |= LayerImporterMode::SINGLE_MASK_E | LayerImporterMode::BITPLANE_MASK_E |
      LayerImporterMode::LABEL_MASK_E | LayerImporterMode::DATA_E;
  }
  
  return importer_modes;
}

bool MatlabLayerImporter::load_data( Core::DataBlockHandle& data_block, 
                  Core::GridTransform& grid_transform )
{
  this->private_->import_mat_file( this->get_filename() );
  
  data_block = this->private_->data_block_;
  grid_transform = this->private_->grid_transform_;
  
  if ( data_block ) return true; else return false;
}

} // end namespace seg3D
