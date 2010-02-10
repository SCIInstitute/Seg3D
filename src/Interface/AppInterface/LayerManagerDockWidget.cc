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
#include <sstream>
#include <iostream>

#include <Utils/Core/Log.h>
#include <boost/lexical_cast.hpp>

// Application Includes
#include <Application/Layer/LayerManager.h>
#include <Application/Interface/Interface.h>

#include <Application/Layer/Actions/ActionCloneLayer.h>
#include <Application/Layer/Actions/ActionLayer.h>
#include <Application/Layer/Actions/ActionLayerFromFile.h>
#include <Application/Layer/Actions/ActionNewMaskLayer.h>
#include <Application/Layer/Actions/ActionRemoveLayer.h>


// Interface includes
#include <Interface/AppInterface/LayerManagerDockWidget.h>



namespace Seg3D  {
  

LayerManagerDockWidget::LayerManagerDockWidget(QWidget *parent) :
    QDockWidget("LayerManagerDockWidget")
{
  setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
  setWindowTitle("Layer Manager");
  
  resize(250, 640);
  layer_manager_ = new LayerManagerWidget(this);
  setWidget(layer_manager_);
  

  
  
}

LayerManagerDockWidget::~LayerManagerDockWidget()
{
  
}
  
void  
LayerManagerDockWidget::new_group()
{
  //TODO implement new group function
}
void  
LayerManagerDockWidget::close_group()
{
  //TODO implement close group function
}
void  
LayerManagerDockWidget::layer_from_file()
{
  //TODO implement open data layer function
}
void
LayerManagerDockWidget::clone_layer(LayerHandle layer)
{
  //TODO implement clone layer function
}
void  
LayerManagerDockWidget::new_mask_layer()
{
  //TODO implement new mask layer function
}
void
LayerManagerDockWidget::remove_layer(LayerHandle layer)
{
  //TODO implement remove layer function
}

void
LayerManagerDockWidget::layer_changed(int index)
{
  //TODO implement layer changed slot
}
  
} // end namespace
