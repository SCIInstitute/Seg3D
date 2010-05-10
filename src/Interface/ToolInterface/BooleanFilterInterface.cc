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
#include <Interface/ToolInterface/CustomWidgets/MaskComboBox.h>

//Qt Gui Includes
#include <Interface/ToolInterface/BooleanFilterInterface.h>
#include "ui_BooleanFilterInterface.h"

//Application Includes
#include <Application/Tools/BooleanFilter.h>
#include <Application/Filters/Actions/ActionBoolean.h>

namespace Seg3D
{

SCI_REGISTER_TOOLINTERFACE(BooleanFilterInterface)

class BooleanFilterInterfacePrivate
{
public:
  Ui::BooleanFilterInterface ui_;
  MaskComboBox *mask_a_;
  MaskComboBox *mask_b_;
  MaskComboBox *mask_c_;
  MaskComboBox *mask_d_;
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
  this->private_->ui_.setupUi( frame );
  
    // Add the MaskComboBoxes
    this->private_->mask_a_ = new MaskComboBox( this );
    this->private_->mask_a_->setMinimumHeight( 26 );
    this->private_->ui_.maskAHLayout->addWidget( this->private_->mask_a_ );
    this->private_->ui_.maskAHLayout->setStretch(1, 3);
    
    this->private_->mask_b_ = new MaskComboBox( this );
    this->private_->mask_b_->setMinimumHeight( 26 );
    this->private_->ui_.maskBHLayout->addWidget( this->private_->mask_b_ );
    
    this->private_->mask_c_ = new MaskComboBox( this );
    this->private_->mask_c_->setMinimumHeight( 26 );
    this->private_->ui_.maskCHLayout->addWidget( this->private_->mask_c_ );
    
    this->private_->mask_d_ = new MaskComboBox( this );
    this->private_->mask_d_->setMinimumHeight( 26 );
    this->private_->ui_.maskDHLayout->addWidget( this->private_->mask_d_ );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  BooleanFilter* tool = dynamic_cast< BooleanFilter* > ( base_tool_.get() );
  
  //Step 3 - set the values for the tool ui from the state engine
  
      //set default falues for the example list
    std::vector< std::string > temp_option_list = tool->example_expressions_state_->option_list();
      for( size_t i = 0; i < temp_option_list.size(); i++)
      {   
          this->private_->ui_.exampleExpComboBox->addItem( QString::fromStdString( temp_option_list[i] ) );
      } 
        this->private_->ui_.exampleExpComboBox->setCurrentIndex(tool->example_expressions_state_->index());
        
        // set the default for the replace state
        this->private_->ui_.replaceCheckBox->setChecked( tool->replace_state_->get() );


  //Step 4 - connect the gui to the tool through the QtBridge
  QtBridge::Connect( this->private_->mask_a_, tool->mask_a_state_ );
  QtBridge::Connect( this->private_->mask_b_, tool->mask_b_state_ );
  QtBridge::Connect( this->private_->mask_c_, tool->mask_c_state_ );
  QtBridge::Connect( this->private_->mask_d_, tool->mask_d_state_ );
  connect( this->private_->mask_a_, SIGNAL( valid( bool ) ), this, SLOT( enable_run_filter( bool ) ) );
  
  QtBridge::Connect( this->private_->ui_.exampleExpComboBox, tool->example_expressions_state_ );
  QtBridge::Connect( this->private_->ui_.replaceCheckBox, tool->replace_state_ );
  
  connect( this->private_->ui_.runFilterButton, SIGNAL( clicked() ), this, SLOT( execute_filter() ) );
  
  this->private_->mask_a_->sync_layers();
  this->private_->mask_b_->sync_layers();
  this->private_->mask_c_->sync_layers();
  this->private_->mask_d_->sync_layers();

  //Send a message to the log that we have finised with building the Boolean Filter Interface
  CORE_LOG_DEBUG("Finished building a Boolean Filter Interface");
  return ( true );

}// end build_widget

void BooleanFilterInterface::enable_run_filter( bool valid )
{
  if( valid )
    this->private_->ui_.runFilterButton->setEnabled( true );
  else
    this->private_->ui_.runFilterButton->setEnabled( false );
}

void BooleanFilterInterface::execute_filter()
{
  ToolHandle base_tool_ = tool();
  BooleanFilter* tool =
    dynamic_cast< BooleanFilter* > ( base_tool_.get() );

  ActionBoolean::Dispatch( tool->mask_a_state_->export_to_string(), 
    tool->mask_b_state_->export_to_string(), tool->mask_c_state_->export_to_string(),
    tool->mask_d_state_->export_to_string(), tool->example_expressions_state_->get(), 
    tool->replace_state_->get() ); 
}


} // end namespace Seg3D
