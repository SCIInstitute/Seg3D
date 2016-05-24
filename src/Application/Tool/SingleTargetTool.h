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

#ifndef APPLICATION_TOOL_SINGLETARGETTOOL_H
#define APPLICATION_TOOL_SINGLETARGETTOOL_H

// boost includes
#include <boost/tuple/tuple.hpp>

// Application includes
#include <Application/Tool/Tool.h>

namespace Seg3D
{

// Forward declaration
class SingleTargetToolPrivate;
typedef boost::shared_ptr< SingleTargetToolPrivate > SingleTargetToolPrivateHandle;

// Class definition
class SingleTargetTool : public Tool
{

  // -- constructor/destructor --
public:
  SingleTargetTool( int target_volume_type, const std::string& tool_type );
  
  virtual ~SingleTargetTool();

  // -- state --
public:
  // Layer ID of the target layer
  Core::StateLabeledOptionHandle target_layer_state_;

  /// Whether to use the active of one from the list
  Core::StateBoolHandle use_active_layer_state_;
  Core::StateBoolHandle valid_primary_target_state_;

  /// Whether a valid layer has been selected
  Core::StateBoolHandle valid_target_state_;

  /// Add a state whose input is linked to the target.
  void add_extra_layer_input( Core::StateLabeledOptionHandle input_layer_state,
    Core::VolumeType type, bool required = false, bool dependent = true );

private:
  SingleTargetToolPrivateHandle private_;

};

} // end namespace Seg3D

#endif
