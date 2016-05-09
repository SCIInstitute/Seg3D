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

#include <Core/VolumeRenderer/TransferFunctionFeature.h>

namespace Core
{

//////////////////////////////////////////////////////////////////////////
// Class TransferFunctionFeaturePrivate
//////////////////////////////////////////////////////////////////////////

class TransferFunctionFeaturePrivate
{
public:
  TransferFunctionControlPointVector control_points_;
  Color diffuse_color_;
  float ambient_coefficient_;
  float specular_intensity_;
  int shininess_;
  bool enabled_;
  bool solid_;
};

//////////////////////////////////////////////////////////////////////////
// Class TransferFunctionFeature
//////////////////////////////////////////////////////////////////////////

TransferFunctionFeature::TransferFunctionFeature() :
  StateHandler( "tffeature", true ),
  private_( new TransferFunctionFeaturePrivate )
{
  this->initialize_states();
}

TransferFunctionFeature::TransferFunctionFeature( const std::string& feature_id ) :
  StateHandler( feature_id, true ),
  private_( new TransferFunctionFeaturePrivate )
{
  this->initialize_states();
}

TransferFunctionFeature::~TransferFunctionFeature()
{

}

const std::string& TransferFunctionFeature::get_feature_id() const
{
  return this->get_statehandler_id();
}

void TransferFunctionFeature::initialize_states()
{
  TransferFunctionControlPointVector control_points;
  control_points.push_back( TransferFunctionControlPoint( 0.0f, 0.0f ) );
  control_points.push_back( TransferFunctionControlPoint( 1.0f, 1.0f ) );
  this->add_state( "control_points", this->control_points_state_, control_points );
  this->add_state( "ambient_coefficient", this->ambient_coefficient_state_, 1.0, 0.0, 1.0, 0.1 );
  this->add_state( "diffuse_red", this->diffuse_color_red_state_, 128, 0, 255, 1 );
  this->add_state( "diffuse_green", this->diffuse_color_green_state_, 128, 0, 255, 1 );
  this->add_state( "diffuse_blue", this->diffuse_color_blue_state_, 128, 0, 255, 1 );
  this->add_state( "specular_intensity", this->specular_intensity_state_, 0.0, 0.0, 1.0, 0.1 );
  this->add_state( "shininess", this->shininess_state_, 128, 0, 128, 1 );
  this->add_state( "enabled", this->enabled_state_, true );
  this->add_state( "solid", this->solid_state_, false );
}

void TransferFunctionFeature::take_snapshot()
{
  this->private_->control_points_ = this->control_points_state_->get();
  std::sort( this->private_->control_points_.begin(), this->private_->control_points_.end() );
  this->private_->ambient_coefficient_ = static_cast< float >( this->ambient_coefficient_state_->get() );
  this->private_->diffuse_color_ = Color( this->diffuse_color_red_state_->get() / 255.0f,
    this->diffuse_color_green_state_->get() / 255.0f, 
    this->diffuse_color_blue_state_->get() / 255.0f );
  this->private_->specular_intensity_ = static_cast< float >( this->specular_intensity_state_->get() );
  this->private_->shininess_ = this->shininess_state_->get();
  this->private_->enabled_ = this->enabled_state_->get();
  this->private_->solid_ = this->solid_state_->get();
}

float TransferFunctionFeature::interpolate( float value )
{
  const TransferFunctionControlPointVector& points = this->private_->control_points_;
  const size_t num_pts = points.size();
  if ( num_pts <= 1 ||
    points.front().get_value() > value || 
    points.back().get_value() < value )
  {
    return 0;
  }

  if ( this->private_->solid_ )
  {
    return 1;
  }

  for ( size_t i = 0; i < num_pts - 1; ++i )
  {
    if ( points[ i ].get_value() <= value && points[ i + 1 ].get_value() >= value )
    {
      float interval = points[ i + 1 ].get_value() - points[ i ].get_value();
      if ( interval < 1e-4 )
      {
        return ( points[ i ].get_opacity() + points[ i + 1 ].get_opacity() ) * 0.5f;
      }
      else
      {
        float t = ( value - points[ i ].get_value() ) / interval;
        return points[ i ].get_opacity() * ( 1.0f - t ) + points[ i + 1 ].get_opacity() * t;
      }
    }
  }

  assert( false );
  return 0;
}

const Color& TransferFunctionFeature::get_diffuse_color()
{
  return this->private_->diffuse_color_;
}

int TransferFunctionFeature::get_shininess()
{
  return this->private_->shininess_;
}

bool TransferFunctionFeature::is_enabled()
{
  return this->private_->enabled_;
}

float TransferFunctionFeature::get_ambient_coefficient()
{
  return this->private_->ambient_coefficient_;
}

float TransferFunctionFeature::get_specular_intensity()
{
  return this->private_->specular_intensity_;
}

} // end namespace Core
