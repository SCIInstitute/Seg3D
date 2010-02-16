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
#include <map>
#include <set>

// Boost includes

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
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

// CLASS TOOLMANAGER:
// This class manages the tools in the toolbox of the application

// Forward declaration
class ToolManager;

// Class definition
class ToolManager : public StateHandler {

// -- constructor/destructor --
  private:
    // NOTE: Constructor is private: use Instance() to generate this singleton
    friend class Utils::Singleton<ToolManager>;
    ToolManager();
 
  public:
    virtual ~ToolManager();
 
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
    typedef boost::signals2::signal<void (ToolHandle)>  tool_signal_type;
  
    // OPEN_TOOL_SIGNAL:
    // This signal is triggered after a tool has been opened
    tool_signal_type open_tool_signal_;
    
    // CLOSE_TOOL_SIGNAL:
    // This signal is triggered when before a tool is closed
    tool_signal_type close_tool_signal_;

    // ACTIVATE_TOOL_SIGNAL:
    // This signal is triggered when before a tool is closed
    tool_signal_type activate_tool_signal_;
    
// -- Access to toollist --
  public:
    typedef std::map<std::string,ToolHandle>  tool_list_type;
    typedef std::set<std::string>             toolid_list_type;
    typedef boost::recursive_mutex            mutex_type;
    typedef boost::unique_lock<mutex_type>    lock_type;
  
    // TOOL_LIST:
    // Get the current open tool list
    tool_list_type tool_list();

    // ACTIVE_TOOLID:
    // Get the active toolid
    std::string active_toolid();

    // GET_MUTEX:
    // Get the mutex, so it can be locked by the interface that is built
    // on top of this
    mutex_type& get_mutex();

  private:
  
    // All the open tools are stored in this hash map
    tool_list_type  tool_list_;

    // Lock for the tool_list
    mutex_type      tool_list_mutex_;
    
    // The tool that is currently active is stored here
    std::string     active_toolid_;

// -- Singleton interface --
  public:
    static ToolManager* Instance() { return instance_.instance(); }

  private:
    static Utils::Singleton<ToolManager> instance_;
};

} // namespace Seg3D

#endif
