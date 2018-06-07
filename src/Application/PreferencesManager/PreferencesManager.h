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

#ifndef APPLICATION_PREFERENCESMANAGER_PREFERENCESMANAGER_H
#define APPLICATION_PREFERENCESMANAGER_PREFERENCESMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/Utils/Singleton.h>
#include <Core/State/StateHandler.h>

namespace Seg3D
{

// Forward declaration
class PreferencesManager;
class PreferencesManagerPrivate;
typedef boost::shared_ptr< PreferencesManagerPrivate > PreferencesManagerPrivateHandle;

// Class definition
class PreferencesManager : public Core::StateHandler
{
  CORE_SINGLETON( PreferencesManager );

  // -- Constructor/Destructor --
private:
  PreferencesManager();
  virtual ~PreferencesManager();

//public:
//  typedef boost::signals2::signal< void() > enable_large_volume_signal_type;
//  enable_large_volume_signal_type enable_large_volume_signal_;

  // -- State variables --
public:
  //General Preferences
  Core::StateStringHandle project_path_state_;
  Core::StateStringHandle export_path_state_;
  Core::StateBoolHandle full_screen_on_startup_state_;
  Core::StateBoolHandle auto_save_state_;
  Core::StateRangedIntHandle auto_save_time_state_;
  Core::StateBoolHandle smart_save_state_;
  Core::StateBoolHandle advanced_visibility_settings_state_;
  Core::StateBoolHandle paint_cursor_invisibility_state_;
  Core::StateBoolHandle compression_state_;
  Core::StateRangedIntHandle compression_level_state_;
  Core::StateIntHandle slice_step_multiplier_state_;
  Core::StateLabeledOptionHandle axis_labels_option_state_;
  Core::StateStringHandle x_axis_label_state_;
  Core::StateStringHandle y_axis_label_state_;
  Core::StateStringHandle z_axis_label_state_;

  Core::StateBoolHandle enable_undo_state_;
  Core::StateRangedDoubleHandle percent_of_memory_state_;
  Core::StateBoolHandle embed_input_files_state_;
  Core::StateBoolHandle generate_osx_project_bundle_state_;

  Core::StateBoolHandle export_dicom_headers_state_;
  Core::StateBoolHandle export_nrrd0005_state_;
  
  //Viewers Preferences
  Core::StateOptionHandle default_viewer_mode_state_;
  Core::StateRangedIntHandle grid_size_state_;
  Core::StateLabeledOptionHandle background_color_state_;
  Core::StateBoolHandle show_slice_number_state_;
  Core::StateBoolHandle zero_based_slice_numbers_state_;
  Core::StateBoolHandle active_layer_navigation_state_;
  Core::StateBoolHandle status_sci_notation_state_;

  //Layers Preferences
  Core::StateRangedDoubleHandle default_layer_opacity_state_;
  Core::StateOptionHandle default_mask_fill_state_;
  Core::StateOptionHandle default_mask_border_state_;
  std::vector< Core::StateColorHandle > color_states_;
  Core::StateOptionHandle default_colormap_state_;

  //Interface Controls Preferences
  Core::StateBoolHandle reverse_slice_navigation_state_;
      
  //Sidebars Preferences
  Core::StateBoolHandle show_tools_bar_state_;
  Core::StateBoolHandle show_layermanager_bar_state_;
  Core::StateBoolHandle show_projectmanager_bar_state_;
  Core::StateBoolHandle show_provenance_bar_state_;
  Core::StateBoolHandle show_rendering_bar_state_;

  // Large volume preferences
  Core::StateBoolHandle enable_large_volume_state_;
  
public:
  /// GET_DEFAULT_COLORS:
  /// This function returns a vector of the default colors
  const std::vector< Core::Color >& get_default_colors() const;

  /// GET_COLOR:
  /// Get the color at the index.
  Core::Color get_color( int index ) const;
  Core::Color get_color( const Core::Color& color ) const;

  /// GET_BACKGROUND_COLOR:
  /// Get the currently selected background color for the viewers.
  Core::Color get_background_color() const;

  /// SAVE_STATE:
  /// Save the preferences to file.
  void save_state();
  
private:
  /// INITIALIZE:
  /// this function loads the values for PreferencesManager from file
  void initialize();

  /// INITIALIZE_STATES:
  /// This function is called by the constructor to initialize the state variables
  void initialize_states();
  
  /// INITIALIZE_DEFAULT_COLORS:
  /// This function is called by the constructor to initialize a vector of default color values
  bool initialize_default_colors();
  
private:
  
  PreferencesManagerPrivateHandle private_;
};

} // end namespace seg3D

#endif

