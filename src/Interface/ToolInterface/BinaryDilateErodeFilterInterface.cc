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

//Interface Includes
#include <Interface/QtInterface/QtBridge.h>

//Qt Gui Includes
#include <Interface/ToolInterface/BinaryDilateErodeFilterInterface.h>
#include "ui_BinaryDilateErodeFilterInterface.h"

//Application Includes
#include <Application/Tools/BinaryDilateErodeFilter.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(BinaryDilateErodeFilterInterface)

class BinaryDilateErodeFilterInterfacePrivate
{
public:
  Ui::BinaryDilateErodeFilterInterface ui_;
  
    SliderIntCombo *erode_;
  SliderIntCombo *dilate_;
};

// constructor
BinaryDilateErodeFilterInterface::BinaryDilateErodeFilterInterface() :
  private_( new BinaryDilateErodeFilterInterfacePrivate )
{
}

// destructor
BinaryDilateErodeFilterInterface::~BinaryDilateErodeFilterInterface()
{
}

// build the interface and connect it to the state manager
bool BinaryDilateErodeFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  private_->ui_.setupUi( frame );

  // add sliderspinnercombo's
  private_->erode_ = new SliderIntCombo();
  private_->ui_.erodeHLayout_bottom->addWidget( private_->erode_ );

  private_->dilate_ = new SliderIntCombo();
  private_->ui_.dialateHLayout_bottom->addWidget( private_->dilate_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  BinaryDilateErodeFilter* tool = dynamic_cast< BinaryDilateErodeFilter* > ( base_tool_.get() );
  
  //Step 3 - set the values for the tool ui from the state engine
  
      //set default falues for the target option list 
      std::vector< std::string > temp_option_list = tool->target_layer_state_->option_list();
      for( size_t i = 0; i < temp_option_list.size(); i++)
      {   
          this->private_->ui_.targetComboBox->addItem( QString::fromStdString( temp_option_list[i] ) );
      } 
        this->private_->ui_.targetComboBox->setCurrentIndex(tool->target_layer_state_->index());
      
      // set the defaults for erode
      int erode_min = 0; 
      int erode_max = 0;
      int erode_step = 0;
      tool->erode_state_->get_step( erode_step );
      tool->erode_state_->get_range( erode_min, erode_max );
      private_->erode_->setStep( erode_step );
        private_->erode_->setRange( erode_min, erode_max );
        private_->erode_->setCurrentValue( tool->erode_state_->get() );
        
        // set the defaults for dialate
      int dilate_min = 0; 
      int dilate_max = 0;
      int dilate_step = 0;
      tool->dilate_state_->get_step( dilate_step );
      tool->dilate_state_->get_range( dilate_min, dilate_max );
      private_->dilate_->setStep( dilate_step );
        private_->dilate_->setRange( dilate_min, dilate_max );
        private_->dilate_->setCurrentValue( tool->dilate_state_->get() );

        // set the default for the replace state
        this->private_->ui_.replaceCheckBox->setChecked( tool->replace_state_->get() );
 

  //Step 4 - connect the gui to the tool through the QtBridge
  QtBridge::Connect( private_->ui_.targetComboBox, tool->target_layer_state_ );
  QtBridge::Connect( private_->erode_, tool->erode_state_ );
  QtBridge::Connect( private_->dilate_, tool->dilate_state_ );
  QtBridge::Connect( private_->ui_.replaceCheckBox, tool->replace_state_ );

  //Send a message to the log that we have finised with building the Binary Dialate Erode Filter Interface
  SCI_LOG_DEBUG("Finished building a Binary Dilate Erode Filter Interface");
  return ( true );

} // end build_widget

} // end namespace Seg3D
