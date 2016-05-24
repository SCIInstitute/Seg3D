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

#ifndef APPLICATION_TOOL_TOOL_H
#define APPLICATION_TOOL_TOOL_H

// STL includes
#include <string>

// Boost includes
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/State/StateHandler.h>
#include <Core/Viewer/Mouse.h>

// Application includes
#include <Application/Layer/LayerFWD.h>
#include <Application/Tool/ToolFWD.h>
#include <Application/Tool/ToolInfo.h>
#include <Application/Viewer/Viewer.h>

namespace Seg3D
{

// Forward declaration
class ToolPrivate;
typedef boost::shared_ptr< ToolPrivate > ToolPrivateHandle;

/// CLASS TOOL:
/// The Tool class forms the basis of the tool classes

// Class definition
class Tool : public Core::StateHandler
{
  friend class ToolFactory;

  // -- constructor/destructor --
public:
  Tool( const std::string& tool_type );
  virtual ~Tool();

  // -- query properties of tool --
public:
  /// GET_TOOL_INFO:
  /// Get the tool information class that contains all the information about the
  /// tool.
  /// NOTE: this function is generated using the macro for each function
  virtual ToolInfoHandle get_tool_info() const = 0;

  /// GET_NAME:
  /// Name of the tool
  std::string get_name() const;
  
  /// GET_MENU:
  /// Get the menu in which the tool is located
  std::string get_menu() const;
  
  /// GET_MENU_LABEL:
  /// Get the label with which the tool is displayed in the menu
  std::string get_menu_label() const;
  
  /// GET_SHORTCUT_KEY:
  /// Get the short cut key for this tool
  std::string get_shortcut_key() const;
  
  /// GET_HOTKEYS_AND_DESCRIPTIONS:
  /// Get the hotkeys for this tool
  std::string get_hotkeys_and_descriptions() const;
  
  /// GET_URL:
  /// Get the URL where more information on the tool can be found
  std::string get_url() const;
  
  /// GET_DEFINITION:
  /// Get the definition of the tool in XML format
  std::string get_definition() const;

  /// TOOLID:
  /// Get the statehandler id of this tool
  const std::string& toolid() const;
  
  /// TOOL_NAME:
  /// THe name that appears in the tool header
  virtual std::string tool_name() const;

  // -- mouse and keyboard event handlers --
public:
  /// HANDLE_MOUSE_ENTER:
  /// Called when the mouse has entered a viewer.
  virtual bool handle_mouse_enter( ViewerHandle viewer, int x, int y );

  /// HANDLE_MOUSE_LEAVE:
  /// Called when the mouse has left a viewer.
  virtual bool handle_mouse_leave( ViewerHandle viewer );

  /// HANDLE_MOUSE_MOVE:
  /// Called when the mouse moves in a viewer.
  virtual bool handle_mouse_move( ViewerHandle viewer, 
    const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  /// HANDLE_MOUSE_PRESS:
  /// Called when a mouse button has been pressed.
  virtual bool handle_mouse_press( ViewerHandle viewer, 
    const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  /// HANDLE_MOUSE_RELEASE:
  /// Called when a mouse button has been released.
  virtual bool handle_mouse_release( ViewerHandle viewer, 
    const Core::MouseHistory& mouse_history, 
    int button, int buttons, int modifiers );

  /// HANDLE_WHEEL:
  /// Called when the mouse wheel has been rotated.
  virtual bool handle_wheel( ViewerHandle viewer, int delta, 
    int x, int y, int buttons, int modifiers );

  /// HANDLE_KEY_PRESS:
  /// Called when a key is pressed
  virtual bool handle_key_press( ViewerHandle viewer, int key, int modifiers );

  /// HANDLE_KEY_RELEASE:
  /// Called when a key is released
  virtual bool handle_key_release( ViewerHandle viewer, int key, int modifiers );

  /// HANDLE_UPDATE_CURSOR:
  /// Called when a viewer requires an update to its cursor.
  virtual bool handle_update_cursor( ViewerHandle viewer );

public:
  /// REDRAW:
  /// Draw the tool in the specified viewer. Default implementation does nothing.
  /// The function should only be called by the renderer, which has a valid GL context.
  /// NOTE: If the viewer layout changes, viewer attributes could be changed by the interface thread 
  /// underneath us.  Since there is no way to force Qt to lock the viewer before making these 
  /// changes, we grab the width and height up front and check for validity so that we at least
  /// don't crash later due to a width or height of 0.  The width and height are passed to the
  /// tools and should be used instead of querying the viewer directly.
  virtual void redraw( size_t viewer_id, const Core::Matrix& proj_mat, 
    int viewer_width, int viewer_height );

  /// HAS_2D_VISUAL:
  /// Returns true if the tool draws itself in the 2D view, otherwise false.
  /// The default implementation returns false.
  virtual bool has_2d_visual();

  /// HAS_3D_VISUAL:
  /// Returns true if the tool draws itself in the volume view, otherwise false.
  /// The default implementation returns false.
  virtual bool has_3d_visual();

  // -- close tool --
public:
  /// CLOSE:
  /// This function is called when the application closes the tool. It is meant
  /// to disconnect all connections. Since close tool is called synchronously
  /// on the application thread, it can clean out most of the underlying
  /// connections safely.
  /// NOTE: since the tool handle is given to the user interface, the user
  /// interface thread may issue the final destruction of the class. Hence all
  /// thread critical pieces should be done by this function.
  virtual void close();

  // -- activate/deactivate --
public:
  /// ACTIVATE:
  /// Activate a tool: this tool is set as the active tool and hence it should
  /// setup the right mouse tools in the viewers.
  virtual void activate();

  /// DEACTIVATE:
  /// Deactivate a tool. A tool is always deactivate before the next one is
  /// activated.
  virtual void deactivate();

  // -- execute --
public:
  /// EXECUTE:
  /// Fire off the action that executes the filter
  virtual void execute( Core::ActionContextHandle context );

  // -- internals --
private:
  ToolPrivateHandle private_;

public:

  const static std::string NONE_OPTION_C;
};


#define SEG3D_TOOL_NAME( name, description ) \
"<tool name=\"" name "\">" description "</tool>"

#define SEG3D_TOOL_MENULABEL( name ) \
"<menulabel>" name "</menulabel>"

#define SEG3D_TOOL_MENU( name ) \
"<menu>" name "</menu>"

#define SEG3D_TOOL_URL( urlname ) \
"<url>" urlname "</url>"

#define SEG3D_TOOL_SHORTCUT_KEY( key ) \
"<shortcutkey>" key "</shortcutkey>"

#define SEG3D_TOOL_HOTKEYS( hotkeys ) \
"<hotkeys>" hotkeys "</hotkeys>"

#define SEG3D_TOOL_VERSION( version ) \
"<version>" version "</version>"

#define SEG3D_LARGE_VOLUME_TOOL( is_large_volume_tool ) \
"<largevol>" is_large_volume_tool "</largevol>"

#define SEG3D_TOOL( definition_string ) \
public: \
  static std::string Definition() { return GetToolInfo()->get_definition(); }\
    static std::string MenuLabel() { return GetToolInfo()->get_menu_label(); } \
    static std::string Menu() { return GetToolInfo()->get_menu(); } \
    static std::string Name() { return GetToolInfo()->get_name(); } \
    static std::string ShortcutKey() { return GetToolInfo()->get_shortcut_key(); } \
    static std::string Url() { return GetToolInfo()->get_url(); } \
    static std::string HotKeys() { return GetToolInfo()->get_hotkeys(); } \
    static int Version() { return GetToolInfo()->get_version(); } \
    static bool IsLargeVolume() { return GetToolInfo()->get_is_large_volume(); } \
  static Seg3D::ToolInfoHandle GetToolInfo() \
  {\
    static bool initialized = false; \
    static Seg3D::ToolInfoHandle info; \
    if ( !initialized ) \
    {\
      {\
        Seg3D::ToolInfo::lock_type lock( Seg3D::ToolInfo::GetMutex() );\
        std::string definition = std::string( "<?xml version=\"1.0\"?>\n" definition_string "\n" ); \
        if ( !info ) info = Seg3D::ToolInfoHandle( new Seg3D::ToolInfo( definition ) ); \
      }\
      {\
        Seg3D::ToolInfo::lock_type lock( Seg3D::ToolInfo::GetMutex() );\
        initialized = true;\
      }\
    }\
    return info;\
  } \
  \
  virtual Seg3D::ToolInfoHandle get_tool_info() const { return GetToolInfo(); } \
  virtual int get_version() { return GetToolInfo()->get_version(); }

} // end namespace Seg3D

#endif
