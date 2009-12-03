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

#ifndef APPLICATION_TOOL_TOOLMANAGER_H
#define APPLICATION_TOOL_TOOLMANAGER_H

// STL includes
#include <string>

// Boost includes
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Utils/Core/Log.h>
#include <Utils/Core/Exception.h>

// Application includes
#include <Application/State/StateHandler.h>

namespace Seg3D {

class Tool;
typedef boost::shared_ptr<Tool> ToolHandle;

class ToolManager : public StateHandler {

// -- constructor/destructor --
  public:
    ToolManager();

// -- Dispatcher functions for User Interface -- 
  public:
    
    // -- Open a tool --
    void dispatch_opentool(const std::string& tool_name) const;
 
    // -- Close a tool --
    void dispatch_closetool(const std::string& tool_id) const;
 
    // -- Activate tool --
    void dispatch_activatetool(const std::string& tool_id) const;
 
// -- Handler functions --
  protected:
  
    friend class ActionOpenTool;
    friend class ActionCloseTool;
    friend class ActionActivateTool;
  
    // OPEN_TOOL (accessed through Action):
    // Open a new tool into the current collection of active tools
    bool open_tool(const std::string& toolid, std::string& new_toolid);

    // CLOSE_TOOL (accessed through Action):
    // Close tool in current collection of active tools
    void close_tool(const std::string& toolid);
  
    // ACTIVATE_TOOL (accessed through Action):
    // Set which tool is currently highlighted
    // The active tool has access to the viewer
    void activate_tool(const std::string& toolid);
  
// -- Signals for the User Interface --

  public:
    typedef boost::signals2::signal<void (ToolHandle)>  open_tool_signal_type;
    typedef boost::signals2::signal<void (ToolHandle)>  close_tool_signal_type;
    typedef boost::signals2::signal<void (ToolHandle)>  activate_tool_signal_type;
  
    // CONNECT_OPEN_TOOL:
    // Connect to signal that signals new tool being created
    boost::signals2::connection 
    connect_open_tool(open_tool_signal_type::slot_type slot)
    {
      return open_tool_signal_.connect(slot);
    }
  
    // CONNECT_CLOSE_TOOL:
    // Connect to signal that signals tool being closed
    boost::signals2::connection 
    connect_close_tool(close_tool_signal_type::slot_type slot)
    {
      return close_tool_signal_.connect(slot);
    }

    // CONNECT_ACTIVATE_TOOL:
    // Connect to signal that signals tool being closed
    boost::signals2::connection 
    connect_activate_tool(activate_tool_signal_type::slot_type slot)
    {
      return activate_tool_signal_.connect(slot);
    }

  protected:
    
    // OPEN_TOOL_SIGNAL:
    // This signal is triggered after a tool has been opened
    open_tool_signal_type open_tool_signal_;
    
    // CLOSE_TOOL_SIGNAL:
    // This signal is triggered when before a tool is closed
    close_tool_signal_type close_tool_signal_;

    // ACTIVATE_TOOL_SIGNAL:
    // This signal is triggered when before a tool is closed
    activate_tool_signal_type activate_tool_signal_;

// -- Tool database --
  protected:
    friend class Tool;

    // ADD_TOOLID:
    // Mark a toolid as used
    void add_toolid(const std::string& toolid);
    
    // REMOVE_TOOLID:
    // Remove the toolid from the list
    void remove_toolid(const std::string& toolid);

    // IS_TOOLID:
    // Check whether toolid is taken
    bool is_toolid(const std::string& toolid);

    // CREATE_TOOLID:
    // Create a new tool id that is not yet in the list
    std::string create_toolid(const std::string& tool_type);

  private:
    typedef boost::unordered_map<std::string,ToolHandle> tool_list_type;
    typedef boost::unordered_set<std::string> toolid_list_type;
  
    // All the open tools are stored in this hash map
    tool_list_type   tool_list_;

    // Lock for the tool_list
    boost::mutex tool_list_lock_;
    
    // The tool that is currently active is stored here
    std::string      active_toolid_;
    
    // All the names that are currently in use
    toolid_list_type toolid_list_;

    // Lock for the tool_list
    boost::mutex toolid_list_lock_;
    

// -- Singleton interface --
  public:
    static ToolManager* instance() { instance_.instance(); }

  private:
    static Utils::Singleton<ToolManager> instance_;
};

} // namespace Seg3D

#endif
