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

namespace Seg3D {

class ToolManager {

// -- constructor/destructor --
  public:
    ToolManager();
    virtual ~ToolManager();


// -- Dispatcher functions for User Interface -- 
  public:
    
    // -- Open a tool --
    void dispatch_opentool(const std::string& tool_name) const;
 
    // -- Close a tool --
    void dispatch_closetool(const std::string& tool_id) const;
 
 
// -- Handler functions --
  protected:
  
    friend class ActionOpenTool;
    friend class ActionCloseTool;
  
    void open_tool(const std::string& toolname, const std::string& toolid);
    void close_tool(const std::string& toolid);
  
// -- Signals for the User Interface --

  public:
  
    boost::signals2::signal<void (ToolHandle)>  open_tool_signal_type;
    boost::signals2::signal<void (ToolHandle)>  close_tool_signal_type;
    
    // OPEN_TOOL_SIGNAL:
    // This signal is triggered after a tool has been opened
    open_tool_signal_type open_tool_signal_;
    
    // CLOSE_TOOL_SIGNAL:
    // This signal is triggered when before a tool is closed
    close_tool_signal_type close_tool_signal_;

// -- Tool database --
  private:
    typedef boost::unordered_map<std::string,ToolHandle> tool_list_type;
  
    // All the open tools are stored in this hash map
    tool_list_type tool_list_;
};

} // namespace Seg3D

#endif
