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

// Qt Gui Includes
#include <QtUtils/Bridge/QtBridge.h>

// Interface Includes
#include <Interface/ToolInterface/MeasurementToolInterface.h>
#include <Interface/ToolInterface/detail/MeasurementTableModel.h>
#include <Interface/ToolInterface/detail/MeasurementTableView.h>
#include "ui_MeasurementToolInterface.h"

// Application Includes
#include <Application/Tools/MeasurementTool.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, MeasurementToolInterface )

namespace Seg3D
{

class MeasurementToolInterfacePrivate 
{
public:
  Ui::MeasurementToolInterface ui_;

  MeasurementTableModel*  table_model_;
  MeasurementTableView* table_view_;
};

// constructor
MeasurementToolInterface::MeasurementToolInterface() :
  private_( new MeasurementToolInterfacePrivate )
{
}

// destructor
MeasurementToolInterface::~MeasurementToolInterface()
{
  this->disconnect_all();
}

// build the interface and connect it to the state manager
bool MeasurementToolInterface::build_widget( QFrame* frame )
{
  // Build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

  // Create table model with tool, set table model in table view
  MeasurementToolHandle tool_handle = boost::dynamic_pointer_cast< MeasurementTool >( tool() );
  this->private_->table_model_ = new MeasurementTableModel( tool_handle, this );
  this->private_->table_view_ = this->private_->ui_.table_view_;
  this->private_->table_view_->set_measurement_model( this->private_->table_model_ );

  qpointer_type measurement_interface( this );
  UpdateMeasurementModel( measurement_interface );

  /* Normally Qt syncs the model and the view without us having to worry about it.  But that 
  relies on everything being on Qt thread, which is not true in our case.  So instead the 
  MeasurementToolInterface calls update on the model when the measurements state is modified.  
  The model in turn gets its data from the measurements state. */ 
  
  this->add_connection( tool_handle->measurements_state_->state_changed_signal_.connect( 
    boost::bind( &MeasurementToolInterface::UpdateMeasurementModel, measurement_interface ) ) );
  this->add_connection( tool_handle->active_index_state_->state_changed_signal_.connect( 
    boost::bind( &MeasurementToolInterface::UpdateMeasurementNote, measurement_interface ) ) );

  // Connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.copy_button_, boost::bind(
    &MeasurementTableView::copy, this->private_->table_view_ ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.delete_button_, boost::bind(
    &MeasurementTableView::delete_selected_measurements, this->private_->table_view_ ) );

  // Connect note column and text box using Qt signals/slots since currently can't hook up two
  // widgets to same state object.
  QObject::connect( this->private_->table_model_, SIGNAL( active_note_changed( QString ) ), 
    this, SLOT( set_measurement_note_box( QString ) ) );
  QObject::connect( this->private_->ui_.note_textbox_, SIGNAL( textChanged() ), 
    this, SLOT( set_measurement_note_table() ) );
  QObject::connect( this->private_->ui_.note_textbox_, SIGNAL( editing_finished() ), 
    this->private_->table_model_, SLOT( save_active_note() ) );

  //Send a message to the log that we have finished with building the Measure Tool Interface
  CORE_LOG_MESSAGE( "Finished building an Measure Tool Interface" );

  return ( true );
} // end build_widget 

void MeasurementToolInterface::set_measurement_note_box( const QString & note )
{
  // If text edit has focus, this is a circular update from the model -- don't modify text
  if( !this->private_->ui_.note_textbox_->hasFocus() )
  {
    this->private_->ui_.note_textbox_->blockSignals( true );
    this->private_->ui_.note_textbox_->setText( note );
    this->private_->ui_.note_textbox_->blockSignals( false );
  } 
}

void MeasurementToolInterface::set_measurement_note_table()
{
  this->private_->table_model_->set_active_note( 
    this->private_->ui_.note_textbox_->document()->toPlainText() );
}

void MeasurementToolInterface::UpdateMeasurementModel( qpointer_type measurement_interface )
{   
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( 
      &MeasurementToolInterface::UpdateMeasurementModel, measurement_interface ) );
    return;
  }

  // Protect interface pointer, so we do not execute if interface does not exist anymore
  if ( measurement_interface.data() )
  {
    // Updates table view
    measurement_interface->private_->table_model_->update();

    // Enable widgets only if measurements exist and active index is valid
    if( measurement_interface->private_->table_model_->rowCount( QModelIndex() ) > 0 &&
      measurement_interface->private_->table_model_->get_active_index() != 
      MeasurementTool::INVALID_ACTIVE_INDEX_C )
    {
      measurement_interface->private_->ui_.note_textbox_->setEnabled( true );
      measurement_interface->private_->ui_.goto_button_->setEnabled( true );
      measurement_interface->private_->ui_.copy_button_->setEnabled( true );
      measurement_interface->private_->ui_.delete_button_->setEnabled( true );
    }
    else
    {
      measurement_interface->private_->ui_.note_textbox_->setEnabled( false );
      measurement_interface->private_->ui_.goto_button_->setEnabled( false );
      measurement_interface->private_->ui_.copy_button_->setEnabled( false );
      measurement_interface->private_->ui_.delete_button_->setEnabled( false );
    }
  }
}

void MeasurementToolInterface::UpdateMeasurementNote( qpointer_type measurement_interface )
{
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( 
      &MeasurementToolInterface::UpdateMeasurementNote, measurement_interface ) );
    return;
  }

  // Protect interface pointer, so we do not execute if interface does not exist anymore
  if ( measurement_interface.data() )
  {
    // Updates table view
    measurement_interface->private_->ui_.note_textbox_->setText( 
      measurement_interface->private_->table_model_->get_active_note() );
  }
}

} // end namespace Seg3D
