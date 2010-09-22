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

#ifndef APPLICATION_TOOLS_ARITHMETICFILTER_H
#define APPLICATION_TOOLS_ARITHMETICFILTER_H

#include <Application/Tool/Tool.h>

namespace Seg3D
{

class ArithmeticFilterPrivate;
typedef boost::shared_ptr< ArithmeticFilterPrivate > ArithmeticFilterPrivateHandle;

class ArithmeticFilter : public Tool
{
SEG3D_TOOL(
SEG3D_TOOL_NAME( "ArithmeticFilter", "Filter for evaluating an expression on data" )
SEG3D_TOOL_MENULABEL( "Arithmetic" )
SEG3D_TOOL_MENU( "filter_data_to_data" )
SEG3D_TOOL_SHORTCUT_KEY( "Alt+Shift+A" )
SEG3D_TOOL_URL( "http://seg3d.org/" )
)

public:
  ArithmeticFilter( const std::string& toolid );
  virtual ~ArithmeticFilter();

public:
  virtual void execute( Core::ActionContextHandle context );

  // -- state --
public:
  Core::StateLabeledOptionHandle target_group_state_;
  Core::StateBoolHandle use_active_group_state_;
  Core::StateLabeledOptionHandle input_layers_state_[ 4 ];

  Core::StateStringHandle expressions_state_;
  Core::StateStringVectorHandle predefined_expressions_state_;
  
  Core::StateLabeledOptionHandle output_type_state_;
  Core::StateBoolHandle replace_state_;
  Core::StateBoolHandle preserve_data_format_state_;

private:
  ArithmeticFilterPrivateHandle private_;

public:
  const static int NUMBER_OF_INPUTS_C;

};

} // end namespace

#endif
