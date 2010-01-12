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

#ifndef INTERFACE_QTWIDGETS_LAYERMANAGERWIDGET_H
#define INTERFACE_QTWIDGETS_LAYERMANAGERWIDGET_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <QtGui>
#include <QSharedPointer>

// Application includes
#include <Application/Tool/ToolInterface.h>
#include <Application/Tool/Tool.h>
#include <Application/Tool/ToolManager.h>

namespace Seg3D {

class LayerManagerWidget : public QScrollArea
{   
  // Needed to make it a Qt object
  Q_OBJECT
  
Q_SIGNALS:
  void active_group_changed( int index );
  void active_layer_changec( int group_index );
  
public:
  enum layer_type{
      data_layer,
      mask_layer,
      label_layer
  };
  
private:
  
  class Group 
  {
      
    
    // Private subclass for the groups
    public:
    
      QWidget *page_;
      QVBoxLayout *verticalLayout_5;
      QWidget *background_group_;
      QVBoxLayout *verticalLayout_3;
      QWidget *header_;
      QHBoxLayout *horizontalLayout;
      QToolButton *open_button_;
      QPushButton *activate_button_;
      QToolButton *add_new_button_;
      QToolButton *close_button_;
      QFrame *group_frame_;
      QVBoxLayout *verticalLayout_2;
   
  };
   
  typedef QSharedPointer<Group> GroupHandle;
  typedef QList<GroupHandle> GroupList;
  
  // Private subclass for the layers
  class Layer : public QWidget 
  {
    public:
      std::string *name_;
      std::string *color_;
      QWidget *layer_;
      QVBoxLayout *verticalLayout_2;
      QWidget *background_;
      QWidget *progress_bar_;
      QWidget *layoutWidget;
      QHBoxLayout *backgroundHLayout;
      QWidget *typeBackground_;
      QWidget *typeGradient_;
      QHBoxLayout *horizontalLayout_9;
      QToolButton *colorChooseButton_;
      QVBoxLayout *verticalLayout;
      QPushButton *label_;
      QHBoxLayout *toolButtonHLayout_;
      QToolButton *fillOpacityButton_;
      QToolButton *brightContrastButton_;
      QToolButton *visibleButton_;
      QToolButton *isoSurfaceButton_;
      QToolButton *computeIsoSurfaceButton_;
      QToolButton *volumeTargetButton_;
      QToolButton *lockButton_;
      QHBoxLayout *brightContrastHLayout;
      QWidget *brightContrastBar;
      QVBoxLayout *verticalLayout_6;
      QHBoxLayout *brightnessHLayout;
      QLabel *brightnessLabel;
      QSlider *brightnessSlider;
      QHBoxLayout *contrastHLayout;
      QLabel *contrastLabel;
      QSlider *contrastSlider;
      QHBoxLayout *colorHLayout;
      QWidget *colorChooseBar;
      QHBoxLayout *horizontalLayout_14;
      QToolButton *color_button_01_;
      QToolButton *color_button_02_;
      QToolButton *color_button_03_;
      QToolButton *color_button_04_;
      QToolButton *color_button_05_;
      QToolButton *color_button_06_;
      QToolButton *color_button_07_;
      QToolButton *color_button_08_;
      QToolButton *color_button_09_;
      QToolButton *color_button_10_;
      QToolButton *color_button_11_;
      QToolButton *color_button_12_;
      QSharedPointer<Group> container_group_;
    
    
  };
  
  typedef QSharedPointer<Layer> LayerHandle;
  typedef QList<LayerHandle> LayerList;
  
  GroupList group_list_;
  LayerList layer_list_;                      
        
  
  int active_group_index_;
  GroupHandle active_group_;
  
  int active_layer_index_;
  LayerHandle active_layer_;
  
  
  QWidget*     main_;
  QVBoxLayout* main_layout_;
  QVBoxLayout* group_layout_;
  
  QIcon active_close_icon_;    
  QIcon inactive_close_icon_;    
  
  QIcon active_help_icon_;    
  QIcon inactive_help_icon_; 
  
  QIcon active_new_icon_;    
  QIcon inactive_new_icon_;
  
  QIcon mask_icon_;
  QIcon label_icon_;
  QIcon data_icon_;
  QIcon border_icon_;
  QIcon brightness_icon_;
  QIcon volume_visible_icon_;
  QIcon isosurface_visible_icon_;
  QIcon isosurface_computer_icon_;
  QIcon lock_icon_;
  
   
private Q_SLOTS:
  void hide_show_brightness_contrast_bar(bool);
  void hide_show_color_choose_bar(bool);
  void hide_show_group_layers(bool);
  void activate_group_button_clicked();
  void activate_layer_button_clicked();
  
  
public:
  
  LayerManagerWidget(QWidget* parent=0);
  virtual ~LayerManagerWidget();
  
  //void add_layer( layer_type type, const QString &label, boost::function<void()>, Group &container_group );
  void add_layer( layer_type type, const QString &label, const QString &dimensions );
  //void new_group( const QString &dimensions, boost::function<void()> );
  void new_group( const QString &dimensions );
  void remove_mask_layer(int);
  
  inline int get_active_layer_index(){ return active_layer_index_; }
  inline int get_active_group_index(){ return active_group_index_; }
  
  void set_active_layer(int);
  void set_active_group(int);
  
  int index_of_layer(LayerHandle);
  int index_of_group(GroupHandle);
  
  inline GroupHandle get_active_group(){ return active_group_; }
  inline LayerHandle get_active_layer(){ return active_layer_; }
  
  LayerHandle layer( QWidget *layer_);


};

}  //endnamespace Seg3d

#endif