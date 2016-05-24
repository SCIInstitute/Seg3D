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

// Qt includes
#include <QPointer>

// Core includes
#include <Core/Interface/Interface.h>

// QtGui includes
#include "ui_HistogramEqualizationFilterInterface.h"

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/Tools/HistogramEqualizationFilter.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>

// Interface includes
#include <Interface/ToolInterface/HistogramEqualizationFilterInterface.h>

// Core includes
#include <Core/DataBlock/Histogram.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, HistogramEqualizationFilterInterface )

namespace Seg3D
{

class HistogramEqualizationFilterInterfacePrivate
{
public:
  Ui::HistogramEqualizationFilterInterface ui_;
};

HistogramEqualizationFilterInterface::HistogramEqualizationFilterInterface() :
  private_( new HistogramEqualizationFilterInterfacePrivate )
{
}

HistogramEqualizationFilterInterface::~HistogramEqualizationFilterInterface()
{
}

// build the interface and connect it to the state manager
bool HistogramEqualizationFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  this->private_->ui_.horizontalLayout_2->setAlignment( Qt::AlignHCenter );

  //Step 2 - get a pointer to the tool
  HistogramEqualizationFilter* tool = dynamic_cast< HistogramEqualizationFilter* >( 
    this->tool().get() );
    
  // Step 3 - Qt connections
  {
    this->connect( this->private_->ui_.target_layer_, SIGNAL( currentIndexChanged( QString ) ), 
      this, SLOT( refresh_histogram( QString ) ) ); 
  } 

  //Step 4 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_layer_, 
    tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, 
    tool->use_active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.replaceCheckBox, 
    tool->replace_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.amount_, 
    tool->amount_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.bins_, 
    tool->bins_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.ignore_bins_, 
    tool->ignore_bins_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.runFilterButton, tool->valid_target_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );
  QtUtils::QtBridge::Enable( this->private_->ui_.histogram_, tool->valid_target_state_ );

  QtUtils::QtBridge::Enable( this->private_->ui_.target_layer_, 
    tool->use_active_layer_state_, true );
  QtUtils::QtBridge::Connect( this->private_->ui_.runFilterButton, boost::bind(
    &Tool::execute, tool, Core::Interface::GetWidgetActionContext() ) );
  
  this->private_->ui_.amount_->set_description( "Equalization" );
  this->private_->ui_.bins_->set_description( "Histogram Bins" );
  this->private_->ui_.ignore_bins_->set_description( "Bins to Ignore" );
  
      
  return true;
} // end build_widget

void HistogramEqualizationFilterInterface::refresh_histogram( QString layer_name )
{
  if( layer_name == "" || 
    layer_name == Tool::NONE_OPTION_C.c_str() )
  {
    return;
  }

  DataLayerHandle data_layer = boost::dynamic_pointer_cast< DataLayer >(
    LayerManager::Instance()->find_layer_by_name( layer_name.toStdString() ) );
  if ( !data_layer )
  {
    return;
  }

  this->private_->ui_.histogram_->set_histogram( data_layer->get_data_volume()->
    get_data_block()->get_histogram() );  
}

} // end namespace Seg3D
