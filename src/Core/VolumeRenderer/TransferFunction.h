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

#ifndef CORE_VOLUMERENDERER_TRANSFERFUNCTION_H
#define CORE_VOLUMERENDERER_TRANSFERFUNCTION_H

#include <Core/Graphics/Texture.h>
#include <Core/State/StateHandler.h>
#include <Core/State/StateValue.h>
#include <Core/VolumeRenderer/TransferFunctionControlPoint.h>
#include <Core/VolumeRenderer/TransferFunctionFeature.h>

namespace Core
{

class TransferFunction;
typedef boost::shared_ptr< TransferFunction > TransferFunctionHandle;
typedef boost::shared_ptr< TransferFunction const > TransferFunctionConstHandle;

class TransferFunctionPrivate;
typedef boost::shared_ptr< TransferFunctionPrivate > TransferFunctionPrivateHandle;

class TransferFunction : public Core::StateHandler
{
public:
  TransferFunction();
  ~TransferFunction();

  TransferFunctionFeatureHandle create_feature();
  void delete_feature( const std::string& feature_id );
  void clear();

  // GET_DIFFUSE_LUT:
  /// Get the lookup texture of the diffuse color.
  TextureHandle get_diffuse_lut() const;

  // GET_SPECULAR_LUT:
  /// Get the lookup texture of the specular color.
  /// The alpha component contains the shininess value.
  TextureHandle get_specular_lut() const;

  TransferFunctionFeatureHandle get_feature( const std::string& feature_id ) const;

protected:

  // POST_LOAD_STATES:
  /// This virtual function can be implemented in the StateHandlers and will be called after its
  /// states are loaded.  If it doesn't succeed it needs to return false.
  virtual bool post_load_states( const StateIO& state_io );

  // POST_SAVE_STATES:
  /// This virtual function can be implemented in the StateHandlers and will be called after its
  /// states are saved.  If it doesn't succeed it needs to return false.
  virtual bool post_save_states( StateIO& state_io );

public:
  /// Whether to use faux shading
  StateBoolHandle faux_shading_state_;

public:
  typedef boost::signals2::signal< void ( TransferFunctionFeatureHandle ) > feature_signal_type;
  feature_signal_type feature_added_signal_;
  feature_signal_type feature_deleted_signal_;

  boost::signals2::signal< void () > transfer_function_changed_signal_;

private:
  TransferFunctionPrivateHandle private_;
};

} // end namespace Core

#endif
