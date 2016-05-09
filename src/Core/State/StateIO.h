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

#ifndef CORE_STATE_STATEIO_H
#define CORE_STATE_STATEIO_H

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>

// TinyXML includes
#include <tinyxml.h>

namespace Core
{

class StateIO;
class StateIOPrivate;
typedef boost::shared_ptr< StateIOPrivate > StateIOPrivateHandle;

class StateIO : public boost::noncopyable
{

public:
  StateIO();
  ~StateIO();

public:

  void initialize();

  const TiXmlElement* get_current_element() const;
  TiXmlElement* get_current_element();

  bool import_from_file( const boost::filesystem::path& path );
    bool import_from_file( const boost::filesystem::path& path, std::string& error );
  
    bool export_to_file( const boost::filesystem::path& path );

  void push_current_element() const;
  void pop_current_element() const;
  void set_current_element( const TiXmlElement* element ) const;

  int get_major_version() const;
  int get_minor_version() const;
  int get_patch_version() const;

private:
  StateIOPrivateHandle private_;
};


} // end namespace Core

#endif //CORE_STATE_STATEIO_H
