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

#ifndef APPLICATION_TOOLS_CONNECTEDCOMPONENTFILTER_H
#define APPLICATION_TOOLS_CONNECTEDCOMPONENTFILTER_H

// Core includes
#include <Core/State/StateLabeledOption.h>

// Application includes
#include <Application/Tool/SeedPointsTool.h>

namespace Seg3D
{

class ConnectedComponentFilterPrivate;
typedef boost::shared_ptr< ConnectedComponentFilterPrivate > ConnectedComponentFilterPrivateHandle;

class ConnectedComponentFilter : public SeedPointsTool
{

SEG3D_TOOL(
  SEG3D_TOOL_NAME( "ConnectedComponentFilter", "Select all components connected to the seed points." )
  SEG3D_TOOL_MENULABEL( "Connected Component" )
  SEG3D_TOOL_MENU( "Mask Filters" )
  SEG3D_TOOL_SHORTCUT_KEY( "CTRL+ALT+C" )
  SEG3D_TOOL_URL( "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Seg3D2:ConnectedComponentFilter:1" )
  SEG3D_TOOL_VERSION( "1" )
  SEG3D_TOOL_HOTKEYS("C=Clear seed points.|Left Mouse Button=Add seed point.|Right Mouse Button=Delete seed point.")
)

  // -- constructor/destructor --
public:
  ConnectedComponentFilter( const std::string& toolid );
  virtual ~ConnectedComponentFilter();

  // -- state --
public:
  /// Whether the layer needs to be replaced
  Core::StateBoolHandle replace_state_;
  
  /// Whether to ue the seed points
  Core::StateBoolHandle use_seeds_state_;
  
  /// Layerid of the mask layer
  Core::StateLabeledOptionHandle mask_state_;
    
  /// Whether mask should be inverted in this filter
  Core::StateBoolHandle mask_invert_state_;

  // -- execute --
public:
  /// Execute the tool and dispatch the action
  virtual void execute( Core::ActionContextHandle context );

private:
  ConnectedComponentFilterPrivateHandle private_;
};

} // end namespace

#endif
