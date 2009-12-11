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

#include <Interface/AppInterface/ViewerInterface.h>
#include "ui_ViewerInterface.h"
//#include <QSettings>

namespace Seg3D {
  
  class ViewerInterfacePrivate {
  public:
  
    Ui::ViewerInterface ui_;
  };


  
  
  
  
ViewerInterface::ViewerInterface(QWidget *parent) :
    QWidget(parent),
    private_(new ViewerInterfacePrivate)
{
  if(private_){
    
    private_->ui_.setupUi(this);

    set_focused_view(1);
    
    //full_screen_toggle(true);
  }

}


//TODO - build viewer sizes state saver and recoverer
void ViewerInterface::writeSizeSettings()
{
    QSettings settings( "SCI", "Seg3D2.0" );
    settings.beginGroup("ViewerInterface");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("mainSplitter", private_->ui_.mainSplitter->saveState());
    settings.setValue("eastSplitter", private_->ui_.eastSplitter->saveState());
    settings.setValue("westSplitter", private_->ui_.westSplitter->saveState());
    settings.endGroup();
}

void ViewerInterface::readSizeSettings()
{
    QSettings settings( "SCI", "Seg3D2.0" );
    settings.beginGroup("ViewerInterface");
    restoreGeometry(settings.value("geometry").toByteArray());
    private_->ui_.mainSplitter->restoreState(settings.value("mainSplitter").toByteArray());
    private_->ui_.eastSplitter->restoreState(settings.value("eastSplitter").toByteArray());
    private_->ui_.westSplitter->restoreState(settings.value("westSplitter").toByteArray());
    settings.endGroup();
}

  
//  --- Public Slots ---  //  

void ViewerInterface::set_focused_view(int viewForFocus)
{
  private_->ui_.viewerFrame_1->setAutoFillBackground(false);
    private_->ui_.viewerFrame_1->setAutoFillBackground(false);
    private_->ui_.viewerFrame_2->setAutoFillBackground(false);
    private_->ui_.viewerFrame_3->setAutoFillBackground(false);
    private_->ui_.viewerFrame_4->setAutoFillBackground(false);
    private_->ui_.viewerFrame_5->setAutoFillBackground(false);
    private_->ui_.viewerFrame_6->setAutoFillBackground(false);

    switch (viewForFocus)
    {
        case 1:
            private_->ui_.viewerFrame_1->setAutoFillBackground(true);
            private_->ui_.viewerFrame_1->setFocus();
            break;
        case 2:
            private_->ui_.viewerFrame_2->setAutoFillBackground(true);
            private_->ui_.viewerFrame_2->setFocus();
            break;
        case 3:
            private_->ui_.viewerFrame_3->setAutoFillBackground(true);
            private_->ui_.viewerFrame_3->setFocus();
            break;
        case 4:
            private_->ui_.viewerFrame_4->setAutoFillBackground(true);
            private_->ui_.viewerFrame_4->setFocus();
            break;
        case 5:
            private_->ui_.viewerFrame_5->setAutoFillBackground(true);
            private_->ui_.viewerFrame_5->setFocus();
            break;
        case 6:
            private_->ui_.viewerFrame_6->setAutoFillBackground(true);
            private_->ui_.viewerFrame_6->setFocus();
            break;
    }

}


void ViewerInterface::set_views(int west, int east)
{
    QList<int> westWindows;
    QList<int> eastWindows;
    QList<int> mainWindows;


    if ((west == 1)&&(east == 0))
    {
        mainWindows.push_back(4000);
        private_->ui_.mainSplitter->setSizes(mainWindows);
        westWindows.push_back(4000);
        private_->ui_.westSplitter->setSizes(westWindows);
    }

    if ((west == 1)&&(east == 1))
    {
        mainWindows.push_back(4000);
        mainWindows.push_back(4000);
        private_->ui_.mainSplitter->setSizes(mainWindows);
        westWindows.push_back(4000);
        private_->ui_.westSplitter->setSizes(westWindows);
        eastWindows.push_back(4000);
        private_->ui_.eastSplitter->setSizes(eastWindows);

    }

    if ((west == 1)&&(east == 2))
    {
        mainWindows.push_back(6000);
        mainWindows.push_back(4000);
        private_->ui_.mainSplitter->setSizes(mainWindows);

        westWindows.push_back(4000);
        private_->ui_.westSplitter->setSizes(westWindows);
        eastWindows.push_back(4000);
        eastWindows.push_back(4000);
        private_->ui_.eastSplitter->setSizes(eastWindows);

    }

    if ((west == 1)&&(east == 3))
    {
        mainWindows.push_back(4000);
        mainWindows.push_back(2000);
        private_->ui_.mainSplitter->setSizes(mainWindows);

        westWindows.push_back(4000);
        private_->ui_.westSplitter->setSizes(westWindows);

        eastWindows.push_back(4000);
        eastWindows.push_back(4000);
        eastWindows.push_back(4000);
        private_->ui_.eastSplitter->setSizes(eastWindows);
    }

    if ((west == 2)&&(east == 2))
    {
        mainWindows.push_back(4000);
        mainWindows.push_back(4000);
        private_->ui_.mainSplitter->setSizes(mainWindows);

        westWindows.push_back(4000);
        westWindows.push_back(4000);
        private_->ui_.westSplitter->setSizes(westWindows);

        eastWindows.push_back(4000);
        eastWindows.push_back(4000);
        private_->ui_.eastSplitter->setSizes(eastWindows);

    }

    if ((west == 2)&&(east == 3))
    {
        mainWindows.push_back(5000);
        mainWindows.push_back(4000);
        private_->ui_.mainSplitter->setSizes(mainWindows);

        westWindows.push_back(4000);
        westWindows.push_back(4000);
        private_->ui_.westSplitter->setSizes(westWindows);

        eastWindows.push_back(4000);
        eastWindows.push_back(4000);
        eastWindows.push_back(4000);
        private_->ui_.eastSplitter->setSizes(eastWindows);

    }

    if ((west == 3)&&(east == 3))
    {
        mainWindows.push_back(4000);
        mainWindows.push_back(4000);
        private_->ui_.mainSplitter->setSizes(mainWindows);

        westWindows.push_back(4000);
        westWindows.push_back(4000);
        westWindows.push_back(4000);
        private_->ui_.westSplitter->setSizes(westWindows);

        eastWindows.push_back(4000);
        eastWindows.push_back(4000);
        eastWindows.push_back(4000);
        private_->ui_.eastSplitter->setSizes(eastWindows);
    }
}


ViewerInterface::~ViewerInterface()
{

}

}
