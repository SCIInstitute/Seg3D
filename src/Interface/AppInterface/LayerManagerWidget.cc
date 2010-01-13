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
  
  // setup icons 
  {
    active_close_icon_.addFile(QString::fromUtf8(":/Images/CloseWhite.png"), 
                               QSize(), QIcon::Normal, QIcon::Off);    
    inactive_close_icon_.addFile(QString::fromUtf8(":/Images/Close.png"), 
                                 QSize(), QIcon::Normal, QIcon::Off);
    
    active_help_icon_.addFile(QString::fromUtf8(":/Images/HelpWhite.png"), 
                              QSize(), QIcon::Normal, QIcon::Off);    
    inactive_help_icon_.addFile(QString::fromUtf8(":/Images/Help.png"), 
                                QSize(), QIcon::Normal, QIcon::Off);
    
    active_new_icon_.addFile(QString::fromUtf8(":/Images/NewWhite.png"), 
                              QSize(), QIcon::Normal, QIcon::Off);    
    inactive_new_icon_.addFile(QString::fromUtf8(":/Images/New.png"), 
                                QSize(), QIcon::Normal, QIcon::Off);
    
    mask_icon_.addFile(QString::fromUtf8(":/Images/MaskWhite_shadow.png"), QSize(), QIcon::Normal, QIcon::Off);
    label_icon_.addFile(QString::fromUtf8(":/Images/LabelMapWhite.png"), QSize(), QIcon::Normal, QIcon::Off);
    data_icon_.addFile(QString::fromUtf8(":/Images/DataWhite.png"), QSize(), QIcon::Normal, QIcon::Off);
    
    border_icon_.addFile(QString::fromUtf8(":/Images/BorderOff.png"), QSize(), QIcon::Normal, QIcon::Off);
    border_icon_.addFile(QString::fromUtf8(":/Images/Border.png"), QSize(), QIcon::Normal, QIcon::On);
    
    brightness_icon_.addFile(QString::fromUtf8(":/Images/BrightnessOff.png"), QSize(), QIcon::Normal, QIcon::Off);
    brightness_icon_.addFile(QString::fromUtf8(":/Images/Brightness.png"), QSize(), QIcon::Normal, QIcon::On);
    
    volume_visible_icon_.addFile(QString::fromUtf8(":/Images/VolumeVisibleOff.png"), QSize(), QIcon::Normal, QIcon::Off);
    volume_visible_icon_.addFile(QString::fromUtf8(":/Images/VolumeVisible.png"), QSize(), QIcon::Normal, QIcon::On);
    
    isosurface_visible_icon_.addFile(QString::fromUtf8(":/Images/IsosurfaceVisibleOff.png"), QSize(), QIcon::Normal, QIcon::Off);
    isosurface_visible_icon_.addFile(QString::fromUtf8(":/Images/IsosurfaceVisible.png"), QSize(), QIcon::Normal, QIcon::On);
    
    isosurface_computer_icon_.addFile(QString::fromUtf8(":/Images/IsosurfaceComputeOff.png"), QSize(), QIcon::Normal, QIcon::Off);
    isosurface_computer_icon_.addFile(QString::fromUtf8(":/Images/IsosurfaceCompute.png"), QSize(), QIcon::Normal, QIcon::On);
    
    lock_icon_.addFile(QString::fromUtf8(":/Images/LockBigOff.png"), QSize(), QIcon::Normal, QIcon::Off);
    lock_icon_.addFile(QString::fromUtf8(":/Images/LockBig.png"), QSize(), QIcon::Normal, QIcon::On);
  }
  
  
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setContentsMargins(1, 1, 1, 1);
  setWidgetResizable(true);    
  
  main_ = new QWidget(this);
  setWidget(main_);
  
  main_layout_ = new QVBoxLayout( main_ );
  main_layout_->setContentsMargins(1, 1, 1, 1);
  main_layout_->setSpacing(1);
  
  group_layout_ = new QVBoxLayout;
  main_layout_->addLayout(group_layout_);
  main_layout_->addStretch();
  
  main_->setLayout(main_layout_);
  
  ///////////////  Testing Stuff //////
  new_group("450x900x500");
  
  // test adding a layer with a group that exists
  add_layer(data_layer, "green_footed_lemur_fetus", "450x900x500");
  add_layer(mask_layer, "blue_footed_lemur_fetus", "450x900x500");
  
  // test adding a layer to a group that doesnt exist
  add_layer(label_layer, "peace_pipe", "200x1800x200");
  
  new_group("234x654x432");
  
}
  
LayerManagerWidget::~LayerManagerWidget()
{
}
  
  
void
LayerManagerWidget::hide_show_brightness_contrast_bar(bool hideshow)
{
  SCI_LOG_MESSAGE("Hide show brightness bar button has been clicked.");
  QToolButton *hide_show_button = ::qobject_cast<QToolButton*>(sender());
  
  for (LayerList::ConstIterator i = layer_list_.constBegin(); i != layer_list_.constEnd(); i++)
  {
   if ((*i)->brightContrastButton_ == hide_show_button)
   {
     if (!hideshow) 
     {
       (*i)->brightContrastBar->hide();
       main_->adjustSize();
     }
     else {
       (*i)->brightContrastBar->show();
     }
   } 
  }
}

void
LayerManagerWidget::hide_show_color_choose_bar(bool hideshow)
{
  QToolButton *hide_show_button = ::qobject_cast<QToolButton*>(sender());
  
  
  for (LayerList::ConstIterator i = layer_list_.constBegin(); i != layer_list_.constEnd(); i++)
  {
    if ((*i)->colorChooseButton_ == hide_show_button)
    {
      if (!hideshow) 
      {
        (*i)->colorChooseBar->hide();
      }
      else {
        (*i)->colorChooseBar->show();
      }
      main_->adjustSize();
    } 
  }
  
}
  
void
LayerManagerWidget::hide_show_group_layers(bool hideshow)
{
  QToolButton *hide_show_button = ::qobject_cast<QToolButton*>(sender());
  
  for (GroupList::ConstIterator i = group_list_.constBegin(); i != group_list_.constEnd(); i++)
  {
    if ((*i)->open_button_ == hide_show_button)
    {
      if (hideshow) 
      {
        (*i)->group_frame_->show();
      }
      else {
        (*i)->group_frame_->hide();
      }
      main_->adjustSize();
    } 
  }
  
  
}

void
LayerManagerWidget::activate_group_button_clicked()
{
  QPushButton *active_button = ::qobject_cast<QPushButton*>(sender());
  int group_index = 0;
  
  for (GroupList::ConstIterator i = group_list_.constBegin(); i != group_list_.constEnd(); i++)
  {
    if ((*i)->activate_button_ != active_button)
    {
      (*i)->background_group_->setStyleSheet(QString::fromUtf8("QWidget#background_group_ { background-color: rgb(220, 220, 220); }"));
    } 
    else 
    {
      (*i)->background_group_->setStyleSheet(QString::fromUtf8("QWidget#background_group_ { background-color: rgb(255, 128, 0); }"));
      active_group_index_ = group_index;
      Q_EMIT active_group_changed(group_index);
    }

  }
}
  
void
LayerManagerWidget::activate_layer_button_clicked()
{
  QPushButton *active_button = ::qobject_cast<QPushButton*>(sender());
  int layer_index = 0;
  int group_index = 0;
  
  for (LayerList::ConstIterator i = layer_list_.constBegin(); i != layer_list_.constEnd(); i++)
  {
    if ((*i)->label_ == active_button)
    {
      for (GroupList::ConstIterator j = group_list_.constBegin(); j != group_list_.constEnd(); j++)
      {
        if ((*i)->container_group_ != (*j)) 
        {
          (*j)->background_group_->setStyleSheet(QString::fromUtf8("QWidget#background_group_ { background-color: rgb(220, 220, 220); }"));
        } 
        else 
        {
          (*j)->background_group_->setStyleSheet(QString::fromUtf8("QWidget#background_group_ { background-color: rgb(255, 128, 0); }"));
          active_group_index_ = group_index;
          Q_EMIT active_group_changed(group_index);
        }
        group_index++;
      }
      active_layer_index_ = layer_index;
      Q_EMIT active_layer_changed(layer_index);
    }
    layer_index++;
  }
}
  
  
void 
LayerManagerWidget::add_layer( layer_type type, const QString &label, const QString &dimensions )
{
  LayerHandle layer_handle_(new Layer);
  
  layer_handle_->layer_ = new QWidget;
  layer_handle_->layer_->setObjectName(QString::fromUtf8("layer_"));
  layer_handle_->layer_->setGeometry(QRect(8, 10, 213, 144));
  QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(layer_handle_->layer_->sizePolicy().hasHeightForWidth());
  layer_handle_->layer_->setSizePolicy(sizePolicy);
  // Set Style Sheet for the layer Widget
  {
  layer_handle_->layer_->setStyleSheet(QString::fromUtf8("QWidget#progress_bar_{\n"
                                          " border-radius:6px;\n"
                                          " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(255, 128, 0, 170), stop:0.5 rgba(255, 255, 255, 0), stop:1 rgba(255, 128, 0, 170));\n"
                                          "}\n"
                                          "\n"
                                          "QWidget#brightContrastBar{\n"
                                          " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(221, 221, 221, 255), stop:0.155779 rgba(228, 228, 228, 255), stop:1 rgba(202, 204, 204, 255));\n"
                                          " border: 1px solid gray;\n"
                                          " border-radius: 6px;\n"
                                          "\n"
                                          "}\n"
                                          "\n"
                                          "QWidget#colorChooseBar{\n"
                                          " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(221, 221, 221, 255), stop:0.155779 rgba(228, 228, 228, 255), stop:1 rgba(202, 204, 204, 255));\n"
                                          " border: 1px solid gray;\n"
                                          " border-radius: 6px;\n"
                                          "}\n"
                                          "\n"
                                          "QWidget#background_{\n"
                                          " \n"
                                          " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(221, 221, 221, 255), stop:0.155779 rgba(228, 228, 228, 255), stop:1 rgba(202"
                                          ", 204, 204, 255));\n"
                                          " border-radius: 3px;\n"
                                          " border: 1px solid rgb(127, 127, 127);\n"
                                          "}\n"
                                          "\n"
                                          "QToolButton#colorChooseButton_{\n"
                                          " background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          " border: 1px solid qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          " border-radius: 4px;\n"
                                          " color: white;\n"
                                          "}\n"
                                          "\n"
                                          "QWidget#typeBackground_{\n"
                                          " background-color: rgb(255, 128, 0);\n"
                                          " border-radius: 4px;\n"
                                          " border: 1px solid white;\n"
                                          "}\n"
                                          "QWidget#typeGradient_{\n"
                                          " /*background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(0, 0, 0, 12), stop:0.190955 rgba(255, 255, 255, 0), stop:1 rgba(0, 0, 0, 63));*/\n"
                                          " border-radius: 4px;\n"
                                          " border: 1px solid white;\n"
                                          "}\n"
                                          "\n"
                                          "\n"
                                          "QToolButton#colorChooseButton_:pressed{\n"
                                          " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.50005, y2:1, stop:0 rgba(207, 104, 0, 255), stop:0.879397 rgba(255, 128, 0, 255), st"
                                          "op:1 rgba(230, 115, 0, 255));\n"
                                          "    border: 1px solid white;\n"
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "\n"
                                          "QToolButton#lockButton_{\n"
                                          " background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          " border-radius: 4px;\n"
                                          " color: white;\n"
                                          "}\n"
                                          "\n"
                                          "QToolButton#lockButton_:pressed{\n"
                                          " \n"
                                          " background-color: rgb(170,170,170);\n"
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "\n"
                                          "QPushButton#label_{\n"
                                          " text-align: left;\n"
                                          " color: black;\n"
                                          " background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          "}\n"
                                          "QPushButton#label_:pressed{\n"
                                          " background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          "}\n"
                                          "\n"
                                          "/* Buttons */\n"
                                          "QToolButton#fillOpacityButton_{\n"
                                          " background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          " border: 1px solid qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255"
                                          ", 255, 255, 0));\n"
                                          "}\n"
                                          "QToolButton#fillOpacityButton_:pressed{\n"
                                          " background-color: gray;\n"
                                          " color: white;\n"
                                          "}\n"
                                          "\n"
                                          "QToolButton#brightContrastButton_{\n"
                                          " background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          " border: 1px solid qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          "}\n"
                                          "QToolButton#brightContrastButton_:pressed{\n"
                                          " background-color: gray;\n"
                                          " color: white;\n"
                                          "}\n"
                                          "\n"
                                          "QToolButton#visibleButton_{\n"
                                          " background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          " border: 1px solid qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          "}\n"
                                          "QToolButton#visibleButton_:pressed{\n"
                                          " background-color: gray;\n"
                                          " color: white;\n"
                                          "}\n"
                                          "\n"
                                          "\n"
                                          "QToolButton#isoSurfaceButton_{\n"
                                          " background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          ""
                                          " border: 1px solid qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          "}\n"
                                          "QToolButton#isoSurfaceButton_:pressed{\n"
                                          " background-color: gray;\n"
                                          " color: white;\n"
                                          "}\n"
                                          "\n"
                                          "QToolButton#computeIsoSurfaceButton_{\n"
                                          " background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          " border: 1px solid qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          "}\n"
                                          "QToolButton#computeIsoSurfaceButton_:pressed{\n"
                                          " background-color: gray;\n"
                                          " color: white;\n"
                                          "}\n"
                                          "\n"
                                          "QToolButton#volumeTargetButton_{\n"
                                          " background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          " border: 1px solid qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));\n"
                                          "}\n"
                                          "QToolButton#volumeTargetButton_:pressed{\n"
                                          " background-color: gray;\n"
                                          " color: white;\n"
                                          "}\n"
                                          "\n"
                                          "QToolButton#color_button_01_{\n"
                                          " backgrou"
                                          "nd-color: rgb(251,255,74);\n"
                                          " border: 1px solid white;\n"
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "QToolButton#color_button_01_:pressed{\n"
                                          " border: 1px solid gray;\n"
                                          "}\n"
                                          "\n"
                                          "\n"
                                          "QToolButton#color_button_02_{\n"
                                          " background-color: rgb(248,188,37);\n"
                                          " border: 1px solid white;\n"
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "QToolButton#color_button_02_:pressed{\n"
                                          " border: 1px solid gray;\n"
                                          "}\n"
                                          "\n"
                                          "\n"
                                          "QToolButton#color_button_03_{\n"
                                          " background-color: rgb(248,152,28);\n"
                                          " border: 1px solid white;\n"
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "QToolButton#color_button_03_:pressed{\n"
                                          " border: 1px solid gray;\n"
                                          "}\n"
                                          "\n"
                                          "\n"
                                          "QToolButton#color_button_04_{\n"
                                          " background-color: rgb(251,78,15);\n"
                                          " border: 1px solid white;\n"
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "QToolButton#color_button_04_:pressed{\n"
                                          " border: 1px solid gray;\n"
                                          "}\n"
                                          "\n"
                                          "\n"
                                          "QToolButton#color_button_05_{\n"
                                          " background-color: rgb(252,21,17);\n"
                                          " border: 1px solid white;\n"
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "QToolButton#color_button_05_:pressed{\n"
                                          " bord"
                                          "er: 1px solid gray;\n"
                                          "}\n"
                                          "\n"
                                          "\n"
                                          "QToolButton#color_button_06_{\n"
                                          " background-color: rgb(166,12,73);\n"
                                          " border: 1px solid white;\n"
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "QToolButton#color_button_06_:pressed{\n"
                                          " border: 1px solid gray;\n"
                                          "}\n"
                                          "\n"
                                          "\n"
                                          "QToolButton#color_button_07_{\n"
                                          " background-color: rgb(135,0,172);\n"
                                          " border: 1px solid white;\n"
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "QToolButton#color_button_07_:pressed{\n"
                                          " border: 1px solid gray;\n"
                                          "}\n"
                                          "\n"
                                          "\n"
                                          "QToolButton#color_button_08_{\n"
                                          " background-color: rgb(66,0,161);\n"
                                          " border: 1px solid white;\n"
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "QToolButton#color_button_08_:pressed{\n"
                                          " border: 1px solid gray;\n"
                                          "}\n"
                                          "\n"
                                          "\n"
                                          "QToolButton#color_button_09_{\n"
                                          " background-color: rgb(45,66,250);\n"
                                          " border: 1px solid white;\n"
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "QToolButton#color_button_09_:pressed{\n"
                                          " border: 1px solid gray;\n"
                                          "}\n"
                                          "\n"
                                          "\n"
                                          "QToolButton#color_button_10_{\n"
                                          " background-color: rgb(32,146,204);\n"
                                          " border: 1px solid white;\n"
                                          ""
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "QToolButton#color_button_10_:pressed{\n"
                                          " border: 1px solid gray;\n"
                                          "}\n"
                                          "\n"
                                          "\n"
                                          "QToolButton#color_button_11_{\n"
                                          " background-color: rgb(100,177,61);\n"
                                          " border: 1px solid white;\n"
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "QToolButton#color_button_11_:pressed{\n"
                                          " border: 1px solid gray;\n"
                                          "}\n"
                                          "\n"
                                          "\n"
                                          "QToolButton#color_button_12_{\n"
                                          " background-color: rgb(205,235,66);\n"
                                          " border: 1px solid white;\n"
                                          " border-radius: 4px;\n"
                                          "}\n"
                                          "QToolButton#color_button_12_:pressed{\n"
                                          " border: 1px solid gray;\n"
                                          "}"));
  }
  
  layer_handle_->verticalLayout_2 = new QVBoxLayout(layer_handle_->layer_);
  layer_handle_->verticalLayout_2->setSpacing(0);
  layer_handle_->verticalLayout_2->setContentsMargins(0, 0, 0, 0);
  layer_handle_->verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
  layer_handle_->background_ = new QWidget(layer_handle_->layer_);
  layer_handle_->background_->setObjectName(QString::fromUtf8("background_"));
  QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy1.setHorizontalStretch(0);
  sizePolicy1.setVerticalStretch(0);
  sizePolicy1.setHeightForWidth(layer_handle_->background_->sizePolicy().hasHeightForWidth());
  layer_handle_->background_->setSizePolicy(sizePolicy1);
  layer_handle_->background_->setMinimumSize(QSize(213, 46));
  layer_handle_->background_->setMaximumSize(QSize(213, 46));
  layer_handle_->progress_bar_ = new QWidget(layer_handle_->background_);
  layer_handle_->progress_bar_->setObjectName(QString::fromUtf8("progress_bar_"));
  layer_handle_->progress_bar_->setGeometry(QRect(0, 0, 0, 46));
  layer_handle_->progress_bar_->setStyleSheet(QString::fromUtf8(""));
  layer_handle_->layoutWidget = new QWidget(layer_handle_->background_);
  layer_handle_->layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
  layer_handle_->layoutWidget->setGeometry(QRect(2, -1, 215, 49));
  layer_handle_->backgroundHLayout = new QHBoxLayout(layer_handle_->layoutWidget);
  layer_handle_->backgroundHLayout->setSpacing(0);
  layer_handle_->backgroundHLayout->setContentsMargins(11, 11, 11, 11);
  layer_handle_->backgroundHLayout->setObjectName(QString::fromUtf8("backgroundHLayout"));
  layer_handle_->backgroundHLayout->setContentsMargins(0, 0, 0, 0);
  layer_handle_->typeBackground_ = new QWidget(layer_handle_->layoutWidget);
  layer_handle_->typeBackground_->setObjectName(QString::fromUtf8("typeBackground_"));
  sizePolicy1.setHeightForWidth(layer_handle_->typeBackground_->sizePolicy().hasHeightForWidth());
  layer_handle_->typeBackground_->setSizePolicy(sizePolicy1);
  layer_handle_->typeBackground_->setMinimumSize(QSize(31, 42));
  layer_handle_->typeBackground_->setMaximumSize(QSize(31, 42));
  layer_handle_->typeGradient_ = new QWidget(layer_handle_->typeBackground_);
  layer_handle_->typeGradient_->setObjectName(QString::fromUtf8("typeGradient_"));
  layer_handle_->typeGradient_->setGeometry(QRect(0, 0, 31, 42));
  sizePolicy1.setHeightForWidth(layer_handle_->typeGradient_->sizePolicy().hasHeightForWidth());
  layer_handle_->typeGradient_->setSizePolicy(sizePolicy1);
  layer_handle_->typeGradient_->setMinimumSize(QSize(31, 42));
  layer_handle_->typeGradient_->setMaximumSize(QSize(31, 42));
  layer_handle_->horizontalLayout_9 = new QHBoxLayout(layer_handle_->typeGradient_);
  layer_handle_->horizontalLayout_9->setSpacing(0);
  layer_handle_->horizontalLayout_9->setContentsMargins(0, 0, 0, 0);
  layer_handle_->horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
  layer_handle_->colorChooseButton_ = new QToolButton(layer_handle_->typeGradient_);
  layer_handle_->colorChooseButton_->setObjectName(QString::fromUtf8("colorChooseButton_"));
  sizePolicy1.setHeightForWidth(layer_handle_->colorChooseButton_->sizePolicy().hasHeightForWidth());
  layer_handle_->colorChooseButton_->setSizePolicy(sizePolicy1);
  layer_handle_->colorChooseButton_->setMinimumSize(QSize(31, 42));
  layer_handle_->colorChooseButton_->setMaximumSize(QSize(31, 42));
  QFont font;
  font.setPointSize(25);
  font.setBold(true);
  font.setWeight(75);
  
  layer_handle_->colorChooseButton_->setFont(font);
  switch (type)
  {
    case data_layer:
      layer_handle_->colorChooseButton_->setIcon(data_icon_);
      break;
    case mask_layer:
      layer_handle_->colorChooseButton_->setIcon(mask_icon_);
      break;
    case label_layer:
      layer_handle_->colorChooseButton_->setIcon(label_icon_);
      break;
    default:
      break;
  }
  
  layer_handle_->colorChooseButton_->setIconSize(QSize(25, 25));
  layer_handle_->colorChooseButton_->setCheckable(true);
  layer_handle_->colorChooseButton_->setAutoRaise(false);
  
  layer_handle_->horizontalLayout_9->addWidget(layer_handle_->colorChooseButton_);
  connect(layer_handle_->colorChooseButton_, SIGNAL(clicked(bool)), this, SLOT(hide_show_color_choose_bar(bool)));
  
  
  layer_handle_->backgroundHLayout->addWidget(layer_handle_->typeBackground_);
  
  layer_handle_->verticalLayout = new QVBoxLayout();
  layer_handle_->verticalLayout->setSpacing(0);
  layer_handle_->verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
  layer_handle_->label_ = new QPushButton(layer_handle_->layoutWidget);
  layer_handle_->label_->setObjectName(QString::fromUtf8("label_"));
  sizePolicy1.setHeightForWidth(layer_handle_->label_->sizePolicy().hasHeightForWidth());
  layer_handle_->label_->setSizePolicy(sizePolicy1);
  layer_handle_->label_->setMinimumSize(QSize(180, 21));
  layer_handle_->label_->setMaximumSize(QSize(180, 21));
  layer_handle_->label_->setFlat(true);
  layer_handle_->label_->setText(label);
  
  layer_handle_->verticalLayout->addWidget(layer_handle_->label_);
  
  connect(layer_handle_->label_, SIGNAL(clicked()), this, SLOT(activate_layer_button_clicked()));
  
  layer_handle_->toolButtonHLayout_ = new QHBoxLayout();
  layer_handle_->toolButtonHLayout_->setSpacing(6);
  layer_handle_->toolButtonHLayout_->setObjectName(QString::fromUtf8("toolButtonHLayout_"));
  layer_handle_->fillOpacityButton_ = new QToolButton(layer_handle_->layoutWidget);
  layer_handle_->fillOpacityButton_->setObjectName(QString::fromUtf8("fillOpacityButton_"));
  layer_handle_->fillOpacityButton_->setMinimumSize(QSize(16, 16));
  layer_handle_->fillOpacityButton_->setMaximumSize(QSize(16, 16));
  layer_handle_->fillOpacityButton_->setIcon(border_icon_);
  layer_handle_->fillOpacityButton_->setCheckable(true);
  
  layer_handle_->toolButtonHLayout_->addWidget(layer_handle_->fillOpacityButton_);
  
  layer_handle_->brightContrastButton_ = new QToolButton(layer_handle_->layoutWidget);
  layer_handle_->brightContrastButton_->setObjectName(QString::fromUtf8("brightContrastButton_"));
  layer_handle_->brightContrastButton_->setMinimumSize(QSize(16, 16));
  layer_handle_->brightContrastButton_->setMaximumSize(QSize(16, 16));
  layer_handle_->brightContrastButton_->setIcon(brightness_icon_);
  layer_handle_->brightContrastButton_->setCheckable(true);
  
  layer_handle_->toolButtonHLayout_->addWidget(layer_handle_->brightContrastButton_);
  connect(layer_handle_->brightContrastButton_, SIGNAL(clicked(bool)), this, SLOT(hide_show_brightness_contrast_bar(bool)));

  
  
  layer_handle_->visibleButton_ = new QToolButton(layer_handle_->layoutWidget);
  layer_handle_->visibleButton_->setObjectName(QString::fromUtf8("visibleButton_"));
  layer_handle_->visibleButton_->setMinimumSize(QSize(16, 16));
  layer_handle_->visibleButton_->setMaximumSize(QSize(16, 16));
  layer_handle_->visibleButton_->setIcon(volume_visible_icon_);
  layer_handle_->visibleButton_->setCheckable(true);
  
  layer_handle_->toolButtonHLayout_->addWidget(layer_handle_->visibleButton_);
  
  layer_handle_->isoSurfaceButton_ = new QToolButton(layer_handle_->layoutWidget);
  layer_handle_->isoSurfaceButton_->setObjectName(QString::fromUtf8("isoSurfaceButton_"));
  layer_handle_->isoSurfaceButton_->setMinimumSize(QSize(16, 16));
  layer_handle_->isoSurfaceButton_->setMaximumSize(QSize(16, 16));
  layer_handle_->isoSurfaceButton_->setIcon(isosurface_visible_icon_);
  layer_handle_->isoSurfaceButton_->setCheckable(true);
  
  layer_handle_->toolButtonHLayout_->addWidget(layer_handle_->isoSurfaceButton_);
  
  layer_handle_->computeIsoSurfaceButton_ = new QToolButton(layer_handle_->layoutWidget);
  layer_handle_->computeIsoSurfaceButton_->setObjectName(QString::fromUtf8("computeIsoSurfaceButton_"));
  layer_handle_->computeIsoSurfaceButton_->setMinimumSize(QSize(16, 16));
  layer_handle_->computeIsoSurfaceButton_->setMaximumSize(QSize(16, 16));
  layer_handle_->computeIsoSurfaceButton_->setIcon(isosurface_computer_icon_);
  layer_handle_->computeIsoSurfaceButton_->setCheckable(false);
  
  layer_handle_->toolButtonHLayout_->addWidget(layer_handle_->computeIsoSurfaceButton_);
  
  layer_handle_->volumeTargetButton_ = new QToolButton(layer_handle_->layoutWidget);
  layer_handle_->volumeTargetButton_->setObjectName(QString::fromUtf8("volumeTargetButton_"));
  layer_handle_->volumeTargetButton_->setMinimumSize(QSize(16, 16));
  layer_handle_->volumeTargetButton_->setMaximumSize(QSize(16, 16));
  
  layer_handle_->toolButtonHLayout_->addWidget(layer_handle_->volumeTargetButton_);
  
  layer_handle_->lockButton_ = new QToolButton(layer_handle_->layoutWidget);
  layer_handle_->lockButton_->setObjectName(QString::fromUtf8("lockButton_"));
  layer_handle_->lockButton_->setMinimumSize(QSize(28, 16));
  layer_handle_->lockButton_->setMaximumSize(QSize(28, 16));
  layer_handle_->lockButton_->setFont(font);
  layer_handle_->lockButton_->setIcon(lock_icon_);
  layer_handle_->lockButton_->setIconSize(QSize(25, 25));
  layer_handle_->lockButton_->setCheckable(true);
  layer_handle_->lockButton_->setChecked(false);
  
  layer_handle_->toolButtonHLayout_->addWidget(layer_handle_->lockButton_);
  
  
  layer_handle_->verticalLayout->addLayout(layer_handle_->toolButtonHLayout_);
  
  
  layer_handle_->backgroundHLayout->addLayout(layer_handle_->verticalLayout);
  
  
  layer_handle_->verticalLayout_2->addWidget(layer_handle_->background_);
  
  layer_handle_->brightContrastHLayout = new QHBoxLayout();
  layer_handle_->brightContrastHLayout->setSpacing(6);
  layer_handle_->brightContrastHLayout->setObjectName(QString::fromUtf8("brightContrastHLayout"));
  layer_handle_->brightContrastBar = new QWidget(layer_handle_->layer_);
  layer_handle_->brightContrastBar->setObjectName(QString::fromUtf8("brightContrastBar"));
  sizePolicy1.setHeightForWidth(layer_handle_->brightContrastBar->sizePolicy().hasHeightForWidth());
  layer_handle_->brightContrastBar->setSizePolicy(sizePolicy1);
  layer_handle_->brightContrastBar->setMinimumSize(QSize(191, 44));
  layer_handle_->brightContrastBar->setMaximumSize(QSize(191, 44));
  layer_handle_->brightContrastBar->setStyleSheet(QString::fromUtf8(""));
  layer_handle_->verticalLayout_6 = new QVBoxLayout(layer_handle_->brightContrastBar);
  layer_handle_->verticalLayout_6->setSpacing(0);
  layer_handle_->verticalLayout_6->setContentsMargins(0, 0, 0, 0);
  layer_handle_->verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
  layer_handle_->brightnessHLayout = new QHBoxLayout();
  layer_handle_->brightnessHLayout->setSpacing(0);
  layer_handle_->brightnessHLayout->setObjectName(QString::fromUtf8("brightnessHLayout"));
  layer_handle_->brightnessHLayout->setContentsMargins(4, -1, 10, -1);
  layer_handle_->brightnessLabel = new QLabel(layer_handle_->brightContrastBar);
  layer_handle_->brightnessLabel->setObjectName(QString::fromUtf8("brightnessLabel"));
  QFont font1;
  font1.setPointSize(12);
  layer_handle_->brightnessLabel->setFont(font1);
  layer_handle_->brightnessLabel->setText("Brightness:");
  
  layer_handle_->brightnessHLayout->addWidget(layer_handle_->brightnessLabel);
  
  layer_handle_->brightnessSlider = new QSlider(layer_handle_->brightContrastBar);
  layer_handle_->brightnessSlider->setObjectName(QString::fromUtf8("brightnessSlider"));
  layer_handle_->brightnessSlider->setOrientation(Qt::Horizontal);
  layer_handle_->brightnessSlider->setTickPosition(QSlider::NoTicks);
  
  layer_handle_->brightnessHLayout->addWidget(layer_handle_->brightnessSlider);
  
  layer_handle_->brightnessHLayout->setStretch(0, 2);
  layer_handle_->brightnessHLayout->setStretch(1, 3);
  
  layer_handle_->verticalLayout_6->addLayout(layer_handle_->brightnessHLayout);
  
  layer_handle_->contrastHLayout = new QHBoxLayout();
  layer_handle_->contrastHLayout->setSpacing(0);
  layer_handle_->contrastHLayout->setObjectName(QString::fromUtf8("contrastHLayout"));
  layer_handle_->contrastHLayout->setContentsMargins(4, -1, 10, -1);
  layer_handle_->contrastLabel = new QLabel(layer_handle_->brightContrastBar);
  layer_handle_->contrastLabel->setObjectName(QString::fromUtf8("contrastLabel"));
  layer_handle_->contrastLabel->setFont(font1);
  layer_handle_->contrastLabel->setText("Contrast:");
  
  layer_handle_->contrastHLayout->addWidget(layer_handle_->contrastLabel);
  
  layer_handle_->contrastSlider = new QSlider(layer_handle_->brightContrastBar);
  layer_handle_->contrastSlider->setObjectName(QString::fromUtf8("contrastSlider"));
  layer_handle_->contrastSlider->setSingleStep(1);
  layer_handle_->contrastSlider->setOrientation(Qt::Horizontal);
  layer_handle_->contrastSlider->setTickPosition(QSlider::NoTicks);
  
  layer_handle_->contrastHLayout->addWidget(layer_handle_->contrastSlider);
  
  layer_handle_->contrastHLayout->setStretch(0, 2);
  layer_handle_->contrastHLayout->setStretch(1, 3);
  
  layer_handle_->verticalLayout_6->addLayout(layer_handle_->contrastHLayout);
  
  
  layer_handle_->brightContrastHLayout->addWidget(layer_handle_->brightContrastBar);
  
  
  layer_handle_->verticalLayout_2->addLayout(layer_handle_->brightContrastHLayout);
  
  layer_handle_->colorHLayout = new QHBoxLayout();
  layer_handle_->colorHLayout->setSpacing(0);
  layer_handle_->colorHLayout->setObjectName(QString::fromUtf8("colorHLayout"));
  layer_handle_->colorChooseBar = new QWidget(layer_handle_->layer_);
  layer_handle_->colorChooseBar->setObjectName(QString::fromUtf8("colorChooseBar"));
  sizePolicy1.setHeightForWidth(layer_handle_->colorChooseBar->sizePolicy().hasHeightForWidth());
  layer_handle_->colorChooseBar->setSizePolicy(sizePolicy1);
  layer_handle_->colorChooseBar->setMinimumSize(QSize(201, 23));
  layer_handle_->colorChooseBar->setMaximumSize(QSize(201, 23));
  layer_handle_->colorChooseBar->setStyleSheet(QString::fromUtf8(""));
  layer_handle_->horizontalLayout_14 = new QHBoxLayout(layer_handle_->colorChooseBar);
  layer_handle_->horizontalLayout_14->setSpacing(0);
  layer_handle_->horizontalLayout_14->setContentsMargins(11, 11, 11, 11);
  layer_handle_->horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
  layer_handle_->horizontalLayout_14->setContentsMargins(5, 0, 0, 0);
  
  layer_handle_->color_button_01_ = new QToolButton(layer_handle_->colorChooseBar);
  layer_handle_->color_button_01_->setObjectName(QString::fromUtf8("color_button_01_"));
  layer_handle_->color_button_01_->setMinimumSize(QSize(16, 16));
  layer_handle_->color_button_01_->setMaximumSize(QSize(16, 16));
  layer_handle_->color_button_01_->setIconSize(QSize(25, 25));
  
  layer_handle_->horizontalLayout_14->addWidget(layer_handle_->color_button_01_);
  
  layer_handle_->color_button_02_ = new QToolButton(layer_handle_->colorChooseBar);
  layer_handle_->color_button_02_->setObjectName(QString::fromUtf8("color_button_02_"));
  layer_handle_->color_button_02_->setMinimumSize(QSize(16, 16));
  layer_handle_->color_button_02_->setMaximumSize(QSize(16, 16));
  layer_handle_->color_button_02_->setIconSize(QSize(25, 25));
  
  layer_handle_->horizontalLayout_14->addWidget(layer_handle_->color_button_02_);
  
  layer_handle_->color_button_03_ = new QToolButton(layer_handle_->colorChooseBar);
  layer_handle_->color_button_03_->setObjectName(QString::fromUtf8("color_button_03_"));
  layer_handle_->color_button_03_->setMinimumSize(QSize(16, 16));
  layer_handle_->color_button_03_->setMaximumSize(QSize(16, 16));
  layer_handle_->color_button_03_->setIconSize(QSize(25, 25));
  
  layer_handle_->horizontalLayout_14->addWidget(layer_handle_->color_button_03_);
  
  layer_handle_->color_button_04_ = new QToolButton(layer_handle_->colorChooseBar);
  layer_handle_->color_button_04_->setObjectName(QString::fromUtf8("color_button_04_"));
  layer_handle_->color_button_04_->setMinimumSize(QSize(16, 16));
  layer_handle_->color_button_04_->setMaximumSize(QSize(16, 16));
  layer_handle_->color_button_04_->setIconSize(QSize(25, 25));
  
  layer_handle_->horizontalLayout_14->addWidget(layer_handle_->color_button_04_);
  
  layer_handle_->color_button_05_ = new QToolButton(layer_handle_->colorChooseBar);
  layer_handle_->color_button_05_->setObjectName(QString::fromUtf8("color_button_05_"));
  layer_handle_->color_button_05_->setMinimumSize(QSize(16, 16));
  layer_handle_->color_button_05_->setMaximumSize(QSize(16, 16));
  layer_handle_->color_button_05_->setIconSize(QSize(25, 25));
  
  layer_handle_->horizontalLayout_14->addWidget(layer_handle_->color_button_05_);
  
  layer_handle_->color_button_06_ = new QToolButton(layer_handle_->colorChooseBar);
  layer_handle_->color_button_06_->setObjectName(QString::fromUtf8("color_button_06_"));
  layer_handle_->color_button_06_->setMinimumSize(QSize(16, 16));
  layer_handle_->color_button_06_->setMaximumSize(QSize(16, 16));
  layer_handle_->color_button_06_->setIconSize(QSize(25, 25));
  
  layer_handle_->horizontalLayout_14->addWidget(layer_handle_->color_button_06_);
  
  layer_handle_->color_button_07_ = new QToolButton(layer_handle_->colorChooseBar);
  layer_handle_->color_button_07_->setObjectName(QString::fromUtf8("color_button_07_"));
  layer_handle_->color_button_07_->setMinimumSize(QSize(16, 16));
  layer_handle_->color_button_07_->setMaximumSize(QSize(16, 16));
  layer_handle_->color_button_07_->setIconSize(QSize(25, 25));
  
  layer_handle_->horizontalLayout_14->addWidget(layer_handle_->color_button_07_);
  
  layer_handle_->color_button_08_ = new QToolButton(layer_handle_->colorChooseBar);
  layer_handle_->color_button_08_->setObjectName(QString::fromUtf8("color_button_08_"));
  layer_handle_->color_button_08_->setMinimumSize(QSize(16, 16));
  layer_handle_->color_button_08_->setMaximumSize(QSize(16, 16));
  layer_handle_->color_button_08_->setIconSize(QSize(25, 25));
  
  layer_handle_->horizontalLayout_14->addWidget(layer_handle_->color_button_08_);
  
  layer_handle_->color_button_09_ = new QToolButton(layer_handle_->colorChooseBar);
  layer_handle_->color_button_09_->setObjectName(QString::fromUtf8("color_button_09_"));
  layer_handle_->color_button_09_->setMinimumSize(QSize(16, 16));
  layer_handle_->color_button_09_->setMaximumSize(QSize(16, 16));
  layer_handle_->color_button_09_->setIconSize(QSize(25, 25));
  
  layer_handle_->horizontalLayout_14->addWidget(layer_handle_->color_button_09_);
  
  layer_handle_->color_button_10_ = new QToolButton(layer_handle_->colorChooseBar);
  layer_handle_->color_button_10_->setObjectName(QString::fromUtf8("color_button_10_"));
  layer_handle_->color_button_10_->setMinimumSize(QSize(16, 16));
  layer_handle_->color_button_10_->setMaximumSize(QSize(16, 16));
  layer_handle_->color_button_10_->setIconSize(QSize(25, 25));
  
  layer_handle_->horizontalLayout_14->addWidget(layer_handle_->color_button_10_);
  
  layer_handle_->color_button_11_ = new QToolButton(layer_handle_->colorChooseBar);
  layer_handle_->color_button_11_->setObjectName(QString::fromUtf8("color_button_11_"));
  layer_handle_->color_button_11_->setMinimumSize(QSize(16, 16));
  layer_handle_->color_button_11_->setMaximumSize(QSize(16, 16));
  layer_handle_->color_button_11_->setIconSize(QSize(25, 25));
  
  layer_handle_->horizontalLayout_14->addWidget(layer_handle_->color_button_11_);
  
  layer_handle_->color_button_12_ = new QToolButton(layer_handle_->colorChooseBar);
  layer_handle_->color_button_12_->setObjectName(QString::fromUtf8("color_button_12_"));
  layer_handle_->color_button_12_->setMinimumSize(QSize(16, 16));
  layer_handle_->color_button_12_->setMaximumSize(QSize(16, 16));
  layer_handle_->color_button_12_->setIconSize(QSize(25, 25));
  
  layer_handle_->horizontalLayout_14->addWidget(layer_handle_->color_button_12_);
  
  
  layer_handle_->colorHLayout->addWidget(layer_handle_->colorChooseBar);
  
  
  layer_handle_->verticalLayout_2->addLayout(layer_handle_->colorHLayout);
  layer_handle_->colorChooseBar->hide();
  layer_handle_->brightContrastBar->hide();
  
  layer_list_.append(layer_handle_);
  
  QList<GroupHandle>::iterator it = group_list_.begin();
  QList<GroupHandle>::iterator it_end = group_list_.end();
  
  bool found_it = false;
  int group_counter = 0;
  while( it != it_end )
  {
    if ((*it)->activate_button_->text() == dimensions)
    {
      (*it)->verticalLayout_2->addWidget(layer_handle_->layer_, 0, Qt::AlignTop|Qt::AlignCenter);
      layer_handle_->container_group_ = (*it);
      found_it = true;
    }
    ++it; group_counter++;
  }
  
  if (!found_it) {
    new_group(dimensions);
    it = group_list_.begin();
    it_end = group_list_.end();
    
    while( it != it_end )
    {
      if ((*it)->activate_button_->text() == dimensions)
      {
        (*it)->verticalLayout_2->addWidget(layer_handle_->layer_, 0, Qt::AlignTop|Qt::AlignCenter);
        layer_handle_->container_group_ = (*it);
      }
      ++it;    
    }
    
  }
  
}
  

void
LayerManagerWidget::new_group( const QString &dimensions )
{
  
  GroupHandle group_handle(new Group);
  
  
  //  --- Begin QT Widget Design --- //
  
  group_handle->page_ = new QWidget;
  group_handle->page_->setStyleSheet(QString::fromUtf8("QPushButton#activate_button_{\n"
                                        " \n"
                                        " margin-right: 7px;\n"
                                        " height: 18px;\n"
                                        " text-align: left;\n"
                                        " padding-left: 4px;\n"
                                        " color: white;\n"
                                        " font: 11pt; \n"
                                        " font: bold;\n"
                                        "\n"
                                        "}\n"
                                        "\n"
                                        "QPushButton#activate_button_:pressed{\n"
                                        " color: black;\n"
                                        " margin-right: 7px; \n"
                                        " padding-left: 4px; \n"      
                                        " height: 18px; border-radius: 4px; \n"
                                        "\n"
                                        "\n"
                                        "}\n"
                                        "\n"
                                        "QFrame#group_frame_{\n"
                                        " border-radius: 4px;\n"
                                        " border: 1px solid gray;\n"
                                        " \n"
                                        " background-color: gray;\n"
                                        "}\n"
                                        "\n"
                                        "\n"
                                        "QToolButton{\n"
                                        " background-color: qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0), stop:1 rgba(136, 0, 0, 0));\n"
                                        " border-radius: 4px;\n"
                                        "\n"
                                        "}\n"
                                        "QToolButton:pressed{\n"
                                        " background-color: qlineargradient(spread:pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 0), stop:1 rgba(136, 0, 0, 0));\n"
                                        " background-color: black; border-radius: 4px;\n"
                                        " border: none;\n"
                                        "\n"
                                        "}\n"
                                        "\n"
                                        "QWidget#header_{\n"
                                        " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(25, 25, 25, 0), stop:0.753769 rgba(0, 0, 0, 100), stop:1 rgba(0, 0, 0, 84));\n"
                                        " /*background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(25, 25, 25, 0), stop:0.753769 rgba(0, 0, 0, 173), stop:1 rgba(0, 0, 0, 84));*/\n"
                                        " /*background-color: qlineargradient(spread:"
                                        "pad, x1:0.5, y1:0.733364, x2:0.5, y2:0, stop:0 rgba(25, 25, 25, 190), stop:1 rgba(136, 0, 0, 0 ));*/\n"
                                        "    border-radius: 4px;\n"
                                        " border: 1px solid black;\n"
                                        "}\n"
                                        "\n"
                                        "QWidget#background_group_{\n"
                                        " background-color: rgb(255, 128, 0);\n"
                                        " border-radius: 6px;\n"
                                        "}"));
  
  group_handle->page_->setObjectName(QString::fromUtf8("group_handle->page_"));
  QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  group_handle->verticalLayout_5 = new QVBoxLayout(group_handle->page_);
  group_handle->verticalLayout_5->setSpacing(0);
  group_handle->verticalLayout_5->setContentsMargins(0, 0, 0, 0);
  group_handle->verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
  group_handle->background_group_ = new QWidget(group_handle->page_);
  group_handle->background_group_->setObjectName(QString::fromUtf8("background_group_"));
  QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy1.setHorizontalStretch(0);
  sizePolicy1.setVerticalStretch(0);
  sizePolicy1.setHeightForWidth(group_handle->background_group_->sizePolicy().hasHeightForWidth());
  group_handle->background_group_->setSizePolicy(sizePolicy1);
  group_handle->background_group_->setMinimumSize(QSize(215, 21));
  group_handle->background_group_->setMaximumSize(QSize(215, 21));
  group_handle->verticalLayout_3 = new QVBoxLayout(group_handle->background_group_);
  group_handle->verticalLayout_3->setSpacing(0);
  group_handle->verticalLayout_3->setContentsMargins(0, 0, 0, 0);
  group_handle->verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
  group_handle->header_ = new QWidget(group_handle->background_group_);
  group_handle->header_->setObjectName(QString::fromUtf8("header_"));
  sizePolicy.setHeightForWidth(group_handle->header_->sizePolicy().hasHeightForWidth());
  group_handle->header_->setSizePolicy(sizePolicy);
  group_handle->header_->setMinimumSize(QSize(215, 21));
  group_handle->header_->setMaximumSize(QSize(215, 21));
  group_handle->horizontalLayout = new QHBoxLayout(group_handle->header_);
  group_handle->horizontalLayout->setSpacing(0);
  group_handle->horizontalLayout->setContentsMargins(0, 0, 0, 0);
  group_handle->horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
  group_handle->open_button_ = new QToolButton(group_handle->header_);
  group_handle->open_button_->setObjectName(QString::fromUtf8("open_button_"));
  group_handle->open_button_->setMinimumSize(QSize(21, 21));
  group_handle->open_button_->setMaximumSize(QSize(21, 21));
  group_handle->open_button_->setCheckable(true);
  group_handle->open_button_->setChecked(true);
  
  QIcon icon;
  icon.addFile(QString::fromUtf8(":/new/images/OpenWhite.png"), QSize(), QIcon::Normal, QIcon::Off);
  group_handle->open_button_->setIcon(icon);
  group_handle->open_button_->setIconSize(QSize(18, 18));
  
  group_handle->horizontalLayout->addWidget(group_handle->open_button_);
  
  connect(group_handle->open_button_, SIGNAL(clicked(bool)), this, SLOT(hide_show_group_layers(bool)));
  
  group_handle->activate_button_ = new QPushButton(group_handle->header_);
  group_handle->activate_button_->setObjectName(QString::fromUtf8("activate_button_"));
  sizePolicy1.setHeightForWidth(group_handle->activate_button_->sizePolicy().hasHeightForWidth());
  group_handle->activate_button_->setSizePolicy(sizePolicy1);
  group_handle->activate_button_->setMinimumSize(QSize(152, 21));
  group_handle->activate_button_->setMaximumSize(QSize(152, 21));
  group_handle->activate_button_->setCheckable(false);
  group_handle->activate_button_->setFlat(true);
  group_handle->activate_button_->setText(dimensions);
  
  connect(group_handle->activate_button_, SIGNAL(clicked()), this, SLOT(activate_group_button_clicked()));
  
  group_handle->horizontalLayout->addWidget(group_handle->activate_button_);
  
  group_handle->add_new_button_ = new QToolButton(group_handle->header_);
  group_handle->add_new_button_->setObjectName(QString::fromUtf8("add_new_button_"));
  group_handle->add_new_button_->setMinimumSize(QSize(21, 21));
  group_handle->add_new_button_->setMaximumSize(QSize(21, 21));
  
  group_handle->add_new_button_->setIcon(active_new_icon_);
  group_handle->add_new_button_->setIconSize(QSize(16, 16));
  
  group_handle->horizontalLayout->addWidget(group_handle->add_new_button_);
  
  group_handle->close_button_ = new QToolButton(group_handle->header_);
  group_handle->close_button_->setObjectName(QString::fromUtf8("close_button_"));
  group_handle->close_button_->setMinimumSize(QSize(21, 21));
  group_handle->close_button_->setMaximumSize(QSize(21, 21));
  
  group_handle->close_button_->setIcon(active_close_icon_);
  group_handle->close_button_->setIconSize(QSize(18, 18));
  
  group_handle->horizontalLayout->addWidget(group_handle->close_button_);
  
  group_handle->verticalLayout_3->addWidget(group_handle->header_);
  
  group_handle->verticalLayout_5->addWidget(group_handle->background_group_);
  
  group_handle->group_frame_ = new QFrame(group_handle->page_);
  group_handle->group_frame_->setObjectName(QString::fromUtf8("group_frame_"));
  
  group_handle->group_frame_->setFrameShape(QFrame::StyledPanel);
  group_handle->group_frame_->setFrameShadow(QFrame::Raised);
  
  group_handle->verticalLayout_2 = new QVBoxLayout(group_handle->group_frame_);
  group_handle->verticalLayout_2->setSpacing(0);
  group_handle->verticalLayout_2->setContentsMargins(0, 0, 0, 0);
  group_handle->verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
  
  group_handle->verticalLayout_5->addWidget(group_handle->group_frame_);
  group_handle->verticalLayout_5->setStretch(1, 1);
   
  group_layout_->addWidget(group_handle->page_, 0, Qt::AlignTop|Qt::AlignCenter);
  group_list_.append(group_handle);

}


void
LayerManagerWidget::remove_layer(int index)
{
  
}

void
LayerManagerWidget::set_active_layer(int index)
{
  // Setup the iterators
  QList<LayerHandle>::iterator it = layer_list_.begin();
  QList<LayerHandle>::iterator it_end = layer_list_.end();
  
  
  
  if (index >= 0) 
  {
    int counter = 0;
    
    while (it != it_end) 
    { 
      if (index == counter) 
      {
        //set_active_layer( (*it)->???);
      }
      ++it; counter++;
    }
    
    active_layer_index_ = index;
    Q_EMIT active_layer_changed(active_layer_index_);
  }
  else if(index < 0)
  {
    if(it != layer_list_.end())
    {
      //set_active_layer( (*it)->???);
    }
    active_layer_index_ = 0;
    Q_EMIT active_layer_changed(active_layer_index_);
  }

}

void
LayerManagerWidget::set_active_group(int index)
{
  // Setup the iterators
  QList<GroupHandle>::iterator it = group_list_.begin();
  QList<GroupHandle>::iterator it_end = group_list_.end();
  
  
  
  if (index >= 0) 
  {
    int counter = 0;
    
    while (it != it_end) 
    { 
      if (index == counter) 
      {
        //set_active_group( (*it)->???);
      }
      ++it; counter++;
    }
    
    active_group_index_ = index;
    Q_EMIT active_group_changed(active_group_index_);
  }
  else if(index < 0)
  {
    if(it != group_list_.end())
    {
      //set_active_layer( (*it)->???);
      //SCI_LOG_MESSAGE("the index is -1 so i am setting it to 0");
    }
    active_group_index_ = 0;
    Q_EMIT active_group_changed(active_group_index_);
  }
  
  
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

