/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#if defined( __APPLE__ )  
#include <CoreServices/CoreServices.h>
#endif

// Boost includes
#include <boost/filesystem.hpp>

// Qt includes
#include <QtGui/QMenuBar>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtCore/QProcess>

// Core includes
#include <Core/State/State.h>
#include <Core/State/Actions/ActionSet.h>

//  Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/Layer/LayerManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ToolManager/Actions/ActionOpenTool.h>
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/ProjectManager/Actions/ActionSaveSession.h>
#include <Application/ProjectManager/Actions/ActionLoadProject.h>
#include <Application/Tools/Actions/ActionCopy.h>
#include <Application/Tools/Actions/ActionPaste.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/UndoBuffer/Actions/ActionRedo.h>
#include <Application/UndoBuffer/Actions/ActionUndo.h>

// QtUtils includes
#include <QtUtils/Utils/QtPointer.h>
#include <QtUtils/Bridge/QtBridge.h>

// Interface includes
#include <Interface/Application/ApplicationInterface.h>
#include <Interface/Application/LayerIOFunctions.h>
#include <Interface/Application/Menu.h>
#include <Interface/Application/ProjectWizard.h>
#include <Interface/Application/SaveProjectAsWizard.h>
#include <Interface/Application/ViewAction.h>
#include <Interface/Application/ViewerInterface.h>


namespace Seg3D
{

Menu::Menu( QMainWindow* parent ) :
  QObject( parent ),
  main_window_(parent)
{
  // Obtain the menubar from the main widget
  QMenuBar* menubar = parent->menuBar();

  // Create all the menus
  this->create_file_menu( menubar );  
  this->create_edit_menu( menubar );
  this->create_view_menu( menubar );
  this->create_tool_menus( menubar );
  this->create_window_menu( menubar );
  this->create_help_menu( menubar );
  
  // Setup all the connections for menus
  // NOTE: These connections and updates need to be done while no other thread can update the
  // state engine. As soon as the tags are updated and connections are in place the system
  // will synchronize properly.
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    
    // Update the tags in the undo and redo buffers
    this->update_undo_tag( UndoBuffer::Instance()->get_undo_tag() );
    this->update_redo_tag( UndoBuffer::Instance()->get_redo_tag() );

    // Update to the most recent list
    this->set_recent_file_list(); 

    // Ensure we have the right state
    bool mask_layer_found = false;
    bool mask_isosurface_found = false;
    auto result = FindMaskLayer();
    std::tie(mask_layer_found, mask_isosurface_found) = result;
    bool active_data_layer_found = FindActiveLayer();

    // Check what type of layer is active
    this->enable_disable_mask_actions( mask_layer_found );
    this->enable_disable_isosurface_actions( mask_isosurface_found );
    this->enable_disable_data_layer_actions( active_data_layer_found );
    this->show_hide_large_volume_actions( PreferencesManager::Instance()->enable_large_volume_state_->get() );

    // Automatically update the recent file list in the menu
    this->add_connection( ProjectManager::Instance()->recent_projects_changed_signal_.connect( 
      boost::bind( &Menu::SetRecentFileList, qpointer_type( this ) ) ) );
      
    // Automatically switch on exporting segmentations, depending on the choice of 
    // project or active layer. 
    this->add_connection( ProjectManager::Instance()->current_project_changed_signal_.connect( 
      boost::bind( &Menu::EnableDisableLayerActions, qpointer_type( this ) ) ) ); 
    
    this->add_connection( LayerManager::Instance()->layers_changed_signal_.connect( 
      boost::bind( &Menu::EnableDisableLayerActions, qpointer_type( this ) ) ) );
  
    this->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect( 
      boost::bind( &Menu::EnableDisableLayerActions, qpointer_type( this ) ) ) );

    this->add_connection( LayerManager::Instance()->layer_data_changed_signal_.connect( 
      boost::bind( &Menu::EnableDisableLayerActions, qpointer_type( this ) ) ) );

    this->add_connection( LayerManager::Instance()->mask_layer_isosurface_created_signal_.connect(
      boost::bind( &Menu::EnableDisableLayerActions, qpointer_type( this ) ) ) );

    this->add_connection( LayerManager::Instance()->mask_layer_isosurface_deleted_signal_.connect(
      boost::bind( &Menu::EnableDisableLayerActions, qpointer_type( this ) ) ) );

    this->add_connection( PreferencesManager::Instance()->enable_large_volume_state_->state_changed_signal_.connect(
      boost::bind( &Menu::ShowHideLargeVolume, qpointer_type( this ) ) ) );

    // Automatically update the tag in the undo menu    
    this->add_connection( UndoBuffer::Instance()->update_undo_tag_signal_.connect(
      boost::bind( &Menu::UpdateUndoTag, qpointer_type( this ), _1 ) ) ); 

    // Automatically update the tag in the redo menu    
    this->add_connection( UndoBuffer::Instance()->update_redo_tag_signal_.connect(
      boost::bind( &Menu::UpdateRedoTag, qpointer_type( this ), _1 ) ) ); 
      
  } 
}

Menu::~Menu()
{
}

void Menu::create_file_menu( QMenuBar* menubar )
{
  QMenu* qmenu = menubar->addMenu( tr( "&File" ) );

  // == New Project ==
  QAction* qaction;

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    bool project_creation = InterfaceManager::Instance()->enable_project_creation_state_->get();

    qaction = qmenu->addAction( tr( "&New Project" ) );
    qaction->setShortcut( tr( "Ctrl+N" ) );
    qaction->setToolTip( tr( "Start a new project." ) );
    qaction->setEnabled( project_creation );
    connect( qaction, SIGNAL( triggered() ), this, SLOT( new_project() ) );
  }
  
  // == Open Project ==
  qaction = qmenu->addAction( tr( "&Open Project" ) );
  qaction->setShortcut( QKeySequence::Open );
  qaction->setToolTip( tr( "Open an existing project" ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( open_project() ) );
  
  // == Show Project Folder ==
  // NOTE: For now this functionality is only available on Mac and Windows
  // NOTE: For Linux we need to figure out how to launch the explorer
#if defined( _WIN32 ) || defined ( __APPLE__ )  
  qaction = qmenu->addAction( tr( "Show Project Folder" ) );
  qaction->setShortcut( tr( "Ctrl+Shift+F" ) );
  qaction->setToolTip( tr( "Open the current project folder" ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( open_project_folder() ) );
#endif
  
  // == Save Project ==
  qaction = qmenu->addAction( tr( "Save Project" ) );
  qaction->setShortcut( tr( "Ctrl+S" ) );
  qaction->setToolTip( tr( "Save the current project." ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( save_project() ) );
  
  // == Save Project As ... ==  
  qaction = qmenu->addAction( tr( "Save Project As..." ) );
  qaction->setShortcut( tr( "Ctrl+Shift+S" ) );
  qaction->setToolTip( tr( "Save the current project as..." ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( save_project_as() ) ); 
  
  // == Launch Another Copy of Seg3D ==
  // NOTE: This is Mac only functionality, as it is no problem on other platforms.
  // NOTE: The Mac only allows the program to run once from the dock, hence this is
  // a hack around that limit.
#if defined( __APPLE__ )  
  std::string menu_label = std::string( "Launch Another Copy of " ) + 
    Core::Application::GetApplicationName();
  std::string menu_tooltip = std::string( "Open another copy of " ) +
    Core::Application::GetApplicationName();
    
  qaction = qmenu->addAction( QString::fromStdString( menu_label ) );
  qaction->setToolTip( QString::fromStdString( menu_tooltip ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( mac_open_another_version() ) );
#endif
  
  qmenu->addSeparator();

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    bool file_import = InterfaceManager::Instance()->enable_file_import_state_->get();

    // == Import Layer From Single File... ==
    qaction = qmenu->addAction( tr( "Import Layer From Single File...") );
    qaction->setShortcut( tr( "Ctrl+Shift+O" ) );
    qaction->setToolTip( tr( "Import new layer(s) into the layer manager from a file(s)." ) );
    qaction->setEnabled( file_import );
    QtUtils::QtBridge::Connect( qaction, 
      boost::bind( &LayerIOFunctions::ImportFiles, this->main_window_, "" ) );

    // == Import Layer From Image Series... ==
    qaction = qmenu->addAction( tr( "Import Layer From Image Series...") );
    qaction->setShortcut( tr( "Ctrl+Shift+I" ) );
    qaction->setToolTip( tr( "Import new data layer into the layer manager from a series." ) );
    qaction->setEnabled( file_import );
    QtUtils::QtBridge::Connect( qaction, 
      boost::bind( &LayerIOFunctions::ImportSeries, this->main_window_ ) );


    // == Import Layer From Image Series... ==
    this->import_large_volume_qaction_ = qmenu->addAction( tr( "Import Large Volume File...") );
    this->import_large_volume_qaction_->setShortcut( tr( "Ctrl+Shift+L" ) );
    this->import_large_volume_qaction_->setToolTip( tr( "Import pregenerated large volume file." ) );
    this->import_large_volume_qaction_->setEnabled( file_import );
    this->import_large_volume_qaction_->setVisible( false );
    QtUtils::QtBridge::Connect( this->import_large_volume_qaction_, 
      boost::bind( &LayerIOFunctions::ImportLargeVolume, this->main_window_ ) );
  }
  qmenu->addSeparator();

  // == Export Segmentation... ==
  this->export_segmentation_qaction_ = qmenu->addAction( tr( "Export Segmentation..." ) );
  this->export_segmentation_qaction_->setShortcut( tr( "Ctrl+E" ) );
  this->export_segmentation_qaction_->setToolTip( tr( "Export masks as a segmentation." ) );
  QtUtils::QtBridge::Connect( this->export_segmentation_qaction_, 
    boost::bind( &LayerIOFunctions::ExportSegmentation, this->main_window_ ) );
  this->export_segmentation_qaction_->setEnabled( false );

  // == Export Isosurface... ==
  this->export_isosurface_qaction_ = qmenu->addAction( tr( "Export Isosurface..." ) );
  this->export_isosurface_qaction_->setShortcut( tr( "Ctrl+I" ) );
  this->export_isosurface_qaction_->setToolTip( tr( "Export isosurface to file." ) );
  QtUtils::QtBridge::Connect( this->export_isosurface_qaction_,
    boost::bind( &LayerIOFunctions::ExportIsosurface, this->main_window_ ) );
  this->export_isosurface_qaction_->setEnabled( false );

  // == Export Active Data Layer... ==
  this->export_active_data_layer_qaction_ = qmenu->addAction( tr( "Export Active Data Layer...") );
  this->export_active_data_layer_qaction_->setShortcut( tr( "Ctrl+Shift+E" ) );
  this->export_active_data_layer_qaction_->setToolTip( tr( "Export the active data layer to file." ) );
  QtUtils::QtBridge::Connect( this->export_active_data_layer_qaction_, 
    boost::bind( &LayerIOFunctions::ExportLayer, this->main_window_ ) );
  this->export_active_data_layer_qaction_->setEnabled( false );
    
  qmenu->addSeparator();
    
  // == Recent Projects ==
  this->file_menu_recents_ = qmenu->addMenu( tr( "Recent Projects" ) );
  
  qmenu->addSeparator();

  // == Quit ==
  qaction = qmenu->addAction( tr( "Quit" ) );
  qaction->setShortcut( tr( "Ctrl+Q" ) );
  qaction->setToolTip( tr( "Quit application." ) );
  connect( qaction, SIGNAL( triggered() ), this->parent(), SLOT( close() ) );
}

void Menu::create_edit_menu( QMenuBar* menubar )
{
  QMenu* qmenu = menubar->addMenu( tr( "&Edit" ) );
  
  // == Undo ==
  this->undo_qaction_ = qmenu->addAction( tr( "Undo" ) );
  this->undo_qaction_->setShortcut( tr( "Ctrl+Z" ) );
  this->undo_qaction_->setToolTip( tr( "Undo last action that modified the layers" ) );
    QtUtils::QtBridge::Connect( this->undo_qaction_ , boost::bind(
      &ActionUndo::Dispatch, Core::Interface::GetWidgetActionContext() ) );

  // == Redo ==
  this->redo_qaction_ = qmenu->addAction( tr( "Redo" ) );
  this->redo_qaction_->setShortcut( tr( "Shift+Ctrl+Z" ) );
  this->redo_qaction_->setToolTip( tr( "Redo last action that modified the layers" ) );
  QtUtils::QtBridge::Connect( this->redo_qaction_ , boost::bind(
      &ActionRedo::Dispatch, Core::Interface::GetWidgetActionContext() ) );
      
  qmenu->addSeparator();    
  
  // == Copy Mask Slice ==
  this->copy_qaction_ = qmenu->addAction( tr( "Copy Mask Slice" ) );
  this->copy_qaction_->setShortcut( tr( "Ctrl+C" ) );
  this->copy_qaction_->setToolTip( tr( "Copy the current mask slice" ) );
  QtUtils::QtBridge::Connect( this->copy_qaction_, boost::bind( &ActionCopy::Dispatch,
    Core::Interface::GetWidgetActionContext() ) );
  this->copy_qaction_->setEnabled( false );

  // == Paste Mask Slice ==
  this->paste_qaction_ = qmenu->addAction( tr( "Paste Mask Slice" ) );
  this->paste_qaction_->setShortcut( tr( "Ctrl+V" ) );
  this->paste_qaction_->setToolTip( tr( "Paste to the current mask slice" ) );
  QtUtils::QtBridge::Connect( this->paste_qaction_, boost::bind( &ActionPaste::Dispatch,
    Core::Interface::GetWidgetActionContext(), false ) );
  this->paste_qaction_->setEnabled( false );

  // == Punch Through Volume ==
  this->punch_qaction_ = qmenu->addAction( tr( "Punch Through Volume" ) );
  this->punch_qaction_->setShortcut( tr( "Ctrl+P" ) );
  this->punch_qaction_->setToolTip( tr( "Punch the copied mask slice through the volume" ) );
  QtUtils::QtBridge::Connect( this->punch_qaction_, boost::bind( &ActionPaste::Dispatch,
    Core::Interface::GetWidgetActionContext(), true ) );
  this->punch_qaction_->setEnabled( false );

}

void Menu::create_view_menu( QMenuBar* menubar )
{
  QAction* qaction;
  QMenu* qmenu = menubar->addMenu( "&View" );

  // == Full Screen ==
  qaction = qmenu->addAction( "Full Screen" );
  qaction->setShortcut( tr( "Ctrl+F" ) );
  qaction->setToolTip( tr( "Toggle the view between full screen and normal" ) );
  qaction->setCheckable( true );
  QtUtils::QtBridge::Connect( qaction, InterfaceManager::Instance()->full_screen_state_ );

  qmenu->addSeparator();

  // == Only One Viewer ==
  qaction = qmenu->addAction( tr( "Only One Viewer" ) );
  qaction->setShortcut( tr( "ALT+0" ) );
  qaction->setToolTip( tr( "Set the view to one large view" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::DispatchState< Core::StateOption >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, ViewerManager::VIEW_SINGLE_C ) );

  // == One and One ==
  qaction = qmenu->addAction( tr( "One and One" ) );
  qaction->setShortcut( tr( "ALT+1" ) );
  qaction->setToolTip( tr( "Set the view to two large views" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, ViewerManager::VIEW_1AND1_C ) );

  // == One and Two ==
  qaction = qmenu->addAction( tr( "One and Two" ) );
  qaction->setShortcut( tr( "ALT+2" ) );
  qaction->setToolTip( tr( "Set the view one large and two smaller views" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, ViewerManager::VIEW_1AND2_C ) );

  // == One and Three ==
  qaction = qmenu->addAction( tr( "One and Three" ) );
  qaction->setShortcut( tr( "ALT+3" ) );
  qaction->setToolTip( tr( "Set the view one large and three smaller views" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, ViewerManager::VIEW_1AND3_C ) );

  // == Two and Two ==
  qaction = qmenu->addAction( tr( "Two and Two" ) );
  qaction->setShortcut( tr( "ALT+4" ) );
  qaction->setToolTip( tr( "Set the view one large and three smaller views" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, ViewerManager::VIEW_2AND2_C ) );

  // == Two and Three ==
  qaction = qmenu->addAction( tr( "Two and Three" ) );
  qaction->setShortcut( tr( "ALT+5" ) );
  qaction->setToolTip( tr( "Set the view two larger and three smaller views" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, ViewerManager::VIEW_2AND3_C ) );

  // == Three and Three ==
  qaction = qmenu->addAction( tr( "Three and Three" ) );
  qaction->setShortcut( tr( "ALT+6" ) );
  qaction->setToolTip( tr( "Set the view to 6 equally sized views" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, ViewerManager::VIEW_3AND3_C ) );
}

void Menu::create_tool_menus( QMenuBar* qmenubar )
{
  ToolMenuList menu_list;

  // Get all the different tool menus
  ToolFactory::Instance()->list_menus( menu_list );

  // Build each of the tool menus
  for (auto &menu : menu_list)
  {
    QMenu* qmenu = qmenubar->addMenu( QString::fromUtf8( "&" ) + QString::fromStdString( menu ) );
    ToolInfoList tool_types_list;

    ToolFactory::Instance()->list_tools( tool_types_list, menu );

    int largeVolCounter = 0;
    // Loop through all the tools
    QAction* qaction;
    for (auto &toolInfo: tool_types_list)
    {
      // Add menu option to open tool
      qaction = qmenu->addAction( QString::fromStdString( toolInfo->get_menu_label() ) );
      // qaction->setShortcut( QString::fromStdString( toolInfo->get_shortcut_key() ) );
      if ( toolInfo->get_is_large_volume() )
      {
        large_volume_tools_.push_back( qaction );
        qaction->setVisible( false );
        ++largeVolCounter;
      }

      // Connect the action with dispatching a command in the ToolManager
      QtUtils::QtBridge::Connect( qaction, boost::bind( &ActionOpenTool::Dispatch,
        Core::Interface::GetWidgetActionContext(), toolInfo->get_name() ) );
    }

    // menu only contains large volume tools
    if (tool_types_list.size() == largeVolCounter)
    {
      qmenu->menuAction()->setVisible( false );
      large_volume_tools_.push_back( qmenu->menuAction() );
    }
  }
}

void Menu::create_window_menu( QMenuBar* menubar )
{
  QAction* qaction;

  QMenu* qmenu = menubar->addMenu( "&Window" );

  // == Project Window ==
  qaction = qmenu->addAction( "Project Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+P" ) );
  qaction->setCheckable( true );
  QtUtils::QtBridge::Connect( qaction, 
    InterfaceManager::Instance()->project_dockwidget_visibility_state_ );

//  // History Widget
//  qaction = qmenu->addAction( "History Widget" );
//  qaction->setShortcut( tr( "Ctrl+Shift+H" ) );
//  qaction->setCheckable( true );
//  QtUtils::QtBridge::Connect( qaction, 
//    InterfaceManager::Instance()->history_dockwidget_visibility_state_ );

  // == Tools Window ==
  qaction = qmenu->addAction( "Tools Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+T" ) );
  qaction->setCheckable( true );
  QtUtils::QtBridge::Connect( qaction, 
    InterfaceManager::Instance()->toolmanager_dockwidget_visibility_state_ );

  // == Layer Manager Window ==
  qaction = qmenu->addAction( "Layer Manager Window" );
  qaction->setCheckable( true );
  qaction->setShortcut( tr( "Ctrl+Shift+L" ) );
  QtUtils::QtBridge::Connect( qaction, 
    InterfaceManager::Instance()->layermanager_dockwidget_visibility_state_ );
  
  // == Volume View Window ==
  qaction = qmenu->addAction( "Volume View Window");
  qaction->setCheckable( true );
  qaction->setShortcut( tr( "Ctrl+Shift+V" ) );
  QtUtils::QtBridge::Connect( qaction,
    InterfaceManager::Instance()->rendering_dockwidget_visibility_state_ );

  // Provenance Widget
  qaction = qmenu->addAction( "Provenance Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+H" ) );
  qaction->setCheckable( true );
  QtUtils::QtBridge::Connect( qaction, 
    InterfaceManager::Instance()->provenance_dockwidget_visibility_state_ );

  qmenu->addSeparator();

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    if ( InterfaceManager::Instance()->enable_controller_state_->get() )
    {
      // == Controller Window ==
      qaction = qmenu->addAction( "Controller Window" );
      qaction->setShortcut( tr( "Ctrl+Shift+C" ) );
      qaction->setCheckable( true );
      QtUtils::QtBridge::Connect( qaction, 
        InterfaceManager::Instance()->controller_visibility_state_ );
    }
  }
  
  // == Preferences Window ==
  // NOTE: On the Mac Qt will move preferences to the main program menu
  qaction = qmenu->addAction( "Preferences Window" );
  qaction->setShortcut( tr( "Ctrl+," ) );
  qaction->setCheckable( true );
  QtUtils::QtBridge::Connect( qaction, 
    InterfaceManager::Instance()->preferences_manager_visibility_state_ );

  // Python Console
#ifdef BUILD_WITH_PYTHON
  qaction = qmenu->addAction( "Python Console" );
  qaction->setShortcut( tr( "Ctrl+Shift+Y" ) );
  qaction->setCheckable( true );
  QtUtils::QtBridge::Connect( qaction, 
    InterfaceManager::Instance()->python_console_visibility_state_ );
#endif
}
  
void Menu::create_help_menu( QMenuBar* menubar )
{
  QAction* qaction ;

  QMenu* qmenu = menubar->addMenu( "&Help" );

  // == About ==
  qaction = qmenu->addAction( tr( "&About" ) );
  std::string about = std::string( "About " ) + 
    Core::Application::GetApplicationNameAndVersion();
  qaction->setToolTip( QString::fromStdString( about ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( about() ) );
  
  // == Keyboard Shortcuts ==
  qaction = qmenu->addAction( tr( "&Keyboard Shortcuts" ) ); 
  qaction->setToolTip( QString( "List of the keyboard shortcuts or 'hotkeys' for " ) + 
    QString::fromStdString( Core::Application::GetApplicationNameAndVersion() ) );
  qaction->setShortcut( tr( "SHIFT+CTRL+ALT+K" ) );
  qaction->setCheckable( true );
  QtUtils::QtBridge::Connect( qaction, 
    InterfaceManager::Instance()->keyboard_shortcut_visibility_state_ );
}
  
void Menu::about()
{
  std::string about = std::string( "About " ) + 
    Core::Application::GetApplicationNameAndVersion();

  QMessageBox::about( this->main_window_, QString::fromStdString( about ), 
    QString( "<h3>" ) + 
    QString::fromStdString( Core::Application::GetApplicationNameAndVersion() ) +
    QString( "</h3>" ) +
    QString::fromStdString( Core::Application::GetAbout() ) );
}

void Menu::new_project()
{
  // Check whether we need to save the current project
  ProjectHandle current_project = ProjectManager::Instance()->get_current_project();
  
  // Project needs to exists and it needs to have unsaved information
  if ( current_project && current_project->check_project_changed() )
  {
    // Check whether the users wants to save the current project

    int ret = QMessageBox::warning( this->main_window_, "Create a new Project?",
      "Your current session has not been saved.\n"
      "Do you want to save your changes?",
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );

    // The user selected to save the current session
    if ( ret == QMessageBox::Save )
    {
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
      if ( current_project->project_files_generated_state_->get() == false ||
        current_project->project_files_accessible_state_->get() == false )
      {
        // We actually need to do a save as. Hence call that function
        this->save_project_as();
      }
      else
      {
        ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), "" ); 
      }
    }

    if ( ret == QMessageBox::Cancel ) return;
  }

  // Let the project wizard do the rest of the logic
  ProjectWizard* new_project_wizard_ = new ProjectWizard( this->main_window_);
  new_project_wizard_->exec();
}


void Menu::open_project()
{
  // Check whether we need to save the current project
  ProjectHandle current_project = ProjectManager::Instance()->get_current_project();
  
  // Project needs to exists and it needs to have unsaved information
  if ( current_project && current_project->check_project_changed() )
  {
    // Check whether the users wants to save the current project

    int ret = QMessageBox::warning( this->main_window_, "Open a different Project?",
      "Your current session has not been saved.\n"
      "Do you want to save your changes?",
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );

    // The user selected to save the current session
    if ( ret == QMessageBox::Save )
    {
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
      if ( current_project->project_files_generated_state_->get() == false ||
        current_project->project_files_accessible_state_->get() == false )
      {
        // We actually need to do a save as. Hence call that function
        this->save_project_as();
      }
      else
      {
        ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), "" ); 
      }
    }

    if ( ret == QMessageBox::Cancel ) return;
  }

  // The user did not press cancel, so proceed with opening the next file
  boost::filesystem::path current_projects_path = boost::filesystem::absolute( 
    boost::filesystem::path( ProjectManager::Instance()-> get_current_project_folder() ) );
    
    
  std::string project_type = std::string( "Open " ) + 
    Core::Application::GetApplicationName() + " Project";
    
  std::vector<std::string> project_file_extensions = Project::GetProjectFileExtensions(); 
  std::vector<std::string> project_path_extensions = Project::GetProjectPathExtensions(); 
  std::string project_file_type =  Core::Application::GetApplicationName() + " Project File (";
  
  for ( size_t j = 0; j < project_file_extensions.size(); j++ )
  {
    project_file_type += std::string( " *" ) + project_file_extensions[ j ];
  }

  for ( size_t j = 0; j < project_path_extensions.size(); j++ )
  {
    project_file_type += std::string( " *" ) + project_path_extensions[ j ];
  }

  project_file_type += " )";

    boost::filesystem::path full_path;
    
#if defined(__APPLE__) || defined(_WIN32)
  full_path =  boost::filesystem::path( ( 
    QFileDialog::getOpenFileName ( this->main_window_, 
    QString::fromStdString( project_type ), 
    QString::fromStdString( current_projects_path.string() ), 
    QString::fromStdString( project_file_type ) ) ).toStdString() ); 

#else
    // It seems Ubuntus Qt4 version is broken and its dialog tends to crash
    // Hence use the other way of defining a dialog
    QFileDialog* diag = new QFileDialog( this->main_window_,
        QString::fromStdString( project_type ), 
        QString::fromStdString( current_projects_path.string() ), 
        QString::fromStdString( project_file_type ) );
    diag->setFileMode(QFileDialog::ExistingFile);
    diag->exec();
    
    QList<QString> files = diag->selectedFiles();
    if ( files.count() )
    {
        full_path =  boost::filesystem::path( files[ 0 ].toStdString() );
    }
    delete diag;
#endif




  bool is_path_extension = false;
  for ( size_t j = 0; j < project_path_extensions.size(); j++ )
  {
    if ( boost::filesystem::extension( full_path ) == project_path_extensions[ j ] )
    {
      is_path_extension = true;
      break;
    }
  }


  if ( is_path_extension )
  {
    bool found_s3d_file = false;
    
    if ( boost::filesystem::is_directory( full_path ) )
    {
      boost::filesystem::directory_iterator dir_end;
      for( boost::filesystem::directory_iterator dir_itr( full_path ); 
        dir_itr != dir_end; ++dir_itr )
      {
        std::string filename = dir_itr->path().filename().string();
        boost::filesystem::path dir_file = full_path / filename;
        for ( size_t j = 0; j < project_file_extensions.size(); j++ )
        {
          if ( boost::filesystem::extension( dir_file ) ==project_file_extensions[ j ] )
          {
            full_path = dir_file;
            found_s3d_file = true;
            break;
          }
        }
        
        if ( found_s3d_file ) break;
      }
    }
    
    if ( !found_s3d_file )
    {
      QMessageBox::critical( this->main_window_, 
        "Error reading project file",
        "Error reading project file:\n"
        "The project file is incomplete." );
      return;   
    }
  }


  if( boost::filesystem::exists( full_path ) )
  { 
        std::string error;
    if ( ! ProjectManager::CheckProjectFile( full_path, error ) )
    {
      QMessageBox::critical( this->main_window_, 
        "Error reading project file",
        QString::fromStdString( error ) );
      return;
    }

    ActionLoadProject::Dispatch( Core::Interface::GetWidgetActionContext(), 
      full_path.string() );
  }
}


void Menu::open_project_folder()
{
  ProjectHandle current_project = ProjectManager::Instance()->get_current_project();

  // If the project has not been saved yet, we cannot go to the project directory
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  if ( current_project->project_files_generated_state_->get() == false )
  {
    QMessageBox::critical( this->main_window_, 
      "Error open project folder",
      "Error open project folder:\n"
      "The project file has not yet been saved to disk.\nChoose 'save as' to save the project." );
    return;
  }
  
  boost::filesystem::path project_path = 
    boost::filesystem::path( current_project->project_path_state_->get() );
  
  try
  {
    project_path = boost::filesystem::absolute( project_path );
  }
  catch ( ... )
  {
  }


  if( boost::filesystem::exists( project_path ) )
  {
    QString qstring_path = QString::fromStdString( project_path.string() );

#ifdef __APPLE__

    bool is_bundle = false;

    // MAC CODE
    try
    {
      FSRef file_ref;
      Boolean is_directory;
      FSPathMakeRef( reinterpret_cast<const unsigned char*>( 
        project_path.string().c_str() ), &file_ref, &is_directory );
      FSCatalogInfo info;
      FSGetCatalogInfo( &file_ref, kFSCatInfoFinderInfo, &info, 0, 0, 0 );
      
      FileInfo&  finder_info = *reinterpret_cast<FileInfo*>( &info.finderInfo );
      if ( finder_info.finderFlags & kHasBundle ) is_bundle = true;
    }
    catch( ... )
    {
    }

    if ( is_bundle )
    {
      qstring_path = QString::fromStdString( project_path.parent_path().string() );
    }
    
#endif

    QProcess process;
    process.setReadChannelMode( QProcess::MergedChannels );
    
#ifdef _WIN32
    qstring_path = qstring_path.replace( QString( "/" ), QString( "\\" ) );
    process.start( "explorer.exe", QStringList() << qstring_path );
#else
    process.start( "open", QStringList() << qstring_path );
#endif

    if( !process.waitForFinished() )
    { 
      std::string error = std::string( "Could not open project: " ) + 
        process.errorString().toStdString() + ".";
      CORE_LOG_ERROR( error );
    } 
    return;
  }
  
  CORE_LOG_ERROR( "The current project path seems to be invalid." );
}

void Menu::set_recent_file_list()
{
  QAction* qaction = 0;
  this->file_menu_recents_->clear();
  
  ProjectInfoList recent_projects;
  ProjectManager::Instance()->get_recent_projects( recent_projects );

  for( size_t i = 0; i < recent_projects.size(); ++i )
  {
    qaction = file_menu_recents_->addAction(
      QString::fromStdString( recent_projects[ i ].name() ) );
    qaction->setToolTip( tr( "Load this project" ) );
    QtUtils::QtBridge::Connect( qaction, boost::bind( &Menu::ConfirmRecentFileLoad,
      qpointer_type( this ), recent_projects[ i ].path().string() ) );
  }
}

void Menu::ConfirmRecentFileLoad( qpointer_type qpointer, const std::string& path )
{
  // Check whether we need to save the current project
  ProjectHandle current_project = ProjectManager::Instance()->get_current_project();
  
  // Project needs to exists and it needs to have unsaved information
  if ( current_project && current_project->check_project_changed() )
  {
    // Check whether the users wants to save the current project

    int ret = QMessageBox::warning( qpointer->main_window_, "Open a different Project?",
      "Your current session has not been saved.\n"
      "Do you want to save your changes?",
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );

    // The user selected to save the current session
    if ( ret == QMessageBox::Save )
    {
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
      if ( current_project->project_files_generated_state_->get() == false ||
        current_project->project_files_accessible_state_->get() == false )
      {
        // We actually need to do a save as. Hence call that function
        qpointer->save_project_as();
      }
      else
      {
        ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), "" ); 
      }
    }

    if ( ret == QMessageBox::Cancel ) return;
  }
  
  ActionLoadProject::Dispatch( Core::Interface::GetWidgetActionContext(), path );
}

void Menu::enable_disable_mask_actions( bool mask_layer_found )
{
  this->export_segmentation_qaction_->setEnabled( mask_layer_found );
  this->copy_qaction_->setEnabled( mask_layer_found );
  this->paste_qaction_->setEnabled( mask_layer_found );
  this->punch_qaction_->setEnabled( mask_layer_found );
}

void Menu::enable_disable_isosurface_actions( bool mask_isosurface_found )
{
  this->export_isosurface_qaction_->setEnabled( mask_isosurface_found );
}

void Menu::enable_disable_data_layer_actions( bool active_data_layer_found )
{
  this->export_active_data_layer_qaction_->setEnabled( active_data_layer_found );
}

void Menu::show_hide_large_volume_actions( bool large_volume_visible )
{
  this->import_large_volume_qaction_->setVisible( large_volume_visible );
  for (QAction* a: large_volume_tools_)
  {
    a->setVisible( large_volume_visible );
  }
}

void Menu::save_project()
{
  ProjectHandle current_project = ProjectManager::Instance()->get_current_project();
  
  // Need to lock state engine as we need query state properties of the current project.
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  if ( current_project->project_files_generated_state_->get() == false ||
    current_project->project_files_accessible_state_->get() == false )
  {
    // We actually need to do a save as. Hence call that function
    this->save_project_as();
  }
  else
  {
    ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), "" ); 
  }
}

void Menu::save_project_as()
{
  // Run the wizard that guides one through the save as procedure
  SaveProjectAsWizard* save_project_as_wizard_ = new SaveProjectAsWizard( this->main_window_ );
  save_project_as_wizard_->exec();
}

void Menu::mac_open_another_version()
{
// NOTE: On the mac one can normally open an application only once.
// Hence we add the logic to open another version directly in the menu
// of Seg3D

#if defined( __APPLE__ )
  boost::filesystem::path app_filepath;
  Core::Application::Instance()->get_application_filepath( app_filepath );

  std::string command = std::string( "open -n " ) + 
    app_filepath.parent_path().parent_path().string() + " &";
  
  system( command.c_str() );

#endif
}



void Menu::SetRecentFileList( qpointer_type qpointer )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &Menu::set_recent_file_list, qpointer.data() ) ) );
}

void Menu::ShowHideLargeVolume( qpointer_type qpointer )
{
  // This function should be called from the application thread
  ASSERT_IS_APPLICATION_THREAD();

  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &Menu::show_hide_large_volume_actions, qpointer.data(), PreferencesManager::Instance()->enable_large_volume_state_->get() ) ) );
}

void Menu::EnableDisableLayerActions( qpointer_type qpointer )
{
  // This function should be called from the application thread
  ASSERT_IS_APPLICATION_THREAD();

  bool mask_layer_found = false;
  bool mask_isosurface_found = false;
  auto result = FindMaskLayer();
  std::tie(mask_layer_found, mask_isosurface_found) = result;
  bool active_data_layer_found = FindActiveLayer();

  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &Menu::enable_disable_mask_actions, qpointer.data(), mask_layer_found ) ) );

  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &Menu::enable_disable_isosurface_actions, qpointer.data(), mask_isosurface_found ) ) );

  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &Menu::enable_disable_data_layer_actions, qpointer.data(), active_data_layer_found ) ) );
}


void Menu::UpdateUndoTag( qpointer_type qpointer, std::string tag )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &Menu::update_undo_tag, qpointer.data(), tag ) ) );
}

void Menu::update_undo_tag( std::string tag )
{
  if ( tag == "" )
  {
    this->undo_qaction_->setEnabled( false );
    this->undo_qaction_->setText( "Undo" );
  }
  else
  {
    this->undo_qaction_->setEnabled( true );
    QString text = QString( "Undo " ) + QString::fromStdString( tag );
    this->undo_qaction_->setText( text );   
  }   
}

void Menu::UpdateRedoTag( qpointer_type qpointer, std::string tag )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &Menu::update_redo_tag, qpointer.data(), tag ) ) );
}

void Menu::update_redo_tag( std::string tag )
{
  if ( tag == "" )
  {
    this->redo_qaction_->setEnabled( false );
    this->redo_qaction_->setText( "Redo" );
  }
  else
  {
    this->redo_qaction_->setEnabled( true );
    QString text = QString( "Redo " ) + QString::fromStdString( tag );
    this->redo_qaction_->setText( text );   
  }
}   

bool Menu::FindActiveLayer()
{
  LayerHandle layer = LayerManager::Instance()->get_active_layer();
  if ( layer )
  {
    if ( layer->get_type() == Core::VolumeType::DATA_E &&
         layer->data_state_->get() != Layer::CREATING_C &&
         layer->data_state_->get() != Layer::PROCESSING_C )
    {
      return true;
    }
  }
  return false;
}

std::tuple<bool, bool> Menu::FindMaskLayer()
{
  bool mask_layer_found = false;
  bool mask_isosurface_found = false;

  std::vector< LayerHandle > layer_list;
  LayerManager::Instance()->get_layers( layer_list );
  for( size_t i = 0; i < layer_list.size(); ++i )
  {
    if ( layer_list[ i ]->get_type() == Core::VolumeType::MASK_E &&
         layer_list[ i ]->data_state_->get() != Layer::CREATING_C &&
         layer_list[ i ]->data_state_->get() != Layer::PROCESSING_C )
    {
      // just need one valid mask layer
      mask_layer_found = true;
      break;
    }
  }

  LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
  if ( active_layer )
  {
    MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( active_layer.get() );
    if ( mask_layer && mask_layer->iso_generated_state_->get() )
    {
      mask_isosurface_found = true;
    }
  }
  return std::tuple<bool, bool>(mask_layer_found, mask_isosurface_found);
}

} // end namespace Seg3D
