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

#include <Utils/Render/RenderResources.h>

namespace Utils {

RenderResources::RenderResources() :
  render_context_list_size_(6)
{
}

bool
RenderResources::render_context(size_t index, RenderContextHandle& context)
{
  if (index >= render_context_list_size_) return (false);
  context = render_context_list_[index];
  return (true);
}

void
RenderResources::install_resources_context(RenderResourcesContextHandle& resources_context)
{
  if (!resources_context.get())
  {
    SCI_THROW_LOGICERROR("Cannot install an empty render resources context");
  }
  
  if(!(resources_context->create_shared_render_context(shared_render_context_)))
  {
    SCI_THROW_LOGICERROR("Could not create a shared render context");
  }

  render_context_list_.resize(render_context_list_size_);
  for (size_t j=0; j<render_context_list_size_; j++)
  {
    if(!(resources_context->create_shared_render_context(render_context_list_[j])))
    {
      SCI_THROW_LOGICERROR("Could not create a shared render context");
    }  
  }
  
  resources_context_ = resources_context;
}

bool
RenderResources::valid_render_resources()
{
  return (resources_context_.get()!=0);
}

Singleton<RenderResources> RenderResources::instance_;

} // end namespace Utils

