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

#ifndef APPLICATION_TOOLS_SEEDPOINTSTOOL_H
#define APPLICATION_TOOLS_SEEDPOINTSTOOL_H

#include <Core/Geometry/Point.h>
#include <Core/State/StateVector.h>

#include <Application/Tool/Tool.h>

namespace Seg3D
{

class SeedPointsToolPrivate;
typedef boost::shared_ptr< SeedPointsToolPrivate > SeedPointsToolPrivateHandle;

class SeedPointsTool : public Tool
{
public:
   SeedPointsTool( const std::string& toolid, size_t version_number, bool auto_number = true );
   virtual ~SeedPointsTool() = 0;

public:
  virtual bool handle_mouse_enter( size_t viewer_id, int x, int y );
  virtual bool handle_mouse_leave( size_t viewer_id );
  virtual bool handle_mouse_press( const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  // REDRAW:
  // Draw seed points in the specified viewer.
  // The function should only be called by the renderer, which has a valid GL context.
  virtual void redraw( size_t viewer_id, const Core::Matrix& proj_mat );

public:
  Core::StatePointVectorHandle seed_points_state_;
  Core::StateLabeledOptionHandle target_layer_state_;

private:
  SeedPointsToolPrivateHandle private_;
};

} // end namespace Seg3D
#endif