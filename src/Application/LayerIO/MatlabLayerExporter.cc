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

#include <vector>

// Core includes
#include <Core/DataBlock/NrrdData.h>
#include <Core/DataBlock/StdDataBlock.h>

// Application includes
#include <Application/LayerIO/MatlabLayerExporter.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/PreferencesManager/PreferencesManager.h>

#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

SEG3D_REGISTER_EXPORTER( Seg3D, MatlabLayerExporter );

namespace Seg3D
{

void MatlabLayerExporter::createDoubleArray(const char* name, double value, int index, MatlabIO::matlabarray& array)
{
  MatlabIO::matlabarray testma;
  testma.createdensearray(1, 1, MatlabIO::matlabarray::miDOUBLE);
  testma.setnumericarray(&value, 1);
  array.setfield( index, name, testma );
}

void MatlabLayerExporter::createStringArray(const char* name, const char* value, int index, MatlabIO::matlabarray& array)
{
  MatlabIO::matlabarray testma;
  testma.createstringarray( value );
  array.setfield( index, name, testma );
}

void MatlabLayerExporter::configureDataLayerAxis(MatlabIO::matlabarray& axisma, DataLayer* layer)
{
  Core::GridTransform tf = layer->get_data_volume()->get_grid_transform();  
  configureAxis(axisma, tf);
}

void MatlabLayerExporter::configureMaskLayerAxis(MatlabIO::matlabarray& axisma, MaskLayer* layer)
{
  Core::GridTransform tf = layer->get_mask_volume()->get_grid_transform();
  configureAxis(axisma, tf);
}

void MatlabLayerExporter::configureAxis(MatlabIO::matlabarray& axisma, Core::GridTransform& tf)
{
  // Set the properies of the axis
  std::vector<std::string> axisfieldnames;
  axisfieldnames += "size", "spacing", "min", "max", "center", "label", "unit";
  
  std::vector<int> dims( 2 );
  dims[0] = 3;
  dims[1] = 1;
  
  axisma.createstructarray( dims, axisfieldnames );
  Core::Vector size( tf.get_nx(), tf.get_ny(), tf.get_nz() );
  Core::Vector spacing( tf.project( Core::Vector( 1.0, 0.0, 0.0 ) ).length(), 
                       tf.project( Core::Vector( 0.0, 1.0, 0.0 ) ).length(),
                       tf.project( Core::Vector( 0.0, 0.0, 1.0 ) ).length() );      
  
  
  Core::Point min( tf.project( Core::Point( 0.0 ,0.0, 0.0 ) ) );      
  Core::Point max( tf.project( Core::Point( static_cast<double>( tf.get_nx() - 1 ),
    static_cast<double>( tf.get_ny() - 1 ) , static_cast<double>( tf.get_nz() - 1 ) ) ) );      
  Core::Vector center( 0.0, 0.0, 0.0 );
  
  if ( ! tf.get_originally_node_centered() )
  {
    min -= ( spacing * 0.5 );
    max += ( spacing * 0.5 );
    center = Core::Vector( 1.0, 1.0, 1.0 );
  }
  
  for (int p = 0; p < 3; p++ )
  {
    createDoubleArray("size", static_cast<double>(size[ p ]), p, axisma);
    createDoubleArray("spacing", static_cast<double>(spacing[ p ]), p, axisma);
    createDoubleArray("min", static_cast<double>(min[ p ]), p, axisma);
    createDoubleArray("max", static_cast<double>(max[ p ]), p, axisma);
    createDoubleArray("center", static_cast<double>(center[ p ]), p, axisma);
    createStringArray("label", "axis ", p, axisma);
    createStringArray("unit", "no unit", p, axisma);
  }
}
  
MatlabLayerExporter::MatlabLayerExporter( std::vector< LayerHandle >& layers ) :
  LayerExporter( layers )
{
  if( !layers[ 0 ] ) return;
}

bool MatlabLayerExporter::export_layer( const std::string& mode, const std::string& file_path, 
  const std::string& name )
{
  boost::filesystem::path path = boost::filesystem::path( file_path ) / ( name + ".mat" );
  bool success = false;
  
    // Export data as single volume file
  if ( mode == "data" )
  {
    success = this->export_matfile( path.string() );
  }
    // Export data as separate files
  else if ( mode == "single_mask" )
  {
    success = this->export_single_masks( file_path );
  }
    // Export data as a single file
  else if ( mode == "label_mask" )
  {
    success = this->export_mask_label( path.string() );
  }
    
  if( success ) CORE_LOG_SUCCESS( "Matlab export has been successfully completed." );
  return success;
}

bool MatlabLayerExporter::export_matfile( const std::string& file_path )
{
  DataLayer* layer = dynamic_cast< DataLayer* >( 
    this->layers_[ 0 ].get() );

  MatlabIO::matlabarray mldata;
  std::vector<int> dims( 3 );
  dims[ 0 ] = layer->get_data_volume()->get_nx(); 
  dims[ 1 ] = layer->get_data_volume()->get_ny(); 
  dims[ 2 ] = layer->get_data_volume()->get_nz(); 
  
  MatlabIO::matlabarray::mitype dataformat;
  switch( layer->get_data_volume()->get_data_type() )
  {
    case Core::DataType::CHAR_E: dataformat = MatlabIO::matlabarray::miINT8; break;
    case Core::DataType::UCHAR_E: dataformat = MatlabIO::matlabarray::miUINT8; break;
    case Core::DataType::SHORT_E: dataformat = MatlabIO::matlabarray::miINT16; break;
    case Core::DataType::USHORT_E: dataformat = MatlabIO::matlabarray::miUINT16; break;
    case Core::DataType::INT_E: dataformat = MatlabIO::matlabarray::miINT32; break;
    case Core::DataType::UINT_E: dataformat = MatlabIO::matlabarray::miUINT32; break;
    case Core::DataType::FLOAT_E: dataformat = MatlabIO::matlabarray::miSINGLE; break;
    case Core::DataType::DOUBLE_E: dataformat = MatlabIO::matlabarray::miDOUBLE; break;
    default:
    return false;
  }
  
  mldata.createdensearray( dims, dataformat );

  Core::DataBlockHandle data_block = layer->get_data_volume()->get_data_block();

  switch( layer->get_data_volume()->get_data_type() )
  {
    case Core::DataType::CHAR_E: 
      mldata.setnumericarray( reinterpret_cast<char *>( data_block->get_data() ),
        data_block->get_size(), dataformat );
      break;
    case Core::DataType::UCHAR_E: 
      mldata.setnumericarray( reinterpret_cast<unsigned char *>( data_block->get_data() ),
        data_block->get_size(), dataformat );
      break;
    case Core::DataType::SHORT_E: 
      mldata.setnumericarray( reinterpret_cast<short *>( data_block->get_data() ),
        data_block->get_size(), dataformat );
      break;
    case Core::DataType::USHORT_E: 
      mldata.setnumericarray( reinterpret_cast<unsigned short *>( data_block->get_data() ),
        data_block->get_size(), dataformat );
      break;
    case Core::DataType::INT_E: 
      mldata.setnumericarray( reinterpret_cast<int *>( data_block->get_data() ),
        data_block->get_size(), dataformat );
      break;
    case Core::DataType::UINT_E: 
      mldata.setnumericarray( reinterpret_cast<unsigned int *>( data_block->get_data() ),
        data_block->get_size(), dataformat );
      break;
    case Core::DataType::FLOAT_E: 
      mldata.setnumericarray( reinterpret_cast<float *>( data_block->get_data() ),
        data_block->get_size(), dataformat );
      break;
    case Core::DataType::DOUBLE_E: 
      mldata.setnumericarray( reinterpret_cast<double *>( data_block->get_data() ),
        data_block->get_size(), dataformat );
      break;
    default:
    return false;
  }

  MatlabIO::matlabarray mlarray;

  mlarray.createstructarray();
  mlarray.setfield( 0, "data", mldata );

  MatlabIO::matlabarray axisma;
  configureDataLayerAxis(axisma, layer);
    
  mlarray.setfield( 0, "axis", axisma );

  MatlabIO::matlabfile output( file_path, "w" );
  output.putmatlabarray( mlarray, "scirunnrrd" );
  output.close();
  
  return true;
}

bool MatlabLayerExporter::export_single_masks( const std::string& path )
{
  // We process all of the layers individually in this loop
  for( int i = 0; i < static_cast< int >( this->layers_.size() ); ++i )
  {
    // Step 1: Get a pointer to the mask so that we can get at it's MaskDataBlock
    //std::vector< std::string > mask_name = Core::SplitString( layer_names[ i ], "," );
    MaskLayer* layer = dynamic_cast< MaskLayer* >( this->layers_[ i ].get() );

    // Step 2: Get a pointer to the mask's MaskDataBlock
    Core::MaskDataBlockHandle mask_block = layer->get_mask_volume()->
      get_mask_data_block();
      
    Core::DataBlock::shared_lock_type lock( mask_block->get_mutex() );    

    // Step 3: Using the size and type information from our mask's MaskDataBlock, we create a 
    // new empty DataBlock
    Core::DataBlockHandle new_data_block = Core::StdDataBlock::New( mask_block->get_nx(),
      mask_block->get_ny(), mask_block->get_nz(), Core::DataType::UCHAR_E );

    // Step 4: Using the data in our mask's MaskDataBlock we set the values in our new DataBlock
    for( size_t j = 0; j < mask_block->get_size(); ++j )
    {
      if( mask_block->get_mask_at( j ) )
      {
        new_data_block->set_data_at( j, 1 );
      }
      else
      { 
        new_data_block->set_data_at( j, 0 );
      }
    }

    lock.unlock();
  
    MatlabIO::matlabarray mldata;
    std::vector<int> dims( 3 );
    dims[ 0 ] = layer->get_mask_volume()->get_nx(); 
    dims[ 1 ] = layer->get_mask_volume()->get_ny(); 
    dims[ 2 ] = layer->get_mask_volume()->get_nz(); 
    
    
    MatlabIO::matlabarray::mitype dataformat = MatlabIO::matlabarray::miINT8;   
    mldata.createdensearray( dims, dataformat );

    mldata.setnumericarray( reinterpret_cast<char *>( new_data_block->get_data() ),
          new_data_block->get_size(), dataformat );
    
    MatlabIO::matlabarray mlarray;

    mlarray.createstructarray();
    mlarray.setfield( 0, "data", mldata );

    MatlabIO::matlabarray axisma;
    configureMaskLayerAxis(axisma, layer);
      
    mlarray.setfield( 0, "axis", axisma );

    boost::filesystem::path mask_path = boost::filesystem::path( path ) / 
      ( layer->get_layer_name() + ".mat" );
      
    MatlabIO::matlabfile output( mask_path.string(), "w" );
    output.putmatlabarray( mlarray, "scirunnrrd" );   
    output.close();
  }
  return true;
}

bool MatlabLayerExporter::export_mask_label( const std::string& file_path )
{
  // Step 1: We get a pointer to one of the MaskLayers so we can get its MaskDataBlock
  std::vector < std::string > first_mask_name_and_number;
  MaskLayer* layer = dynamic_cast< MaskLayer* >( this->layers_[ 1 ].get() );

  // Step 2: Get a handle to its MaskDataBlock and use that to build a new DataBlockHandle of the 
  // same size and type.
  Core::MaskDataBlockHandle mask_block = layer->get_mask_volume()->get_mask_data_block();
  Core::DataBlockHandle new_data_block = Core::StdDataBlock::New( mask_block->get_nx(),
    mask_block->get_ny(), mask_block->get_nz(), Core::DataType::UCHAR_E );

  // Step 3: Get the value the user set for the background and preset the contents of our new
  // datablock to that value
  for( size_t i = 0; i < mask_block->get_size(); ++i )
  {
    new_data_block->set_data_at( i, this->label_values_[ 0 ] );
  }
  
  // Step 4: Loop through all the MaskLayers and insert their values into our new DataBlock
  std::vector< MaskLayer* >  mask_layers;
  for( int i = 1; i < static_cast< int >( this->layers_.size() ); ++i )
  {
    layer = dynamic_cast< MaskLayer* >( this->layers_[ i ].get() );
    mask_block = layer->get_mask_volume()->get_mask_data_block();
    
    Core::DataBlock::shared_lock_type lock( mask_block->get_mutex() );
    
    for( size_t j = 0; j < mask_block->get_size(); ++j )
    {
      if( mask_block->get_mask_at( j ) )
      {
        new_data_block->set_data_at( j, this->label_values_[ i ] );
      }
    } 
    
    lock.unlock();
  }

  MatlabIO::matlabarray mldata;
  std::vector<int> dims( 3 );
  dims[ 0 ] = layer->get_mask_volume()->get_nx(); 
  dims[ 1 ] = layer->get_mask_volume()->get_ny(); 
  dims[ 2 ] = layer->get_mask_volume()->get_nz(); 
  
  
  MatlabIO::matlabarray::mitype dataformat = MatlabIO::matlabarray::miINT8;   
  mldata.createdensearray( dims, dataformat );
  mldata.setnumericarray( reinterpret_cast<char *>( new_data_block->get_data() ),
        new_data_block->get_size(), dataformat );
  MatlabIO::matlabarray mlarray;

  mlarray.createstructarray();
  mlarray.setfield( 0, "data", mldata );

  MatlabIO::matlabarray axisma;
  configureMaskLayerAxis(axisma, layer);

  mlarray.setfield( 0, "axis", axisma );

  MatlabIO::matlabfile output( file_path, "w" );
  output.putmatlabarray( mlarray, "scirunnrrd" );
  output.close();

  return true;
}

} // end namespace seg3D
