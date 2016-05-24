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

#include <Application/Tools/detail/Measurement.h>

namespace Seg3D
{

const int MeasurementList::INVALID_ACTIVE_INDEX_C = -1;

CORE_SINGLETON_IMPLEMENTATION( MeasurementList );

// Temporary function
void create_test_data()
{
  // Populate measurements list with test data
  MeasurementList::Instance()->push_back( 
    Measurement( true, "M1", "Knee", Core::Point(0, 0, 0), 
    Core::Point(1, 1, 1), AXIAL_E, 50, 1 ) );
  MeasurementList::Instance()->push_back( 
    Measurement( true, "M2", "Heart", Core::Point(0, 0, 0), 
    Core::Point(2, 2, 2), AXIAL_E, 50, 1 ) );
  MeasurementList::Instance()->push_back( 
    Measurement( true, "M3", "Head", Core::Point(0, 0, 0), 
    Core::Point(3, 3, 3), AXIAL_E, 50, 1 ) ); 
  MeasurementList::Instance()->push_back( 
    Measurement( true, "M4", "Toe", Core::Point(0, 0, 0), 
    Core::Point(4, 4, 4), AXIAL_E, 50, 1 ) ); 
  MeasurementList::Instance()->push_back( 
    Measurement( true, "M5", "Eye", Core::Point(0, 0, 0), 
    Core::Point(5, 5, 5), AXIAL_E, 50, 1 ) ); 
  MeasurementList::Instance()->push_back( 
    Measurement( true, "M6", "Nose", Core::Point(0, 0, 0), 
    Core::Point(6, 6, 6), AXIAL_E, 50, 1 ) ); 
  MeasurementList::Instance()->push_back( 
    Measurement( true, "M7", "Hand", Core::Point(0, 0, 0), 
    Core::Point(7, 7, 7), AXIAL_E, 50, 1 ) ); 
  MeasurementList::Instance()->push_back( 
    Measurement( true, "M8", "Ear", Core::Point(0, 0, 0), 
    Core::Point(8, 8, 8), AXIAL_E, 50, 1 ) ); 
}

// DESIGN NOTE: Originally set this to 0, but later realized needed -1 and that
// constant would be best was to indicate invalid value.
MeasurementList::MeasurementList() :
  active_index_( INVALID_ACTIVE_INDEX_C ),
  highlight_active_measurement_( true ) {}

Measurement& MeasurementList::operator[]( int idx )
{
  lock_type lock( this->get_mutex() );
  assert( ( idx >= 0 ) && ( idx < static_cast< int >( this->size() ) ) );
  return measurements_[ idx ];
}

const Measurement& MeasurementList::operator[]( int idx ) const
{
  lock_type lock( this->get_mutex() );
  assert( ( idx >= 0 ) && ( idx < static_cast< int >( this->size() ) ) );
  return measurements_[ idx ];
}

size_t MeasurementList::size() const
{
  lock_type lock( this->get_mutex() );
  return measurements_.size();
}

void MeasurementList::push_back( const Measurement& measurement )
{
  lock_type lock( this->get_mutex() );
  measurements_.push_back( measurement );
  update_active_index();
}

int MeasurementList::get_active_index() const
{
  lock_type lock( this->get_mutex() );
  return this->active_index_;
}

void MeasurementList::set_active_index( int index )
{
  lock_type lock( this->get_mutex() );
  this->active_index_ = index;
}

void MeasurementList::update_active_index()
{
  lock_type lock( this->get_mutex() );
  if( measurements_.size() > 0 )
  {
    this->active_index_ = static_cast< int >( measurements_.size() ) - 1;
  }
  else
  {
    this->active_index_ = INVALID_ACTIVE_INDEX_C;
  }
}

bool MeasurementList::erase( size_t start_index, size_t end_index )
{
  lock_type lock( this->get_mutex() );
  // Indices can't be negative (size_t is unsigned), so only need to check
  // upper bound.
  if( ( start_index >= this->size() || end_index > this->size() )
    || end_index <= start_index ) 
  {
    return false;
  }

  measurements_.erase( measurements_.begin() + start_index, 
    measurements_.begin() + end_index );

  update_active_index();

  return true;
}

void MeasurementList::get_standard_view_measurements( std::vector< int >& standard_view_measurements ) const
{
  lock_type lock( this->get_mutex() );
  for( size_t i = 0; i < measurements_.size(); i++ )
  {
    ViewAxis view_axis = measurements_[i].get_view_axis();
    if( view_axis == AXIAL_E || view_axis == CORONAL_E || view_axis == SAGITTAL_E )
    {
      standard_view_measurements.push_back( static_cast<int>(i) );
    }
  }
}

MeasurementList::mutex_type& MeasurementList::get_mutex() const
{
  return this->mutex_;
}

void MeasurementList::clear()
{
  lock_type lock( this->get_mutex() );
  this->measurements_.clear();
  active_index_ = INVALID_ACTIVE_INDEX_C;

  // Test code
  create_test_data();
}

bool MeasurementList::get_highlight_active_measurement()
{
  lock_type lock( this->get_mutex() );
  return this->highlight_active_measurement_;
}

void MeasurementList::set_highlight_active_measurement( bool highlight )
{
  lock_type lock( this->get_mutex() );
  this->highlight_active_measurement_ = highlight;
}

std::string Measurement::get_view_name() const
{
  std::string view_axis_str = "";
  if( this->view_axis_ == AXIAL_E ) view_axis_str = "AXIAL";
  else if( this->view_axis_ == CORONAL_E ) view_axis_str = "CORONAL";
  else if( this->view_axis_ == SAGITTAL_E ) view_axis_str = "SAGITTAL";
  else if( this->view_axis_ == VOLUME_E ) view_axis_str = "VOLUME";
  else if( this->view_axis_ == MIP_E ) view_axis_str = "MIP";
  else if( this->view_axis_ == OBLIQUE_E ) view_axis_str = this->oblique_name_;
  return view_axis_str;
}

} // end namespace Seg3D
