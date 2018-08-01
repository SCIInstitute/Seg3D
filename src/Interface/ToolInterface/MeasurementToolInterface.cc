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

// boost includes
#include <boost/algorithm/string/replace.hpp>

// Core includes
#include <Core/Interface/Interface.h>
#include <Core/Utils/Log.h>

// Qt includes
#include <QtGui/QClipboard>
#include <QColorDialog>
#include <QButtonGroup>

// Qt Gui Includes
#include <QtUtils/Bridge/QtBridge.h>

// Interface Includes
#include <Interface/ToolInterface/MeasurementToolInterface.h>
#include <Interface/ToolInterface/detail/MeasurementTableModel.h>
#include <Interface/ToolInterface/detail/MeasurementTableView.h>
#include "ui_MeasurementToolInterface.h"

// Application Includes
#include <Application/Layer/LayerManager.h>
#include <Application/ViewerManager/Actions/ActionPickPoint.h>

// Core Includes
#include <Core/State/Actions/ActionSetAt.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, MeasurementToolInterface )

namespace Seg3D
{

class MeasurementToolInterfacePrivate 
{
public:
  Ui::MeasurementToolInterface ui_;
  MeasurementToolInterface* interface_;

  MeasurementTableModel*  table_model_;
  MeasurementTableView* table_view_;

  void go_to_active_measurement( int point_index );
  void export_measurements_to_clipboard() const;
};

void MeasurementToolInterfacePrivate::go_to_active_measurement( int point_index )
{
  if( !( point_index == 0 || point_index == 1 ) ) return;

  // Go to active measurement
  MeasurementToolHandle tool_handle = 
    boost::dynamic_pointer_cast< MeasurementTool >( this->interface_->tool() );

  // Find 3D point based on active measurement index and point index
  // "Pick" this point

  // Lock state engine so measurements list doesn't change between getting active index
  // and getting list.
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  const std::vector< Core::Measurement >& measurements = tool_handle->measurements_state_->get();
  int active_index = tool_handle->active_index_state_->get();

  if( 0 <= active_index && active_index < static_cast< int >( measurements.size() ) )
  {
    Core::Point pick_point;
    measurements[ active_index ].get_point( point_index, pick_point );

    ActionPickPoint::Dispatch( Core::Interface::GetWidgetActionContext(), pick_point );
  }
}

void MeasurementToolInterfacePrivate::export_measurements_to_clipboard() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  MeasurementToolHandle tool_handle = 
    boost::dynamic_pointer_cast< MeasurementTool >( this->interface_->tool() );
  const std::vector< Core::Measurement >& measurements = tool_handle->measurements_state_->get();

  QString text;
  for( size_t i = 0; i < measurements.size(); i++ )
  {
    Core::Measurement m = measurements[ i ];
    // Name
    text.append( QString::fromStdString( m.get_name() ) );
    text.append( QLatin1Char('\t') ); 

    // Comment
    std::string comment = m.get_comment();
    // Remove line breaks
    boost::replace_all( comment, "\n", " " );
    boost::replace_all( comment, "\r", " " );
    text.append( QString::fromStdString( comment ) );
    text.append( QLatin1Char('\t') ); 

    // Length
    std::string length_unit_string = tool_handle->convert_world_to_unit_string( m.get_length() );
    text.append( QString::fromStdString( length_unit_string ) );
    text.append( QLatin1Char('\n') );
  }

  if( !text.isEmpty() )
  {
    qApp->clipboard()->setText( text );
  } 
}

// constructor
MeasurementToolInterface::MeasurementToolInterface() :
  private_( new MeasurementToolInterfacePrivate )
{
  this->private_->interface_ = this;
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

  /* Normally Qt syncs the model and the view without us having to worry about it.  But that 
  relies on everything being on Qt thread, which is not true in our case.  So instead the 
  MeasurementToolInterface calls update on the model when the measurements state is modified.  
  The model in turn gets its data from the measurements state. */ 
  // NOTE: It is expensive and disruptive to update the entire general tab, so we update only 
  // the widgets needed for each signal.
  qpointer_type measurement_interface( this );  
  this->add_connection( tool_handle->num_measurements_changed_signal_.connect( 
    boost::bind( &MeasurementToolInterface::UpdateGeneralTab, measurement_interface ) ) );

  this->add_connection( tool_handle->measurements_state_->state_changed_signal_.connect( 
    boost::bind( &MeasurementToolInterface::UpdateTableCells, measurement_interface ) ) );
  this->add_connection( tool_handle->measurements_state_->state_changed_signal_.connect( 
    boost::bind( &MeasurementToolInterface::UpdateGeneralComment, measurement_interface ) ) );
  this->add_connection( tool_handle->measurements_state_->state_changed_signal_.connect( 
    boost::bind( &MeasurementToolInterface::UpdateActiveTab, measurement_interface ) ) );

  this->add_connection( tool_handle->units_changed_signal_.connect(
    boost::bind( &MeasurementToolInterface::UpdateTableCells, measurement_interface ) ) );
  this->add_connection( tool_handle->units_changed_signal_.connect(
    boost::bind( &MeasurementToolInterface::UpdateActiveTab, measurement_interface ) ) );

  this->add_connection( tool_handle->active_index_state_->state_changed_signal_.connect( 
    boost::bind( &MeasurementToolInterface::UpdateTableActiveIndex, measurement_interface ) ) );
  this->add_connection( tool_handle->active_index_state_->state_changed_signal_.connect( 
    boost::bind( &MeasurementToolInterface::UpdateGeneralComment, measurement_interface ) ) );
  this->add_connection( tool_handle->active_index_state_->state_changed_signal_.connect( 
    boost::bind( &MeasurementToolInterface::UpdateActiveTab, measurement_interface ) ) );
  
  // Copied from resample tool
  QButtonGroup* units_button_group = new QButtonGroup( this );
  units_button_group->addButton( this->private_->ui_.rb_index_ );
  units_button_group->addButton( this->private_->ui_.rb_world_ );

  // Connect the gui to the tool through the QtBridge

  // General tab
  QObject::connect( this->private_->ui_.general_comment_textbox_, 
    SIGNAL( editing_finished( std::string ) ), 
    this, SLOT( handle_comment_textbox_changed( std::string ) ) );
  QtUtils::QtBridge::Connect( units_button_group, tool_handle->units_selection_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.copy_button_, boost::bind(
    &MeasurementTableView::export_selected_measurements, this->private_->table_view_ ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.delete_button_, boost::bind(
    &MeasurementTableView::delete_selected_measurements, this->private_->table_view_ ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.opacity_slider_, tool_handle->opacity_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.export_button_, boost::bind(
    &MeasurementToolInterfacePrivate::export_measurements_to_clipboard, this->private_ ) );

  this->private_->ui_.opacity_slider_->set_description( "Opacity" );

  // Active measurement tab
  QtUtils::QtBridge::Connect( this->private_->ui_.unit_combobox_, tool_handle->units_selection_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.goto_first_button_, boost::bind(
    &MeasurementToolInterfacePrivate::go_to_active_measurement, this->private_, 0 ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.goto_second_button_, boost::bind(
    &MeasurementToolInterfacePrivate::go_to_active_measurement, this->private_, 1 ) );
  // Don't have a QtBridge that can connect a widget to a particular attribute of a measurement
  // in a StateVector, so use Qt signals/slots directly.  Widgets are updated to reflect state
  // in UpdateActiveTab().
  QObject::connect( this->private_->ui_.name_lineedit_, SIGNAL( editingFinished() ), 
      this, SLOT( handle_name_lineedit_changed() ) );
  QObject::connect( this->private_->ui_.detail_comment_textbox_, 
    SIGNAL( editing_finished( std::string ) ), 
    this, SLOT( handle_comment_textbox_changed( std::string ) ) );
  QObject::connect( this->private_->ui_.length_lineedit_, SIGNAL( editingFinished() ), 
    this, SLOT( handle_length_lineedit_changed() ) );
  QObject::connect( this->private_->ui_.show_checkbox_, SIGNAL( stateChanged( int ) ), 
    this, SLOT( handle_show_checkbox_changed() ) );
  QObject::connect( this->private_->ui_.color_button_, SIGNAL( clicked() ), 
    this, SLOT( handle_color_button_clicked() ) );

  // Don't allow brackets in name
  this->private_->ui_.name_lineedit_->setValidator( new QRegExpValidator( 
    QRegExp( QString::fromStdString( Core::Measurement::REGEX_VALIDATOR_C ) ), this ) );
  
  // Initialize GUI values
  UpdateGeneralTab( measurement_interface );
  UpdateActiveTab( measurement_interface );

  //Send a message to the log that we have finished with building the Measure Tool Interface
  CORE_LOG_MESSAGE( "Finished building an Measure Tool Interface" );

  return ( true );
} // end build_widget 

void MeasurementToolInterface::handle_name_lineedit_changed()
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  MeasurementToolHandle tool_handle = 
    boost::dynamic_pointer_cast< MeasurementTool >( this->tool() );
  const std::vector< Core::Measurement >& measurements = tool_handle->measurements_state_->get();
  int active_index = tool_handle->active_index_state_->get();

  if( 0 <= active_index && active_index < static_cast< int >( measurements.size() ) )
  {
    std::string lineedit_name = this->private_->ui_.name_lineedit_->text().toStdString();
    
    Core::Measurement m = measurements[ active_index ];
    m.set_name( lineedit_name );
    Core::ActionSetAt::Dispatch( Core::Interface::GetWidgetActionContext(),
      tool_handle->measurements_state_, active_index, m );
  }
}

void MeasurementToolInterface::handle_comment_textbox_changed( std::string comment )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  MeasurementToolHandle tool_handle = 
    boost::dynamic_pointer_cast< MeasurementTool >( this->tool() );
  int active_index = tool_handle->active_index_state_->get();
  const std::vector< Core::Measurement >& measurements = tool_handle->measurements_state_->get();

  if( 0 <= active_index && active_index < static_cast< int >( measurements.size() ) )
  {
    Core::Measurement m = measurements[ active_index ];
    m.set_comment( comment );
    Core::ActionSetAt::Dispatch( Core::Interface::GetWidgetActionContext(),
      tool_handle->measurements_state_, active_index, m );
  }
}

void MeasurementToolInterface::handle_length_lineedit_changed()
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  
  MeasurementToolHandle tool_handle = 
    boost::dynamic_pointer_cast< MeasurementTool >( this->tool() );
  int active_index = tool_handle->active_index_state_->get();
  const std::vector< Core::Measurement >& measurements = tool_handle->measurements_state_->get();

  if( 0 <= active_index && active_index < static_cast< int >( measurements.size() ) )
  {
    std::string lineedit_length = this->private_->ui_.length_lineedit_->text().toStdString();
    double world_length = tool_handle->convert_unit_string_to_world( lineedit_length );
  
    Core::Measurement m = measurements[ active_index  ];
    m.set_length( world_length );
    Core::ActionSetAt::Dispatch( Core::Interface::GetWidgetActionContext(),
      tool_handle->measurements_state_, active_index , m );
  }
}

void MeasurementToolInterface::handle_show_checkbox_changed()
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  MeasurementToolHandle tool_handle = 
    boost::dynamic_pointer_cast< MeasurementTool >( this->tool() );
  int active_index = tool_handle->active_index_state_->get();
  const std::vector< Core::Measurement >& measurements = tool_handle->measurements_state_->get();

  if( 0 <= active_index && active_index < static_cast< int >( measurements.size() ) )
  {
    bool visible = this->private_->ui_.show_checkbox_->isChecked();

    Core::Measurement m = measurements[ active_index  ];
    m.set_visible( visible );
    Core::ActionSetAt::Dispatch( Core::Interface::GetWidgetActionContext(),
      tool_handle->measurements_state_, active_index , m );
  }
}

void MeasurementToolInterface::handle_color_button_clicked()
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  MeasurementToolHandle tool_handle = 
    boost::dynamic_pointer_cast< MeasurementTool >( this->tool() );
  int active_index = tool_handle->active_index_state_->get();
  const std::vector< Core::Measurement >& measurements = tool_handle->measurements_state_->get();

  if( 0 <= active_index && active_index < static_cast< int >( measurements.size() ) )
  {
    // Get current color
    Core::Measurement m = measurements[ active_index  ];
    Core::Color m_color;
    m.get_color( m_color );

    // Launch dialog to allow user to select color
    QColor color = QColorDialog::getColor( QColor( m_color.r(), m_color.g(), m_color.b() ), this );
    if( color.isValid() )
    { 
      // Set measurement color
      m.set_color( 
        Core::Color( color.red() / 255.0f, color.green() / 255.0f, color.blue() / 255.0f ) );
      Core::ActionSetAt::Dispatch( Core::Interface::GetWidgetActionContext(),
        tool_handle->measurements_state_, active_index , m );
    }
  }
}

void MeasurementToolInterface::UpdateGeneralTab( qpointer_type measurement_interface )
{   
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( 
      &MeasurementToolInterface::UpdateGeneralTab, measurement_interface ) );
    return;
  }

  // Protect interface pointer, so we do not execute if interface does not exist anymore
  if ( measurement_interface.data() )
  {
    // Update table view
    measurement_interface->private_->table_model_->update_table();

    MeasurementToolHandle tool_handle = 
      boost::dynamic_pointer_cast< MeasurementTool >( measurement_interface->tool() );

    // Enable widgets only if measurements exist and active index is valid
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    
    int active_index = tool_handle->active_index_state_->get();
    const std::vector< Core::Measurement >& measurements = tool_handle->measurements_state_->get();

    bool enable_widgets = measurements.size() > 0 && 0 <= active_index && 
      active_index < static_cast< int >( measurements.size() );

    measurement_interface->private_->ui_.table_view_->setEnabled( enable_widgets );
    measurement_interface->private_->ui_.general_comment_textbox_->setEnabled( enable_widgets );
    measurement_interface->private_->ui_.copy_button_->setEnabled( enable_widgets );
    measurement_interface->private_->ui_.delete_button_->setEnabled( enable_widgets );
    measurement_interface->private_->ui_.export_button_->setEnabled( enable_widgets );

    // Update comment text box
    if( enable_widgets )
    {
      measurement_interface->private_->ui_.general_comment_textbox_->setText( 
        QString::fromStdString( measurements[ active_index ].get_comment() ) );
    }
    else
    {
      measurement_interface->private_->ui_.general_comment_textbox_->clear();
    }
  }
}

void MeasurementToolInterface::UpdateTableCells( qpointer_type measurement_interface )
{

  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( 
      &MeasurementToolInterface::UpdateTableCells, measurement_interface ) );
    return;
  }

  // Protect interface pointer, so we do not execute if interface does not exist anymore
  if ( measurement_interface.data() )
  {
    // Update only table cells, not table dimensions
    // Needed in order to update background color for active/non-active cells
    measurement_interface->private_->table_model_->update_cells();
  }
}

void MeasurementToolInterface::UpdateTableActiveIndex( qpointer_type measurement_interface )
{
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( 
      &MeasurementToolInterface::UpdateTableActiveIndex, measurement_interface ) );
    return;
  }

  // Protect interface pointer, so we do not execute if interface does not exist anymore
  if ( measurement_interface.data() )
  {
    // Select active index and scroll to it
    measurement_interface->private_->table_view_->update_active_index();
  }
}

void MeasurementToolInterface::UpdateGeneralComment( qpointer_type measurement_interface )
{
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( 
      &MeasurementToolInterface::UpdateGeneralComment, measurement_interface ) );
    return;
  }

  // Protect interface pointer, so we do not execute if interface does not exist anymore
  if ( measurement_interface.data() )
  {

    // Update comment text box
    MeasurementToolHandle tool_handle = 
      boost::dynamic_pointer_cast< MeasurementTool >( measurement_interface->tool() );

    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    int active_index = tool_handle->active_index_state_->get();
    const std::vector< Core::Measurement >& measurements = tool_handle->measurements_state_->get();

    if( measurements.size() > 0 && 0 <= active_index && 
      active_index < static_cast< int >( measurements.size() ) )
    {
      measurement_interface->private_->ui_.general_comment_textbox_->setText( 
        QString::fromStdString( measurements[ active_index ].get_comment() ) );
    }
  }
}

void MeasurementToolInterface::UpdateActiveTab( qpointer_type measurement_interface )
{
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( 
      &MeasurementToolInterface::UpdateActiveTab, measurement_interface ) );
    return;
  }

  // Protect interface pointer, so we do not execute if interface does not exist anymore
  if ( measurement_interface.data() )
  {
    MeasurementToolHandle tool_handle = 
      boost::dynamic_pointer_cast< MeasurementTool >( measurement_interface->tool() );

    // Enable table only if measurements exist and active index is valid
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    int active_index = tool_handle->active_index_state_->get();
    const std::vector< Core::Measurement >& measurements = tool_handle->measurements_state_->get();

    bool enable_tab = ( measurements.size() > 0 &&
      0 <= active_index && active_index < static_cast< int >( measurements.size() ) );

    measurement_interface->private_->ui_.tab_widget_->setTabEnabled( 1, enable_tab );

    if( enable_tab )
    {
      // Set widget values based on state

      // Name
      measurement_interface->private_->ui_.name_lineedit_->setText( 
        QString::fromStdString( measurements[ active_index ].get_name() ) );
      
      // Comment
      measurement_interface->private_->ui_.detail_comment_textbox_->setText( 
        QString::fromStdString( measurements[ active_index ].get_comment() ) );
      
      // Length
      double world_length = measurements[ active_index ].get_length();
      std::string unit_length = tool_handle->convert_world_to_unit_string( world_length );
      measurement_interface->private_->ui_.length_lineedit_->setText( 
        QString::fromStdString( unit_length ) );
      
      // Points
      Core::Point p0, p1;
      measurements[ active_index ].get_point( 0, p0 );
      measurements[ active_index ].get_point( 1, p1 );
      std::string p0_str = "( " +  tool_handle->convert_world_to_unit_string( p0.x() ) + 
        ", " + tool_handle->convert_world_to_unit_string( p0.y() ) + ", " + 
        tool_handle->convert_world_to_unit_string( p0.z() ) + " )";
      measurement_interface->private_->ui_.p0_value_->setText( 
        QString::fromStdString( p0_str ) );
      std::string p1_str = "( " +  tool_handle->convert_world_to_unit_string( p1.x() ) + 
        ", " + tool_handle->convert_world_to_unit_string( p1.y() ) + ", " + 
        tool_handle->convert_world_to_unit_string( p1.z() ) + " )";
      measurement_interface->private_->ui_.p1_value_->setText( 
        QString::fromStdString( p1_str ) );
      
      // Hide/Show
      measurement_interface->private_->ui_.show_checkbox_->blockSignals( true );
      measurement_interface->private_->ui_.show_checkbox_->setChecked( 
        measurements[ active_index ].get_visible() );
      measurement_interface->private_->ui_.show_checkbox_->blockSignals( false );
      
      // Color
      // Create QPixMap with color of active measurement
      int height = measurement_interface->private_->ui_.color_button_->height();
      QPixmap pix_map( height, height ); // Width is wrong initially, so use height
      Core::Color m_color;
      measurements[ active_index ].get_color( m_color );
      pix_map.fill( QColor( m_color.r() * 255, m_color.g() * 255, m_color.b() * 255 ) );
      measurement_interface->private_->ui_.color_button_->setIcon( pix_map );
    }
    else
    {
      // Clear widget values
      measurement_interface->private_->ui_.name_lineedit_->clear();
      measurement_interface->private_->ui_.detail_comment_textbox_->clear();
      measurement_interface->private_->ui_.length_lineedit_->clear();
      measurement_interface->private_->ui_.p0_value_->clear();
      measurement_interface->private_->ui_.p1_value_->clear();
      measurement_interface->private_->ui_.show_checkbox_->setChecked( false );
    }
  }
}

} // end namespace Seg3D

