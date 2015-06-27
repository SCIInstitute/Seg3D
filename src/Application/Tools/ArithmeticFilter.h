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

#include <Application/Tool/SingleTargetTool.h>

namespace Seg3D
{

class ArithmeticFilter : public SingleTargetTool
{
SEG3D_TOOL(
SEG3D_TOOL_NAME( "ArithmeticFilter", "Filter for evaluating an expression on data" )
SEG3D_TOOL_MENULABEL( "Arithmetic" )
SEG3D_TOOL_MENU( "Data Filters" )
SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+A" )
SEG3D_TOOL_URL( "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Seg3D2:ArithmeticFilter:1" )
)

public:
  ArithmeticFilter( const std::string& toolid );
  virtual ~ArithmeticFilter();

public:
  virtual void execute( Core::ActionContextHandle context );

private:
  void update_output_type();
  void update_replace_options();

  // -- state --
public:
  Core::StateLabeledOptionHandle input_b_state_;
  Core::StateLabeledOptionHandle input_c_state_;
  Core::StateLabeledOptionHandle input_d_state_;

  Core::StateStringHandle expressions_state_; 
  Core::StateLabeledOptionHandle output_type_state_;

  Core::StateBoolHandle replace_state_;
  Core::StateBoolHandle preserve_data_format_state_;
  Core::StateBoolHandle output_is_data_state_;
  Core::StateBoolHandle input_matches_output_state_;

};

} // end namespace

#endif
