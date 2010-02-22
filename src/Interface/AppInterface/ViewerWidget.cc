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

// Utils includes
#include <Utils/Core/Log.h>

#include <Application/ViewerManager/ViewerManager.h>

// Qt Interface support classes
#include <Interface/QtInterface/QtRenderResources.h>
#include <Interface/QtInterface/QtApplication.h>
#include <Interface/QtInterface/QtRenderWidget.h>

// QT includes
#include <QtGui>
#include <QtOpenGL>

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

  QToolButton*    viewer_type_button_;
  QToolButton*    auto_view_button_;
  QToolButton*    grid_button_;
  QToolButton*    lock_button_;

  QToolButton*    flip_horiz_button_;
  QToolButton*    flip_vert_button_;

  QToolButton*    next_slice_button_;
  QToolButton*    previous_slice_button_;

  QToolButton*    slice_visible_button_;

  // Viewer actions
  // Which viewer to use
  QAction*        axial_viewer_;
  QAction*        sagittal_viewer_;
  QAction*        coronal_viewer_;
  QAction*        volume_viewer_;

  QActionGroup*   viewer_selection_;
  QMenu*          viewer_menu_;

  // Autoview a specific viewer
  QAction*        auto_view_;
  // Switch on grif in 2D viewer
  QAction*        grid_;

  // Lock zoom/pan/slice advance etc with viewers of the same type
  QAction*        lock_;

  // Buttons for next and previous slice in
  // slice viewer
  QAction*        next_slice_;
  QAction*        previous_slice_;

  // Flip the viewer horizontally / vertically
  QAction*        flip_horiz_;
  QAction*        flip_vert_;

  // visibility buttons for 2D viewer
  QAction*        slice_visible_;

  // visibility buttons for 3D viewer
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
  sagittal_icon.addPixmap(QPixmap(":/Images/Xview.png"),QIcon::Normal,QIcon::Off);

  QIcon coronal_icon;
  coronal_icon.addPixmap(QPixmap(":/Images/Yview.png"),QIcon::Normal,QIcon::Off);

  QIcon axial_icon;
  axial_icon.addPixmap(QPixmap(":/Images/Zview.png"),QIcon::Normal,QIcon::Off);

  QIcon volume_icon;
  volume_icon.addPixmap(QPixmap(":/Images/Vview.png"),QIcon::Normal,QIcon::Off);

  QIcon auto_view_icon;
  auto_view_icon.addPixmap(QPixmap(":/Images/AutoViewOff.png"),QIcon::Normal,QIcon::Off);

  QIcon next_slice_icon;
  next_slice_icon.addPixmap(QPixmap(":/Images/NextSliceOff.png"),QIcon::Normal,QIcon::Off);

  QIcon previous_slice_icon;
  previous_slice_icon.addPixmap(QPixmap(":/Images/PreviousSliceOff.png"),QIcon::Normal,QIcon::Off);

  QIcon flipvert_icon;
  flipvert_icon.addPixmap(QPixmap(":/Images/FlipVert.png"),QIcon::Normal,QIcon::On);
  flipvert_icon.addPixmap(QPixmap(":/Images/FlipVertOff.png"),QIcon::Normal,QIcon::Off);

  QIcon fliphoriz_icon;
  fliphoriz_icon.addPixmap(QPixmap(":/Images/FlipHoriz.png"),QIcon::Normal,QIcon::On);
  fliphoriz_icon.addPixmap(QPixmap(":/Images/FlipHorizOff.png"),QIcon::Normal,QIcon::Off);

  QIcon lock_icon;
  lock_icon.addPixmap(QPixmap(":/Images/Lock.png"),QIcon::Normal,QIcon::On);
  lock_icon.addPixmap(QPixmap(":/Images/LockOff.png"),QIcon::Normal,QIcon::Off);

  QIcon grid_icon;
  grid_icon.addPixmap(QPixmap(":/Images/Grid.png"),QIcon::Normal,QIcon::On);
  grid_icon.addPixmap(QPixmap(":/Images/GridOff.png"),QIcon::Normal,QIcon::Off);

  QIcon slice_visible_icon;
  slice_visible_icon.addPixmap(QPixmap(":/Images/Visible.png"),QIcon::Normal,QIcon::On);
  slice_visible_icon.addPixmap(QPixmap(":/Images/VisibleOff.png"),QIcon::Normal,QIcon::Off);

  sagittal_viewer_ = new QAction(parent);
  sagittal_viewer_->setIcon(sagittal_icon);
  sagittal_viewer_->setText(QString("Sagittal"));
  sagittal_viewer_->setToolTip(QString("Sagittal Slice Viewer"));
  sagittal_viewer_->setIconVisibleInMenu(true);
  sagittal_viewer_->setActionGroup(viewer_selection_);

  coronal_viewer_ = new QAction(parent);
  coronal_viewer_->setIcon(coronal_icon);
  coronal_viewer_->setText(QString("Coronal"));
  coronal_viewer_->setToolTip(QString("Coronal Slice Viewer"));
  coronal_viewer_->setIconVisibleInMenu(true);
  coronal_viewer_->setActionGroup(viewer_selection_);

  axial_viewer_ = new QAction(parent);
  axial_viewer_->setIcon(axial_icon);
  axial_viewer_->setText(QString("Axial"));
  axial_viewer_->setToolTip(QString("Axial Slice Viewer"));
  axial_viewer_->setIconVisibleInMenu(true);
  axial_viewer_->setActionGroup(viewer_selection_);  

  volume_viewer_ = new QAction(parent);
  volume_viewer_->setIcon(volume_icon);
  volume_viewer_->setText(QString("Volume"));
  volume_viewer_->setToolTip(QString("3D Volume Viewer"));
  volume_viewer_->setIconVisibleInMenu(true);
  volume_viewer_->setActionGroup(viewer_selection_);  

  auto_view_ = new QAction(parent);
  auto_view_->setIcon(auto_view_icon);
  auto_view_->setText(QString("AutoView"));
  auto_view_->setToolTip(QString("Zoom and Translate to see the full dataset"));
  auto_view_->setIconVisibleInMenu(true);

  next_slice_ = new QAction(parent);
  next_slice_->setIcon(next_slice_icon);
  next_slice_->setText(QString("NextSlice"));
  next_slice_->setToolTip(QString("Go to the next slice"));
  next_slice_->setIconVisibleInMenu(true);

  previous_slice_ = new QAction(parent);
  previous_slice_->setIcon(previous_slice_icon);
  previous_slice_->setText(QString("PreviousSlice"));
  previous_slice_->setToolTip(QString("Go to the previous slice"));
  previous_slice_->setIconVisibleInMenu(true);

  slice_visible_ = new QAction(parent);
  slice_visible_->setCheckable(true);
  slice_visible_->setIcon(slice_visible_icon);
  slice_visible_->setText(QString("SliceVisible"));
  slice_visible_->setToolTip(QString("Show this slice in the volume viewer"));
  slice_visible_->setIconVisibleInMenu(true);

  grid_ = new QAction(parent);
  grid_->setCheckable(true);
  grid_->setIcon(grid_icon);
  grid_->setText(QString("Grid"));
  grid_->setToolTip(QString("Enable measurement grid"));
  grid_->setIconVisibleInMenu(true);

  lock_ = new QAction(parent);
  lock_->setCheckable(true);
  lock_->setIcon(lock_icon);
  lock_->setText(QString("Lock"));
  lock_->setToolTip(QString("Lock changes to view to other viewers"));
  lock_->setIconVisibleInMenu(true);

  flip_horiz_ = new QAction(parent);
  flip_horiz_->setCheckable(true);
  flip_horiz_->setIcon(fliphoriz_icon);
  flip_horiz_->setText(QString("Flip Horizontal"));
  flip_horiz_->setToolTip(QString("Flip the slice horizontal"));
  flip_horiz_->setIconVisibleInMenu(true);

  flip_vert_ = new QAction(parent);
  flip_vert_->setCheckable(true);
  flip_vert_->setIcon(flipvert_icon);
  flip_vert_->setText(QString("Flip Vertical"));
  flip_vert_->setToolTip(QString("Flip the slice vertical"));
  flip_vert_->setIconVisibleInMenu(true);

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
  SCI_LOG_DEBUG("Create button bar for ViewWidget");

  buttonbar_ = new QFrame(parent);

  buttonbar_layout_ = new QHBoxLayout(buttonbar_);
  buttonbar_layout_->setContentsMargins(0,0,0,0);
  buttonbar_layout_->setSpacing(0);

  buttonbar_->setLayout(buttonbar_layout_);

  viewer_type_button_ = new QToolButton(buttonbar_);
  viewer_menu_ = new QMenu(viewer_type_button_);
  viewer_menu_->addAction(sagittal_viewer_);
  viewer_menu_->addAction(coronal_viewer_);
  viewer_menu_->addAction(axial_viewer_);
  viewer_menu_->addAction(volume_viewer_);
  viewer_type_button_->setPopupMode(QToolButton::InstantPopup);  
  viewer_type_button_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  viewer_type_button_->setDefaultAction(sagittal_viewer_);
  viewer_type_button_->setFixedHeight(20);  
  viewer_type_button_->setFixedWidth(80);  
  viewer_type_button_->setMenu(viewer_menu_);

  lock_button_ = new QToolButton(buttonbar_);
  lock_button_->setToolButtonStyle(Qt::ToolButtonIconOnly);  
  lock_button_->setDefaultAction(lock_);
  lock_button_->setFixedHeight(20);  
  lock_button_->setFixedWidth(20);  

  auto_view_button_ = new QToolButton(buttonbar_);
  auto_view_button_->setToolButtonStyle(Qt::ToolButtonIconOnly);  
  auto_view_button_->setDefaultAction(auto_view_);
  auto_view_button_->setFixedHeight(20);  
  auto_view_button_->setFixedWidth(20);  

  flip_horiz_button_ = new QToolButton(buttonbar_);
  flip_horiz_button_->setToolButtonStyle(Qt::ToolButtonIconOnly);  
  flip_horiz_button_->setDefaultAction(flip_horiz_);
  flip_horiz_button_->setFixedHeight(20);  
  flip_horiz_button_->setFixedWidth(20);  

  flip_vert_button_ = new QToolButton(buttonbar_);
  flip_vert_button_->setToolButtonStyle(Qt::ToolButtonIconOnly);  
  flip_vert_button_->setDefaultAction(flip_vert_);
  flip_vert_button_->setFixedHeight(20);
  flip_vert_button_->setFixedWidth(20);

  grid_button_ = new QToolButton(buttonbar_);
  grid_button_->setToolButtonStyle(Qt::ToolButtonIconOnly);  
  grid_button_->setDefaultAction(grid_);
  grid_button_->setFixedHeight(20);
  grid_button_->setFixedWidth(20);

  slice_visible_button_ = new QToolButton(buttonbar_);
  slice_visible_button_->setToolButtonStyle(Qt::ToolButtonIconOnly);  
  slice_visible_button_->setDefaultAction(slice_visible_);
  slice_visible_button_->setFixedHeight(20);
  slice_visible_button_->setFixedWidth(20);

  next_slice_button_ = new QToolButton(buttonbar_);
  next_slice_button_->setToolButtonStyle(Qt::ToolButtonIconOnly);  
  next_slice_button_->setDefaultAction(next_slice_);
  next_slice_button_->setFixedHeight(20);  
  next_slice_button_->setFixedWidth(20);  

  previous_slice_button_ = new QToolButton(buttonbar_);
  previous_slice_button_->setToolButtonStyle(Qt::ToolButtonIconOnly);  
  previous_slice_button_->setDefaultAction(previous_slice_);
  previous_slice_button_->setFixedHeight(20);  
  previous_slice_button_->setFixedWidth(20);  

  buttonbar_layout_->addWidget(viewer_type_button_);
  buttonbar_layout_->addWidget(auto_view_button_);
  buttonbar_layout_->addWidget(lock_button_);
  buttonbar_layout_->addWidget(grid_button_);
  buttonbar_layout_->addWidget(slice_visible_button_);
  buttonbar_layout_->addWidget(flip_horiz_button_);
  buttonbar_layout_->addWidget(flip_vert_button_);
  buttonbar_layout_->addWidget(next_slice_button_);
  buttonbar_layout_->addWidget(previous_slice_button_);
  buttonbar_layout_->addStretch();

  layout_->addWidget(viewer_);
  layout_->addWidget(buttonbar_);

  deselect_color_ =      QColor(85,85,85);
  deselect_color_dark_ = QColor(35,35,35);
  select_color_ =        QColor(225,125,0);
  select_color_dark_ =   QColor(180,90,0);
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

  private_->viewer_->set_viewer_id(viewer_id_);
  this->connect(this->private_->viewer_selection_, 
    SIGNAL(triggered(QAction*)), SLOT(change_view_type(QAction*))); 
}

ViewerWidget::~ViewerWidget()
{
}

void ViewerWidget::select()
{
  QPalette pal(palette());
  pal.setColor(QPalette::Light,private_->select_color_);
  pal.setColor(QPalette::Dark,private_->select_color_dark_);
  setPalette(pal);
}

void ViewerWidget::deselect()
{
  QPalette pal(palette());
  pal.setColor(QPalette::Light,private_->deselect_color_);
  pal.setColor(QPalette::Dark,private_->deselect_color_dark_);
  setPalette(pal);
}

void ViewerWidget::change_view_type( QAction* viewer_type )
{
  QAction* current_type = this->private_->viewer_type_button_->defaultAction();
  if (current_type != viewer_type)
  {
    this->private_->viewer_type_button_->setDefaultAction(viewer_type);
    ViewerHandle viewer = ViewerManager::Instance()->get_viewer(this->viewer_id_);
    if (viewer_type == this->private_->axial_viewer_)
    {
      ActionSet::Dispatch(viewer->view_mode_state, std::string("axial"));
    }
    else if (viewer_type == this->private_->coronal_viewer_)
    {
      ActionSet::Dispatch(viewer->view_mode_state, std::string("coronal"));
    }
    else if (viewer_type == this->private_->sagittal_viewer_)
    {
      ActionSet::Dispatch(viewer->view_mode_state, std::string("sagittal"));
    }
    else
    {
      ActionSet::Dispatch(viewer->view_mode_state, std::string("volume"));
    }
  }
}

} // end namespace Seg3D
