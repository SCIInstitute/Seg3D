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

// QtInterface Includes
#include <QtInterface/Utils/QtBridge.h>

// Interface Includes
#include <Interface/ToolInterface/CustomWidgets/TargetComboBox.h>

//Qt Gui Includes
#include <Interface/ToolInterface/MedianFilterInterface.h>
#include "ui_MedianFilterInterface.h"

//Application Includes
#include <Application/Tools/MedianFilter.h>
//#include <Application/Filters/Actions/ActionMedian.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(MedianFilterInterface)

class MedianFilterInterfacePrivate
{
public:
  Ui::MedianFilterInterface ui_;
  
  Core::QtSliderIntCombo *radius_;
  TargetComboBox *target_;
};

// constructor
MedianFilterInterface::MedianFilterInterface() :
  private_( new MedianFilterInterfacePrivate )
{
}

// destructor
MedianFilterInterface::~MedianFilterInterface()
{
}

// build the interface and connect it to the state manager
bool MedianFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

  //add sliderspincombo
  this->private_->radius_ = new Core::QtSliderIntCombo();
  this->private_->ui_.radiusHLayout_bottom->addWidget( this->private_->radius_ );
  
  this->private_->target_ = new TargetComboBox( this );
  this->private_->ui_.activeHLayout->addWidget( this->private_->target_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  MedianFilter* tool = dynamic_cast< MedianFilter* > ( base_tool_.get() );
  
  //Step 3 - set the values for the tool ui from the state engine
  
        // set the defaults for the radius
      int radius_min = 0; 
      int radius_max = 0;
      int radius_step = 0;
      tool->radius_state_->get_step( radius_step );
      tool->radius_state_->get_range( radius_min, radius_max );
      this->private_->radius_->setStep( radius_step );
        this->private_->radius_->setRange( radius_min, radius_max );
        this->private_->radius_->setCurrentValue( tool->radius_state_->get() );
        
        // set the default for the replace state
        this->private_->ui_.replaceCheckBox->setChecked( tool->replace_state_->get() );

  //Step 4 - connect the gui to the tool through the QtBridge
  Core::QtBridge::Connect( this->private_->target_, tool->target_layer_state_ );
  connect( this->private_->target_, SIGNAL( valid( bool ) ), this, SLOT( enable_run_filter( bool ) ) );
  Core::QtBridge::Connect( this->private_->radius_, tool->radius_state_ );
  Core::QtBridge::Connect( this->private_->ui_.replaceCheckBox, tool->replace_state_ );
  
  connect( this->private_->ui_.runFilterButton, SIGNAL( clicked() ), this, SLOT( execute_filter() ) );
  
  this->private_->target_->sync_layers();
  

  //Send a message to the log that we have finised with building the Median Filter Interface
  CORE_LOG_DEBUG("Finished building an Median Filter Interface");
  return ( true );

} // end build_widget
  
void MedianFilterInterface::enable_run_filter( bool valid )
{
  if( valid )
    this->private_->ui_.runFilterButton->setEnabled( true );
  else
    this->private_->ui_.runFilterButton->setEnabled( false );
}

void MedianFilterInterface::execute_filter()
{
  ToolHandle base_tool_ = tool();
  MedianFilter* tool =
  dynamic_cast< MedianFilter* > ( base_tool_.get() );
  
//  ActionMedian::Dispatch( tool->target_layer_state_->export_to_string(), 
//     tool->radius_state_->get(), tool->replace_state_->get() ); 
}

} // end namespace Seg3D
