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
#include <Application/Layer/LayerGroup.h>

// StateEngine of the tool
#include <Application/Tools/PointSetFilter.h>

// Action associated with tool
#include <Application/Filters/Actions/ActionPointSetRegisterFilter.h>
#include <Application/Filters/Actions/ActionPointSetTransformFilter.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, PointSetFilter )

namespace Seg3D
{

PointSetFilter::PointSetFilter( const std::string& toolid ) :
  SingleTargetTool( Core::VolumeType::MASK_E, toolid )
{
  // Create an empty list of label options
  std::vector< LayerIDNamePair > empty_list( 1, 
    std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  std::vector< std::string > empty_option;
  this->add_state( "target_layers", this->target_layers_state_, empty_option, "" );

  // Whether we use a mask to find which components to use
  this->add_state( "mask", this->mask_state_, Tool::NONE_OPTION_C, empty_list );
  this->mask_state_->set_session_priority( Core::StateBase::DEFAULT_LOAD_E + 2 );
  this->add_extra_layer_input( this->mask_state_, Core::VolumeType::MASK_E, true, false );

  this->add_state( "iterations", this->iterations_state_, 5, 1, 1000, 1 );

  this->target_layers_state_->set_session_priority( Core::StateBase::DEFAULT_LOAD_E + 1 );
  this->add_state( "registration_ready", this->registration_ready_state_, false );

  this->add_connection( this->mask_state_->value_changed_signal_.connect(
    boost::bind( &PointSetFilter::handle_mask_layer_changed, this, _2 ) ) );

  this->add_connection( this->target_layer_state_->value_changed_signal_.connect(
    boost::bind( &PointSetFilter::handle_target_layer_changed, this, _2 ) ) );

  this->add_connection( this->iterations_state_->value_changed_signal_.connect(
    boost::bind( &PointSetFilter::handle_iteration_changed, this ) ) );

  std::vector< double > identity_matrix( 6, 0 );
  this->add_state( "transformation_matrix", this->transform_matrix_state_, identity_matrix );

  std::vector< double > zero_matrix(16, 0);
  this->add_state("complete_transformation_matrix", this->complete_transform_matrix_state_, zero_matrix);

  this->add_connection( LayerManager::Instance()->layers_changed_signal_.connect(
    boost::bind( &PointSetFilter::handle_layers_changed, this ) ) );

  this->add_state( "translation_x", this->translation_state_[ 0 ], 0 );
  this->add_state( "translation_y", this->translation_state_[ 1 ], 0 );
  this->add_state( "translation_z", this->translation_state_[ 2 ], 0 );

  this->add_state( "rotation_x", this->rotation_state_[ 0 ], 0 );
  this->add_state( "rotation_y", this->rotation_state_[ 1 ], 0 );
  this->add_state( "rotation_z", this->rotation_state_[ 2 ], 0 );

  this->add_connection( this->transform_matrix_state_->state_changed_signal_.connect(
    boost::bind( &PointSetFilter::handle_transform_changed, this ) ) );
}
  
PointSetFilter::~PointSetFilter()
{
  this->disconnect_all();
}

void PointSetFilter::handle_target_layer_changed( std::string layer_id )
{
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  registration_ready_state_->set( false );
}


void PointSetFilter::handle_iteration_changed(  )
{
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  registration_ready_state_->set( false );
}


void PointSetFilter::handle_transform_changed(  )
{
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  this->registration_ready_state_->set( true );

  std::vector< double > matrix_params;
  matrix_params = this->transform_matrix_state_->get();

  this->rotation_state_[ 0 ]->set( matrix_params[ 0 ] );
  this->rotation_state_[ 1 ]->set( matrix_params[ 1 ] );
  this->rotation_state_[ 2 ]->set( matrix_params[ 2 ] );

  this->translation_state_[ 0 ]->set( matrix_params[ 3 ] );
  this->translation_state_[ 1 ]->set( matrix_params[ 4 ] );
  this->translation_state_[ 2 ]->set( matrix_params[ 5 ] );

}

void PointSetFilter::handle_layers_changed()
{
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  std::string mask_layer_id = this->mask_state_->get();
  
  LayerHandle layer = LayerManager::Instance()->find_layer_by_id( mask_layer_id );

  if ( layer )
  {
    registration_ready_state_->set( false );
    LayerGroupHandle group = layer->get_layer_group();
    std::string group_id = group->get_group_id();

    Core::VolumeType mask_type = layer->get_type();

    std::vector< LayerIDNamePair > layer_names;
    //std::vector< std::string > selected_layers;
    if ( group_id != "" && group_id != Tool::NONE_OPTION_C )
    {
      group->get_layer_names( layer_names, mask_type );

      LayerHandle target_layer = LayerManager::Instance()->find_layer_by_id( 
        this->target_layer_state_->get());

      std::string target_layer_name = "";
            if ( target_layer ) target_layer_name = target_layer->get_layer_name();
      std::vector<LayerIDNamePair>::iterator it = layer_names.begin();

      for ( ; it!=layer_names.end(); ++it )
      {
        if ( target_layer_name.compare( (*it).second ) == 0 ) 
        {
          break;
        }
      }

      if ( it!= layer_names.end() )
      {
        layer_names.erase( it );
      }
    }

    this->target_layers_state_->set_option_list( layer_names );
  }

}

void PointSetFilter::handle_mask_layer_changed( std::string layer_id )
{
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  LayerHandle layer = LayerManager::Instance()->find_layer_by_id( layer_id );

  if ( layer )
  {
    registration_ready_state_->set( false );

    LayerGroupHandle group = layer->get_layer_group();
    std::string group_id = group->get_group_id();

    Core::VolumeType mask_type = layer->get_type();

    std::vector< LayerIDNamePair > layer_names;
    //std::vector< std::string > selected_layers;
    if ( group_id != "" && group_id != Tool::NONE_OPTION_C )
    {
      group->get_layer_names( layer_names, mask_type );

      LayerHandle target_layer = LayerManager::Instance()->find_layer_by_id( 
        this->target_layer_state_->get());

      std::string target_layer_name = "";
            if ( target_layer ) target_layer_name = target_layer->get_layer_name();
      std::vector<LayerIDNamePair>::iterator it = layer_names.begin();

      for ( ; it!=layer_names.end(); ++it )
      {
        if ( target_layer_name.compare( (*it).second ) == 0 ) 
        {
          break;
        }
      }

      if ( it!= layer_names.end() )
      {
        layer_names.erase( it );
      }
    }

    this->target_layers_state_->set_option_list( layer_names );
  }

}

void PointSetFilter::registration( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  ActionPointSetRegisterFilter::Dispatch( context,
    this->target_layer_state_->get(),
    this->mask_state_->get(),
    this->iterations_state_->get(),
    this->transform_matrix_state_->get_stateid(),
	this->complete_transform_matrix_state_->get_stateid()
    );  

}

void PointSetFilter::apply( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  ActionPointSetTransformFilter::Dispatch( context,
    this->target_layer_state_->get(),
    this->target_layers_state_->get(),
    this->transform_matrix_state_->get()
    );    
}

} // end namespace Seg3D


