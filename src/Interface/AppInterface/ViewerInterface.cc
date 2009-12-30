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
 
// QT includes
#include <QtGui> 
 
// Utils includes
#include <Utils/Core/Log.h>
#include <Utils/Core/Exception.h>

// Interface includes
#include <Interface/AppInterface/ViewerInterface.h>
#include <Interface/AppInterface/ViewerWidget.h>

namespace Seg3D {
  
class ViewerInterfacePrivate {
  public:
  
    ViewerInterfacePrivate(QWidget* parent);
  
  public:
    QVBoxLayout* layout_;

    QSplitter* horiz_splitter_;
    
    QSplitter* vert_splitter1_;
    QSplitter* vert_splitter2_;

    std::vector<ViewerWidget*> viewer_;
    int selected_viewer_;
};


ViewerInterfacePrivate::ViewerInterfacePrivate(QWidget* parent)
{
  layout_ = new QVBoxLayout(parent);
  layout_->setContentsMargins(0, 0, 0, 0);
  layout_->setSpacing(0);

  vert_splitter1_ = new QSplitter(Qt::Vertical,parent);
  vert_splitter2_ = new QSplitter(Qt::Vertical,parent);
  horiz_splitter_ = new QSplitter(Qt::Horizontal,parent);

  horiz_splitter_->addWidget(vert_splitter1_);
  horiz_splitter_->addWidget(vert_splitter2_);
  horiz_splitter_->setOpaqueResize(false);
   
  viewer_.resize(6);
  for (size_t j=0;j<6;j++) viewer_[j] = new ViewerWidget(j,parent);
  
  vert_splitter1_->addWidget(viewer_[0]);
  vert_splitter1_->addWidget(viewer_[1]);
  vert_splitter1_->addWidget(viewer_[2]);
  vert_splitter1_->setOpaqueResize(false);

  vert_splitter2_->addWidget(viewer_[3]);
  vert_splitter2_->addWidget(viewer_[4]);
  vert_splitter2_->addWidget(viewer_[5]);
  vert_splitter2_->setOpaqueResize(false);

  layout_->addWidget(horiz_splitter_);
  parent->setLayout(layout_);
  
  for (size_t j=0;j<6;j++)
    parent->connect(viewer_[j],SIGNAL(selected(int)),
                        SLOT(set_selected_viewer(int)));
                        
  selected_viewer_ = -1;
} 
  
  
  
ViewerInterface::ViewerInterface(QWidget *parent) :
    QWidget(parent)
{
  private_ = ViewerInterfacePrivateHandle(new ViewerInterfacePrivate(this));
  set_selected_viewer(0);
}

ViewerInterface::~ViewerInterface()
{
}

//TODO - build viewer sizes state saver and recoverer

/*
void ViewerInterface::writeSizeSettings()
{
    QSettings settings( "SCI", "Seg3D2.0" );
    settings.beginGroup("ViewerInterface");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("mainSplitter", private_->ui_.mainSplitter->saveState());
    settings.setValue("rightSplitter", private_->ui_.rightSplitter->saveState());
    settings.setValue("leftSplitter", private_->ui_.leftSplitter->saveState());
    settings.endGroup();
}

void ViewerInterface::readSizeSettings()
{
    QSettings settings( "SCI", "Seg3D2.0" );
    settings.beginGroup("ViewerInterface");
    restoreGeometry(settings.value("geometry").toByteArray());
    private_->ui_.mainSplitter->restoreState(settings.value("mainSplitter").toByteArray());
    private_->ui_.rightSplitter->restoreState(settings.value("rightSplitter").toByteArray());
    private_->ui_.leftSplitter->restoreState(settings.value("leftSplitter").toByteArray());
    settings.endGroup();
}
*/
  
void 
ViewerInterface::set_selected_viewer(int selected_viewer)
{
  if (selected_viewer >= 0 && private_->selected_viewer_ == selected_viewer) return;

  private_->selected_viewer_ = selected_viewer;

  for (int j=0;j<6;j++)
  {
    if (j != private_->selected_viewer_) private_->viewer_[j]->deselect();
  }
  private_->viewer_[private_->selected_viewer_]->select();
}


void 
ViewerInterface::set_views(int left, int right)
{
  if ((left == 1)&&(right == 0))
  {
    private_->viewer_[0]->show();
    private_->viewer_[1]->hide();
    private_->viewer_[2]->hide();
    private_->viewer_[3]->hide();
    private_->viewer_[4]->hide();
    private_->viewer_[5]->hide();
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->hide();

    QList<int> sizes; sizes.push_back(1000); sizes.push_back(0);
    private_->horiz_splitter_->setSizes(sizes);
    private_->horiz_splitter_->repaint();
  }

  if ((left == 1)&&(right == 1))
  {
    private_->viewer_[0]->show();
    private_->viewer_[1]->hide();
    private_->viewer_[2]->hide();
    private_->viewer_[3]->show();
    private_->viewer_[4]->hide();
    private_->viewer_[5]->hide();
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->show();

    QList<int> sizes; sizes.push_back(1000); sizes.push_back(1000);
    private_->horiz_splitter_->setSizes(sizes);
    private_->horiz_splitter_->repaint();
  }

  if ((left == 1)&&(right == 2))
  {
    private_->viewer_[0]->show();
    private_->viewer_[1]->hide();
    private_->viewer_[2]->hide();
    private_->viewer_[3]->show();
    private_->viewer_[4]->show();
    private_->viewer_[5]->hide();
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->show();

    QList<int> sizes; sizes.push_back(3000); sizes.push_back(1000);
    private_->horiz_splitter_->setSizes(sizes);
    
    QList<int> vsizes; vsizes.push_back(1000); 
    vsizes.push_back(1000); vsizes.push_back(0);
    private_->vert_splitter2_->setSizes(vsizes);
    private_->horiz_splitter_->repaint();
  }
  
  if ((left == 1)&&(right == 3))
  {
    private_->viewer_[0]->show();
    private_->viewer_[1]->hide();
    private_->viewer_[2]->hide();
    private_->viewer_[3]->show();
    private_->viewer_[4]->show();
    private_->viewer_[5]->show();
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->show();

    QList<int> sizes; sizes.push_back(3000); sizes.push_back(1000);
    private_->horiz_splitter_->setSizes(sizes);
    
    QList<int> vsizes; vsizes.push_back(1000); 
    vsizes.push_back(1000); vsizes.push_back(1000);
    private_->vert_splitter2_->setSizes(vsizes);
    private_->horiz_splitter_->repaint();
  }

  if ((left == 2)&&(right == 2))
  {
    private_->viewer_[0]->show();
    private_->viewer_[1]->show();
    private_->viewer_[2]->hide();
    private_->viewer_[3]->show();
    private_->viewer_[4]->show();
    private_->viewer_[5]->hide();
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->show();

    QList<int> sizes; sizes.push_back(1000); sizes.push_back(1000);
    private_->horiz_splitter_->setSizes(sizes);
    
    QList<int> vsizes; vsizes.push_back(1000); 
    vsizes.push_back(1000); vsizes.push_back(0);
    private_->vert_splitter1_->setSizes(vsizes);
    private_->vert_splitter2_->setSizes(vsizes);
    private_->horiz_splitter_->repaint();
  }

  if ((left == 2)&&(right == 3))
  {
    private_->viewer_[0]->show();
    private_->viewer_[1]->show();
    private_->viewer_[2]->hide();
    private_->viewer_[3]->show();
    private_->viewer_[4]->show();
    private_->viewer_[5]->show();
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->show();

    QList<int> sizes; sizes.push_back(1000); sizes.push_back(1000);
    private_->horiz_splitter_->setSizes(sizes);
    
    QList<int> vsizes; vsizes.push_back(1000); 
    vsizes.push_back(1000); vsizes.push_back(0);
    private_->vert_splitter1_->setSizes(vsizes);
    vsizes.last() = 1000;
    private_->vert_splitter2_->setSizes(vsizes);
    private_->horiz_splitter_->repaint();
  }
  if ((left == 3)&&(right == 3))
  {
    private_->viewer_[0]->show();
    private_->viewer_[1]->show();
    private_->viewer_[2]->show();
    private_->viewer_[3]->show();
    private_->viewer_[4]->show();
    private_->viewer_[5]->show();
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->show();

    QList<int> sizes; sizes.push_back(1000); sizes.push_back(1000);
    private_->horiz_splitter_->setSizes(sizes);
    
    QList<int> vsizes; vsizes.push_back(1000); 
    vsizes.push_back(1000); vsizes.push_back(1000);
    private_->vert_splitter1_->setSizes(vsizes);
    private_->vert_splitter2_->setSizes(vsizes);
    private_->horiz_splitter_->repaint();
  }
  
  set_selected_viewer(0);
}

} // end namespace Seg3D
