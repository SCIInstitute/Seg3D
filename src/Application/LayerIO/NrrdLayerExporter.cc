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

#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Point.h>

SEG3D_REGISTER_EXPORTER( Seg3D, NrrdLayerExporter );

namespace Seg3D
{

NrrdLayerExporter::NrrdLayerExporter( std::vector< LayerHandle >& layers ) :
  LayerExporter( layers )
{
  if( !layers[ 0 ] ) return;
}

bool NrrdLayerExporter::export_layer( const std::string& mode, const std::string& file_path, 
  const std::string& name )
{
  boost::filesystem::path path = boost::filesystem::path( file_path ) / ( name + ".nrrd" );
  bool success;
  
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
    
  if( success ) CORE_LOG_SUCCESS( "NRRD export has been successfully completed." );
  return success;
}

bool NrrdLayerExporter::export_nrrd( const std::string& file_path )
{
//attempt #1
//BENLARSON  - copy of code:  /cibc/trunk/SCIRun/src/Applications/Utils/UnorientNrrdAndGetTransform.cc

// Step 1 -- Read the nrrd

//const char *file_path_char = file_path.c_str();
//
//Nrrd* nrrd = nrrdNew();
//if ( nrrdLoad(nrrd, airStrdup(file_path_char) ,0 ))
//  {
//  char *err = biffGetDone(NRRD);
//
//  std::cerr << "Could not read Nrrd file" << std::endl;
//  std::cerr << "Nrrd Error: " << err << std::endl;
//
//  free(err);
//  return (1);
//  }
//
//Core::GridTransform gt;  
//// This is the old way of defining a coordinate system in a NRRD
//std::vector<int>    size(3);
//std::vector<double> min(3);
//std::vector<double> max(3);
//
//// This is the new way of defining a coordinate system in a NRRD
//std::vector<Core::Vector> SpaceDir(3);
//Core::Vector Origin;
//// We assume the data is on the NODES
//for (size_t p=0; p<3; p++) 
//  {
//  // We get the size, this one is always defined
//  size[p] = nrrd->axis[p].size;
//
//  // min is not always defined. In case it is not it will be NaN
//  if (airExists(nrrd->axis[p].min)) 
//    {
//    min[p] = nrrd->axis[p].min;
//    }
//  else
//    {
//    min[p] = 0.0;
//    }
//
//  // max is not always defined. In case it is not it will be NaN
//  if (airExists(nrrd->axis[p].max)) 
//    {
//    max[p] = nrrd->axis[p].max;
//    }
//  else
//    {
//    if (airExists(nrrd->axis[p].spacing)) 
//      {
//      max[p] = nrrd->axis[p].spacing*(size[p]-1) + min[p];
//      }
//    else
//      {
//      max[p] = static_cast<double>(size[p]-1) + min[p];        
//      }
//    }
//  }  
//if (nrrd->spaceDim > 0)
//  {
//  int sd = nrrd->spaceDim;
//  if (sd != 3)
//    {
//    std::cerr << "Encountered an invalid nrrd file, the space dimension should" <<
//      " match the nrrd dimension" << std::endl;
//
//    // We fail, no need to clean up memory
//    exit(-1);
//    }
//
//  for (size_t p=0; p< 3; p++)
//    {
//    min[p] = 0.0;
//    max[p] = static_cast<double>(size[p]-1);
//    // Deal with voxel centered data
//    if (nrrd->axis[p].center != nrrdCenterNode)
//      {
//      double cor = (max[p]-min[p])/(2*(size[p]-1));
//      min[p] += cor;
//      max[p] += cor;
//      nrrd->axis[p].center = nrrdCenterNode;
//      }
//    }
//
//  // New way of defining origin
//
//  if (airExists(nrrd->spaceOrigin[0])) Origin.x(nrrd->spaceOrigin[0]); else Origin.x(0.0);
//  if (airExists(nrrd->spaceOrigin[1])) Origin.y(nrrd->spaceOrigin[1]); else Origin.y(0.0);
//  if (airExists(nrrd->spaceOrigin[2])) Origin.z(nrrd->spaceOrigin[2]); else Origin.z(0.0);
//
//  for (size_t p=0; p < 3;p++)
//    {
//    if (airExists(nrrd->axis[p].spaceDirection[0])) SpaceDir[p].x(nrrd->axis[p].spaceDirection[0]); else SpaceDir[p].x(0.0);
//    if (airExists(nrrd->axis[p].spaceDirection[1])) SpaceDir[p].y(nrrd->axis[p].spaceDirection[1]); else SpaceDir[p].y(0.0);
//    if (airExists(nrrd->axis[p].spaceDirection[2])) SpaceDir[p].z(nrrd->axis[p].spaceDirection[2]); else SpaceDir[p].z(0.0);
//    }
//  }
//else
//  {
//  Origin.x(min[0]);
//  Origin.y(min[1]);
//  Origin.z(min[2]);
//
//  SpaceDir[0] = Core::Vector((max[0]-min[0])/static_cast<double>(size[0]-1),0.0,0.0);
//  SpaceDir[1] = Core::Vector(0.0,(max[1]-min[1])/static_cast<double>(size[1]-1),0.0);
//  SpaceDir[2] = Core::Vector(0.0,0.0,(max[2]-min[2])/static_cast<double>(size[2]-1));
//  }
//double minspace = SpaceDir[0].length();
//if (SpaceDir[1].length() < minspace) minspace = SpaceDir[1].length();
//if (SpaceDir[2].length() < minspace) minspace = SpaceDir[2].length();
//
//SpaceDir[0] = SpaceDir[0]*(1.0/minspace);
//SpaceDir[1] = SpaceDir[1]*(1.0/minspace);
//SpaceDir[2] = SpaceDir[2]*(1.0/minspace);
//
//// This defines the transform to the new coordinate system for the final field 
//gt.load_basis(Core::Point(Origin),SpaceDir[0],SpaceDir[1],SpaceDir[2]);
//
//// Correct for spacing
//Core::Transform  spacing;
//spacing.pre_scale(Core::Vector(minspace/(SpaceDir[0].length()),minspace/(SpaceDir[1].length()),minspace/(SpaceDir[2].length()))); 
//gt.post_translate(spacing); 
//
// //Clean up the nrrd file
//
//nrrd->spaceDim = 3;
//for (int p=0; p < 3; p++)
//{
//  nrrd->axis[p].min     = static_cast<double>(AIR_NAN);
//  nrrd->axis[p].max     = static_cast<double>(AIR_NAN);
//  nrrd->axis[p].spacing = static_cast<double>(AIR_NAN);
//  nrrd->axis[p].center = nrrdCenterNode;
//
//  nrrd->axis[0].spaceDirection[p] = 0.0;
//  nrrd->axis[1].spaceDirection[p] = 0.0;
//  nrrd->axis[2].spaceDirection[p] = 0.0;
//  
//  nrrd->spaceOrigin[p] = 0.0;
//}
//
//nrrd->axis[0].spaceDirection[0] = SpaceDir[0].length();
//nrrd->axis[1].spaceDirection[1] = SpaceDir[1].length();
//nrrd->axis[2].spaceDirection[2] = SpaceDir[2].length();

//endBENLARSON

const char *file_path_char = file_path.c_str();

  Nrrd* nrrd_original = nrrdNew();
  if ( nrrdLoad(nrrd_original, airStrdup(file_path_char) ,0 ))
    {
    char *err = biffGetDone(NRRD);

    std::cerr << "Could not read Nrrd file" << std::endl;
    std::cerr << "Nrrd Error: " << err << std::endl;

    free(err);
    return (1);
    }

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
  //attempt #2 get the spacing via library calls, then set the info in the saved nrrd? 
  Core::Vector space_dim_vector[NRRD_DIM_MAX]; 
  //this call causes an error that I haven't had time to really look into. 
  nrrdAxisInfoGet_va(nrrd_original, nrrdAxisInfoSpaceDirection, space_dim_vector); 
  return true;
}

bool NrrdLayerExporter::export_single_masks( const std::string& path )
{
  // We process all of the layers individually in this loop
  for( int i = 0; i < static_cast< int >( this->layers_.size() ); ++i )
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
  for( size_t i = 0; i < mask_block->get_size(); ++i )
  {
    new_data_block->set_data_at( i, this->label_values_[ 0 ]  );
  }
  
  // Step 4: Loop through all the MaskLayers and insert their values into our new DataBlock
  std::vector< MaskLayer* >  mask_layers;
  for( int i = 1; i < static_cast< int >( this->layers_.size() ); ++i )
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
