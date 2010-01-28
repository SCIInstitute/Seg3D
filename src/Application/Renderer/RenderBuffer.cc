#include <Application/Renderer/RenderBuffer.h>

namespace Seg3D {

const unsigned int RenderBuffer::TARGET_ = GL_RENDERBUFFER_EXT;

RenderBuffer::RenderBuffer()
{
  glGenRenderbuffersEXT(1, &id_);
  
  _safe_bind();
  _safe_unbind();
}

RenderBuffer::~RenderBuffer()
{
  glDeleteRenderbuffersEXT(1, &id_);
}

void RenderBuffer::bind()
{
  glBindRenderbufferEXT(TARGET_, id_);
}

void RenderBuffer::set_storage(int width, int height, unsigned int internal_format, int samples)
{
  _safe_bind();
  if (samples > 1)
  {
    glRenderbufferStorageMultisampleEXT(TARGET_, samples, internal_format, width, height);
  }
  else
  {
    glRenderbufferStorageEXT(TARGET_, internal_format, width, height);
  }
  _safe_unbind();
}

void RenderBuffer::unbind()
{
  glBindRenderbufferEXT(TARGET_, 0);
}

void RenderBuffer::_safe_bind()
{
  glGetIntegerv(GL_RENDERBUFFER_BINDING_EXT, &saved_id_);
  if (id_ != saved_id_)
  {
    glBindRenderbufferEXT(TARGET_, id_);
  }
}

void RenderBuffer::_safe_unbind()
{
  if (id_ != saved_id_)
  {
    glBindRenderbufferEXT(TARGET_, saved_id_);
  }
}

} // end namespace Seg3D