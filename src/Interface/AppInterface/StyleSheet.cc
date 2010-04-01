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


#include <Interface/AppInterface/StyleSheet.h>

namespace Seg3D
{
  // === ViewerWidget Styles === //
  
  const QString StyleSheet::VIEWERWIDGET_TOOLBAR_C = QString::fromUtf8(
    "QToolButton {"
    " border: 1px solid rgba( 0, 0, 0, 0 );"
    "}"
    "QToolButton:hover {"
    " background-color: rgba( 150, 150, 150, 70 )"
    "}"
    "QToolButton:pressed {"
    " background-color: rgba( 255, 255, 210, 140 )"
    "}"
    "QFrame {"
    " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227,"
    "   stop:0 rgba(241, 241, 241, 255),"
    "   stop:0.155779 rgba(248, 248, 248,255),"
    "   stop:1 rgba(224, 224, 224, 255));"
    "}" );

  // === StatusBar === //

  const QString StyleSheet::STATUSBAR_C = QString::fromUtf8(
    //"QStatusBar {"
    //" border: 2px solid black; }"
    "QStatusBar::item {"
    " border: none; }"
    "QStatusBar QToolButton {"
    " background-color: rgba( 0, 0, 0, 0 );"
    " border: 1px solid  rgba( 0, 0, 0, 0 );"
    "}" );


  //Volume Type Colors
  const QString StyleSheet::DATA_VOLUME_COLOR_C = QString::fromUtf8(
    "QWidget#typeBackground_{"
    " background-color: rgb(166, 12, 73);"
    "}");
    
  const QString StyleSheet::MASK_VOLUME_COLOR_C = QString::fromUtf8(
    "QWidget#typeBackground_{"
    " background-color: rgb(255, 128, 0);"
    "}");
    
  const QString StyleSheet::LABEL_VOLUME_COLOR_C = QString::fromUtf8(
    "QWidget#typeBackground_{ background-color: rgb(90, 90, 90);"
    "}");
  
  // === LayerWidget Styles === //
  // - ACTIVE - //
  const QString StyleSheet::LAYER_WIDGET_BASE_ACTIVE_C = QString::fromUtf8(
    "QWidget#base_{"
    " border-radius: 6px;"
    " border: 1px solid rgb(94, 141, 176);"
    " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1," 
    "   stop:0 rgba(216, 238, 245, 255),"
    "   stop:0.372881 rgba(226, 249, 255, 255),"
    "   stop:1 rgba(204, 224, 230, 255));"
    "}");
  
  // - INACTIVE - //
  const QString StyleSheet::LAYER_WIDGET_BASE_INACTIVE_C = QString::fromUtf8(
    "QWidget#base_{"
    " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227,"
    "   stop:0 rgba(211, 211, 211, 255),"
    "   stop:0.155779 rgba(225, 225, 225, 215),"
    "   stop:1 rgba(194, 194, 194, 255));"
    " border-radius: 6px;"
    " border: 1px solid rgb(90, 90, 90);"
    "}");
  
  const QString StyleSheet::LAYER_WIDGET_LABEL_C = QString::fromUtf8(
    "QLineEdit#label_{"
    " text-align: left;"
    " color: black;"
    " margin-right: 3px;"
    " background-color: rgba( 190, 190, 190, 1 );"
    "}");
    
  // - DROP - //
  const QString StyleSheet::LAYER_WIDGET_BASE_DROP_C = QString::fromUtf8(
    "QWidget#base_{"
    " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
    "   stop:0 rgba(241, 249, 205, 255),"
    "   stop:0.372881 rgba(248, 255, 212, 255),"
    "   stop:1 rgba(255, 253, 185, 255));"
    " border-radius: 6px;"
    " border: 1px solid rgb(90, 90, 90);"
    "}");
    
  // - LOCKED - //
  const QString StyleSheet::LAYER_WIDGET_BASE_LOCKED_C = QString::fromUtf8(
    "QWidget#base_{"
    " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227," 
    "   stop:0 rgba(201, 201, 201, 255),"
    "   stop:0.155779 rgba(208, 208, 208, 255),"
    "   stop:1 rgba(184, 184, 184, 255));"
    " border-radius: 6px;"
    " border: 1px solid rgb(120, 120, 120);"
    "}");
    
  const QString StyleSheet::LAYER_WIDGET_BACKGROUND_LOCKED_C = QString::fromUtf8(
    "QWidget#typeBackground_{"
    " background-color: gray;"
    " border: 1px solid rgb(141, 141, 141);"
    " border-radius: 4px;"
    "}");
    
  const QString StyleSheet::LAYER_WIDGET_LABEL_LOCKED_C = QString::fromUtf8(
    "QLineEdit#label_{"
    " background-color:rgba(208, 208, 208, 1);"
    " color: gray;"
    "}");
  
  // === LayerGroupWidget Styles === //
  // - ACTIVE - //
  const QString StyleSheet::GROUP_WIDGET_BASE_ACTIVE_C = "";
  const QString StyleSheet::GROUP_WIDGET_BACKGROUND_ACTIVE_C = "";
  const QString StyleSheet::GROUP_WIDGET_ACTIVATE_BUTTON_ACTIVE_C = "";
  // - INACTIVE - //
  const QString StyleSheet::GROUP_WIDGET_BASE_INACTIVE_C = "";
  const QString StyleSheet::GROUP_WIDGET_BACKGROUND_INACTIVE_C = "";
  const QString StyleSheet::GROUP_WIDGET_ACTIVATE_BUTTON_INACTIVE_C = "";

StyleSheet::StyleSheet()
{
}    
  
  


}  // end namespace Seg3D
