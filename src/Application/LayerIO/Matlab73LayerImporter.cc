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

// HDF5 includes
#include <itk_hdf5.h>
#include <itk_H5Cpp.h>
#include <itkhdf5/H5LTpublic.h>

// Core includes
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/Utils/Log.h>

// Application includes
#include <Application/LayerIO/Matlab73LayerImporter.h>

#include <iostream>

#define MAX_DIMS 3
#define BUFFER_SIZE 256

SEG3D_REGISTER_IMPORTER( Seg3D, Matlab73LayerImporter );

namespace Seg3D
{

class Matlab73LayerImporterPrivate
{
public:
Matlab73LayerImporterPrivate() :
  grid_transform_( 1, 1, 1 ),
  data_type_( Core::DataType::UNKNOWN_E ),
  read_header_( false ),
  read_data_( false ),
  object_is_struct_(false),
  ROOT_GROUP_C("/")
  {
  }

  // Pointer back to the main class
  Matlab73LayerImporter* importer_;

  public:
  // Information extracted from the header
  Core::GridTransform grid_transform_;
  Core::DataType data_type_;

  // Data extracted from the file
  Core::DataBlockHandle data_block_;

  // Cache the Matlab object to be read
  H5std_string matlab_object_name_;
  H5std_string matlab_dataset_name_;

public:
  // SCAN_MAT_OBJECT:
  // Check whether a dataset can be imported and if so extract data type and transform from
  // file.
  bool scan_mat_object( hid_t obj_id, H5O_type_t type, H5::Group& parent_group, std::string& error );

  // SCAN_MAT_MATRIX:
  bool scan_mat_array( hid_t obj_id, const char *attribute_data, std::string& error );

  // SCAN_MAT_STRUCT:
  bool scan_mat_struct( hid_t obj_id, H5::Group& parent_group, std::string& error );

  // SCAN_MAT_FILE:
  // Scan whether there is a compatible object in the mat file
  bool scan_mat_file( const std::string& filename );

  // IMPORT_MAT_STRUCT:
  //bool import_mat_struct( H5::Group& group, std::string& error );

  // IMPORT_MAT_ARRAY:
  // Import the matlab array into the program.
  bool import_mat_array( H5::DataSet& dataset, std::string& error );

  // IMPORT_MAT_FILE:
  // Scan through the file and now import the data
  bool import_mat_file( const std::string& filename );

  // CONVERT_TYPE:
  // Convert the matlabio types to Seg3D types
  Core::DataType convert_type( const std::string& matlab_type );

  // Whether the header has been read
  bool read_header_;

  // Whether the data has been read
  bool read_data_;

  bool object_is_struct_;

  const std::string ROOT_GROUP_C;
};

class H5ObjectWrapperPrivate
{
public:
  H5ObjectWrapperPrivate(const hid_t& object_id) :
    object_id_(object_id) {}

  ~H5ObjectWrapperPrivate()
  {
    if (H5Oclose(object_id_) < 0)
    {
    CORE_LOG_ERROR("Error closing object");
    }
    CORE_LOG_DEBUG("closed object in ~H5ObjectWrapper()");
  }
private:
  hid_t object_id_;
};


Core::DataType Matlab73LayerImporterPrivate::convert_type( const std::string& matlab_type )
{
  // Convert type information
  if      (matlab_type == "int8")   return Core::DataType::CHAR_E;
  else if (matlab_type == "uint8")  return Core::DataType::UCHAR_E;
  else if (matlab_type == "int16")  return Core::DataType::SHORT_E;
  else if (matlab_type == "uint16") return Core::DataType::USHORT_E;
  else if (matlab_type == "int32")  return Core::DataType::INT_E;
  else if (matlab_type == "uint32") return Core::DataType::UINT_E;
  else if (matlab_type == "single") return Core::DataType::FLOAT_E;
  else if (matlab_type == "double") return Core::DataType::DOUBLE_E;
  else                              return Core::DataType::UNKNOWN_E;
}

bool Matlab73LayerImporterPrivate::scan_mat_object( hid_t obj_id, H5O_type_t type, H5::Group& parent_group, std::string& error )
{
  error = "";

  if ( H5Aexists(obj_id, "MATLAB_sparse") > 0)
  {
    error = "importing sparse matrices is not supported";
    CORE_LOG_DEBUG(error);
    return false;
  }

  hid_t mat_class_id;
  // group should have an attribute with type: struct, scalar type (matrix) etc.
  // attribute should always be called MATLAB_class
  if ( (mat_class_id = H5Aopen(obj_id, "MATLAB_class", H5P_DEFAULT)) < 0)
  {
    error = "error opening dataset MATLAB_class attribute";
    return false;
  }

  char attribute_data[BUFFER_SIZE];

  if (H5LTget_attribute_string(mat_class_id, ".", "MATLAB_class", attribute_data ) < 0)
  {
    error = "error getting attribute MATLAB_class string";
    return false;
  }
  H5Aclose(mat_class_id);

  // TODO: is 'object' still used?
  if ( (type == H5O_TYPE_GROUP) && (std::string(attribute_data).find("struct") != std::string::npos) )
  {
    this->object_is_struct_ = true;
    return scan_mat_struct(obj_id, parent_group, error);
  }
  else
  {
    return scan_mat_array(obj_id, attribute_data, error);
  }
  return false;
}

bool Matlab73LayerImporterPrivate::scan_mat_array( hid_t obj_id, const char *attribute_data, std::string& error )
{
  error = "";

  hsize_t storage_size = H5Dget_storage_size(obj_id);
  if (storage_size == 0)
  {
    error = "matlab dataset does not contain any data.";
    return false;
  }

  hsize_t dims[MAX_DIMS];
  H5T_class_t type_class;
  size_t type_size;

  int rank;
  if (H5LTget_dataset_ndims(obj_id, ".", &rank) < 0)
  {
    error = "H5LTget_dataset_info failed";
    return false;
  }

  if (H5LTget_dataset_info(obj_id, ".", dims, &type_class, &type_size) < 0)
  {
    error = "H5LTget_dataset_info failed";
    return false;
  }

  if (rank > MAX_DIMS)
  {
    error = "Matrix dimension is larger than 3.";
    return false;
  }

  std::vector<hsize_t> vdims(rank);
  for (int i = 0; i < MAX_DIMS; ++i)
  {
    if (i < rank)
      vdims[i] = dims[i];
    else
      vdims.push_back(1);
  }

  this->grid_transform_ = Core::GridTransform(static_cast<size_t>( vdims[ 2 ] ),
                                              static_cast<size_t>( vdims[ 1 ] ),
                                              static_cast<size_t>( vdims[ 0 ] ));
  this->grid_transform_.set_originally_node_centered( false );
  this->data_type_ = this->convert_type(attribute_data);
  this->read_header_ = true;

  return true;
}

bool Matlab73LayerImporterPrivate::scan_mat_struct( hid_t obj_id, H5::Group& parent_group, std::string& error )
{
  error = "";

  try
  {
    H5::Group group(parent_group.openGroup(this->matlab_object_name_));
    hid_t group_id = group.getId();

    const hsize_t num_obj = group.getNumObjs();
    if (num_obj < 0)
    {
      error = "H5Gget_num_objs failed";
      return false;
    }

    for ( hsize_t i = 0; i < num_obj; ++i)
    {
      // there doesn't seem to be another way to do this through the C++ interface
      hid_t obj_id;
      if ( (obj_id = H5Oopen_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_NATIVE, i, H5P_DEFAULT)) < 0 )
      {
        error = "H5Oopen_by_idx failed";
        return false;
      }
      H5ObjectWrapperPrivate object_wrapper(obj_id);

      H5O_info_t obj_info;
      if (H5Oget_info(obj_id, &obj_info) < 0)
      {
        error = "H5Oopen_by_idx failed";
        return false;
      }

      H5std_string obj_name = group.getObjnameByIdx(i);

      // TODO: could not find a Matlab dataset with an 'axis' field
      // so not sure if that would be a sub-group?
      //
      // For now, only looking at datasets
      if (obj_info.type == H5O_TYPE_DATASET)
      {
        if ((obj_name == "data") ||
          (obj_name == "potvals") ||
          (obj_name == "field") ||
          (obj_name == "scalarfield"))
        {
          // TODO: until we can figure out how axis info is structured,
          // assume that no spacing, min or max given
          //
          // Use the same method as for simple dataset
          this->matlab_dataset_name_ = obj_name;
          if (scan_mat_object( obj_id, obj_info.type, group, error ))
          {
            return true;
          }
        }
      }
    }
    error = "Matlab struct does not have a 'data', 'potvals', or 'field' in its main data structure.";
  }
  catch( H5::GroupIException e )
  {
    error = e.getDetailMsg();
  }
  catch ( ... )
  {
    error = "Scanning Matlab struct failed";
  }
  return false;
}

bool Matlab73LayerImporterPrivate::scan_mat_file( const std::string& filename )
{
  // Check if we already read it
  if ( this->read_header_ ) return true;

  std::string error;

  try
  {
    H5::Exception::dontPrint();

    H5::H5File file(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    H5::Group root_group(file.openGroup(ROOT_GROUP_C.c_str()));
    hid_t root_id = root_group.getId();

    const hsize_t num_obj = root_group.getNumObjs();
    if (num_obj < 0)
    {
      error = "H5Gget_num_objs failed";
      return false;
    }

    for ( hsize_t i = 0; i < num_obj; ++i)
    {
      // there doesn't seem to be another way to do this through the C++ interface
      hid_t obj_id;
      if ( (obj_id = H5Oopen_by_idx(root_id, ".", H5_INDEX_NAME, H5_ITER_NATIVE, i, H5P_DEFAULT)) < 0 )
      {
        error = "H5Oopen_by_idx failed";
        return false;
      }
      H5ObjectWrapperPrivate object_wrapper(obj_id);

      H5O_info_t obj_info;
      if (H5Oget_info(obj_id, &obj_info) < 0)
      {
        error = "H5Oopen_by_idx failed";
        return false;
      }

      H5std_string obj_name = root_group.getObjnameByIdx(i);

      H5L_info_t link_info;
      if (H5Lget_info_by_idx(root_group.getId(), ".", H5_INDEX_NAME, H5_ITER_NATIVE, i, &link_info, H5P_DEFAULT) < 0)
      {
        error = "H5Lget_info failed";
        return false;
      }

      // link types info: http://www.hdfgroup.org/HDF5/doc/UG/UG_frame09Groups.html
      bool hard_link = false;
      switch (link_info.type)
      {
        case H5L_TYPE_ERROR:
        error = "link type is H5L_TYPE_ERROR";
        return false;
        case H5L_TYPE_HARD:
        CORE_LOG_DEBUG("H5L_TYPE_HARD");
        hard_link = true;
        break;
        case H5L_TYPE_SOFT:
        CORE_LOG_DEBUG("H5L_TYPE_SOFT");
        default:
        break;
      }

      if (! hard_link) continue;

      // Two hierarchies found so far:
      //
      // Root group -> dataset(s): matlab array or matrix
      // Root group -> group(s) -> dataset(s): compound data structures
      //
      // Can use the h5dump tool to explore files

      if ( (obj_info.type == H5O_TYPE_GROUP) || (obj_info.type == H5O_TYPE_DATASET) )
      {
        this->matlab_object_name_ = obj_name;
        this->matlab_dataset_name_ = obj_name;

        if (scan_mat_object( obj_id, obj_info.type, root_group, error ))
        {
          return true;
        }
      }
    }
  }
  catch( H5::FileIException e )
  {
    error = e.getDetailMsg();
  }
  catch( H5::GroupIException e )
  {
    error = e.getDetailMsg();
  }
  catch ( ... )
  {
    error = "Matlab v7.3 Importer crashed while reading file header.";
  }

  // If the cause is known report an error to the user
  if ( ! error.empty() )
  {
    this->importer_->set_error( error );
  }

  return false;
}

bool Matlab73LayerImporterPrivate::import_mat_array( H5::DataSet& dataset, std::string& error )
{
  // Generate a new data block
  this->data_block_ = Core::StdDataBlock::New( this->grid_transform_.get_nx(),
                    this->grid_transform_.get_ny(),
                    this->grid_transform_.get_nz(),
                    this->data_type_ );

  // We need to check if we could allocate the destination datablock
  if ( !this->data_block_ )
  {
  error = "Could not allocate enough memory to read Matlab file.";
  return false;
  }

  try
  {
    const size_t DATA_BLOCK_SIZE = this->data_block_->get_size();
    const int RANK = MAX_DIMS;
    hsize_t dims[MAX_DIMS];
    hsize_t dims2[MAX_DIMS];
    dims2[0] = this->data_block_->get_nx();
    dims2[1] = this->data_block_->get_ny();
    dims2[2] = this->data_block_->get_nz();

    H5::DSetCreatPropList propList = dataset.getCreatePlist();
    for (int i = 0; i < propList.getNfilters(); ++i)
    {
      unsigned int flags, filter_config;
      // We don't care about the filter name or the compression level (cd_values[0] in HDF5 docs)
      size_t cd_values_size = 0, buff_size = 0;

      H5Z_filter_t filter_id = propList.getFilter(i, flags, cd_values_size, 0, buff_size, 0, filter_config);
      unsigned int decode_enabled =  filter_config & H5Z_FILTER_CONFIG_DECODE_ENABLED;
      if ( filter_id == H5Z_FILTER_DEFLATE && decode_enabled == 0 )
      {
        std::ostringstream oss;
        oss << "GZIP data compression was detected in " << this->importer_->get_filename() << ", but the importer was not configured to support GZIP decompression.";
        error = oss.str();
        return false;
      }
    }


  // get full space for dataset
  H5::DataSpace dataspace = dataset.getSpace();
  int rank = dataspace.getSimpleExtentNdims();
  int ndims = dataspace.getSimpleExtentDims( dims, NULL);
  size_t length = this->data_block_->get_size() * Core::GetSizeDataType( this->data_type_ );


  // get memory space to read dataset
  H5::DataSpace memspace(MAX_DIMS, dims2);

  // PredType wraps H5T_* types
  // HDF5 native types are probably best to use here
  switch( this->data_type_ )
  {
    case Core::DataType::CHAR_E:
    {
      signed char* data = reinterpret_cast<signed char*>( this->data_block_->get_data() );
      dataset.read(data, H5::PredType::NATIVE_CHAR, memspace, dataspace);
      break;
    }
    case Core::DataType::UCHAR_E:
    {
      unsigned char* data = reinterpret_cast<unsigned char*>( this->data_block_->get_data() );
      dataset.read(data, H5::PredType::NATIVE_UCHAR, memspace, dataspace);
      break;
    }
    case Core::DataType::SHORT_E:
    {
      short* data = reinterpret_cast<short*>( this->data_block_->get_data() );
      dataset.read(data, H5::PredType::NATIVE_SHORT, memspace, dataspace);
      break;
    }
    case Core::DataType::USHORT_E:
    {
      unsigned short* data = reinterpret_cast<unsigned short*>( this->data_block_->get_data() );
      dataset.read(data, H5::PredType::NATIVE_USHORT, memspace, dataspace);
      break;
    }
    case Core::DataType::INT_E:
    {
      int* data = reinterpret_cast<int*>( this->data_block_->get_data() );
      dataset.read(data, H5::PredType::NATIVE_INT, memspace, dataspace);
      break;
    }
    case Core::DataType::UINT_E:
    {
      unsigned int* data = reinterpret_cast<unsigned int*>( this->data_block_->get_data() );
      dataset.read(data, H5::PredType::NATIVE_UINT, memspace, dataspace);
      break;
    }
    case Core::DataType::FLOAT_E:
    {
      float* data = reinterpret_cast<float*>( this->data_block_->get_data() );
      dataset.read(data, H5::PredType::NATIVE_FLOAT, memspace, dataspace);
      break;
    }
    case Core::DataType::DOUBLE_E:
    {
      double* data = reinterpret_cast<double*>( this->data_block_->get_data() );
      dataset.read(data, H5::PredType::NATIVE_DOUBLE, memspace, dataspace);
      break;
    }
    default:
      error = "trying to read unknown data type";
      return false;
    }
    return true;
  }
  catch( H5::Exception e )
  {
    error = e.getDetailMsg();
  }
  catch ( ... )
  {
    error = "Matlab v7.3 Importer crashed while reading data.";
  }

  return false;
}


bool Matlab73LayerImporterPrivate::import_mat_file( const std::string& filename )
{
  // Check if we already read the data.
  if ( this->read_data_ ) return true;

  // Ensure that we read the header of this file.
  if ( ! this->read_header_ )
  {
    this->importer_->set_error( "Failed to read header of Matlab file." );
    return false;
  }

  std::string error;
  H5::DataSet dataset;

  try
  {
    H5::Exception::dontPrint();

    H5::H5File file(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    H5::Group root_group(file.openGroup(ROOT_GROUP_C.c_str()));
    hid_t root_id = root_group.getId();

    CORE_LOG_DEBUG( std::string( "H5Fopen " ) + filename );

    if (this->object_is_struct_)
    {
      H5::Group group = root_group.openGroup(this->matlab_object_name_);
      dataset = group.openDataSet(this->matlab_dataset_name_);
    }
    else
    {
      dataset = root_group.openDataSet(this->matlab_dataset_name_);
    }

    if ( import_mat_array( dataset, error ) )
    {
      this->read_data_ = true;
      return true;
    }
  }
  catch( H5::FileIException e )
  {
    error = e.getDetailMsg();
  }
  catch( H5::GroupIException e )
  {
    error = e.getDetailMsg();
  }
  catch( H5::DataSetIException e )
  {
    error = e.getDetailMsg();
  }
  catch ( ... )
  {
    error = "Matlab v7.3 Importer crashed while reading file.";
  }

  // If the cause is known report an error to the user
  if ( ! error.empty() )
  {
    this->importer_->set_error( error );
  }

  return false;
}

Matlab73LayerImporter::Matlab73LayerImporter() :
private_( new Matlab73LayerImporterPrivate )
{
  private_->importer_ = this;
}

Matlab73LayerImporter::~Matlab73LayerImporter()
{
}

bool Matlab73LayerImporter::get_file_info( LayerImporterFileInfoHandle& info )
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
    this->set_error( "Matlab v7.3 Importer crashed while reading header." );
    return false;
  }
  return true;
}

bool Matlab73LayerImporter::get_file_data( LayerImporterFileDataHandle& data )
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
    this->set_error( "Matlab v7.3 Importer crashed while reading file." );
    return false;
  }

  return true;
}

} // end namespace seg3D
