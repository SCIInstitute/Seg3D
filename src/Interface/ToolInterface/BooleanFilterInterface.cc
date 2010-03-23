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
#include <Interface/ToolInterface/BooleanFilterInterface.h>
#include "ui_BooleanFilterInterface.h"

//Application Includes
#include <Application/Tools/BooleanFilter.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(BooleanFilterInterface)

class BooleanFilterInterfacePrivate
{
public:
  Ui::BooleanFilterInterface ui_;
};

// constructor
BooleanFilterInterface::BooleanFilterInterface() :
  private_( new BooleanFilterInterfacePrivate )
{
}

// destructor
BooleanFilterInterface::~BooleanFilterInterface()
{
}

// build the interface and connect it to the state manager
bool BooleanFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  private_->ui_.setupUi( frame );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  BooleanFilter* tool = dynamic_cast< BooleanFilter* > ( base_tool_.get() );
  
  //Step 3 - set the values for the tool ui from the state engine
  
      //set default falues for the mask a option list 
      std::vector< std::string > temp_option_list = tool->mask_a_state_->option_list();
      for( size_t i = 0; i < temp_option_list.size(); i++)
      {   
          this->private_->ui_.maskAComboBox->addItem( QString::fromStdString( temp_option_list[i] ) );
      } 
        this->private_->ui_.maskAComboBox->setCurrentIndex(tool->mask_a_state_->index());
      
      //set default falues for the mask b option list 
      temp_option_list = tool->mask_b_state_->option_list();
      for( size_t i = 0; i < temp_option_list.size(); i++)
      {   
          this->private_->ui_.maskBComboBox->addItem( QString::fromStdString( temp_option_list[i] ) );
      } 
        this->private_->ui_.maskBComboBox->setCurrentIndex(tool->mask_b_state_->index());
        
        //set default falues for the mask c option list 
      temp_option_list = tool->mask_c_state_->option_list();
      for( size_t i = 0; i < temp_option_list.size(); i++)
      {   
          this->private_->ui_.maskCComboBox->addItem( QString::fromStdString( temp_option_list[i] ) );
      } 
        this->private_->ui_.maskCComboBox->setCurrentIndex(tool->mask_c_state_->index());
        
        //set default falues for the mask d option list 
      temp_option_list = tool->mask_d_state_->option_list();
      for( size_t i = 0; i < temp_option_list.size(); i++)
      {   
          this->private_->ui_.maskDComboBox->addItem( QString::fromStdString( temp_option_list[i] ) );
      } 
        this->private_->ui_.maskDComboBox->setCurrentIndex(tool->mask_d_state_->index());
        
        //set default falues for the example list
        temp_option_list = tool->example_expressions_state_->option_list();
      for( size_t i = 0; i < temp_option_list.size(); i++)
      {   
          this->private_->ui_.exampleExpComboBox->addItem( QString::fromStdString( temp_option_list[i] ) );
      } 
        this->private_->ui_.exampleExpComboBox->setCurrentIndex(tool->example_expressions_state_->index());
        
        // set the default for the replace state
        this->private_->ui_.replaceCheckBox->setChecked( tool->replace_state_->get() );


  //Step 4 - connect the gui to the tool through the QtBridge
  QtBridge::Connect( private_->ui_.maskAComboBox, tool->mask_a_state_ );
  QtBridge::Connect( private_->ui_.maskBComboBox, tool->mask_b_state_ );
  QtBridge::Connect( private_->ui_.maskCComboBox, tool->mask_c_state_ );
  QtBridge::Connect( private_->ui_.maskDComboBox, tool->mask_d_state_ );
  QtBridge::Connect( private_->ui_.exampleExpComboBox, tool->example_expressions_state_ );
  QtBridge::Connect( private_->ui_.replaceCheckBox, tool->replace_state_ );

  //Send a message to the log that we have finised with building the Boolean Filter Interface
  SCI_LOG_DEBUG("Finished building a Boolean Filter Interface");
  return ( true );

}// end build_widget

} // end namespace Seg3D
