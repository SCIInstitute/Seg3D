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

#ifndef APPLICATION_TOOLS_DETAIL_MEASUREMENT_H
#define APPLICATION_TOOLS_DETAIL_MEASUREMENT_H

// System includes
#include <string>
#include <vector>

// Boost includes
#include <boost/thread.hpp>
#include <boost/utility.hpp> // Needed for noncopyable

// Core includes
#include <Core/Geometry/Point.h>
#include <Core/Utils/Singleton.h>

namespace Seg3D
{

enum ViewAxis {
  AXIAL_E,
  CORONAL_E,
  SAGITTAL_E,
  VOLUME_E,
  MIP_E,
  OBLIQUE_E,
  NOVIEW_E
};

//=============================================================================
// Class: Measurement
//=============================================================================

// This class may need to be modified when we complete the state design across layers.  For now
// this class stores test data for display in the Measurements tool panel.
class Measurement 
{
public:

  Measurement( bool visible, std::string label, std::string note, Core::Point p1, Core::Point p2, 
    ViewAxis view_axis, int slice_num, int slice_thickness, std::string oblique_name = "" ) : 
    visible_( visible ), 
    label_( label ), 
    note_( note ), 
    p1_( p1 ), 
    p2_( p2 ), 
    view_axis_( view_axis ),
    slice_num_( slice_num ), 
    slice_thickness_( slice_thickness ),
    oblique_name_( oblique_name )
  {}
  Measurement() :
    visible_( false ), 
    label_( "" ), 
    note_( "" ), 
    view_axis_( NOVIEW_E ),
    slice_num_( 0 ), 
    slice_thickness_( 1 ),
    oblique_name_( "" ) {}

  // DESIGN NOTE: Originally I made a virtual destructor out of habit.  But since this is not
  // designed to be a base class, should not make destructor virtual in order to avoid cost of
  // vtable.  Because this class contains no dynamically allocated memory, can use default 
  // destructor.

  // DESIGN NOTE: Using const for member functions that don't modify member data
  bool      get_visible() const { return visible_; }
  void      set_visible( bool visible ) { visible_ = visible; } // User-editable
  std::string   get_label() const { return label_; } 
  void      set_label( std::string label ) { label_ = label; }
  double      get_length() const { return (p2_ - p1_).length(); }
  std::string   get_note() const { return note_; }
  void      set_note( std::string note ) { note_ = note; } // User-editable
  Core::Point   get_point1() const { return p1_; }
  void      set_point1( Core::Point p1 ) { p1_ = p1; }
  Core::Point   get_point2() const { return p2_; }
  void      set_point2( Core::Point p2 ) { p2_ = p2; } // P2 moves during measurement creation
  int       get_slice_num() const { return slice_num_; } 
  void      set_slice_num( int slice_num ) { slice_num_ = slice_num; } 
  ViewAxis    get_view_axis() const { return view_axis_; }
  void      set_view_axis( ViewAxis view_axis ) { view_axis_ = view_axis; }
  std::string   get_view_name() const; 
  void      set_oblique_name( std::string oblique_name ) { oblique_name_ = oblique_name; }
  int       get_slice_thickness() const { return slice_thickness_; } 
  void      set_slice_thickness( int slice_thickness ) { slice_thickness_ = slice_thickness; }

private:
  bool      visible_;
  std::string   label_; // Unique ID
  std::string   note_;
  Core::Point   p1_; // 3D world coordinate of 1st point
  Core::Point   p2_; // 3D world coordinate of 2nd point
  ViewAxis    view_axis_;
  int       slice_num_;
  int       slice_thickness_;
  std::string   oblique_name_; // In case ViewAxis = OBLIQUE
};

//=============================================================================
// Class: MeasurementList
//=============================================================================

class MeasurementList : public boost::noncopyable
{
  CORE_SINGLETON( MeasurementList );

protected:
  MeasurementList();

  // DESIGN NOTE: Originally I made a virtual destructor out of habit.  But since this is not
  // designed to be a base class, should not make destructor virtual in order to avoid cost of
  // vtable.  Because this class contains no dynamically allocated memory, don't need destructor.

public:
  // DESIGN NOTE: Need const and non-const versions so that const code can use this.
  // DESIGN NOTE: Debated about returning a reference or a copy.  Reference requires less memory,
  // but requires that the member data persists.  Using a reference because in this case the 
  // member data does persist and I often use this operator without wanting/needing a copy.  
  // STL vector also returns reference in this case.
  Measurement& operator[]( int idx );
  const Measurement& operator[]( int idx ) const;

  void      clear();
  // DESIGN NOTE: size_t should be used when representing virtual address space.
  size_t      size() const;
  // DESIGN NOTE: Use const ref to avoid overhead of creating a copy
  void      push_back( const Measurement& measurement );
  // Erase range of elements [start_index, end_index)
  bool      erase( size_t start_index, size_t end_index );
  int       get_active_index() const;
  void      set_active_index( int index );
  bool      get_highlight_active_measurement();
  void      set_highlight_active_measurement( bool highlight );
  void      get_standard_view_measurements( 
            std::vector< int >& standard_view_measurements ) const;

  typedef boost::recursive_mutex mutex_type;
  typedef boost::unique_lock< mutex_type > lock_type;

private:
  void      update_active_index();
  mutex_type&   get_mutex() const;

  // DESIGN NOTE: Originally used size_t since that represents addressable memory, but
  // needed to use -1 to signify an invalid value.  Don't imagine ever having more than 2^31
  // measurements.
  int               active_index_; // Index of active measurement
  bool              highlight_active_measurement_;
  std::vector< Measurement >    measurements_; // List of measurements
  // Has to be mutable so that member functions can be const
  mutable mutex_type        mutex_;

public:
  static const int INVALID_ACTIVE_INDEX_C;
};

} // end namespace Seg3D

#endif 
