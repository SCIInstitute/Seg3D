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

#ifndef SEG3D_TOOL_TOOLINFO_H
#define SEG3D_TOOL_TOOLINFO_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>

namespace Seg3D
{

class ToolInfo;
class ToolInfoPrivate;

typedef boost::shared_ptr< ToolInfo > ToolInfoHandle;
typedef boost::shared_ptr< ToolInfoPrivate > ToolInfoPrivateHandle;

/// CLASS TOOLINFO:
/// Information class about an action

class ToolInfo : public boost::noncopyable
{
  // -- constructor --
public:
  ToolInfo( const std::string& definition );
  
public:
  /// GET_DEFINITION:
  /// Get the definition of the action (in xml format)
  std::string get_definition() const;

  /// GET_DESCRIPTION:
  /// Get the description of the action
  std::string get_description() const;

  /// GET_NAME:
  /// The type/name of the action
  std::string get_name() const;

  /// GET_MENU:
  /// The name of menu where to put the tool
  std::string get_menu() const;
  
  /// GET_MENU_LABEL:
  /// The name of menu item to activate this tool
  std::string get_menu_label() const;

  /// GET_SHORTCUT_KEY:
  /// The shortcut key for this tool
  std::string get_shortcut_key() const;
  
  /// GET_HOTKEYS_AND_DEFINITIONS:
  /// The hotkeys for this tool
  std::string get_hotkeys() const;

  /// GET_URL:
  /// Get the help url
  std::string get_url() const;
  
  /// GET_VERSION:
  /// Get the tool version
  int get_version() const;

  bool get_is_large_volume() const;

private:
  // Private implementation of this class
  ToolInfoPrivateHandle private_;

public:
  typedef boost::mutex mutex_type;
  typedef boost::unique_lock<mutex_type> lock_type;
  
  /// GET_MUTEX:
  /// Get the mutex that protects the generation of ToolInfo instances
  static mutex_type& GetMutex();

private:
  /// the actual mutex that is shared between all the ToolInfo classes
  static mutex_type mutex_;
};

} // end namespace Seg3D

#endif
