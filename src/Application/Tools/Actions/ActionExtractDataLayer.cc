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

#include <Core/DataBlock/StdDataBlock.h>

// Application includes
#include <Application/Filters/LayerFilter.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LargeVolumeLayer.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Tools/Actions/ActionExtractDataLayer.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, ExtractDataLayer )

namespace Seg3D
{

bool ActionExtractDataLayer::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistenceAndType( this->target_layer_, 
    Core::VolumeType::LARGE_DATA_E, context, this->sandbox_ ) ) return false;
  
  // Check for layer availability 
  if ( ! LayerManager::CheckLayerAvailability( this->target_layer_, 
    false, context, this->sandbox_ ) ) return false;
  
  
  LayerHandle layer = LayerManager::FindLayer( this->target_layer_, this->sandbox_ );
  LargeVolumeLayerHandle lv = boost::dynamic_pointer_cast<LargeVolumeLayer>( layer );

  Core::LargeVolumeSchemaHandle schema = lv->get_schema();
  size_t num_levels = schema->get_num_levels();

  if ( this->level_.size() < 6 )
  {
    context->report_error( "Level needs to be a level between level0 and level" + Core::ExportToString( num_levels ) );
    return false;   
  }

  int level = 0;
  if ( !Core::ImportFromString(  this->level_.substr( 5 ), level ) )
  {
    context->report_error( "Level needs to be a level between level0 and level" + Core::ExportToString( num_levels ) );
    return false;   
  }

  if ( this->level_.substr( 0, 5 ) != "level"  || level >= num_levels )
  {
    context->report_error( "Level needs to be a level between level0 and level" + Core::ExportToString( num_levels ) );
    return false;   
  }
  
  // Validation successful
  return true;
}


// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class ExtractDataLayerAlgo : public LayerFilter
{

public:
  
  LayerHandle src_layer_;
  LayerHandle dst_layer_;

  int level_;
  Core::GridTransform grid_transform_;
  Core::IndexVector start_;
  Core::IndexVector end_;

public:
  // RUN:
  // Implemtation of run of the Runnable base class, this function is called when the thread
  // is launched.

  // NOTE: The macro needs a data type to select which version to run. This needs to be
  // a member variable of the algorithm class.

  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "ExtractDataLayer Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "ExtractDataLayer";  
  }

  void run_filter()
  {
  
    LargeVolumeLayerHandle lv = boost::dynamic_pointer_cast<LargeVolumeLayer>( this->src_layer_ );
    Core::LargeVolumeSchemaHandle schema = lv->get_schema();

    Core::IndexVector::index_type overlap = static_cast<Core::IndexVector::index_type> ( schema->get_overlap() );

    if ( this->level_ >= schema->get_num_levels() )
    {
      this->report_error( "Incorrect level selected." );
      return;
    }

    Core::IndexVector level_size = schema->get_level_size( this->level_ );
    Core::IndexVector layout = schema->get_level_layout( this->level_ );

    if ( ( this->end_.x() - this->start_.x() ) > 4096 || ( this->end_.y() - this->start_.y() ) > 4096 || 
      ( this->end_.z() - this->start_.z() ) > 4096 )
    {
      this->report_error( "Resulting data layer has a dimension larger than 4096, layers larger than this are currently not allowed." );
      return;   
    }

    Core::DataBlockHandle data_block = Core::StdDataBlock::New( 
      ( this->end_.x() - this->start_.x() ) , ( this->end_.y() - this->start_.y() ), 
      ( this->end_.z() - this->start_.z() ), schema->get_data_type());

    if (! data_block )
    {
      this->report_error( "Could not allocate enough memory to extract data layer." );
      return;   
    }

    Core::IndexVector eff_brick_size = schema->get_effective_brick_size();

    this->dst_layer_->update_progress( 0.0, 0.0, 1.0);

    // Calculate the number of bricks we need to load
    size_t total = 0;

    for( Core::IndexVector::index_type z = 0; z < layout.z(); z++)
    {
      for( Core::IndexVector::index_type y = 0; y < layout.y(); y++)
      {
        for( Core::IndexVector::index_type x = 0; x < layout.x(); x++)
        {
          Core::BrickInfo bi( z * layout.x() * layout.y() + y * layout.x() + x, this->level_ );

          Core::IndexVector size = schema->get_brick_size( bi );
          Core::IndexVector origin = Core::IndexVector( 
            x * eff_brick_size.x(),
            y * eff_brick_size.y(),
            z * eff_brick_size.z());
      
          if ( ( this->start_.x() < ( origin.x() + ( size.x() - 2 * overlap ) ) && this->end_.x() > origin.x() ) &&
            ( this->start_.y() < ( origin.y() + ( size.y() - 2 * overlap ) ) && this->end_.y() > origin.y() ) &&
            ( this->start_.z() < ( origin.z() + ( size.z() - 2 * overlap ) ) && this->end_.z() > origin.z() ) )
          {
            total++;
          }
        }
      }
    }

    // Now do the actual loading
    size_t count = 0;

    for( Core::IndexVector::index_type z = 0; z < layout.z(); z++)
    {
      for( Core::IndexVector::index_type y = 0; y < layout.y(); y++)
      {
        for( Core::IndexVector::index_type x = 0; x < layout.x(); x++)
        {
          Core::BrickInfo bi( z * layout.x() * layout.y() + y * layout.x() + x, this->level_ );

          Core::IndexVector size = schema->get_brick_size( bi );
          Core::IndexVector origin = Core::IndexVector( 
            x * eff_brick_size.x(),
            y * eff_brick_size.y(),
            z * eff_brick_size.z());
      
          if ( ( this->start_.x() < ( origin.x() + ( size.x() - 2 * overlap ) ) && this->end_.x() > origin.x() ) &&
            ( this->start_.y() < ( origin.y() + ( size.y() - 2 * overlap ) ) && this->end_.y() > origin.y() ) &&
            ( this->start_.z() < ( origin.z() + ( size.z() - 2 * overlap ) ) && this->end_.z() > origin.z() ) )
          {
            Core::DataBlockHandle brick;

            std::string error;
            if (! schema->read_brick( brick, bi, error ))
            {
              this->report_error( error );
              return;
            }

            Core::IndexVector clip_start = Core::IndexVector(
              Core::Max( static_cast< Core::IndexVector::index_type>( 0 ) , this->start_.x() - origin.x() ),
              Core::Max( static_cast< Core::IndexVector::index_type>( 0 ) , this->start_.y() - origin.y() ),
              Core::Max( static_cast< Core::IndexVector::index_type>( 0 ) , this->start_.z() - origin.z() ) );

            Core::IndexVector clip_end = Core::IndexVector(
              Core::Min( size.x() - 2 * overlap, this->end_.x() - origin.x() ),
              Core::Min( size.y() - 2 * overlap, this->end_.y() - origin.y() ),
              Core::Min( size.z() - 2 * overlap, this->end_.z() - origin.z() ) );
          
            Core::IndexVector offset = Core::IndexVector( 
              Core::Max( static_cast< Core::IndexVector::index_type>( 0 ), x * eff_brick_size.x() - this->start_.x() ),
              Core::Max( static_cast< Core::IndexVector::index_type>( 0 ), y * eff_brick_size.y() - this->start_.y() ), 
              Core::Max( static_cast< Core::IndexVector::index_type>( 0 ), z * eff_brick_size.z() - this->start_.z() ) );

            schema->insert_brick( data_block, brick, offset, clip_start, clip_end );

            count++;
            this->dst_layer_->update_progress( static_cast<double>( count ) / static_cast<double>( total ), 0.0, 0.8);
          }
        }
      }
    }

    Core::DataVolumeHandle volume( new Core::DataVolume( this->grid_transform_, data_block ) );
    this->dispatch_insert_data_volume_into_layer( this->dst_layer_, volume, true );

    return;
  }
};


bool ActionExtractDataLayer::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<ExtractDataLayerAlgo> algo( new ExtractDataLayerAlgo );

  // Copy the parameters over to the algorithm that runs the filter
  algo->set_sandbox( this->sandbox_ );

  // Find the handle to the layer
  if ( !( algo->find_layer( this->target_layer_, algo->src_layer_ ) ) )
  {
    return false;
  }
  
  if (! Core::ImportFromString( this->level_.substr( 5 ), algo->level_ ) )
  {
    return false;
  }

  // Figure out dimensions

  LayerHandle layer = LayerManager::FindLayer( this->target_layer_, this->sandbox_ );
  LargeVolumeLayerHandle lv = boost::dynamic_pointer_cast<LargeVolumeLayer>( layer );

  Core::LargeVolumeSchemaHandle schema = lv->get_schema();
  Core::GridTransform gt = schema->get_grid_transform();

  if ( lv->crop_volume_state_->get() )
  {
    gt = lv->cropped_grid_state_->get();
  }

  // Compute the start and end of extracting a level
  schema->get_level_start_and_end( gt, algo->level_, algo->start_, algo->end_, algo->grid_transform_ );

  // Lock the src layer, so it cannot be used else where
  algo->lock_for_use( algo->src_layer_ );
    
  // Create the destination layer, which will show progress
  algo->create_and_lock_data_layer( algo->grid_transform_, algo->src_layer_, algo->dst_layer_ );

  // Return the id of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( algo->dst_layer_->get_layer_id() ) );

  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
    context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }

  // Build the undo-redo record
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );
  
  // Start the filter.
  Core::Runnable::Start( algo );

  return true;
}

void ActionExtractDataLayer::Dispatch( Core::ActionContextHandle context, std::string target_layer,
  std::string level )
{ 
  // Create a new action
  ActionExtractDataLayer* action = new ActionExtractDataLayer;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->level_ = level; 

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
