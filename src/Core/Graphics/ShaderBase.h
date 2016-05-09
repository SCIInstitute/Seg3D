/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#ifndef CORE_GRAPHICS_SHADERBASE_H
#define CORE_GRAPHICS_SHADERBASE_H

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

namespace Core
{

class ShaderBasePrivate;
typedef boost::shared_ptr< ShaderBasePrivate > ShaderBasePrivateHandle;

class ShaderBase : public boost::noncopyable
{
public:
  ShaderBase();
  virtual ~ShaderBase();

  bool initialize();
  void enable();
  void disable();
  bool is_valid();

protected:
  virtual bool get_vertex_shader_source( std::string& source );
  virtual bool get_fragment_shader_source( std::string& source );
  virtual bool pre_link();
  virtual bool post_initialize();

  int get_uniform_location( const char* name );
  void bind_attrib_location( unsigned int index, const char* name );

private:
  ShaderBasePrivateHandle private_;
};

} // end namespace Core
#endif
