/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#ifndef APPLICATION_TOOLS_EXTRACTDATALAYER_H
#define APPLICATION_TOOLS_EXTRACTDATALAYER_H

// Application includes
#include <Application/Tool/SingleTargetTool.h>

namespace Seg3D
{

class ExtractDataLayerPrivate;
typedef boost::shared_ptr<ExtractDataLayerPrivate> ExtractDataLayerPrivateHandle;

class ExtractDataLayer : public SingleTargetTool
{
SEG3D_TOOL(
  SEG3D_TOOL_NAME( "ExtractDataLayer", "Extract a normal data layer from a large data layer." )
  SEG3D_TOOL_MENULABEL( "Extract DataLayer" )
  SEG3D_TOOL_MENU( "Large Volume Tools" )
  SEG3D_TOOL_SHORTCUT_KEY( "" )
  SEG3D_TOOL_URL( "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Seg3D2:ExtractDataLayer:1" )
  SEG3D_TOOL_VERSION( "1" )
  SEG3D_LARGE_VOLUME_TOOL( "true" )
)

  // -- constructor/destructor --
public:
  ExtractDataLayer( const std::string& toolid );
  virtual ~ExtractDataLayer();

  // -- state --
public:
  /// Layerid of the mask layer
  Core::StateLabeledOptionHandle level_state_;

  // -- execute --
public:
  /// Execute the tool and dispatch the action
  virtual void execute( Core::ActionContextHandle context );

private:
  ExtractDataLayerPrivateHandle private_;

};

} // end namespace

#endif
