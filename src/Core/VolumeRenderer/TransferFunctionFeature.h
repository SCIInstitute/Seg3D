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

#ifndef CORE_VOLUMERENDERER_TRANSFERFUNCTIONFEATURE_H
#define CORE_VOLUMERENDERER_TRANSFERFUNCTIONFEATURE_H

#include <Core/VolumeRenderer/TransferFunctionControlPoint.h>
#include <Core/State/StateHandler.h>
#include <Core/State/StateRangedValue.h>

namespace Core
{

class TransferFunctionFeature;
typedef boost::shared_ptr< TransferFunctionFeature > TransferFunctionFeatureHandle;

class TransferFunctionFeaturePrivate;
typedef boost::shared_ptr< TransferFunctionFeaturePrivate > TransferFunctionFeaturePrivateHandle;

class TransferFunctionFeature : public Core::StateHandler
{
public:
  TransferFunctionFeature();
  TransferFunctionFeature( const std::string& feature_id );
  ~TransferFunctionFeature();

  const std::string& get_feature_id() const;

  /// Functions used by TransferFunction class to build the lookup table
private:
  friend class TransferFunction;
  friend class TransferFunctionPrivate;

  // TAKE_SNAPSHOT:
  /// Get a snapshot of all the states.
  void take_snapshot();

  // INTERPOLATE:
  /// Get the opacity corresponding to the given value by interpolating between
  /// control points. If value is outside the range defined by controls points, 
  /// 0 is returned.
  float interpolate( float value );

  // GET_AMBIENT_COEFFICIENT:
  /// Returns the coefficient by which the material color will be multiplied to compute
  /// the ambient light component. The value is in range [0, 1].
  float get_ambient_coefficient();

  // GET_DIFFUSE_COLOR:
  /// Returns the diffuse color of the feature. The values are normalized to [0, 1].
  const Color& get_diffuse_color();

  // GET_SPECULAR_INTENSITY:
  /// Returns the intensity of the specular light. The value is in range [0, 1].
  float get_specular_intensity();

  // GET_SHININESS:
  /// Returns the shininess of the feature.
  int get_shininess();

  // IS_ENABLED:
  /// Returns true if the feature is enabled, otherwise false.
  bool is_enabled();

private:
  void initialize_states();

public:
  StateTransferFunctionControlPointVectorHandle control_points_state_;
  StateRangedDoubleHandle ambient_coefficient_state_;
  StateRangedIntHandle diffuse_color_red_state_;
  StateRangedIntHandle diffuse_color_green_state_;
  StateRangedIntHandle diffuse_color_blue_state_;
  StateRangedDoubleHandle specular_intensity_state_;
  StateRangedIntHandle shininess_state_;
  StateBoolHandle enabled_state_;
  StateBoolHandle solid_state_;

private:
  TransferFunctionFeaturePrivateHandle private_;
};

} // end namespace Core

#endif
