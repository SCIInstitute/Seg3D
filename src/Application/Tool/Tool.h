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

#ifndef APPLICATION_TOOL_TOOL_H
#define APPLICATION_TOOL_TOOL_H

// STL includes
#include <string>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Utils/Core/Log.h>
#include <Utils/Core/Exception.h>


namespace Seg3D {

// CLASS TOOL:
// Base class of each tool

class Tool;
typedef boost::shared_ptr<Tool> ToolHandle;

class Tool {
// -- definition of tool types --
  // Tool types help organize the tools in different catagories
  
  enum tool_properties_type {
    // TOOL - Tool is not a filter
    TOOL_E   = 0x0001,
    
    // MASK_FILTER - Tool is a filter
    MASK_FILTER_E = 0x0002,

    // DATA_FILTER - Tool is a filter
    DATA_FILTER_E = 0x0002,
    
    // OUTPUTS_DATAVOLUME - Filter/tool works on datavolume
    OUTPUTS_DATAVOLUME_E  = 0x0100,

    // OUTPUTS_MASKVOLUME - Filter/tool works on maskvolume
    OUTPUTS_MASKVOLUME_E  = 0x0200,

    // OUTPUTS_LABELVOLUME - Filter/tool works on labelvolume
    OUTPUTS_LABELVOLUME_E = 0x0400
  };


// -- constructor/destructor --
  public:
    Tool();
    
    virtual ~Tool();

    tool_properties_type  properties() const { return properties_; }
    std::string           type_name() const  { return type_name_; }

 protected:
    friend class ToolFactory;
    // As each tool is created through the factory the properties are set
    // in the factory so it only needs to be set once
    void set_properties(tool_properties_type properties) { properties_ = properties; }
    void set_type_name(const std::string& type_name)     { type_name_ = type_name; }

  private:
    tool_properties_type  properties_;    
    std::string           type_name_;

// -- unique tool id --
  
  public:
    // SET_TOOLID:
    // Copy the ID the ToolManager uses into the tool class
    void set_toolid(const std::string& toolid) { toolid_ = toolid; }
    
    // GET_TOOLID:
    // Retrieve the ID the ToolManager inserter
    std::string get_toolid()  { return toolid_; }
    
  private:
    std::string toolid_;

};

} // end namespace Seg3D

#endif
