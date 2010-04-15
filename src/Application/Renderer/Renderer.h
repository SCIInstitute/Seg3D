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
#include <Application/Renderer/RendererBase.h>
#include <Application/Renderer/SliceShader.h>
#include <Application/Viewer/Viewer.h>

#include <Utils/Core/ConnectionHandler.h>
#include <Utils/Graphics/Texture.h>
#include <Utils/Graphics/UnitCube.h>
#include <Utils/TextRenderer/TextRenderer.h>

namespace Seg3D
{

// Forward declarations
class Renderer;
typedef boost::shared_ptr< Renderer > RendererHandle;

// Class definitions
class Renderer : public RendererBase, private Utils::ConnectionHandler
{

  // -- constructor/destructor --
public:
  Renderer();
  virtual ~Renderer();

  void set_viewer_id( size_t viewer_id )
  {
    this->viewer_id_ = viewer_id;
  }

protected:
  virtual void post_initialize();
  virtual void post_resize();
  virtual bool render();
  virtual bool render_overlay();

private:

  void process_slices( LayerSceneHandle& layer_scene, ViewerHandle& viewer );

  Utils::UnitCubeHandle cube_;
  SliceShaderHandle slice_shader_;
  Utils::Texture3DHandle pattern_texture_;

  Utils::TextRendererHandle text_renderer_;
  Utils::Texture2DHandle text_texture_;
  
  size_t viewer_id_;
};

} // end namespace Seg3D

#endif
