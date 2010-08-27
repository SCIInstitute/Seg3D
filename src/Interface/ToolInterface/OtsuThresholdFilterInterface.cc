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

// Qt includes
#include <QPointer>

// QtGui includes
#include "ui_OtsuThresholdFilterInterface.h"

// Application includes
#include <Application/Tools/OtsuThresholdFilter.h>
#include <Application/LayerManager/LayerManager.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>

// Interface includes
#include <Interface/ToolInterface/OtsuThresholdFilterInterface.h>

// Core includes
#include <Core/DataBlock/Histogram.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, OtsuThresholdFilterInterface )

namespace Seg3D
{

class OtsuThresholdFilterInterfacePrivate
{
public:
  Ui::OtsuThresholdFilterInterface ui_;
  
  QtUtils::QtSliderIntCombo *amount_;
  QtUtils::QtHistogramWidget *histogram_;

public:
  static void UpdateHistogram( QPointer<QtUtils::QtHistogramWidget> qpointer,
    std::string layer_name, std::string old_layer_name, Core::ActionSource source );
};

void OtsuThresholdFilterInterfacePrivate::UpdateHistogram( 
  QPointer<QtUtils::QtHistogramWidget> qpointer, std::string layer_name, 
  std::string old_layer_name, Core::ActionSource source )
{
  if ( ! Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( 
      &OtsuThresholdFilterInterfacePrivate::UpdateHistogram,
      qpointer, layer_name, old_layer_name, source ) );
    return;
  }

  if ( ! qpointer.isNull() )
  {
    DataLayerHandle layer = boost::dynamic_pointer_cast<DataLayer>( LayerManager::Instance()->
      get_layer_by_name( layer_name ) );
    
    if ( layer )
    {
      qpointer->set_histogram( layer->get_data_volume()->get_data_block()->get_histogram() );
    }
    else
    {
      qpointer->reset_histogram();
    }
  }
}


// constructor
OtsuThresholdFilterInterface::OtsuThresholdFilterInterface() :
  private_( new OtsuThresholdFilterInterfacePrivate )
{
}

// destructor
OtsuThresholdFilterInterface::~OtsuThresholdFilterInterface()
{
  this->disconnect_all();
}

// build the interface and connect it to the state manager
bool OtsuThresholdFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

  this->private_->amount_ = new QtUtils::QtSliderIntCombo();
  this->private_->ui_.amountHLayout_bottom->addWidget( this->private_->amount_ );
  
  this->private_->histogram_ = new QtUtils::QtHistogramWidget( this );
  this->private_->ui_.histogramHLayout->addWidget( this->private_->histogram_ );
    
  //Step 2 - get a pointer to the tool
  OtsuThresholdFilter* tool = dynamic_cast< OtsuThresholdFilter* > ( this->tool().get() );
  
  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_layer_, 
    tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, 
    tool->use_active_layer_state_ );
      
  QtUtils::QtBridge::Connect( this->private_->amount_, 
    tool->amount_state_ );
      
  QPointer<QtUtils::QtHistogramWidget> qpointer( this->private_->histogram_ );
  this->add_connection( tool->target_layer_state_->value_changed_signal_.connect( boost::bind(
    &OtsuThresholdFilterInterfacePrivate::UpdateHistogram, qpointer, _1, _2, _3 ) ) );
  
  // Step 4 - Qt connections
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() ); 
    this->private_->ui_.target_layer_->setDisabled( tool->use_active_layer_state_->get() );
    
    this->connect( this->private_->ui_.use_active_layer_, SIGNAL( toggled( bool ) ),
      this->private_->ui_.target_layer_, SLOT( setDisabled( bool ) ) );

    this->connect( this->private_->ui_.runFilterButton, SIGNAL( clicked() ), 
      this, SLOT( run_filter() ) );
  }
  
  return true;
} // end build_widget
  
void OtsuThresholdFilterInterface::run_filter()
{
  tool()->execute( Core::Interface::GetWidgetActionContext() );
}

} // end namespace Seg3D
