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

#include <boost/foreach.hpp>

#include <tinyxml.h>

#include <Core/State/StateIO.h>
#include <Core/VolumeRenderer/TransferFunction.h>

namespace Core
{

typedef std::map< std::string, TransferFunctionFeatureHandle > tf_feature_map_type;

class TransferFunctionPrivate
{
public:
  tf_feature_map_type tf_feature_map_;
  bool dirty_;
  Texture1DHandle lut_;
};

TransferFunction::TransferFunction() :
  StateHandler( "tf", true ),
  private_( new TransferFunctionPrivate )
{
  this->private_->dirty_ = true;

  this->add_connection( Application::Instance()->reset_signal_.connect(
    boost::bind( &TransferFunction::clear, this ) ) );
}

TransferFunction::~TransferFunction()
{
  this->disconnect_all();
}

TextureHandle TransferFunction::get_lookup_texture() const
{
  StateEngine::lock_type lock( StateEngine::GetMutex() );
  return this->private_->lut_;
}

Core::TransferFunctionFeatureHandle TransferFunction::create_feature()
{
  StateEngine::lock_type lock( StateEngine::GetMutex() );

  TransferFunctionFeatureHandle feature( new TransferFunctionFeature );
  this->private_->tf_feature_map_[ feature->get_feature_id() ] = feature;
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
  this->private_->tf_feature_map_.clear();
  this->private_->dirty_ = true;
}

} // end namespace Core