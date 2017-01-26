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

#include <Application/Tools/ImplicitModelTool.h>
#include <Application/Filters/Actions/ActionImplicitModel.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Viewer/Viewer.h>
#include <Application/ViewerManager/ViewerManager.h>

#include <Core/State/Actions/ActionAdd.h>
#include <Core/State/Actions/ActionClear.h>
#include <Core/State/Actions/ActionRemove.h>
#include <Core/Viewer/Mouse.h>
#include <Core/Volume/VolumeSlice.h>
#include <Core/Interface/Interface.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, ImplicitModelTool )

namespace Seg3D
{

using namespace ImplicitModel;

const std::string ImplicitModelTool::CONVEX_HULL_2D_C( "2d" );
const std::string ImplicitModelTool::CONVEX_HULL_3D_C( "3d" );

ImplicitModelTool::ImplicitModelTool( const std::string& toolid ) :
  SeedPointsTool( Core::VolumeType::DATA_E, toolid )
{
  // TODO: slider max should be up to 20% of source data range.
  // See threshold tool for good implementation example.
  this->add_state( "normal_offset", this->normalOffset_state_, 2.0, 0.0, 20.0, 0.1 );
  this->add_state( "kernel", this->kernel_state_, "thin_plate",
                   "thin_plate|gaussian|multi_quadratic" );
  this->add_state( "view_modes", this->view_modes_state_ );
  this->add_state( "compute_2D_convex_hull", this->compute_2D_convex_hull_state_, true );
  this->add_state( "invert_seed_order", this->invert_seed_order_state_, false );
  this->add_state( "convex_hull_selection",
                   this->convex_hull_selection_state_,
                   CONVEX_HULL_2D_C,
                   CONVEX_HULL_2D_C + "=2D_convex_hull|" + CONVEX_HULL_3D_C + "=3D_convex_hull" );

  this->add_connection( this->convex_hull_selection_state_->value_changed_signal_.connect(
    boost::bind( &ImplicitModelTool::handle_convex_hull_type_changed, this, _2 ) ) );

  // TODO: temporary
  this->add_state( "disabled", this->disabled_widget_state_, false );


}

ImplicitModelTool::~ImplicitModelTool()
{
  this->disconnect_all();
}

bool ImplicitModelTool::handle_mouse_press( ViewerHandle viewer,
                                            const Core::MouseHistory& mouse_history,
                                            int button, int buttons, int modifiers )
{
  std::string view_mode;
  Core::VolumeSliceHandle target_slice;
  double world_x, world_y;

  {
    Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );

    if ( viewer->is_volume_view() )
    {
      return false;
    }

    view_mode = viewer->view_mode_state_->get();

    std::string target_layer_id = this->target_layer_state_->get();
    if ( target_layer_id == Tool::NONE_OPTION_C )
    {
      return false;
    }
    target_slice = viewer->get_volume_slice( target_layer_id );

    if ( ! target_slice || target_slice->out_of_boundary() )
    {
      return false;
    }

    viewer->window_to_world( mouse_history.current_.x_,
                             mouse_history.current_.y_, world_x, world_y );
  }

  if ( button == Core::MouseButton::LEFT_BUTTON_E &&
       ( modifiers == Core::KeyModifier::NO_MODIFIER_E ||
         modifiers & ( Core::KeyModifier::ALT_MODIFIER_E) ) )
  {
    int u, v;
    target_slice->world_to_index( world_x, world_y, u, v );
    if ( u >= 0 && u < static_cast< int >( target_slice->nx() ) &&
         v >= 0 && v < static_cast< int >( target_slice->ny() ) )
    {
      Core::Point pos;
      target_slice->get_world_coord( world_x, world_y, pos );
      Core::ActionAdd::Dispatch( Core::Interface::GetMouseActionContext(),
                                 this->seed_points_state_, pos );
      Core::ActionAdd::Dispatch( Core::Interface::GetMouseActionContext(),
                                 this->view_modes_state_, view_mode );
      return true;
    }
  }
  else if ( button == Core::MouseButton::RIGHT_BUTTON_E &&
           ( modifiers == Core::KeyModifier::NO_MODIFIER_E ||
            modifiers & (Core::KeyModifier::ALT_MODIFIER_E) )  )
  {
    Core::Point pt;
    if ( this->find_point( viewer, world_x, world_y, target_slice, pt ) )
    {
      Core::ActionRemove::Dispatch( Core::Interface::GetMouseActionContext(),
                                    this->seed_points_state_, pt );
      Core::ActionRemove::Dispatch( Core::Interface::GetMouseActionContext(),
                                    this->view_modes_state_, view_mode );
    }
    return true;
  }

  return false;
}

void ImplicitModelTool::handle_convex_hull_type_changed( const std::string& type )
{
  ASSERT_IS_APPLICATION_THREAD();

  if ( type == ImplicitModelTool::CONVEX_HULL_2D_C )
  {
    this->compute_2D_convex_hull_state_->set( true );
  }
  else
  {
    this->compute_2D_convex_hull_state_->set( false );
  }
}

void ImplicitModelTool::handle_seed_points_changed()
{
  if (this->seed_points_state_->size() == 0)
  {
    Core::ActionClear::Dispatch( Core::Interface::GetWidgetActionContext(),
                                 this->view_modes_state_ );
  }
  SeedPointsTool::handle_seed_points_changed();
}

void ImplicitModelTool::execute( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  ActionImplicitModel::Dispatch( context,
                                 this->target_layer_state_->get(),
                                 this->seed_points_state_->get(),
                                 this->view_modes_state_->get(),
                                 this->normalOffset_state_->get(),
                                 this->compute_2D_convex_hull_state_->get(),
                                 this->invert_seed_order_state_->get(),
                                 this->kernel_state_->get()
                               );
}

} // end namespace Seg3D
