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

#include <Interface/AppInterface/LayerManagerWidget.h>
#include <Utils/Core/Log.h>

#include <sstream>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include <Interface/QtInterface/QtBridge.h>

namespace Seg3D  {

LayerManagerWidget::LayerManagerWidget(QWidget* parent) :
  QScrollArea(parent)
{
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setContentsMargins(1, 1, 1, 1);
  setWidgetResizable(true);
  
  main_ = new QWidget(this);
  setWidget(main_);
  
  main_layout_ = new QVBoxLayout( main_ );
  main_layout_->setContentsMargins(1, 1, 1, 1);
  main_layout_->setSpacing(1);
  
  tool_layout_ = new QVBoxLayout;
  main_layout_->addLayout(tool_layout_);
  main_layout_->addStretch();
  
  main_->setLayout(main_layout_);
}
  
LayerManagerWidget::~LayerManagerWidget()
{
}

void 
LayerManagerWidget::add_mask_layer( const QString &label, boost::function<void ()> close_function, Group &container_group )
{
  LayerHandle mask_handle(new Layer);


}

void
LayerManagerWidget::new_group()
{
}

void
LayerManagerWidget::remove_mask_layer(int index)
{
  
}

void
LayerManagerWidget::set_active_layer(int index)
{
  
}

void
LayerManagerWidget::set_active_group(int index)
{
  
}
  
int
LayerManagerWidget::index_of_layer(LayerHandle layer)
{
  return 0;
}
  
int
LayerManagerWidget::index_of_group(GroupHandle group_)
{
  return 0;
}
  
  




} //end Seg3D namespace

