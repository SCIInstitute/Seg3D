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

// MatlabIO includes
#include <MatlabIO.h>

// Core includes
#include <Core/DataBlock/StdDataBlock.h>

// Application includes
#include <Application/LayerIO/MatlabLayerImporter.h>

SEG3D_REGISTER_IMPORTER( Seg3D, MatlabLayerImporter );

namespace Seg3D
{

class MatlabLayerImporterPrivate
{
public:
  MatlabLayerImporterPrivate() :
    grid_transform_( 1, 1, 1 ),
    data_type_( Core::DataType::UNKNOWN_E ),
    matlab_array_index_( -1 ),
    read_header_( false ),
    read_data_( false )
  {
  }

  // Pointer back to the main class
  MatlabLayerImporter* importer_;
  
public:
  // Information extracted from the header
  Core::GridTransform grid_transform_;
  Core::DataType data_type_;

  // Data extracted from the file
  Core::DataBlockHandle data_block_;
  
  int matlab_array_index_;
  
public:

  // SCAN_MAT_ARRAY:
  // Check whether an array can be imported and if so extract data type and transform from
  // file.
  bool scan_mat_array( MatlabIO::matlabarray &mlarray, std::string& error );

  // SCAN_MAT_FILE:
  // Scan whether there is a compatible object in the mat file
  bool scan_mat_file( const std::string& filename );
  
  // IMPORT_MAT_ARRAY:
  // Import the matlab array into the program.
  bool import_mat_array( MatlabIO::matlabarray &mlarray, std::string& error );
    
  // IMPORT_MAT_FILE:
  // Scan through the file and now import the data
  bool import_mat_file( const std::string& filename );
  
  // CONVERT_TYPE:
  // Convert the matlabio types to Seg3D types
  Core::DataType convert_type( MatlabIO::matlabarray::mitype type );
  
  // Whether the header has been read
  bool read_header_;
  
  // Whether the data has been read
  bool read_data_;
  
};

Core::DataType MatlabLayerImporterPrivate::convert_type( MatlabIO::matlabarray::mitype type )
{
  // Convert type information from one enum into another enum
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

bool MatlabLayerImporterPrivate::scan_mat_array( MatlabIO::matlabarray &mlarray, std::string& error )
{
  // Clear error string
  error = "";
  
  // Check if the matlab array is empty
  if ( mlarray.isempty() )
  {
    // empty array cannot contain any data
    error = "Matlab Object does not contain any data.";
    return false ;
  }
  
  // Check if there are any elements in the matlab array
  if ( mlarray.getnumelements() == 0 ) 
  {
    error = "Matlab Object is empty.";
    return false ;
  }
  
  // What type of matlab array are we dealing with
  MatlabIO::matlabarray::mlclass mclass;
  mclass = mlarray.getclass();
        
  // parse matrices are dealt with in a separate 
  // pr as the the data needs to be divided over
  // three separate Nrrds
        
  // If the class is struct or class, check whether the required data fields are present  
  if ( mclass == MatlabIO::matlabarray::mlSTRUCT || mclass == MatlabIO::matlabarray::mlOBJECT )
  {
    int fieldnameindex;
    MatlabIO::matlabarray subarray;
              
    // Check for different versions of where the data may be stored   
    fieldnameindex = mlarray.getfieldnameindexCI( "data" );
    if (fieldnameindex == -1) fieldnameindex = mlarray.getfieldnameindexCI( "potvals" );
    if (fieldnameindex == -1) fieldnameindex = mlarray.getfieldnameindexCI( "field" );
    if (fieldnameindex == -1) fieldnameindex = mlarray.getfieldnameindexCI( "scalarfield" );
    if (fieldnameindex == -1) 
    {
      // Could not find main data array in structure
      error = "Matlab array does not have a data, potvals, or field in its main data structure.";
      return false;
    }
    
    // Check whether the data array is not empty
    subarray = mlarray.getfield( 0, fieldnameindex );      
    if ( subarray.isempty() ) 
    {
      error = "Matlab array does not contain any data.";
      return false;
    }

    MatlabIO::matlabarray::mlclass mclass;
    mclass = subarray.getclass();
              
    if ( mclass != MatlabIO::matlabarray::mlDENSE ) 
    {
      error = "Matrix that should contain data is not a dense matrix.";
      return false;
    }

    std::vector<int> dims = subarray.getdims();
    bool old_version_4d_matrix = false;
              
    if ( dims.size() > 3 )    
    { 
      // The old version of Seg3D had one version that generated 4D matrices instead
      // of 3D matrices. Only one dimension is 1 in this case
      
      if ( dims.size() == 4 && dims[ 0 ] ==  1 )
      {
        dims[ 0 ] = dims[ 1 ];
        dims[ 1 ] = dims[ 2 ];
        dims[ 2 ] = dims[ 3 ];

        old_version_4d_matrix = true;
      } 
      
      if ( !old_version_4d_matrix )
      {
        error = "Matrix dimension is larger than 3.";
        return false;    

      }
    }
    
    if ( subarray.getnumelements() == 0 )
    {   
      error = "Matlab array does not contain any data.";
      return false;
    }      

    // Get dimensions of the data
    while ( dims.size() < 3 ) dims.push_back( 1 );

    size_t nx = static_cast<size_t>( dims[ 0 ] );
    size_t ny = static_cast<size_t>( dims[ 1 ] );
    size_t nz = static_cast<size_t>( dims[ 2 ] );

    float nxf = static_cast<float>( dims[ 0 ] );
    float nyf = static_cast<float>( dims[ 1 ] );
    float nzf = static_cast<float>( dims[ 2 ] );

    // Add axes properties if they are specified
    int axisindex;
    axisindex = mlarray.getfieldnameindexCI( "axis" );
              
    if ( axisindex != -1 )
    {
      if ( mlarray.isfieldCI(  "axis" ) )
      {
        MatlabIO::matlabarray axisarray = mlarray.getfieldCI( 0, "axis" );
                    
        if ( ! axisarray.isstruct() ) 
        {
          error = "Axis field needs to be structured objects.";
          return false;
        }
      
        int dim_start = 0;
        int numaxis = axisarray.getm();

        if ( numaxis == 4 && dims.size() == 4 )
        {
          numaxis = 4;
          dim_start = 1;
        }
        
        bool read_spacing = false;
        bool read_min = false;
        bool read_max = false;
        
        Core::Vector spacing;
        Core::Point min;
        Core::Point max;


        for ( int p = dim_start ; p < numaxis; p++ )
        { 
          if ( axisarray.isfield( "spacing" ) )
          {
            MatlabIO::matlabarray farray = axisarray.getfield( p, "spacing" );
            if ( farray.isdense() && farray.getnumelements() > 0 )
            {
              farray.getnumericarray( &spacing[ p - dim_start ], 1 );
              read_spacing = true;
            }
          }

          if ( axisarray.isfield( "min" ) )
          {
            MatlabIO::matlabarray farray = axisarray.getfield( p, "min" );
            if ( farray.isdense() && farray.getnumelements() > 0 )
            {
              farray.getnumericarray( &min[ p - dim_start], 1 );
              read_min = true;
            }
          }

          if ( axisarray.isfield( "max" ) )
          {
            MatlabIO::matlabarray farray = axisarray.getfield( p, "max" );
            if ( farray.isdense() && farray.getnumelements() > 0 )
            {
              farray.getnumericarray( &max[ p - dim_start ], 1 );
              read_max = true;              
            }
          }           
        }
        
        // Sets up cell centered data
        Core::Point Origin;
        if ( ! read_spacing )
        {
          if ( read_min && read_max )
          {
            spacing = Core::Vector( 
              ( max.x() - min.x() ) / Core::Max( 1.0f, nxf ),
              ( max.y() - min.y() ) / Core::Max( 1.0f, nyf ),
              ( max.z() - min.z() ) / Core::Max( 1.0f, nzf ) );
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
          Origin = max - Core::Vector( spacing.x() * ( nxf - 0.5 ), 
            spacing.y() * ( nyf - 0.5 ), spacing.z() * ( nzf - 0.5 ) );
        }
        
        this->grid_transform_ = Core::GridTransform( nx, ny, nz, Origin, 
          spacing.x() * Core::Vector( 1.0, 0.0, 0.0 ), 
          spacing.y() * Core::Vector( 0.0, 1.0, 0.0 ),
          spacing.z() * Core::Vector( 0.0, 0.0, 1.0 ) );
        this->grid_transform_.set_originally_node_centered( false );
      }
    }

    this->data_type_ = this->convert_type( subarray.gettype() );
    this->read_header_ = true;
    
    return true;  
  }
        
  if ( mclass != MatlabIO::matlabarray::mlDENSE )
  {
    error = "Matrix that should contain data is not a dense matrix.";
    return false;
  }

  if ( mlarray.isempty() ) 
  {
    error = "Matlab array does not contain any data.";
    return false;
  }
    
  if ( mlarray.getnumelements() == 0 )
  {   
    error = "Matlab array does not contain any data.";
    return false;
  }      

  std::vector<int> dims = mlarray.getdims();
  while ( dims.size() < 3 ) dims.push_back( 1 );

  size_t nx = static_cast<size_t>( dims[ 0 ] );
  size_t ny = static_cast<size_t>( dims[ 1 ] );
  size_t nz = static_cast<size_t>( dims[ 2 ] );

  this->grid_transform_ = Core::GridTransform( nx, ny, nz );
  this->data_type_ = this->convert_type( mlarray.gettype() );   
  this->read_header_ = true;

  return true;
}


bool MatlabLayerImporterPrivate::scan_mat_file( const std::string& filename )
{
  // Check if we already read it
  if ( this->read_header_ ) return true;

  // A matlabfile is an abstraction of the matlab file itself, and represents the database
  // of objects contained in the file
  MatlabIO::matlabfile matlab_file;
  // A matlabarray is a hierarchical object that is stored in the matlabfile
  MatlabIO::matlabarray matlab_array;

  // String from returning error
  std::string error;
  
  // Scan through all objects in the file and pick the first one that we can import.
  try
  {
    // Open the matlab file in read mode
    matlab_file.open( filename, "r" );
    
    // Get the number of objects stored in the file
    int numarrays = matlab_file.getnummatlabarrays();
    
    // Extract the objects one by one
    for ( int j = 0; j < numarrays; j++ )
    {
      // The info call scans through the entire file and reads all objects except the
      // large datablocks, entries with more than 100 elements. 
      matlab_array = matlab_file.getmatlabarray( j );
      if ( this->scan_mat_array( matlab_array, error ) )
      {
        this->matlab_array_index_ = j;
        this->read_header_ = true;
        return true;
      }
    }
  }
  catch( ... )
  {
    error = "Importer crashed while reading header of matlab file.";
  }
  
  // If the cause is known report an error to the user
  if ( ! error.empty() )
  {
    this->importer_->set_error( error );
  }

  return false;
}

bool MatlabLayerImporterPrivate::import_mat_array( MatlabIO::matlabarray &mlarray,
  std::string& error )
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
        if ( dims.size() == 4 )
        {
          dims[ 0 ] = dims[ 1 ];
          dims[ 1 ] = dims[ 2 ];
          dims[ 2 ] = dims[ 3 ];        
        }
 
        // node-centered (default)

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
            error = "Encountered unknown datatype in matlab file.";
            return false;
        }
        
        return true;
      }
      case MatlabIO::matlabarray::mlSTRUCT:
      case MatlabIO::matlabarray::mlOBJECT:
      {
        int dataindex;
        dataindex = mlarray.getfieldnameindexCI( "data" );
        if ( dataindex == -1 ) dataindex = mlarray.getfieldnameindexCI( "potvals" );
        if ( dataindex == -1 ) dataindex = mlarray.getfieldnameindexCI( "field" );
        if ( dataindex == -1 ) dataindex = mlarray.getfieldnameindexCI( "scalarfield" );             
        if ( dataindex == -1 )
        {
          error = "Matlab array needs to have field called data, potvals, or field.";
           return false;
        }
                
        MatlabIO::matlabarray subarray;
        subarray = mlarray.getfield( 0, dataindex );
                  
        MatlabIO::matlabarray::mlclass subclass;
        subclass = subarray.getclass();
                  
        if ( subclass != MatlabIO::matlabarray::mlDENSE )
        {
          error = "Only importing dense matrices is supported.";
          return false;
        }
            
        if ( !( this->import_mat_array( subarray, error ) ) ) 
        {
          return false;
        }
        return true;        
      }
      default:
      error = "Matlab array needs to be a dense matrix or a structure array.";
      return false;
    }
  }
  catch ( ... )
  {
  }
  
  error = "Could not open file.";
  return false;
}


bool MatlabLayerImporterPrivate::import_mat_file( const std::string& filename )
{
  // Check if we already read it
  if ( this->read_data_ ) return true;

  MatlabIO::matlabfile matlab_file;
  MatlabIO::matlabarray matlab_array;
  
  // String from returning error
  std::string error;
    
  try
  {
    matlab_file.open( filename, "r" );
    
    matlab_array = matlab_file.getmatlabarray( this->matlab_array_index_ );
    if ( this->import_mat_array( matlab_array, error ) )
    {
      this->read_data_ = true;
      return true;
    }
  }
  catch( ... )
  {
    error = "Importer crashed while reading matlab file.";
  }

  // If the cause is known report an error to the user
  if ( ! error.empty() )
  {
    this->importer_->set_error( error );
  }

  return false;
}



MatlabLayerImporter::MatlabLayerImporter() :
  private_( new MatlabLayerImporterPrivate )
{
  this->private_->importer_ = this;
}

MatlabLayerImporter::~MatlabLayerImporter()
{
}

bool MatlabLayerImporter::get_file_info( LayerImporterFileInfoHandle& info )
{
  try
  { 
    // Try to read the header
    if ( ! this->private_->scan_mat_file( this->get_filename() ) ) return false;
  
    // Generate an information structure with the information.
    info = LayerImporterFileInfoHandle( new LayerImporterFileInfo );
    info->set_data_type( this->private_->data_type_ );
    info->set_grid_transform( this->private_->grid_transform_ );
    info->set_file_type( "matlab" ); 
    info->set_mask_compatible( true );
  }
  catch ( ... )
  {
    // In case something failed, recover from here and let the user
    // deal with the error. 
    this->set_error( "Matlab Importer crashed while reading file." );
    return false;
  }
    
  return true;
}

bool MatlabLayerImporter::get_file_data( LayerImporterFileDataHandle& data )
{
  try
  { 
    // Read the data from the file
    if ( !this->private_->import_mat_file( this->get_filename() ) ) return false;
  
    // Create a data structure with handles to the actual data in this file 
    data = LayerImporterFileDataHandle( new LayerImporterFileData );
    data->set_data_block( this->private_->data_block_ );
    data->set_grid_transform( this->private_->grid_transform_ );
    data->set_name( this->get_file_tag() );
  }
  catch ( ... )
  {
    // In case something failed, recover from here and let the user
    // deal with the error. 
    this->set_error( "Matlab Importer crashed while reading file." );
    return false;
  }

  return true;
}

} // end namespace seg3D
