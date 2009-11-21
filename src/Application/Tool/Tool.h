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

// Utils includes
#include <Utils/Core/Log.h>
#include <Utils/Core/Exception.h>

// Application includes
#include <Application/State/StateManager.h>
#include <Application/State/State.h>

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
    Tool(const std::string& toolid);
    virtual ~Tool();

// -- tool_type/toolid --

    int         properties() const { return properties_; }
    std::string tool_type() const  { return tool_type_; }
    std::string toolid() const     { return toolid_; }

  protected:
    friend class ToolFactory;
    // As each tool is created through the factory the properties and the 
    // type name are set through the factory so it only needs to be set once.
    
    // The detailed reason for not including these in the constructor is that
    // the ToolFactory will not know about them until the object has been
    // instantiated. In the current design it is recorded separately in the
    // factory and then inserted into the Tool object upon creation. This
    // avoids duplicating the properties and typename registration.
    
    void set_properties(int properties)              { properties_ = properties; }
    void set_tool_type(const std::string& tool_type) { tool_type_ = tool_type; }

  private:
    std::string           toolid_;
    int                   properties_;    
    std::string           tool_type_;

// -- close tool --
  protected:
    friend class ToolManager;
    
    // CLOSE_TOOL:
    // Function for cleaning up the application part of the tool
    // This one needs to cleanup the state variables.
    virtual void close_tool();

// -- state variables --
  protected:
  
    // ADD_STATE:
    // Add a local state variable to the Tool.
    bool add_state(const std::string& key, StateBase* state) const;
    
};

} // end namespace Seg3D

#endif
