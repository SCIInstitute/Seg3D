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
  Color color_;
  int shininess_;
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
  control_points.push_back( TransferFunctionControlPoint( 0.0f, 0.5f ) );
  control_points.push_back( TransferFunctionControlPoint( 1.0f, 0.5f ) );
  this->add_state( "control_points", this->control_points_state_, control_points );
  this->add_state( "red", this->red_color_state_, 128, 0, 255, 1 );
  this->add_state( "green", this->green_color_state_, 128, 0, 255, 1 );
  this->add_state( "blue", this->blue_color_state_, 128, 0, 255, 1 );
  this->add_state( "shininess", this->shininess_state_, 0, 0, 128, 1 );
}

void TransferFunctionFeature::take_snapshot()
{
  this->private_->control_points_ = this->control_points_state_->get();
  std::sort( this->private_->control_points_.begin(), this->private_->control_points_.end() );
  this->private_->color_ = Color( this->red_color_state_->get() / 255.0f,
    this->green_color_state_->get() / 255.0f, this->blue_color_state_->get() / 255.0f );
  this->private_->shininess_ = this->shininess_state_->get();
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

const Color& TransferFunctionFeature::get_color()
{
  return this->private_->color_;
}

int TransferFunctionFeature::get_shininess()
{
  return this->private_->shininess_;
}

} // end namespace Core