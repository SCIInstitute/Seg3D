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

// Core includes
#include <Core/DataBlock/NrrdData.h>
#include <Core/DataBlock/StdDataBlock.h>

// Application includes
#include <Application/LayerIO/NrrdLayerExporter.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/PreferencesManager/PreferencesManager.h>

SEG3D_REGISTER_EXPORTER( Seg3D, NrrdLayerExporter );

namespace Seg3D
{

NrrdLayerExporter::NrrdLayerExporter( std::vector< LayerHandle >& layers ) :
  LayerExporter( layers )
{
  if ( ! layers[ 0 ] ) return;
}

// public interface
bool NrrdLayerExporter::export_layer( const std::string& mode,
                                      const std::string& file_path, 
                                      const std::string& name )
{
  boost::filesystem::path path = boost::filesystem::path( file_path ) / ( name + ".nrrd" );
  bool success = false;
  
  if ( mode == "data" )
  {
    success = this->export_nrrd( path.string() );
  }
  else if ( mode == "single_mask" )
  {
    success = this->export_single_masks( file_path );
  }
  else if ( mode == "label_mask" )
  {
    success = this->export_mask_label( path.string() );
  }
    
  if ( success ) CORE_LOG_SUCCESS( "NRRD export has been successfully completed." );
  return success;
}


bool NrrdLayerExporter::export_nrrd( const std::string& file_path )
{
  DataLayer* temp_handle = dynamic_cast< DataLayer* >( 
    this->layers_[ 0 ].get() );

  Core::NrrdDataHandle nrrd = Core::NrrdDataHandle( new Core::NrrdData( 
    temp_handle->get_data_volume()->get_data_block(), temp_handle->get_grid_transform() ) );
  
  bool compress = PreferencesManager::Instance()->compression_state_->get();
  int level = PreferencesManager::Instance()->compression_level_state_->get();

  std::string error;
  if ( !( Core::NrrdData::SaveNrrd( file_path, nrrd, error, compress, level ) ) ) 
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  return true;
}

bool NrrdLayerExporter::export_single_masks( const std::string& path )
{
  // We process all of the layers individually in this loop
  for ( int i = 0; i < static_cast< int >( this->layers_.size() ); ++i )
  {
    // Step 1: Get a pointer to the mask so that we can get at it's MaskDataBlock
    //std::vector< std::string > mask_name = Core::SplitString( layer_names[ i ], "," );
    MaskLayer* temp_handle = dynamic_cast< MaskLayer* >( this->layers_[ i ].get() );

    // Step 2: Get a pointer to the mask's MaskDataBlock
    Core::MaskDataBlockHandle mask_block = temp_handle->get_mask_volume()->
      get_mask_data_block();
      
    Core::DataBlock::shared_lock_type lock( mask_block->get_mutex() );    

    // Step 3: Using the size and type information from our mask's MaskDataBlock, we create a 
    // new empty DataBlock
    Core::DataBlockHandle new_data_block = Core::StdDataBlock::New( mask_block->get_nx(),
      mask_block->get_ny(), mask_block->get_nz(), Core::DataType::UCHAR_E );

    // Step 4: Using the data in our mask's MaskDataBlock we set the values in our new DataBlock
    for ( size_t j = 0; j < mask_block->get_size(); ++j )
    {
      if ( mask_block->get_mask_at( j ) )
      {
        new_data_block->set_data_at( j, 1 );
      }
      else
      { 
        new_data_block->set_data_at( j, 0 );
      }
    }

    lock.unlock();
    
    // Step 5: Make a new nrrd using our new DataBlock
    Core::NrrdDataHandle nrrd = Core::NrrdDataHandle( new Core::NrrdData( 
      new_data_block, temp_handle->get_grid_transform() ) );

    std::string error;
    boost::filesystem::path mask_path = boost::filesystem::path( path ) / 
                                          ( temp_handle->get_layer_name() + ".nrrd" );

    // Step 6: Attempt to save the nrrd to the path that was passed and we return false if we 
    // can't
    bool compress = PreferencesManager::Instance()->compression_state_->get();
    int level = PreferencesManager::Instance()->compression_level_state_->get();
    
    if ( !( Core::NrrdData::SaveNrrd( mask_path.string(), nrrd, error, compress, level ) ) ) 
    {
      CORE_LOG_ERROR( error );
      return false;
    }
  }
  return true;
}

bool NrrdLayerExporter::export_mask_label( const std::string& file_path )
{
  // Step 1: We get a pointer to one of the MaskLayers so we can get its MaskDataBlock
  std::vector < std::string > first_mask_name_and_number;
  MaskLayer* temp_handle = dynamic_cast< MaskLayer* >( this->layers_[ 1 ].get() );

  // Step 2: Get a handle to its MaskDataBlock and use that to build a new DataBlockHandle of the 
  // same size and type.
  Core::MaskDataBlockHandle mask_block = temp_handle->get_mask_volume()->get_mask_data_block();
  Core::DataBlockHandle new_data_block = Core::StdDataBlock::New( mask_block->get_nx(),
    mask_block->get_ny(), mask_block->get_nz(), Core::DataType::UCHAR_E );

  // Step 3: Get the value the user set for the background and preset the contents of our new
  // datablock to that value
  for ( size_t i = 0; i < mask_block->get_size(); ++i )
  {
    new_data_block->set_data_at( i, this->label_values_[ 0 ]  );
  }
  
  // Step 4: Loop through all the MaskLayers and insert their values into our new DataBlock
  std::vector< MaskLayer* >  mask_layers;
  for ( int i = 1; i < static_cast< int >( this->layers_.size() ); ++i )
  {
    temp_handle = dynamic_cast< MaskLayer* >( this->layers_[ i ].get() );
    mask_block = temp_handle->get_mask_volume()->get_mask_data_block();
    
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

  // Step 5: Make a new nrrd using our new DataBlock
  Core::NrrdDataHandle nrrd = Core::NrrdDataHandle( new Core::NrrdData( 
    new_data_block, temp_handle->get_grid_transform() ) );

  std::string error;

  // Step 6: Attempt to save the nrrd to the path that was passed and we return false if we can't
  bool compress = PreferencesManager::Instance()->compression_state_->get();
  int level = PreferencesManager::Instance()->compression_level_state_->get();
  
  if ( !( Core::NrrdData::SaveNrrd( file_path, nrrd, error, compress, level ) ) ) 
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  return true;
}

} // end namespace seg3D
