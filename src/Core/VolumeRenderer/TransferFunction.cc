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

#include <boost/foreach.hpp>

#include <tinyxml.h>

#include <Core/State/StateIO.h>
#include <Core/Graphics/PixelBufferObject.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/VolumeRenderer/TransferFunction.h>

namespace Core
{

typedef std::map< std::string, TransferFunctionFeatureHandle > tf_feature_map_type;

class TransferFunctionPrivate
{
public:
  void handle_tf_state_changed();
  void build_lookup_texture();

  tf_feature_map_type tf_feature_map_;
  bool dirty_;
  Texture1DHandle diffuse_lut_;
  Texture1DHandle specular_lut_;
  TransferFunction* tf_;
};

void TransferFunctionPrivate::handle_tf_state_changed()
{
  {
    StateEngine::lock_type lock( StateEngine::GetMutex() );
    this->dirty_ = true;
  }
  this->tf_->transfer_function_changed_signal_();
}

void TransferFunctionPrivate::build_lookup_texture()
{
  static const int LUT_SIZE_C = 256;
  static const int SECONDARY_OFFSET_C = LUT_SIZE_C * 4;
  static const Color BLACK_COLOR_C( 0.0f, 0.0f, 0.0f );
  
  BOOST_FOREACH( tf_feature_map_type::value_type feature_entry, this->tf_feature_map_ )
  {
    feature_entry.second->take_snapshot();
  }
  bool use_faux_shading = this->tf_->faux_shading_state_->get();

  RenderResources::lock_type lock( RenderResources::GetMutex() );
  PixelBufferObjectHandle pbo( new PixelUnpackBuffer );
  pbo->bind();
  pbo->set_buffer_data( LUT_SIZE_C * 4 * sizeof( unsigned char ) * 2,
    NULL, GL_STREAM_DRAW );
  unsigned char* buffer = reinterpret_cast< unsigned char* >(
    pbo->map_buffer( GL_WRITE_ONLY ) );

  for ( int i = 0; i < LUT_SIZE_C; ++i )
  {
    // NOTE: Texels are cell centered.
    float s = ( i + 0.5f ) / LUT_SIZE_C;
    float total_alpha = 0.0f;
    Color diffuse_color( 0.0f, 0.0f, 0.0f );
    float ambient_coefficient = 0;
    float specular_intensity = 0;
    float shininess = 0;
    int total_blended_features = 0;

    BOOST_FOREACH( tf_feature_map_type::value_type feature_entry, this->tf_feature_map_ )
    {
      if ( feature_entry.second->is_enabled() )
      {
        float alpha = feature_entry.second->interpolate( s );
        if ( alpha > 0 )
        {
          ++total_blended_features;
          total_alpha += alpha;
          if ( use_faux_shading )
          {
            Color faux_color = feature_entry.second->get_diffuse_color() * alpha + 
              BLACK_COLOR_C * ( 1.0f - alpha );
            diffuse_color += faux_color * alpha;
          }
          else
          {
            diffuse_color += feature_entry.second->get_diffuse_color() * alpha;
          }
          ambient_coefficient += feature_entry.second->get_ambient_coefficient() * alpha;
          specular_intensity += feature_entry.second->get_specular_intensity() * alpha;
          shininess += feature_entry.second->get_shininess() * alpha;
        }
      }
    }

    if ( total_blended_features > 0 )
    {
      // The final color is the weighted average of all the features that
      // are defined at the texel.
      diffuse_color = diffuse_color * ( 1.0f / total_alpha );
      ambient_coefficient /= total_alpha;
      specular_intensity /= total_alpha;
      shininess /= total_alpha;
      // The final alpha is the average of all the features that are defined
      // at the texel.
      total_alpha /= total_blended_features;
    }

    buffer[ i << 2 ] = static_cast< unsigned char >( Clamp( diffuse_color.r(), 0.0f, 1.0f ) * 255 );
    buffer[ ( i << 2 ) + 1 ] = static_cast< unsigned char >( Clamp( diffuse_color.g(), 0.0f, 1.0f ) * 255 );
    buffer[ ( i << 2 ) + 2 ] = static_cast< unsigned char >( Clamp( diffuse_color.b(), 0.0f, 1.0f ) * 255 );
    buffer[ ( i << 2 ) + 3 ] = static_cast< unsigned char >( Clamp( total_alpha, 0.0f, 1.0f ) * 255 );
    buffer[ SECONDARY_OFFSET_C + ( i << 2 ) ] = static_cast< unsigned char >( 
      Clamp( ambient_coefficient, 0.0f, 1.0f ) * 255 );
    buffer[ SECONDARY_OFFSET_C + ( i << 2 ) + 1 ] = static_cast< unsigned char >( 
      Clamp( specular_intensity, 0.0f, 1.0f ) * 255 );
    buffer[ SECONDARY_OFFSET_C + ( i << 2 ) + 2 ] = static_cast< unsigned char >( shininess );
    buffer[ SECONDARY_OFFSET_C + ( i << 2 ) + 3 ] = 0;
  }

  pbo->unmap_buffer();

  if ( !this->diffuse_lut_ )
  {
    this->diffuse_lut_.reset( new Texture1D );
    this->diffuse_lut_->bind();
    this->diffuse_lut_->set_mag_filter( GL_LINEAR );
    this->diffuse_lut_->set_min_filter( GL_LINEAR );
    this->diffuse_lut_->set_wrap_s( GL_CLAMP );
  }

  if ( !this->specular_lut_ )
  {
    this->specular_lut_.reset( new Texture1D );
    this->specular_lut_->bind();
    this->specular_lut_->set_mag_filter( GL_LINEAR );
    this->specular_lut_->set_min_filter( GL_LINEAR );
    this->specular_lut_->set_wrap_s( GL_CLAMP );
  }

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  {
    Texture::lock_type tex_lock( this->diffuse_lut_->get_mutex() );
    this->diffuse_lut_->bind();
    this->diffuse_lut_->set_image( LUT_SIZE_C, GL_RGBA, 0, GL_RGBA, GL_UNSIGNED_BYTE );
    this->diffuse_lut_->unbind();
  }

  {
    Texture::lock_type tex_lock( this->specular_lut_->get_mutex() );
    this->specular_lut_->bind();
    this->specular_lut_->set_image( LUT_SIZE_C, GL_RGBA, 
      reinterpret_cast< const void* >( SECONDARY_OFFSET_C ), 
      GL_RGBA, GL_UNSIGNED_BYTE );
    this->specular_lut_->unbind();
  }

  pbo->unbind();

  glFinish();
}

TransferFunction::TransferFunction() :
  StateHandler( "tf", true ),
  private_( new TransferFunctionPrivate )
{
  this->private_->dirty_ = true;
  this->private_->tf_ = this;

  this->add_state( "faux_shading", this->faux_shading_state_, true );
  this->faux_shading_state_->set_session_priority( StateBase::DO_NOT_LOAD_E );
  this->add_connection( this->faux_shading_state_->state_changed_signal_.connect(
    boost::bind( &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) ) );

  this->add_connection( Application::Instance()->reset_signal_.connect(
    boost::bind( &TransferFunction::clear, this ) ) );
}

TransferFunction::~TransferFunction()
{
  this->disconnect_all();
}

TextureHandle TransferFunction::get_diffuse_lut() const
{
  StateEngine::lock_type lock( StateEngine::GetMutex() );
  if ( this->private_->dirty_ )
  {
    this->private_->build_lookup_texture();
    this->private_->dirty_ = false;
  }
  return this->private_->diffuse_lut_;
}

TextureHandle TransferFunction::get_specular_lut() const
{
  StateEngine::lock_type lock( StateEngine::GetMutex() );
  if ( this->private_->dirty_ )
  {
    this->private_->build_lookup_texture();
    this->private_->dirty_ = false;
  }
  return this->private_->specular_lut_;
}

Core::TransferFunctionFeatureHandle TransferFunction::create_feature()
{
  StateEngine::lock_type lock( StateEngine::GetMutex() );

  TransferFunctionFeatureHandle feature( new TransferFunctionFeature );
  this->private_->tf_feature_map_[ feature->get_feature_id() ] = feature;

  feature->control_points_state_->state_changed_signal_.connect( boost::bind(
    &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
  feature->diffuse_color_red_state_->state_changed_signal_.connect( boost::bind(
    &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
  feature->diffuse_color_green_state_->state_changed_signal_.connect( boost::bind(
    &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
  feature->diffuse_color_blue_state_->state_changed_signal_.connect( boost::bind(
    &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
  feature->ambient_coefficient_state_->state_changed_signal_.connect( boost::bind(
    &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
  feature->specular_intensity_state_->state_changed_signal_.connect( boost::bind(
    &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
  feature->shininess_state_->state_changed_signal_.connect( boost::bind(
    &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
  feature->enabled_state_->state_changed_signal_.connect( boost::bind(
    &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
  feature->solid_state_->state_changed_signal_.connect( boost::bind(
    &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );

  this->private_->dirty_ = true;
  this->feature_added_signal_( feature );
  this->transfer_function_changed_signal_();
  return feature;
}

void TransferFunction::delete_feature( const std::string& feature_id )
{
  StateEngine::lock_type lock( StateEngine::GetMutex() );

  tf_feature_map_type::iterator it = this->private_->tf_feature_map_.find( feature_id );
  if ( it != this->private_->tf_feature_map_.end() )
  {
    TransferFunctionFeatureHandle feature = ( *it ).second;
    this->private_->tf_feature_map_.erase( it );
    feature->invalidate();
    this->private_->dirty_ = true;
    this->feature_deleted_signal_( feature );
    this->transfer_function_changed_signal_();
  }
}

bool TransferFunction::post_save_states( StateIO& state_io )
{
  TiXmlElement* tf_element = state_io.get_current_element();
  assert( this->get_statehandler_id() == tf_element->Value() );

  TiXmlElement* features_element = new TiXmlElement( "features" );
  tf_element->LinkEndChild( features_element );

  state_io.push_current_element();
  state_io.set_current_element( features_element );

  BOOST_FOREACH( tf_feature_map_type::value_type feature_entry, this->private_->tf_feature_map_ )
  {
    feature_entry.second->save_states( state_io );
  }

  state_io.pop_current_element();

  return true;
}

bool TransferFunction::post_load_states( const StateIO& state_io )
{
  assert( this->private_->tf_feature_map_.empty() );
  this->private_->dirty_ = true;

  const TiXmlElement* features_element = state_io.get_current_element()->
    FirstChildElement( "features" );
  if ( features_element == 0 )
  {
    return false;
  }

  state_io.push_current_element();
  state_io.set_current_element( features_element );

  const TiXmlElement* feature_element = features_element->FirstChildElement();
  while ( feature_element != 0 )
  {
    std::string feature_id( feature_element->Value() );
    TransferFunctionFeatureHandle feature( new TransferFunctionFeature( feature_id ) );

    if ( feature->load_states( state_io ) )
    {
      this->private_->tf_feature_map_[ feature_id ] = feature;
      feature->control_points_state_->state_changed_signal_.connect( boost::bind(
        &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
      feature->diffuse_color_red_state_->state_changed_signal_.connect( boost::bind(
        &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
      feature->diffuse_color_green_state_->state_changed_signal_.connect( boost::bind(
        &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
      feature->diffuse_color_blue_state_->state_changed_signal_.connect( boost::bind(
        &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
      feature->ambient_coefficient_state_->state_changed_signal_.connect( boost::bind(
        &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
      feature->specular_intensity_state_->state_changed_signal_.connect( boost::bind(
        &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
      feature->shininess_state_->state_changed_signal_.connect( boost::bind(
        &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
      feature->enabled_state_->state_changed_signal_.connect( boost::bind(
        &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
      feature->solid_state_->state_changed_signal_.connect( boost::bind(
        &TransferFunctionPrivate::handle_tf_state_changed, this->private_ ) );
      this->feature_added_signal_( feature );
    }

    feature_element = feature_element->NextSiblingElement();
  }

  state_io.pop_current_element();

  return true;
}

void TransferFunction::clear()
{
  ASSERT_IS_APPLICATION_THREAD();
  StateEngine::lock_type lock( StateEngine::GetMutex() );
  BOOST_FOREACH( tf_feature_map_type::value_type feature_entry, this->private_->tf_feature_map_ )
  {
    feature_entry.second->invalidate();
  }
  this->private_->tf_feature_map_.clear();
  this->private_->dirty_ = true;
  this->transfer_function_changed_signal_();
}

} // end namespace Core
