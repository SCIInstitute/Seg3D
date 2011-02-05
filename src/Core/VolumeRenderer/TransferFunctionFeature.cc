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

TransferFunctionFeature::TransferFunctionFeature() :
  StateHandler( "tffeature", true )
{
  this->initialize_states();
}

TransferFunctionFeature::TransferFunctionFeature( const std::string& feature_id ) :
  StateHandler( feature_id, true )
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
  this->add_state( "control_points", this->control_points_state_ );
  this->add_state( "red", this->red_color_state_, 128, 0, 255, 1 );
  this->add_state( "green", this->green_color_state_, 128, 0, 255, 1 );
  this->add_state( "blue", this->blue_color_state_, 128, 0, 255, 1 );
  this->add_state( "shininess", this->shininess_state_, 0, 0, 128, 1 );
}

} // end namespace Core