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

#ifndef APPLICATION_TOOLS_RESAMPLETOOL_H
#define APPLICATION_TOOLS_RESAMPLETOOL_H

#include <Application/Tool/GroupTargetTool.h>

namespace Seg3D
{

class ResampleToolPrivate;
typedef boost::shared_ptr< ResampleToolPrivate > ResampleToolPrivateHandle;

class ResampleTool : public GroupTargetTool
{

SEG3D_TOOL
(
  SEG3D_TOOL_NAME( "ResampleTool", "Tool for resampling data" )
  SEG3D_TOOL_MENULABEL( "Resample" )
  SEG3D_TOOL_MENU( "Tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+7" )
  SEG3D_TOOL_URL( "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Seg3D2:ResampleTool:1" )
)

  // -- constructor/destructor --
public:
  ResampleTool( const std::string& toolid );
  virtual ~ResampleTool();

public:

  /// EXECUTE:
  /// Execute the tool and dispatch the action
  virtual void execute( Core::ActionContextHandle context );

  // -- state --
public:
  Core::StateIntHandle input_dimensions_state_[ 3 ];

  Core::StateLabeledOptionHandle size_scheme_state_;
  Core::StateBoolHandle manual_size_state_;

  Core::StateLabeledOptionHandle dst_group_state_;
  Core::StateLabeledOptionHandle padding_value_state_;

  Core::StateRangedIntHandle output_dimensions_state_[ 3 ];
  Core::StateBoolHandle constraint_aspect_state_;
  Core::StateRangedDoubleHandle scale_state_;
  
  Core::StateLabeledOptionHandle kernel_state_;
  Core::StateRangedDoubleHandle gauss_sigma_state_;
  Core::StateRangedDoubleHandle gauss_cutoff_state_;
  Core::StateBoolHandle has_params_state_;
  
  Core::StateBoolHandle valid_size_state_;
  Core::StateBoolHandle replace_state_;

private:
  ResampleToolPrivateHandle private_;

public:
  static const std::string SIZE_OTHER_GROUP_C;
  static const std::string SIZE_MANUAL_C;
};

} // end namespace

#endif
