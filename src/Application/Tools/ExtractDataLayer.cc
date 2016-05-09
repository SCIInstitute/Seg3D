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

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>
#include <Core/Volume/Volume.h>

// StateEngine of the tool
#include <Application/Tools/ExtractDataLayer.h>

// Action associated with tool
#include <Application/Tools/Actions/ActionExtractDataLayer.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, ExtractDataLayer )

namespace Seg3D
{

class ExtractDataLayerPrivate
{
public:
  ExtractDataLayerPrivate( ExtractDataLayer* tool ) :
  tool_( tool )
  {}

public:
  void handle_target_layer_changed( std::string layer_id );


  ExtractDataLayer* tool_;
};

void ExtractDataLayerPrivate::handle_target_layer_changed( std::string layer_id )
{
  LayerHandle layer = LayerManager::Instance()->find_layer_by_id( layer_id );

  if ( layer && layer->get_type() == Core::VolumeType::LARGE_DATA_E )
  {
    LargeVolumeLayerHandle lv = boost::dynamic_pointer_cast<LargeVolumeLayer>( layer );

    Core::LargeVolumeSchemaHandle schema = lv->get_schema();
    size_t num_levels = schema->get_num_levels();

    Core::GridTransform gt = schema->get_grid_transform();
    if ( lv->crop_volume_state_->get() )
    {
      gt = lv->cropped_grid_state_->get();
    }

    std::vector< LayerIDNamePair > list;

    for ( size_t j = 0; j < num_levels; j++ )
    {
      Core::IndexVector start, end;
      Core::GridTransform transform;
      // Compute the start and end of extracting a level
      schema->get_level_start_and_end( gt, j , start, end, transform );

      Core::Vector spacing =  schema->get_level_spacing( j );
      std::string label = "Level " + Core::ExportToString( j ) +
                          " Size [" + Core::ExportToString( end.x()-start.x() ) + " x " +
                                      Core::ExportToString( end.y()-start.y() ) + " x " +
                                      Core::ExportToString( end.z()-start.z() ) + "]";
      std::string key = "level" + Core::ExportToString( j );
      LayerIDNamePair entry =  std::make_pair( key , label );
      list.push_back( entry );
    }

    this->tool_->level_state_->set_option_list( list );
  }
  else
  {
    std::vector< LayerIDNamePair > empty_list( 1, std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );
    this->tool_->level_state_->set_option_list( empty_list );
  }
}


ExtractDataLayer::ExtractDataLayer( const std::string& toolid ) :
SingleTargetTool( Core::VolumeType::LARGE_DATA_E, toolid )
{
  this->private_ = ExtractDataLayerPrivateHandle( new ExtractDataLayerPrivate( this ) );

  // Create an empty list of label options
  std::vector< LayerIDNamePair > empty_list( 1,
                                            std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  // Whether we use a mask to find which components to use
  this->add_state( "level", this->level_state_, Tool::NONE_OPTION_C, empty_list );

  this->add_connection( this->target_layer_state_->value_changed_signal_.connect(
    boost::bind( &ExtractDataLayerPrivate::handle_target_layer_changed, this->private_.get(), _2 ) ) );

  this->private_->handle_target_layer_changed( this->target_layer_state_->get() );
}

ExtractDataLayer::~ExtractDataLayer()
{
  this->disconnect_all();
}

void ExtractDataLayer::execute( Core::ActionContextHandle context )
{ 
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  
  ActionExtractDataLayer::Dispatch( context,
                                    this->target_layer_state_->get(),
                                    this->level_state_->get()
                                  );
}

} // end namespace Seg3D


