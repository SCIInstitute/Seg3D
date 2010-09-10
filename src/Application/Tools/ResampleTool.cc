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

#include <Core/Utils/ScopedCounter.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/ResampleTool.h>
#include <Application/Filters/Actions/ActionResample.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/LayerManager.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, ResampleTool )

namespace Seg3D
{

class ResampleToolPrivate
{
public:
  void handle_target_group_changed();
  void handle_size_scheme_changed( std::string size_scheme );
  void handle_dst_group_changed( std::string group_id );
  void handle_output_dimension_changed( int index, int size );
  void handle_scale_changed( double scale );
  void handle_constraint_aspect_changed( bool constraint );
  void handle_kernel_changed( std::string kernel_name );

  size_t signal_block_count_;
  ResampleTool* tool_;
};

void ResampleToolPrivate::handle_target_group_changed()
{
  const std::string& group_id = this->tool_->target_group_state_->get();
  std::vector< Core::OptionLabelPair > dst_groups;
  if ( group_id == "" || group_id == Tool::NONE_OPTION_C )
  {
    this->tool_->dst_group_state_->set_option_list( dst_groups );
    return;
  }

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  LayerGroupHandle layer_group = LayerManager::Instance()->get_layer_group( group_id );
  const Core::GridTransform& grid_trans = layer_group->get_grid_transform();
  int nx = static_cast< int >( grid_trans.get_nx() );
  int ny = static_cast< int >( grid_trans.get_ny() );
  int nz = static_cast< int >( grid_trans.get_nz() );
  this->tool_->input_dimensions_state_[ 0 ]->set( nx );
  this->tool_->input_dimensions_state_[ 1 ]->set( ny );
  this->tool_->input_dimensions_state_[ 2 ]->set( nz );
  
  double scale = 1.0;
  if ( this->tool_->constraint_aspect_state_->get() )
  {
    scale = this->tool_->scale_state_->get();
  }
  this->tool_->output_dimensions_state_[ 0 ]->set_range( 1, nx * 2 );
  this->tool_->output_dimensions_state_[ 0 ]->set( Core::Round( nx * scale ) );
  this->tool_->output_dimensions_state_[ 1 ]->set_range( 1, ny * 2 );
  this->tool_->output_dimensions_state_[ 1 ]->set( Core::Round( ny * scale ) );
  this->tool_->output_dimensions_state_[ 2 ]->set_range( 1, nz * 2 );
  this->tool_->output_dimensions_state_[ 2 ]->set( Core::Round( nz * scale ) );

  std::vector< LayerGroupHandle > layer_groups;
  LayerManager::Instance()->get_groups( layer_groups );
  for ( size_t i = 0; i < layer_groups.size(); ++i )
  {
    if ( layer_groups[ i ] == layer_group )
    {
      continue;
    }
    const Core::GridTransform& grid_trans = layer_groups[ i ]->get_grid_transform();
    std::string group_name = Core::ExportToString( grid_trans.get_nx() ) + " x " +
      Core::ExportToString( grid_trans.get_ny() ) + " x " + 
      Core::ExportToString( grid_trans.get_nz() );
    dst_groups.push_back( std::make_pair( layer_groups[ i ]->get_group_id(), group_name ) );
  }
  this->tool_->dst_group_state_->set_option_list( dst_groups );
}

void ResampleToolPrivate::handle_output_dimension_changed( int index, int size )
{
  if ( this->signal_block_count_ > 0 ||
    !this->tool_->constraint_aspect_state_->get() )
  {
    return;
  }
  
  Core::ScopedCounter signal_block( this->signal_block_count_ );

  double scale = size * 1.0 / this->tool_->input_dimensions_state_[ index ]->get();
  this->tool_->scale_state_->set( scale );
  this->tool_->output_dimensions_state_[ ( index + 1 ) % 3 ]->set( Core::Round(
    this->tool_->input_dimensions_state_[ ( index + 1 ) % 3 ]->get() * scale ) );
  this->tool_->output_dimensions_state_[ ( index + 2 ) % 3 ]->set( Core::Round(
    this->tool_->input_dimensions_state_[ ( index + 2 ) % 3 ]->get() * scale ) );
}

void ResampleToolPrivate::handle_scale_changed( double scale )
{
  if ( this->signal_block_count_ > 0 ||
    !this->tool_->constraint_aspect_state_->get() )
  {
    return;
  }

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  for ( int i = 0; i < 3; i++ )
  {
    this->tool_->output_dimensions_state_[ i ]->set( Core::Round(
      this->tool_->input_dimensions_state_[ i ]->get() * scale ) );
  }
}

void ResampleToolPrivate::handle_constraint_aspect_changed( bool constraint )
{
  if ( !constraint )
  {
    return;
  }
  
  Core::ScopedCounter signal_block( this->signal_block_count_ );

  double scale = this->tool_->scale_state_->get();
  for ( int i = 0; i < 3; i++ )
  {
    this->tool_->output_dimensions_state_[ i ]->set( Core::Round(
      this->tool_->input_dimensions_state_[ i ]->get() * scale ) );
  }
}

void ResampleToolPrivate::handle_kernel_changed( std::string kernel_name )
{
  this->tool_->has_params_state_->set( kernel_name == ActionResample::GAUSSIAN_C );
}

void ResampleToolPrivate::handle_size_scheme_changed( std::string size_scheme )
{
  if ( size_scheme == ResampleTool::SIZE_OTHER_GROUP_C )
  {
    this->tool_->manual_size_state_->set( false );
    this->tool_->valid_size_state_->set( this->tool_->dst_group_state_->get() != "" );
  }
  else
  {
    this->tool_->manual_size_state_->set( true );
    this->tool_->valid_size_state_->set( true );
  }
}

void ResampleToolPrivate::handle_dst_group_changed( std::string group_id )
{
  if ( this->tool_->size_scheme_state_->get() == ResampleTool::SIZE_OTHER_GROUP_C )
  {
    this->tool_->valid_size_state_->set( group_id != "" );
  }
}

//////////////////////////////////////////////////////////////////////////
// Class ResampleTool
//////////////////////////////////////////////////////////////////////////

const std::string ResampleTool::SIZE_OTHER_GROUP_C( "other_group" );
const std::string ResampleTool::SIZE_MANUAL_C( "manual" );

ResampleTool::ResampleTool( const std::string& toolid ) :
  GroupTargetTool( Core::VolumeType::ALL_E, toolid ),
  private_( new ResampleToolPrivate )
{
  this->private_->tool_ = this;
  this->private_->signal_block_count_ = 0;

  this->add_state( "input_x", this->input_dimensions_state_[ 0 ], 0 );
  this->add_state( "input_y", this->input_dimensions_state_[ 1 ], 0 );
  this->add_state( "input_z", this->input_dimensions_state_[ 2 ], 0 );

  this->add_state( "size_scheme", this->size_scheme_state_, SIZE_MANUAL_C, 
    SIZE_OTHER_GROUP_C + "=Resample to Match Another Group|" +
    SIZE_MANUAL_C + "=Set Dimensions Manually" );
  this->add_state( "manual_size", this->manual_size_state_, true );

  this->add_state( "dst_group", this->dst_group_state_, "", "" );

  std::vector< Core::OptionLabelPair > padding_values;
  padding_values.push_back( std::make_pair( ActionResample::ZERO_C, "0" ) );
  padding_values.push_back( std::make_pair( ActionResample::MIN_C, "Minimum Value" ) );
  padding_values.push_back( std::make_pair( ActionResample::MAX_C, "Maximum Value" ) );
  this->add_state( "pad_value", this->padding_value_state_, ActionResample::ZERO_C, 
    padding_values );

  this->add_state( "output_x", this->output_dimensions_state_[ 0 ], 1, 1, 500, 1 );
  this->add_state( "output_y", this->output_dimensions_state_[ 1 ], 1, 1, 500, 1 );
  this->add_state( "output_z", this->output_dimensions_state_[ 2 ], 1, 1, 500, 1 );
  this->add_state( "constraint_aspect", this->constraint_aspect_state_, true );
  this->add_state( "scale", this->scale_state_, 1.0, 0.01, 2.0, 0.01 );

  std::vector< Core::OptionLabelPair > kernels;
  kernels.push_back( std::make_pair( ActionResample::BOX_C, "Box" ) );
  kernels.push_back( std::make_pair( ActionResample::TENT_C, "Tent" ) );
  kernels.push_back( std::make_pair( ActionResample::CUBIC_CR_C, "Cubic (Catmull-Rom)" ) );
  kernels.push_back( std::make_pair( ActionResample::CUBIC_BS_C, "Cubic (B-spline)" ) );
  kernels.push_back( std::make_pair( ActionResample::QUARTIC_C, "Quartic" ) );
  kernels.push_back( std::make_pair( ActionResample::GAUSSIAN_C, "Gaussian" ) );
  this->add_state( "kernel", this->kernel_state_, ActionResample::BOX_C, kernels );

  this->add_state( "sigma", this->gauss_sigma_state_, 1.0, 1.0, 100.0, 0.01 );
  this->add_state( "cutoff", this->gauss_cutoff_state_, 1.0, 1.0, 100.0, 0.01 );
  this->add_state( "has_params", this->has_params_state_, false );

  this->add_state( "valid_size", this->valid_size_state_, true );
  this->add_state( "replace", this->replace_state_, false );

  this->add_connection( this->target_group_state_->state_changed_signal_.connect(
    boost::bind( &ResampleToolPrivate::handle_target_group_changed, this->private_ ) ) );
  this->add_connection( this->size_scheme_state_->value_changed_signal_.connect(
    boost::bind( &ResampleToolPrivate::handle_size_scheme_changed, this->private_, _2 ) ) );
  this->add_connection( this->dst_group_state_->value_changed_signal_.connect(
    boost::bind( &ResampleToolPrivate::handle_dst_group_changed, this->private_, _2 ) ) );
  this->add_connection( this->constraint_aspect_state_->value_changed_signal_.connect(
    boost::bind( &ResampleToolPrivate::handle_constraint_aspect_changed, this->private_, _1 ) ) );
  this->add_connection( this->scale_state_->value_changed_signal_.connect(
    boost::bind( &ResampleToolPrivate::handle_scale_changed, this->private_, _1 ) ) );
  this->add_connection( this->kernel_state_->value_changed_signal_.connect(
    boost::bind( &ResampleToolPrivate::handle_kernel_changed, this->private_, _2 ) ) );
  for ( int i = 0; i < 3; ++i )
  {
    this->add_connection( this->output_dimensions_state_[ i ]->value_changed_signal_.connect(
      boost::bind( &ResampleToolPrivate::handle_output_dimension_changed, 
      this->private_, i, _1 ) ) );
  }
  
  this->private_->handle_target_group_changed();
}

ResampleTool::~ResampleTool()
{
  this->disconnect_all();
}

void ResampleTool::execute( Core::ActionContextHandle context )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  if ( this->manual_size_state_->get() )
  {
    ActionResample::Dispatch( context, this->target_layers_state_->get(), 
      this->output_dimensions_state_[ 0 ]->get(), this->output_dimensions_state_[ 1 ]->get(),
      this->output_dimensions_state_[ 2 ]->get(), this->kernel_state_->get(),
      this->gauss_sigma_state_->get(), this->gauss_cutoff_state_->get(), 
      this->replace_state_->get() );
  }
  else
  {
    LayerGroupHandle dst_group = LayerManager::Instance()->get_layer_group(
      this->dst_group_state_->get() );
    if ( dst_group )
    {
      ActionResample::Dispatch( context, this->target_layers_state_->get(),
        dst_group->get_grid_transform(), this->padding_value_state_->get(),
        this->kernel_state_->get(), this->gauss_sigma_state_->get(),
        this->gauss_cutoff_state_->get(), this->replace_state_->get() );
    }
  }
}

} // end namespace Seg3D
