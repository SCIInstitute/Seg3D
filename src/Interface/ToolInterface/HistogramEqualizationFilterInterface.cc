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

//QtInterface Includes
#include <QtInterface/Bridge/QtBridge.h>
#include <QtInterface/Widgets/QtHistogramWidget.h>

//Interface Includes
#include <Interface/ToolInterface/CustomWidgets/TargetComboBox.h>

//Qt Gui Includes
#include <Interface/ToolInterface/HistogramEqualizationFilterInterface.h>
#include "ui_HistogramEqualizationFilterInterface.h"

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Tools/HistogramEqualizationFilter.h>
//#include <Application/Filters/Actions/ActionHistogramEqualization.h>
#include <Application/LayerManager/LayerManager.h>

//Core includes
#include <Core/DataBlock/Histogram.h>


namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(HistogramEqualizationFilterInterface)

class HistogramEqualizationFilterInterfacePrivate
{
public:
  Ui::HistogramEqualizationFilterInterface ui_;
    
    QtUtils::QtSliderDoubleCombo *upper_threshold_;
  QtUtils::QtSliderDoubleCombo *lower_threshold_;
  QtUtils::QtSliderIntCombo *alpha_;
  TargetComboBox *target_;
  QtUtils::QtHistogramWidget *histogram_;
};

// constructor
HistogramEqualizationFilterInterface::HistogramEqualizationFilterInterface() :
  private_( new HistogramEqualizationFilterInterfacePrivate )
{
}

// destructor
HistogramEqualizationFilterInterface::~HistogramEqualizationFilterInterface()
{
}

// build the interface and connect it to the state manager
bool HistogramEqualizationFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

    //Add the SliderSpinCombos
    this->private_->upper_threshold_ = new QtUtils::QtSliderDoubleCombo();
    this->private_->ui_.upperHLayout_bottom->addWidget( this->private_->upper_threshold_ );

    this->private_->lower_threshold_ = new QtUtils::QtSliderDoubleCombo();
    this->private_->ui_.lowerHLayout_bottom->addWidget( this->private_->lower_threshold_ );

    this->private_->alpha_ = new QtUtils::QtSliderIntCombo();
    this->private_->ui_.alphaHLayout_bottom->addWidget( this->private_->alpha_ );
    
    this->private_->target_ = new TargetComboBox( this );
    this->private_->ui_.activeHLayout->addWidget( this->private_->target_ );
    
    this->private_->histogram_ = new QtUtils::QtHistogramWidget( this );
    this->private_->ui_.histogramHLayout->addWidget( this->private_->histogram_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  HistogramEqualizationFilter* tool =
      dynamic_cast< HistogramEqualizationFilter* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->target_, tool->target_layer_state_ );
  this->connect( this->private_->target_, SIGNAL( valid( bool ) ), this, SLOT( enable_run_filter( bool ) ) );
  this->connect( this->private_->target_, SIGNAL( currentIndexChanged( QString ) ), this, SLOT( refresh_histogram( QString ) ) );
  QtUtils::QtBridge::Connect( this->private_->upper_threshold_, tool->upper_threshold_state_ );
  QtUtils::QtBridge::Connect( this->private_->lower_threshold_, tool->lower_threshold_state_ );
  QtUtils::QtBridge::Connect( this->private_->alpha_, tool->alpha_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.replaceCheckBox, tool->replace_state_ );
  
  this->connect( this->private_->ui_.runFilterButton, SIGNAL( clicked() ), this, SLOT( execute_filter() ) );
  this->private_->target_->sync_layers();

  //Send a message to the log that we have finised with building the Histogram Equalization Filter Interface
  CORE_LOG_DEBUG("Finished building a Histogram Equalization Filter Interface");
  return ( true );
} // end build_widget
  
void HistogramEqualizationFilterInterface::enable_run_filter( bool valid )
{
  this->private_->ui_.runFilterButton->setEnabled( valid );
}

void HistogramEqualizationFilterInterface::refresh_histogram( QString layer_name )
{
  if( layer_name == "" )
  {
    return;
  }
  
  Core::Histogram temp_histogram = dynamic_cast< DataLayer* >( LayerManager::Instance()->
    get_layer_by_name( layer_name.toStdString() ).get() )->
    get_data_volume()->data_block()->get_histogram();

  // Now, display histogram!
  this->private_->histogram_->set_histogram( temp_histogram );  
}

void HistogramEqualizationFilterInterface::execute_filter()
{
  ToolHandle base_tool_ = tool();
  HistogramEqualizationFilter* tool =
  dynamic_cast< HistogramEqualizationFilter* > ( base_tool_.get() );
  
//  ActionHistogramEqualization::Dispatch( tool->target_layer_state_->export_to_string(), 
//           tool->upper_threshold_state_->get(), tool->lower_threshold_state_->get(),
//           tool->alpha_state_->get(), tool->replace_state_->get() ); 
}

} // end namespace Seg3D
