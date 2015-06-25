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

// Core includes
#include <Core/Interface/Interface.h>
#include <Core/Utils/Log.h>

// Qt includes
#include <QtGui/QFileDialog>

// Qt Gui Includes
#include <QtUtils/Bridge/QtBridge.h>

// Interface Includes
#include <Interface/ToolInterface/PointsSelectToolInterface.h>
#include "ui_PointsSelectToolInterface.h"

// Application Includes
#include <Application/Layer/LayerManager.h>
#include <Application/Tools/PointsSelectTool.h>
#include <Application/ViewerManager/Actions/ActionPickPoint.h>
#include <Application/LayerIO/Actions/ActionExportPoints.h>
#include <Application/ProjectManager/ProjectManager.h>

// Core Includes
#include <Core/State/Actions/ActionSetAt.h>


SCI_REGISTER_TOOLINTERFACE( Seg3D, PointsSelectToolInterface )

namespace Seg3D
{

class PointsSelectToolInterfacePrivate
{
public:
  Ui::PointsSelectToolInterface ui_;
  PointsSelectToolInterface* interface_;

  void export_points_to_file() const;
};

void PointsSelectToolInterfacePrivate::export_points_to_file() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  PointsSelectTool* tool = dynamic_cast< PointsSelectTool* > ( this->interface_->tool().get() );

  QString filename;
  boost::filesystem::path current_folder = ProjectManager::Instance()->get_current_file_folder();
  std::string file_selector = Core::StringToUpper( "Text File " ) + "(*.txt)";
  
  filename = QFileDialog::getSaveFileName( this->interface_,
                                           "Export Points As...",
                                           current_folder.string().c_str(),
                                           QString::fromStdString( file_selector ) );
  if (! filename.isNull() && ! filename.isEmpty() )
  {
    QFileInfo file(filename);
    if ( file.suffix().isEmpty() ) 
    {
      filename += ".txt";
    }

    if ( tool->use_world_units_state_->get() )
    {
      ActionExportPoints::Dispatch( Core::Interface::GetWidgetActionContext(),
                                    filename.toStdString(),
                                    tool->seed_points_state_->get() );
    }
    else
    {
      ActionExportPoints::Dispatch( Core::Interface::GetWidgetActionContext(),
                                   filename.toStdString(),
                                   tool->seed_points_index_state_->get() );
    }
  }
}

// constructor
PointsSelectToolInterface::PointsSelectToolInterface() :
  private_( new PointsSelectToolInterfacePrivate )
{
  this->private_->interface_ = this;
}

// destructor
PointsSelectToolInterface::~PointsSelectToolInterface()
{
  this->disconnect_all();
}

// build the interface and connect it to the state manager
bool
PointsSelectToolInterface::build_widget( QFrame* frame )
{
  // Build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

  this->private_->ui_.points_table_->setRowCount(0);
  this->private_->ui_.points_table_->setColumnCount(3);
  QStringList tableHeader;
  tableHeader << "X" << "Y" << "Z";
  this->private_->ui_.points_table_->setHorizontalHeaderLabels(tableHeader);

  // Get a pointer to the tool
  PointsSelectTool* tool = dynamic_cast< PointsSelectTool* > ( this->tool().get() );

  qpointer_type points_select_interface( this );

  tool->seed_points_state_->value_changed_signal_.connect(
    boost::bind( &PointsSelectToolInterface::UpdateTable, points_select_interface ) );
  tool->seed_points_index_state_->value_changed_signal_.connect(
    boost::bind( &PointsSelectToolInterface::UpdateTable, points_select_interface ) );
  tool->units_changed_signal_.connect(
    boost::bind( &PointsSelectToolInterface::UpdateTable, points_select_interface ) );
  PointsSelectToolInterface::UpdateTable(points_select_interface);

  tool->seed_points_state_->value_changed_signal_.connect(
    boost::bind( &PointsSelectToolInterface::ToggleSaveButtonEnabled, points_select_interface ) );

  QButtonGroup* units_button_group = new QButtonGroup( this );
  units_button_group->addButton( this->private_->ui_.rb_index_ );
  units_button_group->addButton( this->private_->ui_.rb_world_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.target_layer_, tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, tool->use_active_layer_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.export_button_, boost::bind(
    &PointsSelectToolInterfacePrivate::export_points_to_file, this->private_ ) );

  QtUtils::QtBridge::Connect( units_button_group, tool->units_selection_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.clear_seeds_button_, boost::bind(
    &SeedPointsTool::clear, tool, Core::Interface::GetWidgetActionContext() ) );
  
  QtUtils::QtBridge::Enable( this->private_->ui_.export_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.clear_seeds_button_, tool->valid_target_state_ );
  
  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );
  QtUtils::QtBridge::Enable( this->private_->ui_.target_layer_, tool->use_active_layer_state_, true );
  
  this->private_->ui_.export_button_->setEnabled(tool->seed_points_state_->size() > 0);

  //Send a message to the log that we have finished with building the Measure Tool Interface
  CORE_LOG_MESSAGE( "Finished building an Points Select Tool Interface" );

  return true;
} // end build_widget 

void
PointsSelectToolInterface::ToggleSaveButtonEnabled( qpointer_type points_select_interface )
{
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( 
      &PointsSelectToolInterface::ToggleSaveButtonEnabled, points_select_interface ) );
    return;
  }

  if ( points_select_interface.data() )
  {
    PointsSelectTool* tool = dynamic_cast< PointsSelectTool* >( points_select_interface->tool().get() );
    points_select_interface->private_->ui_.export_button_->setEnabled(tool->seed_points_state_->size() > 0);
  }
}

void
PointsSelectToolInterface::UpdateTable( qpointer_type points_select_interface )
{
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind(
      &PointsSelectToolInterface::UpdateTable, points_select_interface ) );
    return;
  }

  if ( points_select_interface.data() )
  {
    PointsSelectTool* tool = dynamic_cast< PointsSelectTool* >( points_select_interface->tool().get() );
    std::vector< Core::Point > seed_points;
    {
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
      if ( tool->use_world_units_state_->get() )
      {
        seed_points = tool->seed_points_state_->get();
      }
      else
      {
        seed_points = tool->seed_points_index_state_->get();
      }
    }
    QTableWidget* table = points_select_interface->private_->ui_.points_table_;
    table->clearContents();
    table->setRowCount( seed_points.size() );
    for (size_t i = 0; i < seed_points.size(); ++i)
    {
      QTableWidgetItem *itemX = new QTableWidgetItem( QString("%1").arg( seed_points[i].x() ) );
      QTableWidgetItem *itemY = new QTableWidgetItem( QString("%1").arg( seed_points[i].y() ) );
      QTableWidgetItem *itemZ = new QTableWidgetItem( QString("%1").arg( seed_points[i].z() ) );
      table->setItem(i, 0, itemX);
      table->setItem(i, 1, itemY);
      table->setItem(i, 2, itemZ);
    }
  }
}

} // end namespace Seg3D

