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
#include <QtOpenGL>

// Utils includes
#include <Utils/Core/Log.h>

// Qt Interface support classes
#include <Interface/QtInterface/QtRenderResources.h>
#include <Interface/QtInterface/QtApplication.h>
#include <Interface/QtInterface/QtRenderWidget.h>

// Interface includes
#include <Interface/AppInterface/ViewerWidget.h>

namespace Seg3D {
  
class ViewerWidgetPrivate {
  public:
    ViewerWidgetPrivate(QWidget *parent);

  public:
    QVBoxLayout*    layout_;
    QtRenderWidget* viewer_;
    QFrame*         buttonbar_;
    
    QHBoxLayout*    buttonbar_layout_;
    
    QToolButton*    viewer_type_;

    QStackedLayout* viewer_options_;
    
    QFrame*         slice_buttons_;
    QFrame*         volume_buttons_;
    
    // Viewer actions
    // Which viewer to use
    QAction*        axial_viewer_;
    QAction*        sagittal_viewer_;
    QAction*        coronal_viewer_;
    QAction*        volume_viewer_;
    
    QActionGroup*   viewer_selection_;
    QMenu*          viewer_menu_;
    
    // Autoview a specific viewer
    QAction*        autoview_;
    // Switch on grif in 2D viewer
    QAction*        grid_;
    
    // Types of locks in the different viewers
    QAction*        lock_all_;
    QAction*        lock_pan_;
    QAction*        lock_zoom_;
    QAction*        lock_slice_;
    QAction*        lock_rotate_;

    // Buttons for next and previous slice in
    // slice viewer
    QAction*        next_slice_;
    QAction*        prev_slice_;

    // Flip the viewer horizontally / vertically
    QAction*        flip_horiz_;
    QAction*        flip_vert_;

    // visibility buttons for 3D viewer
    QAction*        slice_visible_;
    QAction*        slices_visible_;
    QAction*        isosurfaces_visible_;
    QAction*        volume_visible_;
    
    QColor          select_color_;
    QColor          select_color_dark_;
    QColor          deselect_color_;
    QColor          deselect_color_dark_;
    
};


ViewerWidgetPrivate::ViewerWidgetPrivate(QWidget *parent) 
{
  // ensure viewer selection is mutual exclusive
  viewer_selection_ = new QActionGroup(parent);

  QIcon sagittal_icon;
  sagittal_icon.addPixmap(QPixmap(":/Images/Xview.png"),QIcon::Normal,QIcon::On);
  sagittal_icon.addPixmap(QPixmap(":/Images/Xview.png"),QIcon::Normal,QIcon::Off);

  QIcon coronal_icon;
  coronal_icon.addPixmap(QPixmap(":/Images/Yview.png"),QIcon::Normal,QIcon::On);
  coronal_icon.addPixmap(QPixmap(":/Images/Yview.png"),QIcon::Normal,QIcon::Off);

  QIcon axial_icon;
  axial_icon.addPixmap(QPixmap(":/Images/Zview.png"),QIcon::Normal,QIcon::On);
  axial_icon.addPixmap(QPixmap(":/Images/Zview.png"),QIcon::Normal,QIcon::Off);

  sagittal_viewer_ = new QAction(parent);
//  sagittal_viewer_->setCheckable(true);
  sagittal_viewer_->setIcon(axial_icon);
  sagittal_viewer_->setText(QString("Sagittal"));
  sagittal_viewer_->setToolTip(QString("Sagittal Slice Viewer"));
  sagittal_viewer_->setIconVisibleInMenu(true);
  sagittal_viewer_->setActionGroup(viewer_selection_);

  coronal_viewer_ = new QAction(parent);
//  coronal_viewer_->setCheckable(true);
  coronal_viewer_->setIcon(coronal_icon);
  coronal_viewer_->setText(QString("Coronal"));
  coronal_viewer_->setToolTip(QString("Coronal Slice Viewer"));
  coronal_viewer_->setIconVisibleInMenu(true);
  coronal_viewer_->setActionGroup(viewer_selection_);

  axial_viewer_ = new QAction(parent);
//  axial_viewer_->setCheckable(true);
  axial_viewer_->setIcon(axial_icon);
  axial_viewer_->setText(QString("Axial"));
  axial_viewer_->setToolTip(QString("Axial Slice Viewer"));
  axial_viewer_->setIconVisibleInMenu(true);
  axial_viewer_->setActionGroup(viewer_selection_);  
  
  // BUILD VIEWER WIDGET  

  // --------------------------------------
  // Add frame around widget for selection
  layout_ = new QVBoxLayout;
  layout_->setContentsMargins(0, 0, 0, 0);
  layout_->setSpacing(0);

  // --------------------------------------
  // Generate the OpenGL part of the widget
  
  viewer_ = QtApplication::Instance()->qt_renderresources_context()
      ->create_qt_render_widget(parent);
      
  if (viewer_ == 0)
  {
    SCI_THROW_LOGICERROR("OpenGL was not initialized correctly");
  }
  viewer_->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

      
  // --------------------------------------
  // Generate button bar at the bottom of
  // the Viewer widget
  buttonbar_ = new QFrame(parent);
  
  buttonbar_layout_ = new QHBoxLayout(buttonbar_);
  buttonbar_layout_->setContentsMargins(0,0,0,0);
  buttonbar_layout_->setSpacing(0);
  
  buttonbar_->setLayout(buttonbar_layout_);
  
  viewer_type_ = new QToolButton(buttonbar_);
  viewer_type_->setPopupMode(QToolButton::InstantPopup);  
  viewer_type_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  viewer_type_->setDefaultAction(axial_viewer_);
  viewer_type_->setFixedHeight(20);
  
  viewer_menu_ = new QMenu(viewer_type_);
  viewer_menu_->addAction(sagittal_viewer_);
  viewer_menu_->addAction(coronal_viewer_);
  viewer_menu_->addAction(axial_viewer_);
  viewer_type_->setMenu(viewer_menu_);

  buttonbar_layout_->addWidget(viewer_type_);
  buttonbar_layout_->addStretch();

  layout_->addWidget(viewer_);
  layout_->addWidget(buttonbar_);
  
  deselect_color_ = QColor(85,85,85);
  deselect_color_dark_ = QColor(35,35,35);
  select_color_ = QColor(225,125,0);
  select_color_dark_ = QColor(180,90,0);
}

  
ViewerWidget::ViewerWidget(int viewer_id, QWidget *parent) :
    QFrame(parent),
    viewer_id_(viewer_id)
{
  private_ = ViewerWidgetPrivateHandle(new ViewerWidgetPrivate(this));
  setLayout(private_->layout_);
  setLineWidth(3);
  setFrameShape(QFrame::Panel);
  setFrameShadow(QFrame::Raised);
  select();
}

ViewerWidget::~ViewerWidget()
{
}

void
ViewerWidget::select()
{
  QPalette pal(palette());
  pal.setColor(QPalette::Light,private_->select_color_);
  pal.setColor(QPalette::Dark,private_->select_color_dark_);
  setPalette(pal);
}

void
ViewerWidget::deselect()
{
  QPalette pal(palette());
  pal.setColor(QPalette::Light,private_->deselect_color_);
  pal.setColor(QPalette::Dark,private_->deselect_color_dark_);
  setPalette(pal);
}

} // end namespace Seg3D
