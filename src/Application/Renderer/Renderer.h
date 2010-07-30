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

#ifndef APPLICATION_RENDERER_RENDERER_H
#define APPLICATION_RENDERER_RENDERER_H

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// Application includes
#include <Application/Renderer/SliceShader.h>
#include <Application/Viewer/Viewer.h>

#include <Core/Utils/ConnectionHandler.h>
#include <Core/Geometry/BBox.h>
#include <Core/Graphics/Texture.h>
#include <Core/Graphics/UnitCube.h>
#include <Core/RendererBase/RendererBase.h>
#include <Core/TextRenderer/TextRenderer.h>

namespace Seg3D
{

// Forward declarations
class Renderer;
class RendererPrivate;
typedef boost::shared_ptr< Renderer > RendererHandle;
typedef boost::shared_ptr< RendererPrivate > RendererPrivateHandle;

class ProxyRectangle;
typedef boost::shared_ptr< ProxyRectangle > ProxyRectangleHandle;

// Class definitions
class Renderer : public Core::RendererBase, private Core::ConnectionHandler
{

  // -- constructor/destructor --
public:
  Renderer( size_t viewer_id );
  virtual ~Renderer();

protected:
  virtual void post_initialize();
  virtual void post_resize();
  virtual bool render();
  virtual bool render_overlay();

private:
  void process_slices( LayerSceneHandle& layer_scene, ViewerHandle& viewer );
  void draw_slices_3d( const Core::BBox& bbox, 
    const std::vector< LayerSceneHandle >& layer_scenes, 
    const std::vector< double >& depths,
    const std::vector< std::string >& view_modes );
  void draw_slice( LayerSceneItemHandle layer_item, const Core::Matrix& proj_mat,
    ProxyRectangleHandle rect = ProxyRectangleHandle() );
  void viewer_slice_changed( size_t viewer_id );
  void viewer_mode_changed( size_t viewer_id );
  void picking_target_changed( size_t viewer_id );
  void enable_rendering( bool enable );

  SliceShaderHandle slice_shader_;
  Core::Texture2DHandle pattern_texture_;

  Core::TextRendererHandle text_renderer_;
  Core::Texture2DHandle text_texture_;
  
  size_t viewer_id_;

  RendererPrivateHandle private_;
};

} // end namespace Seg3D

#endif
