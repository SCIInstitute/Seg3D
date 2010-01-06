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
  
private:
  
  class Group 
  {
    // Private subclass for the groups
    public:
    
      std::string *name_;
      QWidget     *page_;
      QWidget     *background_;
      QWidget     *header_;
      QHBoxLayout *hLayout_;
      QHBoxLayout *hLayout_2;
      QVBoxLayout *vLayout_;
      QVBoxLayout *vLayout_2;
      QPushButton *activate_button_;
      QToolButton *hide_show_button_;
      QToolButton *info_button_;
      QToolButton *close_button_;
      
    
      inline bool operator==(const Group& other) const
      {
        return name_ == other.name_;
      }
    
  };
   
  typedef QSharedPointer<Group> GroupHandle;
  typedef QList<GroupHandle> GroupList;
  
  // Private subclass for the layers
  class Layer : public QWidget 
  {
    public:
      QWidget *background_;
      QToolButton *colorChooseButton_;
      QToolButton *lockButton_;
      QPushButton *label_;
      QToolButton *fillOpacityButton_;
      QToolButton *brightContrastButton_;
      QToolButton *visibleButton_;
      QToolButton *isoSurfaceButton_;
      QToolButton *volumeTargetButton_;
      QToolButton *computeIsoSurfaceButton_;
      Group *container_group_;
    
    
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
  QVBoxLayout* tool_layout_;
  
public:
  
  LayerManagerWidget(QWidget* parent=0);
  virtual ~LayerManagerWidget();
  
  void add_mask_layer( const QString &label, boost::function<void()>, Group &container_group );
  void new_group();
  void remove_mask_layer(int);
  
  inline int get_active_layer_index(){ return active_layer_index_; }
  inline int get_active_group_index(){ return active_group_index_; }
  
  void set_active_layer(int);
  void set_active_group(int);
  
  int index_of_layer(LayerHandle);
  int index_of_group(GroupHandle);
  
  inline GroupHandle get_active_group(){ return active_group_; }
  inline LayerHandle get_active_layer(){ return active_layer_; }


};

}  //endnamespace Seg3d

#endif