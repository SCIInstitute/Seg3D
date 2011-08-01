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

// Boost includes
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/MeasurementTool.h>
#include <Application/ViewerManager/ViewerManager.h>

// Core includes
#include <Core/Geometry/Point.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/State/Actions/ActionAdd.h>
#include <Core/State/Actions/ActionRemove.h>
#include <Core/State/Actions/ActionSetAt.h>
#include <Core/TextRenderer/TextRenderer.h>
#include <Core/Utils/EnumClass.h>
#include <Core/Utils/Lockable.h>
#include <Core/Viewer/Mouse.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, MeasurementTool )

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class HoverMeasurement
//////////////////////////////////////////////////////////////////////////

CORE_ENUM_CLASS
(
	MeasurementHoverObject,
	POINT_0_E = 0,
	POINT_1_E = 1,
	LINE_E = 2,
	LABEL_E = 3,
	NONE_E
)

class HoverMeasurement
{
public:
	HoverMeasurement();

	bool is_valid() const;
	void invalidate();

	bool is_point() const;
	bool is_line() const;

	int index_; // Index in measurements state vector

	// Specific part of measurement that is hovered over (point, line, etc.)
	MeasurementHoverObject hover_object_; 
};

HoverMeasurement::HoverMeasurement() 
	: hover_object_( MeasurementHoverObject::NONE_E )
{
	this->invalidate();
}

bool HoverMeasurement::is_valid() const
{
	if( this->index_ == -1 || this->hover_object_ == MeasurementHoverObject::NONE_E )
	{
		return false;
	}
	return true;
}

void HoverMeasurement::invalidate()
{
	this->index_ = -1;
	this->hover_object_ = MeasurementHoverObject::NONE_E;
}

bool HoverMeasurement::is_point() const
{
	return ( this->hover_object_ == MeasurementHoverObject::POINT_0_E || 
		this->hover_object_ == MeasurementHoverObject::POINT_1_E );
}

bool HoverMeasurement::is_line() const
{
	return ( this->hover_object_ == MeasurementHoverObject::LINE_E );
}

//////////////////////////////////////////////////////////////////////////
// Class MeasurementToolPrivate
//////////////////////////////////////////////////////////////////////////

CORE_ENUM_CLASS
(
	 MeasurementEditMode,
	 NONE_E,
	 MOVE_POINT_E,
	 MOVE_MEASUREMENT_E,
	 SNAP_TO_AXIS_E
 )

class MeasurementToolPrivate : public Core::RecursiveLockable
{
public:
	MeasurementToolPrivate();

	//
	// Tool Interface 
	// 

	void handle_measurements_changed();
	void handle_units_selection_changed( std::string units );
	void handle_active_layer_changed( LayerHandle active_layer );
	void handle_opacity_changed();
	void handle_active_viewer_changed( int active_viewer );
	void handle_viewer_slice_changed( size_t viewer_id, int slice_num );

	// INITIALIZE_ID_COUNTER:
	// ID counter used to create unique default names for measurements.
	void initialize_id_counter();
	std::string get_next_measurement_id();

	//
	// Viewer 
	//

	// UPDATE_VIEWERS:
	// Redraw in relevant viewers.
	void update_viewers();

	// FIND_HOVER_MEASUREMENT:
	// Find measurement (if any) that mouse is currently hovering over.
	bool find_hover_measurement();

	// FIND_HOVER_POINT:
	// Find measurement point (if any) that mouse is currently hovering over.
	bool find_hover_point();

	// FIND_HOVER_LINE:
	// Find measurement line (if any) that mouse is currently hovering over.
	bool find_hover_line();

	// MOVE_HOVER_OBJECT_TO_MOUSE:
	// Move hover object (point or entire measurement) to current mouse position.
	void move_hover_object_to_mouse();

	// SNAP_HOVER_POINT_TO_SLICE:
	// Snap the hover point to its projected position on the current slice.
	void snap_hover_point_to_slice();

	// SNAP_HOVER_POINT_TO_AXIS:
	// Snap the hover point to be aligned with the other point along the closest axis in the current
	// view.
	void snap_hover_point_to_axis(); 

	// UPDATE_HOVER_MEASUREMENT:
	// Find or move the hover measurement, depending on whether user is editing or not.
	void update_hover_measurement();

	// GET_HOVER_MEASUREMENT:
	// Locks: StateEngine
	bool get_hover_measurement( Core::Measurement& measurement ) const;
	
	// UPDATE_CURSOR:
	// Set cursor based on current editing/hover state
	void update_cursor();

	void start_editing();
	void finish_editing();

	void create_new_measurement();

	bool get_mouse_world_point( Core::Point& world_point );

	// POINT_IN_SLICE:
	// Return true if the given point is in the current slice.  This is slightly different from seed 
	// points -- in this case the point needs to be within epsilon of the slice, not just nearest
	// to it. 
	bool point_in_slice( ViewerHandle viewer, const Core::Point& world_point, double& point_depth ) const;

	// HOVER_OBJECT_IN_SLICE:
	// Return true if the hover object (point or line) is in slice.  
	bool hover_object_in_slice() const;

	// CONVERT_CURRENT_TO_WORLD:
	// Convert length from current units (show_world_units_state_) to world units.
	double convert_unit_to_world( double length ) const;

	// CONVERT_WORLD_TO_CURRENT:
	// Convert world length to current units (show_world_units_state_).
	double convert_world_to_unit( double length ) const;

	MeasurementTool* tool_;

	ViewerHandle viewer_; // Should be mutex-protected

	std::string active_group_id_; // Only accessed from application thread

	// Is a measurement being edited?  What mode?
	MeasurementEditMode edit_mode_; // Should be mutex-protected

	// Measurement that is currently hovered over by the mouse. 
	// We need to know the hover point or line for left-click + mouse move (editing),
	// middle-click (move point to current slice) and right-click (delete measurement).
	// Want to store this so that we don't have to "find" the hover point every time the mouse moves
	// during editing since we might find a different point and start moving that one instead.  
	// Also need to check this in redraw when determining which measurement (if any) to draw as a 
	// dotted line.
	HoverMeasurement hover_measurement_; // Should be mutex-protected

	int measurement_id_counter_; // Should be mutex-protected

	Core::MousePosition mouse_pos_; // Should be mutex-protected

	int saved_num_measurements_; // Only accessed from application thread

	bool handle_measurements_changed_blocked_; // Only accessed from application thread

	// Start points when dragging measurement line
	Core::Point drag_mouse_start_; // Only accessed from interface thread
	Core::Point drag_p0_start_; // Only accessed from interface thread
	Core::Point drag_p1_start_; // Only accessed from interface thread

	// Modifier key for snap to axis
	bool snap_key_pressed_;

	// Have measurements been initialized from session file or by adding first measurement?
	bool measurements_initialized_;
};

MeasurementToolPrivate::MeasurementToolPrivate()
	: edit_mode_( MeasurementEditMode::NONE_E )// CORE_ENUM_CLASS has no default constructor
{
	this->active_group_id_ = "";
	this->edit_mode_ = MeasurementEditMode::NONE_E;
	this->measurement_id_counter_ = -1;
	this->saved_num_measurements_ = 0;
	this->handle_measurements_changed_blocked_ = false;
	this->snap_key_pressed_ = false;
	this->measurements_initialized_ = false;
}

// Called in response to state changed signal
void MeasurementToolPrivate::handle_measurements_changed()
{
	// Running on application thread, so measurements list and active index won't be changed out 
	// from under us.
	ASSERT_IS_APPLICATION_THREAD();

	// Prevent circular updates
	if( this->handle_measurements_changed_blocked_ ) 
	{
		return;
	}
	this->handle_measurements_changed_blocked_ = true;

	int num_measurements = static_cast< int >( this->tool_->measurements_state_->get().size() );
	
	bool num_measurements_changed = num_measurements != saved_num_measurements_;
	if( num_measurements_changed )
	{
		this->tool_->num_measurements_changed_signal_();
	}

	// Reinitialize ID counter
	this->initialize_id_counter();

	// Measurements may have been added or removed, so update the active index
	if( num_measurements > 0 )
	{
		int active_index = this->tool_->active_index_state_->get();
		bool active_index_invalid = active_index == -1 || active_index >= num_measurements;

		// Don't want to set the active index if we've just read it from a session file, so
		// check to see if measurements have been initialized.
		if( active_index_invalid || 
			( num_measurements_changed && this->measurements_initialized_ ) )
		{
			// Set active index to end of list.	
			Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
				this->tool_->active_index_state_, num_measurements - 1 );
		}
	}
	else
	{
		Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
			this->tool_->active_index_state_, -1 );
	}

	this->saved_num_measurements_ = num_measurements;
	this->handle_measurements_changed_blocked_ = false;

	// Need to update the cursor since hover measurement may have been affected by change
	this->update_cursor();

	// Need to redraw the overlay
	this->update_viewers();

	// Measurements have changed, so they have been initialized
	this->measurements_initialized_ = true;
}

void MeasurementToolPrivate::handle_units_selection_changed( std::string units )
{
	// Don't need to lock state engine because we're running on app thread
	ASSERT_IS_APPLICATION_THREAD();

	bool old_show_world_units_state = this->tool_->show_world_units_state_->get();

	if ( units == MeasurementTool::WORLD_UNITS_C )
	{
		this->tool_->show_world_units_state_->set( true );
	}
	else
	{
		this->tool_->show_world_units_state_->set( false );
	}

	// If units have changed, emit signal
	if( old_show_world_units_state != this->tool_->show_world_units_state_->get() )
	{
		this->tool_->units_changed_signal_();

		// Need to redraw the overlay since length is rendered with measurements
		this->update_viewers();
	}
}

void MeasurementToolPrivate::handle_active_layer_changed( LayerHandle active_layer )
{
	ASSERT_IS_APPLICATION_THREAD();

	// To minimize measurement table updates, only emit units_changed_signal_ when 
	// the active group has changed AND the index units are selected.
	bool show_index_units = !this->tool_->show_world_units_state_->get();
	if ( active_layer )
	{
		// If the active group has changed
		std::string curr_active_group_id = active_layer->get_layer_group()->get_group_id();
		// Don't need to mutex-protect active_group_id_ because it will only be accessed
		// on the application thread.
		// Check to see if the group size is 1 to handle case where layer is deleted, then 
		// delete is undone but group id didn't get changed in process.
		if( this->active_group_id_ != curr_active_group_id || 
			active_layer->get_layer_group()->number_of_layers() == 1 )  
		{
			this->active_group_id_ = curr_active_group_id;

			if( show_index_units )
			{
				// Signal that measurement units have changed
				this->tool_->units_changed_signal_();
			}	
		}
	}
	else
	{
		this->active_group_id_ = "";
	}
}

void MeasurementToolPrivate::handle_opacity_changed()
{
	ASSERT_IS_APPLICATION_THREAD();

	this->update_viewers();
}

void MeasurementToolPrivate::handle_active_viewer_changed( int active_viewer )
{
	ASSERT_IS_APPLICATION_THREAD();

	size_t viewer_id = static_cast< size_t >( active_viewer );
	ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
	if ( viewer && !viewer->is_volume_view() )
	{
		this->handle_viewer_slice_changed( viewer_id, viewer->slice_number_state_->get() );
	}
}

void MeasurementToolPrivate::handle_viewer_slice_changed( size_t viewer_id, int slice_num )
{
	ASSERT_IS_APPLICATION_THREAD();

	size_t active_viewer = static_cast< size_t >( ViewerManager::Instance()->
		active_viewer_state_->get() );
	if ( viewer_id != active_viewer )
	{
		return;
	}

	// Update hover point based on new slice
	this->update_hover_measurement();
}

void MeasurementToolPrivate::initialize_id_counter()
{
	// May be run from application or interface thread
	lock_type lock( this->get_mutex() );

	// Find highest ID in use
	this->measurement_id_counter_ = 0;
	Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
	const std::vector< Core::Measurement >& measurements = this->tool_->measurements_state_->get();
	BOOST_FOREACH( Core::Measurement m, measurements )
	{
		int measurement_id = 0;
		boost::regex reg( "(M)(\\d*)" );
		boost::smatch sub_matches;
		// Note that we can't just pass m.get_id() directly into regex_match because the string will
		// go out of scope and sub_matches contains iterators that point into the string.  
		std::string str = m.get_name();
		if( boost::regex_match( str, sub_matches, reg ) ) 
		{
			Core::ImportFromString( sub_matches[ 2 ].str(), measurement_id );

			if( measurement_id >= this->measurement_id_counter_ )
			{
				this->measurement_id_counter_ = measurement_id + 1;
			}
		}
	}
}

std::string MeasurementToolPrivate::get_next_measurement_id()
{
	ASSERT_IS_INTERFACE_THREAD();

	lock_type lock( this->get_mutex() );

	if( this->measurement_id_counter_ == -1 )
	{
		this->initialize_id_counter();
	}

	// Find first id not already in use (may have been loaded from session file)
	while( true )
	{
		std::string canditate_id = "M" + 
			Core::ExportToString( this->measurement_id_counter_ );
		this->measurement_id_counter_++;

		// Make sure this ID isn't in use
		bool in_use = false;
		Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
		const std::vector< Core::Measurement >& measurements = 
			this->tool_->measurements_state_->get();
		BOOST_FOREACH( Core::Measurement m, measurements )
		{
			if( canditate_id == m.get_name() )
			{
				in_use = true;
				break;
			}
		}
		if( !in_use )
		{
			return canditate_id;
		}
	}
}

void MeasurementToolPrivate::update_viewers()
{
	// May be called from application or interface thread
	ViewerManager::Instance()->update_2d_viewers_overlay();
}

bool MeasurementToolPrivate::find_hover_measurement()
{
	// May be called from application (slice changed) or interface (mouse move) thread
	lock_type lock( this->get_mutex() );

	this->hover_measurement_.invalidate();

	if( !this->viewer_ || !this->viewer_->get_active_volume_slice() ) 
	{
		return false;
	}

	return ( this->find_hover_point() || this->find_hover_line() );
}

bool MeasurementToolPrivate::find_hover_point()
{
	// Need to find first point within radius.
	// NOTE: Should we find closest point within radius instead?

	// May be called from application (slice changed) or interface (mouse move) thread
	lock_type lock( this->get_mutex() );

	Core::VolumeSliceHandle active_slice = this->viewer_->get_active_volume_slice();
	if( !active_slice )
	{
		return false;
	}

	// Compute the size of a pixel in world space
	double x0, y0, x1, y1;
	this->viewer_->window_to_world( 0, 0, x0, y0 );
	this->viewer_->window_to_world( 1, 1, x1, y1 );
	double pixel_width = Core::Abs( x1 - x0 );
	double pixel_height = Core::Abs( y1 - y0 );

	// Compute the mouse position in world space
	double mouse_x, mouse_y;
	this->viewer_->window_to_world( this->mouse_pos_.x_, this->mouse_pos_.y_, mouse_x, mouse_y );

	// Search for the first vertex that's within 2 pixels of current mouse position
	double range_x = pixel_width * 4;
	double range_y = pixel_height * 4;

	Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
	const std::vector< Core::Measurement >& measurements = this->tool_->measurements_state_->get();
	for( size_t m_idx = 0; m_idx < measurements.size(); m_idx++ )
	{
		const Core::Measurement& m = measurements[ m_idx ];
		// Ignore hidden measurements
		if( m.get_visible() )
		{
			// For each measurement point, project onto slice and check to see if in radius
			for( int point_index = 0; point_index < 2; point_index++ )
			{
				Core::Point measurement_point;
				m.get_point( point_index, measurement_point );
				double pt_x, pt_y;
				active_slice->project_onto_slice( measurement_point, pt_x, pt_y );
				if ( Core::Abs( pt_x - mouse_x ) <= range_x &&
					Core::Abs( pt_y - mouse_y ) <= range_y )
				{
					this->hover_measurement_.index_ = static_cast< int >( m_idx );
					MeasurementHoverObject hover_object = ( point_index == 0 ) ? 
						MeasurementHoverObject::POINT_0_E : MeasurementHoverObject::POINT_1_E;
					this->hover_measurement_.hover_object_ = hover_object; 
					return true;
				}
			}
		}
	}

	return false;
}

bool MeasurementToolPrivate::find_hover_line()
{
	// May be called from application (slice changed) or interface (mouse move) thread
	lock_type lock( this->get_mutex() );

	Core::VolumeSliceHandle active_slice = this->viewer_->get_active_volume_slice();
	if( !active_slice )
	{
		return false;
	}

	// Compute the size of a pixel in world space
	double x0, y0, x1, y1;
	this->viewer_->window_to_world( 0, 0, x0, y0 );
	this->viewer_->window_to_world( 1, 1, x1, y1 );
	double pixel_width = Core::Abs( x1 - x0 );
	double epsilon = pixel_width * 4;

	// Compute the mouse position in world space
	double mouse_x, mouse_y;
	this->viewer_->window_to_world( this->mouse_pos_.x_, this->mouse_pos_.y_, mouse_x, mouse_y );
	Core::Point mouse_point( mouse_x, mouse_y, 0 );

	Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
	const std::vector< Core::Measurement >& measurements = this->tool_->measurements_state_->get();

	bool hovering = false;
	double min_dist = DBL_MAX;
	for( size_t m_idx = 0; m_idx < measurements.size(); m_idx++ )
	{
		const Core::Measurement& m = measurements[ m_idx ];

		// Get both measurement points
		Core::Point p0, p1;
		m.get_point( 0, p0 );
		m.get_point( 1, p1 );

		// Project points onto slice so that we can detect if we're hovering over a measurement
		// not in the current slice.
		double p0_x, p0_y;
		active_slice->project_onto_slice( p0, p0_x, p0_y );
		p0 = Core::Point( p0_x, p0_y, 0 );
		double p1_x, p1_y;
		active_slice->project_onto_slice( p1, p1_x, p1_y );
		p1 = Core::Point( p1_x, p1_y, 0 );

		// Calculate the shortest distance between the point and the line segment
		double distance = 0.0;
		double l2 = ( p1 - p0 ).length2();  // i.e. |w-v|^2 -  avoid a sqrt
		if( l2 == 0.0 ) // p0 == p1 case
		{
			distance = ( mouse_point - p0 ).length();
		}
		else
		{
			// Consider the line extending the segment, parameterized as v + t (w - v).
			// We find projection of point p onto the line. 
			// It falls where t = [(p-v) . (w-v)] / |w-v|^2
			double t = Dot( mouse_point - p0, p1 - p0 ) / l2;
			if( t < 0.0 || t > 1.0 )
			{
				// Beyond the segment
				continue;
			}
			else
			{
				Core::Point projection = p0 + t * ( p1 - p0 );  // Projection falls on the segment
				distance = ( mouse_point - projection ).length();
			}	
		}

		if( distance > epsilon || distance > min_dist )
		{
			continue;
		}
		
		// Found the new closest measurement
		hovering = true;
		min_dist = distance;
		this->hover_measurement_.index_ = static_cast< int >( m_idx );
		this->hover_measurement_.hover_object_ = MeasurementHoverObject::LINE_E;
	}

	return hovering;
}

void MeasurementToolPrivate::move_hover_object_to_mouse()
{
	// May be called from application (slice changed) or interface (mouse move) thread
	lock_type lock( this->get_mutex() );
	
	Core::Measurement measurement;
	if( this->get_hover_measurement( measurement ) )
	{
		// Convert current mouse coords to 3D world point
		Core::Point curr_mouse_pt;
		if( this->get_mouse_world_point( curr_mouse_pt ) )
		{
			if( this->hover_measurement_.is_point() )
			{
				// Update single hover point
				measurement.set_point( this->hover_measurement_.hover_object_, curr_mouse_pt );

				Core::ActionSetAt::Dispatch( Core::Interface::GetMouseActionContext(), 
					this->tool_->measurements_state_, this->hover_measurement_.index_, 
					measurement );
			}
			else if( this->hover_measurement_.is_line() )
			{
				// Find vector between mouse start and end
				Core::Vector mouse_vec = curr_mouse_pt - this->drag_mouse_start_;

				// Add vector to p0 and p1
				Core::Measurement measurement;
				if( this->get_hover_measurement( measurement ) )
				{
					// Update measurement
					measurement.set_point( 0, this->drag_p0_start_ + mouse_vec );
					measurement.set_point( 1, this->drag_p1_start_ + mouse_vec );

					Core::ActionSetAt::Dispatch( Core::Interface::GetMouseActionContext(), 
						this->tool_->measurements_state_, this->hover_measurement_.index_, 
						measurement );
				}
			}
		}
	}
}

void MeasurementToolPrivate::snap_hover_point_to_slice()
{
	ASSERT_IS_INTERFACE_THREAD();
	lock_type lock( this->get_mutex() );

	// Find world hover point
	Core::Measurement edited_measurement;
	if( this->get_hover_measurement( edited_measurement ) )
	{
		Core::Point measurement_point;
		edited_measurement.get_point( this->hover_measurement_.hover_object_, measurement_point );

		if( !this->viewer_ )
		{
			return;
		}

		// Project hover point onto current slice, find world point
		Core::VolumeSliceHandle active_slice = this->viewer_->get_active_volume_slice();
		if( !active_slice )
		{
			return;
		}
		
		double world_x, world_y;
		active_slice->project_onto_slice( measurement_point, world_x, world_y );
		Core::Point moved_pt;
		active_slice->get_world_coord( world_x, world_y, moved_pt );

		// Update hover point
		edited_measurement.set_point( this->hover_measurement_.hover_object_, moved_pt );
		Core::ActionSetAt::Dispatch( Core::Interface::GetMouseActionContext(), 
			this->tool_->measurements_state_, this->hover_measurement_.index_, edited_measurement );
		
		return;
	}
	return;
}

void MeasurementToolPrivate::snap_hover_point_to_axis()
{
	// Called from interface thread
	lock_type lock( this->get_mutex() );

	// Get hovered measurement
	Core::Measurement measurement; 
	if( !this->get_hover_measurement( measurement ) )
	{
		return;
	}

	// Get both points
	Core::Point points[ 2 ];
	measurement.get_point( 0, points[ 0 ] );	
	measurement.get_point( 1, points[ 1 ] );
	int hover_pt_idx = this->hover_measurement_.hover_object_;
	if( hover_pt_idx > 1 ) return;
	size_t snap_pt_idx = 1 - hover_pt_idx;

	// Move hover point to mouse 
	Core::Point mouse_point;
	this->get_mouse_world_point( mouse_point );
	points[ hover_pt_idx ] = mouse_point;
	
	// Find vector between points
	Core::Vector measure_vec = Abs( points[ 1 ] - points[ 0 ] );

	// Check the type of viewer and based on that determine which 2 components to check.  
	Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
	std::string view_mode = this->viewer_->view_mode_state_->get();

	// Set smaller of two components to same as the non-hover point.
	if( view_mode == Viewer::AXIAL_C )
	{
		// Compare x and y components
		if( measure_vec.x() > measure_vec.y() )
		{
			points[ hover_pt_idx ].y( points[ snap_pt_idx ].y() );
		}
		else
		{
			points[ hover_pt_idx ].x( points[ snap_pt_idx ].x() );
		}
	}
	else if( view_mode == Viewer::CORONAL_C )
	{
		// Compare x and z components
		if( measure_vec.x() > measure_vec.z() )
		{
			points[ hover_pt_idx ].z( points[ snap_pt_idx ].z() );
		}
		else
		{
			points[ hover_pt_idx ].x( points[ snap_pt_idx ].x() );
		}
	}
	else if( view_mode == Viewer::SAGITTAL_C )
	{
		// Compare y and z components
		if( measure_vec.y() > measure_vec.z() )
		{
			points[ hover_pt_idx ].z( points[ snap_pt_idx ].z() );
		}
		else
		{
			points[ hover_pt_idx ].y( points[ snap_pt_idx ].y() );
		}
	}
	else
	{
		return;
	}

	// Update measurement
	measurement.set_point( hover_pt_idx, points[ hover_pt_idx ] );
	
	Core::ActionSetAt::Dispatch( Core::Interface::GetMouseActionContext(), 
		this->tool_->measurements_state_, this->hover_measurement_.index_, measurement );
}

void MeasurementToolPrivate::update_hover_measurement()
{
	// May be called from application (slice changed) or interface (mouse move) thread
	lock_type lock( this->get_mutex() );

	if( !this->viewer_ )
	{
		return;
	}

	// Note: We don't want to "find" the hover point every time the mouse moves
	// during editing since we might find a different point and start moving that one instead.
	if( this->edit_mode_ == MeasurementEditMode::MOVE_MEASUREMENT_E )
	{	
		this->move_hover_object_to_mouse();
	}
	else if( this->edit_mode_ == MeasurementEditMode::MOVE_POINT_E )
	{
		if( this->snap_key_pressed_ )
		{
			this->snap_hover_point_to_axis();
		}
		else
		{
			this->move_hover_object_to_mouse();
		}		
	}
	else
	{	
		// Find the measurement that the mouse is currently hovering over, if there is one
		this->find_hover_measurement();
	}
	this->update_cursor();
}

// TODO: Get rid of index once IDs are replaced with indexes
bool MeasurementToolPrivate::get_hover_measurement( Core::Measurement& measurement ) const
{
	// May be called from application or interface thread
	lock_type lock( this->get_mutex() );

	if( this->hover_measurement_.is_valid() )
	{
		// Get measurements
		Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
		const std::vector< Core::Measurement >& measurements = 
			this->tool_->measurements_state_->get();

		if( this->hover_measurement_.index_ < measurements.size() )
		{
			measurement = measurements[ this->hover_measurement_.index_ ];
			return true;
		}
	}
	return false;
}

void MeasurementToolPrivate::update_cursor()
{
	// May be called from application or interface thread
	lock_type lock( this->get_mutex() );

	if( !this->viewer_ ) return;

	if( this->edit_mode_ == MeasurementEditMode::MOVE_POINT_E )
	{
		// Use cross cursor so that features of interest are not obscured.  This is the same cursor
		// used normally, but we need a cursor and couldn't find a better one, so the measurement
		// itself will be rendered differently to indicate that editing is happening.
		this->viewer_->set_cursor( Core::CursorShape::CROSS_E );
		return;
	}
	else if( this->edit_mode_ == MeasurementEditMode::MOVE_MEASUREMENT_E )
	{
		// Based on PowerPoint
		this->viewer_->set_cursor( Core::CursorShape::SIZE_ALL_E );
		return;
	}
	else if( this->hover_measurement_.is_valid() ) 
	{
		// State: Hovering, not editing

		bool in_slice = this->hover_object_in_slice();
		if( this->hover_measurement_.is_point() )
		{
			if( in_slice )
			{
				// Set cursor to open hand to indicate that point could be selected (but isn’t)
				this->viewer_->set_cursor( Core::CursorShape::OPEN_HAND_E );
				return;
			}
			else
			{
				// Out of slice -- can be snapped to slice but not edited otherwise
				this->viewer_->set_cursor( Core::CursorShape::UP_ARROW_E );
				return;
			}
		}
		else if( this->hover_measurement_.is_line() )
		{
			if( in_slice )
			{
				// Based on PowerPoint
				this->viewer_->set_cursor( Core::CursorShape::SIZE_ALL_E );
				return;
			}
		}
	}
	
	// Default: Set cross icon color to grey (normal)
	this->viewer_->set_cursor( Core::CursorShape::CROSS_E );
}

void MeasurementToolPrivate::start_editing()
{
	ASSERT_IS_INTERFACE_THREAD();
	lock_type lock( this->get_mutex() );

	if( !this->viewer_ )
	{
		return;
	}
	
	if( this->hover_measurement_.is_point() )
	{
		// Move existing measurement point
		this->edit_mode_ = MeasurementEditMode::MOVE_POINT_E;
	}
	else if( this->hover_measurement_.is_line() )
	{
		// Save current measurement and mouse points so that offsets can be added to these
		// Add vector to p0 and p1
		Core::Measurement measurement;
		if( this->get_hover_measurement( measurement ) )
		{
			// Get both points
			measurement.get_point( 0, this->drag_p0_start_ );	
			measurement.get_point( 1, this->drag_p1_start_ );

			// Store world point for mouse as move start point
			this->get_mouse_world_point( this->drag_mouse_start_ );

			// Move existing measurement line
			this->edit_mode_ = MeasurementEditMode::MOVE_MEASUREMENT_E;
		}
		else
		{
			this->edit_mode_ = MeasurementEditMode::NONE_E;
		}
		
	}

	// Use blank cursor so that features of interest are not obscured
	this->update_cursor();

	// Redraw so that measurement is drawn with dotted line instead of solid line
	this->update_viewers();
}

void MeasurementToolPrivate::finish_editing()
{
	ASSERT_IS_INTERFACE_THREAD();
	lock_type lock( this->get_mutex() );

	if( !this->viewer_ )
	{
		return;
	}

	this->edit_mode_ = MeasurementEditMode::NONE_E;

	// No need to update measurement (interactively updated) 

	// Change cursor to indicate that editing is not happening
	this->update_cursor();
	
	// Redraw so that measurement is drawn with solid line instead of dotted line
	this->update_viewers();
}

void MeasurementToolPrivate::create_new_measurement()
{
	// Called from interface thread
	lock_type lock( this->get_mutex() );

	// Get 3D point from 2D mouse coords, create new measurement with P1 = P2, 
	// add to state vector using action.
	Core::Point pt;
	if( this->get_mouse_world_point( pt ) )
	{
		// Find number of measurements in order to get index
		Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
		const std::vector< Core::Measurement >& measurements = 
			this->tool_->measurements_state_->get();

		this->hover_measurement_.index_ = static_cast< int >( measurements.size() );
		// Second point in measurement needs to be the hover point
		this->hover_measurement_.hover_object_ = MeasurementHoverObject::POINT_1_E; 

		// Create new measurement 
		Core::Measurement measurement;

		// Need ID for measurement
		measurement.set_name( this->get_next_measurement_id() );
		measurement.set_point( 0, pt );
		measurement.set_point( 1, pt );
		measurement.set_visible( true );

		// Find number of measurements in order to get index
		Core::ActionAdd::Dispatch( Core::Interface::GetMouseActionContext(), 
			this->tool_->measurements_state_, measurement );

		this->start_editing();
	}
}

bool MeasurementToolPrivate::get_mouse_world_point( Core::Point& world_point )
{
	// May be called from application or interface thread
	lock_type lock( this->get_mutex() );

	if( !this->viewer_ )
	{
		return false;
	}

	// Make sure to use current viewer, slice, and mouse coordinates.  These aren't passed
	// as parameters because we don't have this info in the case where the view has been changed
	// by a key command.
	double world_x, world_y;
	this->viewer_->window_to_world( this->mouse_pos_.x_, this->mouse_pos_.y_, world_x, world_y );
	Core::VolumeSliceHandle active_slice = this->viewer_->get_active_volume_slice();
	if( !active_slice )
	{
		return false;
	}

	active_slice->get_world_coord( world_x, world_y, world_point );	
	return true;
}

bool MeasurementToolPrivate::point_in_slice( ViewerHandle viewer, const Core::Point& world_point, 
	double& point_depth ) const
{
	// May be called from interface or rendering thread

	// Basically in_slice has to be within epsilon of this slice so that editing won't move the
	// measurement points
	Core::VolumeSliceHandle volume_slice = viewer->get_active_volume_slice();
	if( !volume_slice )
	{
		return false;
	}

	double slice_depth = volume_slice->depth();
	double i_pos, j_pos;
	volume_slice->project_onto_slice( world_point, i_pos, j_pos, point_depth );

	if( point_depth == slice_depth ) 
	{
		return true;
	}
	return false;
}

bool MeasurementToolPrivate::hover_object_in_slice() const
{
	lock_type lock( this->get_mutex() );

	Core::Measurement measurement; 
	if( !this->get_hover_measurement( measurement ) )
	{
		return false;
	}

	// Check single point for hovered point, or both points for hovered line
	double point_depth;
	if( this->hover_measurement_.is_point() )
	{
		// Get the single hovered point
		Core::Point world_point;
		measurement.get_point( this->hover_measurement_.hover_object_, world_point );

		return this->point_in_slice( this->viewer_, world_point, point_depth );
	}
	else if( this->hover_measurement_.is_line() )
	{
		// Get both points
		Core::Point p0, p1;
		measurement.get_point( 0, p0 );	
		measurement.get_point( 1, p1 );

		bool p0_in_slice = this->point_in_slice( this->viewer_, p0, point_depth );
		bool p1_in_slice = this->point_in_slice( this->viewer_, p1, point_depth );

		return ( p0_in_slice && p1_in_slice ) ;
	}
	return false;
}

// Used to convert user-edited length to world units required by Measurement class.
double MeasurementToolPrivate::convert_unit_to_world( double length ) const
{
	// Called from interface thread

	// We need access to the show_world_units_state_, and several functions we call also lock
	// the state engine.  
	Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

	// Thread-safety: We get a handle to the active layer 
	// (get_active_layer is thread-safe), so it can't be deleted out from under 
	// us.  
	LayerHandle active_layer = LayerManager::Instance()->get_active_layer();

	if( !this->tool_->show_world_units_state_->get() && active_layer ) // Current = Index units
	{
		// Index units

		// Convert index units to world units
		// Use grid transform from active layer
		// Grid transfrom takes index coords to world coords
		Core::GridTransform grid_transform = active_layer->get_grid_transform();

		Core::Vector vec( length, 0, 0 );
		vec = grid_transform.project( vec );
		return vec.length();
	}
	return length; // Current = World units
}

// Used to convert Measurement length to index units for display if selected in tool.
double MeasurementToolPrivate::convert_world_to_unit( double length ) const
{
	// May be called from interface or render threads

	// NOTE: Do not call this function if RendererResources is locked.
	// We need access to the show_world_units_state_, and several functions we call also lock
	// the state engine.  
	Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

	// Thread-safety: We get a handle to the active layer 
	// (get_active_layer is thread-safe), so it can't be deleted out from under 
	// us.  
	LayerHandle active_layer = LayerManager::Instance()->get_active_layer();

	if( !this->tool_->show_world_units_state_->get() && active_layer ) // Current = Index units
	{
		// Index units

		// Convert world units to index units
		// Use grid transform from active layer
		Core::GridTransform grid_transform = active_layer->get_grid_transform();

		// Grid transfrom takes index coords to world coords, so we need inverse
		Core::Transform inverse_transform = grid_transform.get_inverse();

		Core::Vector vec( length, 0, 0 );
		vec = inverse_transform.project( vec );
		return vec.length();
	}
	return length; // Current = World units
}

//void create_test_data( std::vector< Core::Measurement >& measurements )
//{
//	// Populate measurements list with test data
//	measurements.push_back( 
//		Core::Measurement( true, "M1", "Knee", Core::Point(0, 0, 0), 
//		Core::Point(1, 1, 1), Core::AXIAL_E, 50, 1 ) );
//	measurements.push_back( 
//		Core::Measurement( true, "M2", "Heart", Core::Point(0, 0, 0), 
//		Core::Point(2, 2, 2), Core::AXIAL_E, 50, 1 ) );
//	measurements.push_back( 
//		Core::Measurement( true, "M3", "Head", Core::Point(0, 0, 0), 
//		Core::Point(3, 3, 3), Core::AXIAL_E, 50, 1 ) );	
//	measurements.push_back( 
//		Core::Measurement( true, "M4", "Toe", Core::Point(0, 0, 0), 
//		Core::Point(4, 4, 4), Core::AXIAL_E, 50, 1 ) );	
//	measurements.push_back( 
//		Core::Measurement( true, "M5", "Eye", Core::Point(0, 0, 0), 
//		Core::Point(5, 5, 5), Core::AXIAL_E, 50, 1 ) );	
//	measurements.push_back( 
//		Core::Measurement( true, "M6", "Nose", Core::Point(0, 0, 0), 
//		Core::Point(6, 6, 6), Core::AXIAL_E, 50, 1 ) );	
//	measurements.push_back( 
//		Core::Measurement( true, "M7", "Hand", Core::Point(0, 0, 0), 
//		Core::Point(7, 7, 7), Core::AXIAL_E, 50, 1 ) );	
//	measurements.push_back( 
//		Core::Measurement( true, "M8", "Ear", Core::Point(0, 0, 0), 
//		Core::Point(8, 8, 8), Core::AXIAL_E, 50, 1 ) );	
//}

//////////////////////////////////////////////////////////////////////////
// Class MeasurementTool
//////////////////////////////////////////////////////////////////////////

const std::string MeasurementTool::INDEX_UNITS_C( "index_units" );
const std::string MeasurementTool::WORLD_UNITS_C( "world_units" );

MeasurementTool::MeasurementTool( const std::string& toolid ) :
	Tool( toolid ),
	private_( new MeasurementToolPrivate )
{
	this->private_->tool_ = this;

	// State variable gets allocated here
	this->add_state( "measurements", this->measurements_state_ );
	this->add_state( "active_index", this->active_index_state_, -1 );
	this->add_state( "units_selection", this->units_selection_state_, WORLD_UNITS_C, 
		INDEX_UNITS_C + "=Pixel|" +
		WORLD_UNITS_C + "=Actual" );
	this->add_state( "show_world_units", this->show_world_units_state_, true );
	this->add_state( "opacity", this->opacity_state_, 1.0, 0.0, 1.0, 0.1 );

	LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
	if( active_layer )
	{
		this->private_->active_group_id_ = active_layer->get_layer_group()->get_group_id();
	}

	// NOTE: Connections are added in activate()
}

MeasurementTool::~MeasurementTool()
{
	this->disconnect_all();
}

bool MeasurementTool::handle_mouse_move( ViewerHandle viewer, 
	const Core::MouseHistory& mouse_history, int button, int buttons, int modifiers )
{
	if( viewer->is_volume_view() ) 
	{
		return false;
	}

	MeasurementToolPrivate::lock_type lock( this->private_->get_mutex() );

	this->private_->viewer_ = viewer;
	this->private_->mouse_pos_.x_ = mouse_history.current_.x_;
	this->private_->mouse_pos_.y_ = mouse_history.current_.y_;
	this->private_->update_hover_measurement();

	// Pass handling on to normal handler 
	return false;
}

bool MeasurementTool::handle_mouse_press( ViewerHandle viewer, 
	const Core::MouseHistory& mouse_history, int button, int buttons, int modifiers )
{
	if( viewer->is_volume_view() )
	{
		return false;
	}

	if( !( modifiers == Core::KeyModifier::NO_MODIFIER_E || 
		modifiers == Core::KeyModifier::CONTROL_MODIFIER_E ||
		modifiers == Core::KeyModifier::GROUPSWITCH_MODIFIER_E ) )
	{
		return false;
	}

	MeasurementToolPrivate::lock_type lock( this->private_->get_mutex() );
	this->private_->viewer_ = viewer;

	// Make hover measurement active
	if( this->private_->hover_measurement_.is_valid() )
	{
		Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
			this->active_index_state_, this->private_->hover_measurement_.index_ );
	}

	if( button == Core::MouseButton::LEFT_BUTTON_E )
	{
		if( this->private_->edit_mode_ != MeasurementEditMode::NONE_E ) 
		{
			// State: We were editing

			// Done editing
			this->private_->finish_editing();

		}
		else 
		{
			// State: We are not editing

			// If hovered point or line 
			if( this->private_->hover_measurement_.is_valid() )
			{
				// If in slice
				if( this->private_->hover_object_in_slice() )
				{
					// Start editing
					this->private_->start_editing();
				}
				// Else: Selected measurement not in slice, do nothing
			}
			else
			{
				// State: Not hovering over a measurement
				this->private_->create_new_measurement();
			}
		}
		return true;
	}
	else if( button == Core::MouseButton::MID_BUTTON_E )
	{
		if( this->private_->hover_measurement_.is_valid() && 
			this->private_->hover_measurement_.is_point() ) // Hovering over point
		{
			// This point does not have to be in current slice
			// Snap point to current slice
			this->private_->snap_hover_point_to_slice();
		}
	}
	else if( button == Core::MouseButton::RIGHT_BUTTON_E )
	{
		// Find 3D world hover point
		if( this->private_->hover_measurement_.is_valid() && 
			this->private_->hover_object_in_slice() )
		{
			Core::Measurement measurement;
			if( this->private_->get_hover_measurement( measurement ) )
			{
				// NOTE: Disabled right-click delete because I think it is dangerous and might cause 
				// users to accidentally lose their data (no undo).
				// TODO: Context menu providing info and delete 

				//// Delete hovered measurement
				//Core::ActionRemove::Dispatch( Core::Interface::GetMouseActionContext(), 
				//	this->measurements_state_, measurement );

				//// Ordering matters here
				//this->private_->hover_measurement_.invalidate();
				//this->private_->finish_editing();
				
				return true;
			}
		}
	}

	return false;
}

bool MeasurementTool::handle_mouse_release( ViewerHandle viewer, 
	const Core::MouseHistory& mouse_history, int button, int buttons, int modifiers )
{
	MeasurementToolPrivate::lock_type lock( this->private_->get_mutex() );
	if( this->private_->edit_mode_ == MeasurementEditMode::MOVE_MEASUREMENT_E )
	{
		this->private_->finish_editing();
	}
	return false;
}

bool MeasurementTool::handle_key_press( ViewerHandle viewer, int key, int modifiers )
{
#ifdef __APPLE__
	if( key == Core::Key::KEY_CONTROL_E || modifiers == Core::KeyModifier::CONTROL_MODIFIER_E ||
		key == Core::Key::KEY_META_E || modifiers == Core::KeyModifier::META_MODIFIER_E ) 
#else
	if( key == Core::Key::KEY_CONTROL_E || modifiers == Core::KeyModifier::CONTROL_MODIFIER_E ) 
#endif
	{
		this->private_->snap_key_pressed_ = true;

		if( this->private_->edit_mode_ == MeasurementEditMode::MOVE_POINT_E )
		{
			this->private_->snap_hover_point_to_axis();
		}
	}

	return false;
}

bool MeasurementTool::handle_key_release( ViewerHandle viewer, int key, int modifiers )
{
#ifdef __APPLE__
	if( key == Core::Key::KEY_CONTROL_E || modifiers == Core::KeyModifier::CONTROL_MODIFIER_E ||
		key == Core::Key::KEY_META_E || modifiers == Core::KeyModifier::META_MODIFIER_E ) 
#else
	if( key == Core::Key::KEY_CONTROL_E || modifiers == Core::KeyModifier::CONTROL_MODIFIER_E ) 
#endif
	{
		this->private_->snap_key_pressed_ = false;

		if( this->private_->edit_mode_ == MeasurementEditMode::MOVE_POINT_E )
		{
			this->private_->move_hover_object_to_mouse();
		}
	}

	return false;
}

void MeasurementTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat,
	int viewer_width, int viewer_height )
{
	// This function can be called from multiple rendering threads -- one per viewer
	ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
	if ( viewer->is_volume_view() )
	{
		return;
	}
	Core::VolumeSliceHandle vol_slice = viewer->get_active_volume_slice();
	if ( !vol_slice )
	{
		return;
	} 

	//-------------- StateEngine locked  -------------------

	// NOTE: The StateEngine and RenderResources should NEVER be locked
	// at the same time because this will lead to deadlock.  So we get all the state we need up
	// front, then unlock the StateEngine, then do the rendering.
	Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

	std::vector< Core::Measurement > measurements = this->measurements_state_->get();
	int active_index = this->active_index_state_->get();
	double opacity = this->opacity_state_->get();
	//std::string label_selection = this->label_selection_state_->get();
	
	// Getting the length string requires locking the state engine because we need access to the
	// show_world_units state and the LayerManager, which locks the state engine.
	std::vector< std::string > length_strings( measurements.size() );
	for( size_t m_idx = 0; m_idx < measurements.size(); m_idx++ )
	{
		if( measurements[ m_idx ].get_visible() ) 
		{
			double world_length = measurements[ m_idx ].get_length();
			length_strings[ m_idx ] = this->convert_world_to_unit_string( world_length );
		}
		else
		{
			length_strings[ m_idx ] = "";
		}
	}
	
	lock.unlock();
	
	//-------------- StateEngine unlocked  -------------------

	Core::Color active_color = Core::Color( 0.55f, 0.82f, 0.96f );
	float out_of_slice_fraction = 0.75f;
	
	glPushAttrib( GL_LINE_BIT | GL_POINT_BIT | GL_TRANSFORM_BIT );
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixd( proj_mat.data() );

	glEnable( GL_LINE_SMOOTH );

	// Projected vertices per measurement
	std::vector< std::vector< Core::Point > > vertices( measurements.size() );
	// Are both points in slice for each measurement
	std::vector< bool > both_in_slice( measurements.size() );

	// Draw lines and points
	for( size_t m_idx = 0; m_idx < measurements.size(); m_idx++ )
	{
		if( measurements[ m_idx ].get_visible() )
		{
			Core::Measurement m = measurements[ m_idx ];
			bool vertex_in_slice[ 2 ];

			// TODO: Visually represent point in front differently?
			// Project points, determine if they are "in slice"
			vertices[ m_idx ].resize( 2 );
			double point_depths[ 2 ]; 
			
			for( size_t v_idx = 0; v_idx < 2; v_idx++ )
			{
				Core::Point measurement_point;
				m.get_point( static_cast< int >( v_idx ), measurement_point );

				vertex_in_slice[ v_idx ] = 
					this->private_->point_in_slice( viewer, measurement_point, point_depths[ v_idx ] );
				
				// Project 3D point onto slice
				double x_pos, y_pos;
				vol_slice->project_onto_slice( measurement_point, x_pos, y_pos );
				
				vertices[ m_idx ][ v_idx ][ 0 ] = x_pos;
				vertices[ m_idx ][ v_idx ][ 1 ] = y_pos;
			}

			// Draw line before points so that points are rendered on top (looks better in case 
			// where one point is "in slice", the other is not)
			MeasurementToolPrivate::lock_type lock( this->private_->get_mutex() );
			both_in_slice[ m_idx ] = vertex_in_slice[ 0 ] && vertex_in_slice[ 1 ];
			bool points_in_same_slice = point_depths[ 0 ] == point_depths[ 1 ];

			Core::Color in_slice_color;
			measurements[ m_idx ].get_color( in_slice_color );
			// If this line is not in slice, display it with reduced color intensity
			Core::Color out_of_slice_color = in_slice_color * out_of_slice_fraction;
			Core::Color color = both_in_slice[ m_idx ] ? in_slice_color : out_of_slice_color;
			glColor4f( color.r(), color.g(), color.b(), static_cast< float >( opacity ) );
			glLineWidth( 2.0f );

			bool editing = this->private_->edit_mode_ != MeasurementEditMode::NONE_E && 
				m_idx == this->private_->hover_measurement_.index_;
			if( editing ) // Editing
			{
				// Draw coarsely dotted line 
				glLineStipple(1, 0x00FF );
				glEnable( GL_LINE_STIPPLE ); 
			}
			else if( !points_in_same_slice ) // Spans slices
			{
				// Draw finely dotted line
				glLineStipple(1, 0x0F0F );
				glEnable( GL_LINE_STIPPLE ); 
			}
			else
			{
				// Draw solid line
				glDisable( GL_LINE_STIPPLE );
			}

			if( !both_in_slice[ m_idx ] ) // Points not both in slice
			{
				// Draw thin line
				glLineWidth( 1.0f );
			}

			glBegin( GL_LINES );
			for ( size_t i = 0; i < 2; i++ )
			{
				glVertex2d( vertices[ m_idx ][ i ].x(), vertices[ m_idx ][ i ].y() );
			}   
			glEnd();

			// Draw points
			for( size_t v_idx = 0; v_idx < 2; v_idx++ )
			{
				// Set point size based on relative point depths
				glPointSize( 5.0f );
				// If this point is farther away than the other point, make it smaller
				if( point_depths[ v_idx ] > point_depths [ 1 - v_idx ] )
				{
					glPointSize( 3.0f );
				}

				Core::Color color;
				if( static_cast<int>( m_idx ) == active_index ) // Active measurement
				{
					color = active_color;
				}
				else
				{
					color = in_slice_color;
				}

				// If this point is not in slice, display it with reduced color intensity
				if( !vertex_in_slice[ v_idx ] ) 
				{
					color = color * out_of_slice_fraction;
				}

				// Make the moving point red in the current viewer (not necessarily active viewer)
				if( this->private_->viewer_ && 
					viewer_id == this->private_->viewer_->get_viewer_id() && 
					this->private_->edit_mode_ != MeasurementEditMode::NONE_E && 
					m_idx == this->private_->hover_measurement_.index_ &&
					v_idx == this->private_->hover_measurement_.hover_object_ )
				{
					color = Core::Color( 1.0, 0.0, 0.0 );
				}

				// Render GL_POINT
				glColor4f( color.r(), color.g(), color.b(), static_cast< float >( opacity ) );
				glBegin( GL_POINTS );
				glVertex2d( vertices[ m_idx ][ v_idx ].x(), vertices[ m_idx ][ v_idx ].y() );
				glEnd();
			}
		}
	}

	// Render length above line, label below line
	// Different projection matrix is required
	glPopMatrix();

	//-------------- StateEngine locked  -------------------

	// Need to lock state engine, but can't lock it and RenderResources at same time or deadlock
	// could occur.  So do conversion to window coordinates first, then do rendering.
	std::vector< std::vector< Core::Point > > window_vertices( measurements.size() );
	for( size_t m_idx = 0; m_idx < measurements.size(); m_idx++ )
	{
		if( measurements[ m_idx ].get_visible() )
		{
			// NOTE: Y values increase from top to bottom of the window
			
			// Convert vertices to window coords
			Core::Point p0 = vertices[ m_idx ][ 0 ];
			Core::Point p1 = vertices[ m_idx ][ 1 ];
			double p0_x, p0_y, p1_x, p1_y;
			// NOTE: Locks state engine, so has to be called before RenderResources is locked
			viewer->world_to_window( p0.x(), p0.y(), p0_x, p0_y );
			viewer->world_to_window( p1.x(), p1.y(), p1_x, p1_y );
			Core::Point window_p0( p0_x, p0_y, 0.0 );
			Core::Point window_p1( p1_x, p1_y, 0.0 );
			window_vertices[ m_idx ].push_back( window_p0 );
			window_vertices[ m_idx ].push_back( window_p1 );
		}
	}

	//-------------- StateEngine unlocked  -------------------

	//-------------- RenderResources locked  -------------------

	// Dealing with textures, so need to lock RenderResources
	Core::RenderResources::lock_type render_lock( Core::RenderResources::GetMutex() );
	// Create local TextRenderer and Texture2D for each render thread to prevent contention 
	Core::TextRendererHandle text_renderer;
	text_renderer.reset( new Core::TextRenderer );
	Core::Texture2DHandle text_texture;
	text_texture.reset( new Core::Texture2D );
	std::vector< unsigned char > buffer( viewer_width * viewer_height, 0 );

	for( size_t m_idx = 0; m_idx < measurements.size(); m_idx++ )
	{
		if( measurements[ m_idx ].get_visible() )
		{
			// Find position of label and length

			Core::Point window_p0 = window_vertices[ m_idx ][ 0 ];
			Core::Point window_p1 = window_vertices[ m_idx ][ 1 ];

			// Find the right-most point
			Core::Point right_point = window_p0.x() > window_p1.x() ? window_p0 : window_p1;

			// Place the labels to the right and below the right-most point
			const int pixel_offset = 4;
			const unsigned int font_size = 14; // Matches slice number font size
			Core::Point label_point( right_point.x() + pixel_offset, 
				right_point.y() + font_size + pixel_offset, 0.0 );
			Core::Point length_point( right_point.x() + pixel_offset, 
				right_point.y() + 2 * font_size + 2 * pixel_offset, 0.0 );

			// Render label
			Core::Measurement m = measurements[ m_idx ];
			std::string label = m.get_name();
			text_renderer->render( label, &buffer[ 0 ], 
				viewer_width, viewer_height, static_cast< int >( label_point.x() ), 
				viewer_height - static_cast< int >( label_point.y() ), font_size, 0 );

			// Render length
			std::string length_string = length_strings[ m_idx ];
			text_renderer->render( length_string, &buffer[ 0 ], 
				viewer_width, viewer_height, static_cast< int >( length_point.x() ), 
				viewer_height - static_cast< int >( length_point.y() ), font_size, 0 );
		}
	}

	text_texture->enable();
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	text_texture->set_image( viewer_width, viewer_height,
		GL_ALPHA, &buffer[ 0 ], GL_ALPHA, GL_UNSIGNED_BYTE );

	render_lock.unlock();

	//-------------- RenderResources unlocked  -------------------

	// Blend the text onto the framebuffer
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD );

	// Render drop-shadow by shifting vertices by 1 pixel
	glBegin( GL_QUADS );
	glColor4f( 0.0f, 0.0f, 0.0f, static_cast< float >( opacity ) ); // Black 
	glTexCoord2f( 0.0f, 0.0f );
	glVertex2i( 1, -1 );
	glTexCoord2f( 1.0f, 0.0f );
	glVertex2i( viewer_width, -1 );
	glTexCoord2f( 1.0f, 1.0f );
	glVertex2i( viewer_width, viewer_height - 2 );
	glTexCoord2f( 0.0f, 1.0f );
	glVertex2i( 1, viewer_height - 2 );
	glEnd();

	// Render foreground text
	glBegin( GL_QUADS );
	Core::Color label_color = Core::Measurement::DEFAULT_COLOR_C;
	glColor4f( label_color.r(), label_color.g(), label_color.b(), static_cast< float >( opacity ) );
	glTexCoord2f( 0.0f, 0.0f );
	glVertex2i( 0, 0 );
	glTexCoord2f( 1.0f, 0.0f );
	glVertex2i( viewer_width - 1, 0 );
	glTexCoord2f( 1.0f, 1.0f );
	glVertex2i( viewer_width - 1, viewer_height - 1 );
	glTexCoord2f( 0.0f, 1.0f );
	glVertex2i( 0, viewer_height - 1 );
	glEnd();

	text_texture->disable();
	
	CORE_CHECK_OPENGL_ERROR();

	glPopAttrib();
	glFinish();
}

bool MeasurementTool::has_2d_visual()
{
	return true;
}

void MeasurementTool::activate()
{
	// Add all connections
	this->add_connection( this->measurements_state_->state_changed_signal_.connect(
		boost::bind( &MeasurementToolPrivate::handle_measurements_changed, this->private_ ) ) );
	this->add_connection( this->active_index_state_->state_changed_signal_.connect(
		boost::bind( &MeasurementToolPrivate::update_viewers, this->private_ ) ) );
	this->add_connection( this->units_selection_state_->value_changed_signal_.connect(
		boost::bind( &MeasurementToolPrivate::handle_units_selection_changed, this->private_, _2 ) ) );
	this->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect(
		boost::bind( &MeasurementToolPrivate::handle_active_layer_changed, this->private_, _1 ) ) );
	this->add_connection( this->opacity_state_->state_changed_signal_.connect(
		boost::bind( &MeasurementToolPrivate::handle_opacity_changed, this->private_ ) ) );
	this->add_connection( ViewerManager::Instance()->active_viewer_state_->value_changed_signal_.
		connect( boost::bind( &MeasurementToolPrivate::handle_active_viewer_changed, 
		this->private_, _1 ) ) );

	size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
	for ( size_t i = 0; i < num_of_viewers; ++i )
	{
		ViewerHandle viewer = ViewerManager::Instance()->get_viewer( i );
		this->add_connection( viewer->slice_number_state_->value_changed_signal_.connect(
			boost::bind( &MeasurementToolPrivate::handle_viewer_slice_changed,
			this->private_, i, _1 ) ) );
	}
}

void MeasurementTool::deactivate()
{
	// Disconnect all connections because we don't want handlers to run when tool isn't active due
	// to performance concerns.
	this->disconnect_all();
}

double MeasurementTool::convert_unit_string_to_world( std::string unit_string )
{
	double unit_value;
	Core::ImportFromString( unit_string, unit_value );
	return this->private_->convert_unit_to_world( unit_value );
}

std::string MeasurementTool::convert_world_to_unit_string( double world_value )
{
	// May be called from interface or render threads

	// NOTE: Do not call this function if RendererResources is locked.
	// We need access to the show_world_units_state_, and several functions we call also lock
	// the state engine.  
	Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

	double unit_value = this->private_->convert_world_to_unit( world_value );

	bool use_scientific = false;

	if( !this->show_world_units_state_->get() ) 
	{
		// Index units

		// Use same formatting policy as status bar for coordinates
		if( 10000 < unit_value ) 
		{
			use_scientific = true;
		}
		else 
		{
			use_scientific = false;
		}
	}
	else
	{
		// World units

		// Use same formatting policy as status bar for coordinates
		if( ( 0.0 < unit_value && unit_value < 0.0001 ) || 1000 < unit_value ) 
		{
			use_scientific = true;
		}
		else 
		{
			use_scientific = false;
		}
	}

	// Use same formatting policy as status bar for coordinates
	std::ostringstream oss;
	if( use_scientific ) 
	{
		// Use scientific notation
		oss.precision( 2 );
		oss << std::scientific << unit_value;
		return ( oss.str() );
	}
	else 
	{
		// Format normally
		oss.precision( 3 );
		oss << std::fixed << unit_value;
		return ( oss.str() );
	}
}

} // end namespace Seg3D


