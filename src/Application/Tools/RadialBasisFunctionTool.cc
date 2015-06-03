/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Application/Tools/RadialBasisFunctionTool.h>
#include <Application/Filters/Actions/ActionRadialBasisFunction.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>

#include <Core/Utils/ScopedCounter.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, RadialBasisFunctionTool )

namespace Seg3D
{

using namespace RadialBasisFunction;

RadialBasisFunctionTool::RadialBasisFunctionTool( const std::string& toolid ) :
  SeedPointsTool( Core::VolumeType::DATA_E, toolid )
{
  // Create an empty list of label options

  // add default values for the the states
	double inf = std::numeric_limits< double >::infinity();
  // TODO: slider max should be up to 20% of source data range.
  // See threshold tool for good implementation example.
  this->add_state( "normal_offset", this->normalOffset_state_, 10.0, 1.0, 20.0, 0.1 );
	this->add_state( "kernel", this->kernel_state_, "thin_plate",
                   "thin_plate|gaussian|multi_quadratic" );
}

RadialBasisFunctionTool::~RadialBasisFunctionTool()
{
  this->disconnect_all();
}
//
//bool RadialBasisFunctionTool::handle_mouse_press( ViewerHandle viewer, const Core::MouseHistory& mouse_history, int button, int buttons, int modifiers )
//{
//}
//
//bool RadialBasisFunctionTool::handle_mouse_release( ViewerHandle viewer, const Core::MouseHistory& mouse_history, int button, int buttons, int modifiers )
//{
//}
//
//bool RadialBasisFunctionTool::handle_mouse_move( ViewerHandle viewer, const Core::MouseHistory& mouse_history, int button, int buttons, int modifiers )
//{
//}
//
//void RadialBasisFunctionTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat, int viewer_width, int viewer_height )
//{
//}
//
//bool RadialBasisFunctionTool::has_2d_visual()
//{
//  return true;
//}

//void RadialBasisFunctionTool::handle_seed_points_changed()
//{
////	std::string target_layer_id = this->target_layer_state_->get();
////	if ( target_layer_id == Tool::NONE_OPTION_C )
////	{
////		return;
////	}
////
////	const Core::StatePointVector::value_type& seed_points = this->seed_points_state_->get();
//////	if ( seed_points.size() == 0 )
//////	{
//////		this->private_->update_viewers();
//////		return;
//////	}
////
////	DataLayerHandle data_layer = boost::dynamic_pointer_cast< DataLayer >(
////    LayerManager::Instance()->find_layer_by_id( target_layer_id ) );
////	Core::DataVolumeHandle data_volume = data_layer->get_data_volume();
////	Core::DataBlockHandle data_block = data_volume->get_data_block();
////	double min_val = std::numeric_limits< double >::max();
////	double max_val = std::numeric_limits< double >::min();
////	for ( size_t i = 0; i < seed_points.size(); ++i )
////	{
////		Core::Point pt = data_volume->apply_inverse_grid_transform( seed_points[ i ] );
////		int x = Core::Round( pt[ 0 ] );
////		int y = Core::Round( pt[ 1 ] );
////		int z = Core::Round( pt[ 2 ] );
////		if ( x >= 0 && x < static_cast< int >( data_block->get_nx() ) &&
////        y >= 0 && y < static_cast< int >( data_block->get_ny() ) &&
////        z >= 0 && z < static_cast< int >( data_block->get_nz() ) )
////		{
////			double val = data_block->get_data_at( static_cast< size_t >( x ), static_cast< size_t >( y ), static_cast< size_t >( z ) );
////			min_val = Core::Min( min_val, val );
////			max_val = Core::Max( max_val, val );
////		}
////	}
////
////	{
////		Core::ScopedCounter signal_block( this->private_->signal_block_count_ );
////		this->upper_threshold_state_->set( max_val );
////		this->lower_threshold_state_->set( min_val );
////	}
////  
//////	this->private_->update_viewers();
//}

//void RadialBasisFunctionTool::handle_target_layer_changed()
//{
//  std::string target_layer_id = this->target_layer_state_->get();
//  if ( target_layer_id != Tool::NONE_OPTION_C )
//  {
////    Core::ScopedCounter signal_block( this->signal_block_count_ );
//
//    DataLayerHandle data_layer = boost::dynamic_pointer_cast< DataLayer >(
//      LayerManager::Instance()->find_layer_by_id( target_layer_id ) );
//    double min_val = data_layer->get_data_volume()->get_data_block()->get_min();
//    double max_val = data_layer->get_data_volume()->get_data_block()->get_max();
//
//    //TODO: We need to fix this.  This causes an inconsistency in the threshold tool between
//    // the histogram and the sliders
//    // 		double epsilon = ( max_val - min_val ) * 0.005;
//    // 		min_val -= epsilon;
//    // 		max_val += epsilon;
////    this->isovalue_state_->set_range( min_val, max_val );
////    
//    if ( this->seed_points_state_->get().size() > 0 )
//    {
//      this->handle_seed_points_changed();
//    }
//  }
////  this->update_viewers();
//}

void RadialBasisFunctionTool::execute( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  ActionRadialBasisFunction::Dispatch( context,
                                       this->target_layer_state_->get(),
                                       this->seed_points_state_->get(),
                                       this->normalOffset_state_->get(),
                                       this->kernel_state_->get()
                                     );
}

} // end namespace Seg3D
