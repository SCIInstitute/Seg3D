/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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


// Interface configuration
#include "InterfaceConfiguration.h"

// Interface includes
#include <Interface/Application/StyleSheet.h>

// Set the accent color of the application
#define ACCENT_COLOR " rgb( " INTERFACE_ACCENT_COLOR " ) "

namespace Seg3D
{

const QString StyleSheet::GROUP_MENUBAR_C = QString::fromUtf8(
  "QPushButton{\n"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, "
  "   stop:0 rgba(221, 221, 221, 255), "
  "   stop:0.155779 rgba(238, 238, 238, 255), "
  "   stop:1 rgba(192, 194, 194, 255));"
  " border-radius: 4px;"
  " border: 1px solid rgb(90, 90, 90);"
  " margin:4px;"
  " padding: 2px;"
  "}"
  "QPushButton:hover{\n"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, "
  "   stop:0 rgba(195, 195, 195, 255), "
  "   stop:0.253769 rgba(206, 206, 206, 255), "
  "   stop:1 rgba(185, 185, 185, 255));"
  "}\n"
  "\n"
  "QPushButton:pressed{\n"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, "
  "   stop:0 rgba(150, 150, 150, 255), "
  "   stop:0.753769 rgba(206, 206, 206, 255), "
  "   stop:1 rgba(185, 185, 185, 255));"
  "}"
  "QWidget#background_{"
  " background-color: rgb( 90,90,90 );"
  "}" 
  "QToolButton#group_new_mask_button_{"
  " background-color: rgba(0, 0, 0, 0);"
  " border: 1px solid rgba(0, 0, 0, 0);"
  " margin: 0 0 0 0;"
  " padding: 0 0 0 0;"
  "}"
  "QToolButton#group_new_mask_button_:hover{"
  " background-color: rgba( 0, 0, 0, 30 ); border-radius: 3px;"
  "}"
  "QToolButton#group_new_mask_button_:pressed{"
  " background-color: " ACCENT_COLOR ";"
  " border-radius: 3px;"
  " border: 1px solid rgb(142, 67, 32);"
  " margin: 0 0 0 0;"
  " padding: 0 0 0 0;" 
  "}"
  "QToolButton#group_delete_button_{"
  " background-color: rgba(0, 0, 0, 0);"
  " border: 1px solid rgba(0, 0, 0, 0);"
  " margin: 0 0 0 0;"
  " padding: 0 0 0 0;"
  "}"
  "QToolButton#group_delete_button_:hover{"
  " background-color: rgba( 0, 0, 0, 30 ); border-radius: 3px;"
  "}"
  "QToolButton#group_delete_button_:checked{"
  " background-color: rgba(0, 0, 0, 0);"
  " border: 1px solid rgba(0, 0, 0, 0);"
  " margin: 0 0 0 0;"
  " padding: 0 0 0 0;"
  "}"
  "QToolButton#group_iso_button_{"
  " background-color: rgba(0, 0, 0, 0);"
  " border: 1px solid rgba(0, 0, 0, 0);"
  " margin: 0 0 0 0;"
  " padding: 0 0 0 0;"
  "}"
  "QToolButton#group_iso_button_:hover{"
  " background-color: rgba( 0, 0, 0, 30 ); border-radius: 3px;"
  "}"
  "QToolButton#group_iso_button_:checked{"
  " background-color: rgba(0, 0, 0, 0);"
  " border: 1px solid rgba(0, 0, 0, 0);"
  " margin: 0 0 0 0;"
  " padding: 0 0 0 0;"
  "}"
  "QToolButton#duplicate_layer_button_{"
  " background-color: rgba(0, 0, 0, 0);"
  " border: 1px solid rgba(0, 0, 0, 0);"
  " margin: 0 0 0 0;"
  " padding: 0 0 0 0;"
  "}"
  "QToolButton#duplicate_layer_button_:hover{"
  " background-color: rgba( 0, 0, 0, 30 ); border-radius: 3px;"
  "}"
  "QToolButton#duplicate_layer_button_:checked{"
  " background-color: rgba(0, 0, 0, 0);"
  " border: 1px solid rgba(0, 0, 0, 0);"
  " margin: 0 0 0 0;"
  " padding: 0 0 0 0;"
  "}"
  "QWidget#group_tools_{"
  " border-radius: 4px;"
  " border: 1px solid rgb(90, 90, 90);"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(161, 161, 161, 255), stop:0.155779 rgba(175, 175, 175, 255), stop:1 rgba(144, 144, 144, 255));"
  "}"
  "QWidget#buttons_{"
  " border-bottom: 1px solid rgb(90, 90, 90);"
  "}" );





// === ViewerWidget Styles === //

const QString StyleSheet::VIEWERWIDGET_TOOLBAR_C = QString::fromUtf8(
  "QToolButton {"
  " border: 1px solid rgba( 0, 0, 0, 0 );"
  "}"
  "QToolButton:hover {"
  " background-color: rgba( 140, 140, 140, 90 );"
  "}"
  "QToolButton:pressed {"
  " background-color: rgba( 255, 255, 210, 140 );"
  "}"
  "QFrame {"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227,"
  "   stop:0 rgba(241, 241, 241, 255),"
  "   stop:0.155779 rgba(248, 248, 248,255),"
  "   stop:1 rgba(224, 224, 224, 255));"
  "}" );

// === StatusBar === //
const QString StyleSheet::STATUSBAR_C = QString::fromUtf8(
  "QStatusBar::item {"
  " border: none; }"
  "QStatusBar QToolButton {"
  " background-color: rgba( 0, 0, 0, 0 );"
  " border: 1px solid  rgba( 0, 0, 0, 0 );"
  "}" );

const QString StyleSheet::STATUSBAR_ERROR_C = QString::fromUtf8(
  "QLabel#status_report_label_{"
  " color: rgb(204, 0, 0);"
  " font: bold;"
  "}" );

const QString StyleSheet::STATUSBAR_WARNING_C = QString::fromUtf8(
  "QLabel#status_report_label_{"
  " color: rgb(165, 161, 34);"
  " font: bold;"
  "}" );
  
const QString StyleSheet::STATUSBAR_MESSAGE_C = QString::fromUtf8(
  "QLabel#status_report_label_{"
  " color: rgb(3, 86, 2);"
  " font: bold;"
  "}" );
const QString StyleSheet::STATUSBAR_DEBUG_C = QString::fromUtf8(
  "QLabel#status_report_label_{"
  " color: purple;"
  " font: bold;"
  "}" );
  
// === LayerImporterWidget === //
const QString StyleSheet::LAYERIMPORTERWIDGET_ACTIVE_BUTTON_C = QString( 
  "QToolButton{"
  " background-color: rgb(233, 111, 53);"
  " border-radius: 3px;"
  " border: 2px solid rgb(142, 67, 32);"
  "}" );
const QString StyleSheet::LAYERIMPORTERWIDGET_INACTIVE_BUTTON_C = QString( 
  "QToolButton{"
  " background-color: rgb(150, 150, 150);"
  " border-radius: 3px;"
  " border: 2px solid rgb(30, 30, 30);"
  "}" );

// === ToolBoxPageWidget Styles === //
// - page_background_ - //
const QString StyleSheet::TOOLBOXPAGEWIDGET_PAGE_BACKGROUND_ACTIVE_C = QString( 
  "QWidget#page_background_ {"
  " background-color:" ACCENT_COLOR ";"
  " border-radius: 3px;"
  "}" );
const QString StyleSheet::TOOLBOXPAGEWIDGET_PAGE_BACKGROUND_INACTIVE_C = QString( 
  "QWidget#page_background_ {"
  " background-color: rgb( 170, 170, 170 );"
  " border-radius: 3px;"
  "}" );
  
// - activate_button_ - //
const QString StyleSheet::TOOLBOXPAGEWIDGET_ACTIVATE_BUTTON_ACTIVE_C = QString( 
  "QToolButton#activate_button_{"
  " margin-right: 7px;"
  " height: 24px;"
  " padding-left: 4px;"
  " border: none;"
  " color: white;"
  " font: bold;"
#ifdef __APPLE__
  "   font-size: 13;"
#endif  
  "}"
  "QToolButton#activate_button_:hover{"
  "   color: white;"
  "}"
  "QToolButton#activate_button_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  "   color: white;"
  "}\n"
  "QToolButton#spacer_{"
  " margin-right: 7px;"
  " height: 24px;"
  " padding-left: 4px;"
  " border: none;"
  "}\n"
  "QToolButton#spacer_:hover{"
  "   color: white;"
  "}"
  "QToolButton#spacer_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  "   color: white;"
  "}\n"
  );
  
const QString StyleSheet::TOOLBOXPAGEWIDGET_ACTIVATE_BUTTON_INACTIVE_C = QString( 
  "QToolButton#activate_button_{"
  " margin-right: 7px;"
  " height: 24px;"
  " text-align: left;"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(25, 25, 25);"
  " font: normal;"
#ifdef __APPLE__
  "   font-size: 13;"
#endif  
  "}\n"
  "QToolButton#activate_button_:hover{"
  " color: silver;"
  "}\n"
  "QToolButton#activate_button_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(68, 68, 68);"
  "}\n"
  "QToolButton#spacer_{"
  " margin-right: 7px;"
  " height: 24px;"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(25, 25, 25);"
  "}\n"
  "QToolButton#spacer_:hover{"
  " color: silver;"
  "}\n"
  "QToolButton#spacer_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(68, 68, 68);"
  "}\n"
  );

const QString StyleSheet::RENDERING_CLIPPING_ACTIVE_C = QString( 
  "QWidget#clipping_header_bkg_ {"
  " background-color:" ACCENT_COLOR ";"
  " border-radius: 3px;"
  "}"
  "ToolButton#clipping_open_button_{"
  " margin-right: 7px;"
  " height: 24px;"
  " text-align: left;"
  " border: none;"
  " color: white;"
  " font: bold;"
#ifdef __APPLE__
  "   font-size: 13;"
#endif  
  "}"
  "QToolButton#clipping_open_button_:hover{"
  "   color: white;"
  "}"
  "QToolButton#clipping_open_button_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  "   color: white;"
  "}\n"

  "QWidget#clipping_header_bkg_ {"
  " background-color:" ACCENT_COLOR ";"
  " border-radius: 3px;"
  "}"
  "ToolButton#clipping_spacer_{"
  " margin-right: 7px;"
  " height: 24px;"
  " padding-left: 4px;"
  " border: none;"
  "}"
  "QToolButton#clipping_spacer_:hover{"
  "   color: white;"
  "}"
  "QToolButton#clipping_spacer_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  "   color: white;"
  "}\n"
  );
  
const QString StyleSheet::RENDERING_CLIPPING_INACTIVE_C = QString( 
  "QWidget#clipping_header_bkg_ {"
  " background-color: rgb( 170, 170, 170 );"
  " border-radius: 3px;"
  "}"
  "QToolButton#clipping_open_button_{"
  " margin-right: 7px;"
  " height: 24px;"
  " text-align: left;"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(25, 25, 25);"
  " font: normal;"
#ifdef __APPLE__
  "   font-size: 13;"
#endif  
  "}"
  "QToolButton#clipping_open_button_:hover{"
  " color: silver;"
  "}"
  "QToolButton#clipping_open_button_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(68, 68, 68);"
  "}"
  "QToolButton#clipping_spacer_{"
  " margin-right: 7px;"
  " height: 24px;"
  " text-align: left;"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(25, 25, 25);"
  " font: normal;"
#ifdef __APPLE__
  "   font-size: 13;"
#endif  
  "}"
  "QToolButton#clipping_spacer_:hover{"
  " color: silver;"
  "}"
  "QToolButton#clipping_spacer_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(68, 68, 68);"
  "}"

  );


const QString StyleSheet::RENDERING_VR_ACTIVE_C = QString( 
  "QWidget#vr_header_bkg_ {"
  " background-color:" ACCENT_COLOR ";"
  " border-radius: 3px;"
  "}"
  "QToolButton#vr_open_button_{"
  " margin-right: 7px;"
  " height: 24px;"
  " text-align: left;"
  " padding-left: 4px;"
  " border: none;"
  " color: white;"
  " font: bold;"
#ifdef __APPLE__
  "   font-size: 13;"
#endif  
  "}"
  "QToolButton#vr_open_button_:hover{"
  "   color: white;"
  "}"
  "QToolButton#vr_open_button_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  "   color: white;"
  "}\n"

  "QToolButton#vr_spacer_{"
  " margin-right: 7px;"
  " height: 24px;"
  " padding-left: 4px;"
  " border: none;"
  "}"
  "QToolButton#vr_spacer_:hover{"
  "   color: white;"
  "}"
  "QToolButton#vr_spacer_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  "   color: white;"
  "}\n"
  );
  
const QString StyleSheet::RENDERING_VR_INACTIVE_C = QString( 
  "QWidget#vr_header_bkg_ {"
  " background-color: rgb( 170, 170, 170 );"
  " border-radius: 3px;"
  "}"
  "QToolButton#vr_open_button_{"
  " margin-right: 7px;"
  " height: 24px;"
  " text-align: left;"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(25, 25, 25);"
  " font: normal;"
#ifdef __APPLE__
  "   font-size: 13;"
#endif  
  "}"
  "QToolButton#vr_open_button_:hover{"
  " color: silver;"
  "}"
  "QToolButton#vr_open_button_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(68, 68, 68);"
  "}"
  "QToolButton#vr_spacer_{"
  " margin-right: 7px;"
  " height: 24px;"
  " text-align: left;"
  " border: none;"
  " color: rgb(25, 25, 25);"
  "}"
  "QToolButton#vr_spacer_:hover{"
  " color: silver;"
  "}"
  "QToolButton#vr_spacer_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(68, 68, 68);"
  "}"
  );

const QString StyleSheet::RENDERING_FOG_ACTIVE_C = QString( 
  "QWidget#fog_header_bkg_ {"
  " background-color:" ACCENT_COLOR ";"
  " border-radius: 3px;"
  "}"
  "QToolButton#fog_open_button_{"
  " margin-right: 7px;"
  " height: 24px;"
  " text-align: left;"
  " padding-left: 4px;"
  " border: none;"
  " color: white;"
  " font: bold;"
#ifdef __APPLE__
  "   font-size: 13;"
#endif  
  "}"
  "QToolButton#fog_open_button_:hover{"
  "   color: white;"
  "}"
  "QToolButton#fog_open_button_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  "   color: white;"
  "}\n"
  "QToolButton#fog_spacer_{"
  " margin-right: 7px;"
  " height: 24px;"
  " padding-left: 4px;"
  " border: none;"
  "}"
  "QToolButton#fog_spacer_:hover{"
  "   color: white;"
  "}"
  "QToolButton#fog_spacer_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  "   color: white;"
  "}\n"
  );
  
const QString StyleSheet::RENDERING_FOG_INACTIVE_C = QString( 
  "QWidget#fog_header_bkg_ {"
  " background-color: rgb( 170, 170, 170 );"
  " border-radius: 3px;"
  "}"
  "QToolButton#fog_open_button_{"
  " margin-right: 7px;"
  " height: 24px;"
  " text-align: left;"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(25, 25, 25);"
  " font: normal;"
#ifdef __APPLE__
  "   font-size: 13;"
#endif  
  "}"
  "QToolButton#fog_open_button_:hover{"
  " color: silver;"
  "}"
  "QToolButton#fog_open_button_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(68, 68, 68);"
  "}"
  "QToolButton#fog_spacer_{"
  " margin-right: 7px;"
  " height: 24px;"
  " padding-left: 4px;"
  " border: none;"
  "}"
  "QToolButton#fog_spacer_:hover{"
  " color: silver;"
  "}"
  "QToolButton#fog_spacer_:pressed{"
  " padding-left: 4px;"
  " border: none;"
  " color: rgb(68, 68, 68);"
  "}"
  );


// === LayerWidget Styles === //
// - LayerWidget - //
const QString StyleSheet::LAYERWIDGET_C = QString::fromUtf8(
  "QWidget#LayerWidget{"
  " background-color: rgb( 255, 0, 0 );"
  " border-radius: 3px;"
  "}");

// - Volume Type Colors - //
const QString StyleSheet::DATA_VOLUME_COLOR_C = QString::fromUtf8(
  "QWidget#type_{"
  " background-color: rgb(90, 90, 90);"
  "}");
const QString StyleSheet::MASK_VOLUME_COLOR_C = QString::fromUtf8(
  "QWidget#type_{"
  " background-color: rgb(237, 149, 31);"
  "}");
const QString StyleSheet::LABEL_VOLUME_COLOR_C = QString::fromUtf8(
  "QWidget#type_{ background-color: rgb(90, 90, 90);"
  "}");

// - ACTIVE - //
const QString StyleSheet::LAYER_WIDGET_BASE_ACTIVE_C = QString::fromUtf8(
  "QWidget#base_{"
  //" border:2px solid rgb(73, 111, 160);"
  " border: 1px solid rgb(240, 245, 255);"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
  "   stop:0 rgba(193, 215, 255, 255),"
  "   stop:0.353234 rgba(204, 222, 255, 255),"
  "   stop:1 rgba(164, 196, 255, 255));"
  "}");

const QString StyleSheet::LAYER_WIDGET_BASE_ACTIVE_IN_USE_C = QString::fromUtf8(
  "QWidget#base_{"
  //" border:2px solid rgb(73, 111, 160);"
  " border: 1px solid rgb(240, 245, 255);"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
  "   stop:0 rgba(193, 255, 255, 255),"
  "   stop:0.353234 rgba(204, 255, 255, 255),"
  "   stop:1 rgba(164, 255, 255, 255));"
  "}");

const QString StyleSheet::LAYER_WIDGET_BASE_IN_USE_C = QString::fromUtf8(
  "QWidget#base_{"
  //" border: 2px solid rgb(73, 111, 160);"
  " border: 1px solid rgb(240, 245, 255);"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
  "   stop:0 rgba(193, 255, 215, 255),"
  "   stop:0.353234 rgba(204, 255, 222, 255),"
  "   stop:1 rgba(164, 255, 196, 255));"
  "}");

// - INACTIVE - //
const QString StyleSheet::LAYER_WIDGET_BASE_INACTIVE_C = QString::fromUtf8(
  "QWidget#base_{"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227,"
  "   stop:0 rgba(211, 211, 211, 255),"
  "   stop:0.155779 rgba(225, 225, 225, 255),"
  "   stop:1 rgba(194, 194, 194, 255));"
  " border: 1px solid rgb(80, 80, 80);"
  "}");


const QString StyleSheet::LAYER_WIDGET_HEADER_ACTIVE_C = QString::fromUtf8(
  "QWidget#header_{"
  " border-bottom: 1px solid rgba(73, 111, 160, 150);"
  "}"
);

const QString StyleSheet::LAYER_WIDGET_HEADER_ACTIVE_IN_USE_C = QString::fromUtf8(
  "QWidget#header_{"
  " border-bottom: 1px solid rgba(73, 111, 160, 150);"
  "}"
);  

const QString StyleSheet::LAYER_WIDGET_HEADER_IN_USE_C = QString::fromUtf8(
  "QWidget#header_{"
  " border-bottom: 1px solid rgba(73, 111, 160, 150);"
  "}"
);

const QString StyleSheet::LAYER_WIDGET_HEADER_INACTIVE_C = QString::fromUtf8(
  "QWidget#header_{"
  " border-bottom: 1px solid rgb(a80, 80, 80, 150);"
  "}"
);

const QString StyleSheet::LAYER_WIDGET_LABEL_INACTIVE_C = QString::fromUtf8(
  "QLineEdit#label_{"
  " text-align: left;"
  " color: black;"
  "   selection-background-color: rgb( 237, 148, 31 ); "
  "   selection-color: rgb( 255, 255, 255 ); "
  " background-color: rgba( 255, 255, 255, 0 );"
  "}"
  "QLineEdit#label_:focus{"
  " background-color: rgb( 255, 255, 255 );"
  "}");

const QString StyleSheet::LAYER_WIDGET_LABEL_ACTIVE_C = QString::fromUtf8(
  "QLineEdit#label_{"
  " text-align: left;"
  " color: black;"
  "   selection-background-color: rgb( 237, 148, 31 ); "
  "   selection-color: rgb( 255, 255, 255 ); "
  " background-color: rgba( 255, 255, 255, 0 );"
  "}"
  "QLineEdit#label_:focus{"
  " background-color: rgb( 255, 255, 255 );"
  "}");

const QString StyleSheet::LAYER_WIDGET_LABEL_ACTIVE_IN_USE_C = QString::fromUtf8(
  "QLineEdit#label_{"
  " text-align: left;"
  " color: black;"
  "   selection-background-color: rgb( 237, 148, 31 ); "
  "   selection-color: rgb( 255, 255, 255 ); "
  " background-color: rgba( 255, 255, 255, 0 );"
  "}"
  "QLineEdit#label_:focus{"
  " background-color: rgb( 255, 255, 255 );"
  "}");
  
const QString StyleSheet::LAYER_WIDGET_LABEL_IN_USE_C = QString::fromUtf8(
  "QLineEdit#label_{"
  " text-align: left;"
  " color: black;"
  "   selection-background-color: rgb( 237, 148, 31 ); "
  "   selection-color: rgb( 255, 255, 255 ); "
  " background-color: rgba( 255, 255, 255, 0 );"
  "}"
  "QLineEdit#label_:focus{"
  " background-color: rgb( 255, 255, 255 );"
  "}");   
      
// - DROP - //
const QString StyleSheet::LAYER_WIDGET_BASE_DROP_C = QString::fromUtf8(
  "QWidget#base_{"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
  "   stop:0 rgba(241, 249, 205, 255),"
  "   stop:0.372881 rgba(248, 255, 212, 255),"
  "   stop:1 rgba(255, 253, 185, 255));"
  " border-radius: 3px;"
  " border: 1px solid rgb(90, 90, 90);"
  "}");
  
const QString StyleSheet::LAYER_WIDGET_BASE_PICKED_UP_C = QString::fromUtf8(
  "QWidget#base_{"
  " background-color: rgba( 0, 0, 0, 0 );"
  " border-radius: 3px;"
  " border: 1px solid rgba( 0, 0, 0, 0 );"
  "}");
  
// - VISUAL_LOCKED - //
const QString StyleSheet::LAYER_WIDGET_BASE_LOCKED_C = QString::fromUtf8(
  "QWidget#base_{"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227," 
  "   stop:0 rgba(201, 201, 201, 255),"
  "   stop:0.155779 rgba(208, 208, 208, 255),"
  "   stop:1 rgba(184, 184, 184, 255));"
  " border-radius: 3px;"
  " border: 1px solid rgb(120, 120, 120);"
  "}");   
  
const QString StyleSheet::LAYER_WIDGET_BACKGROUND_LOCKED_C = QString::fromUtf8(
  "QWidget#type_{"
  " background-color: gray;"
  " border: 1px solid rgb(141, 141, 141);"
  " border-radius: 2px;"
  "}");
const QString StyleSheet::LAYER_WIDGET_LABEL_LOCKED_C = QString::fromUtf8(
  "QLineEdit#label_{"
  " background-color:rgba(255, 255, 255, 1);"
  " color: gray;"
  "}");


// === LayerGroupWidget Styles === //
// - ACTIVE - //
const QString StyleSheet::GROUP_WIDGET_BASE_ACTIVE_C = QString::fromUtf8(
  "QWidget#base_{"
  " background-color: rgb( 90, 90, 90 );"
  " border-radius:4px;"
  " border: 1px solid rgb( 60, 60, 60 );"
  " color: rgb(90,90,90);"
  "}");
const QString StyleSheet::GROUP_WIDGET_BACKGROUND_ACTIVE_C = QString::fromUtf8(  
  "QWidget#group_background_{"
  " background-color: " ACCENT_COLOR ";"
  " border: 1px solid rgb(90, 90, 90);"
  " border-radius: 3px;"
  " color: white;"
  "}");
const QString StyleSheet::GROUP_WIDGET_ACTIVATE_BUTTON_ACTIVE_C = QString::fromUtf8(
  "QPushButton#activate_button_{"
  " background-color:rgba(0, 0, 0, 0);"
  " border-color: rgba(0, 0, 0, 0);"
  " border: none;"
  " height: 24px;"
  " text-align: left;"
  " color: white;"
  " margin: 0 0 0 0;"
  " padding: 0 0 0 0;"
  " font: bold;"
  "}");
  
// - INACTIVE - //
const QString StyleSheet::GROUP_WIDGET_BASE_INACTIVE_C = QString::fromUtf8(
  "QWidget#base_{"
  " background-color: rgb( 110, 110, 110 );"
  " border-radius:4px;"
  " border: 1px solid rgb( 60, 60,60 );"
  "}");
const QString StyleSheet::GROUP_WIDGET_BACKGROUND_INACTIVE_C = QString::fromUtf8(  
  "QWidget#group_background_{"
  " background-color: rgb( 200, 200, 200 );"
  " border: 1px solid rgb( 60, 60, 60 );"
  " border-radius: 3px;"
  " color: gray;"
  "}");
const QString StyleSheet::GROUP_WIDGET_ACTIVATE_BUTTON_INACTIVE_C = QString::fromUtf8(
  "QPushButton#activate_button_{"
  " background-color:rgba(0, 0, 0, 0);"
  " border-color: rgba(0, 0, 0, 0);"
  " border: none;"
  " height: 24px;"
  " text-align: left;"
  " color: rgb( 90, 90, 90 );"
  " margin: 0 0 0 0;"
  " padding: 0 0 0 0;"
  " font: normal;"
  "}");
  
// - PushDragButton - //  
const QString StyleSheet::LAYER_PUSHDRAGBUTTON_C = QString::fromUtf8(
  "QPushButton#activate_button_{"
  " background-color: rgba(0, 0, 0, 0);"
  " border: 1px solid rgba(0, 0, 0, 0);"
  " margin-left:1px;"
  " border-radius: 3px;"
  " color: white;"
  "}");

const QString StyleSheet::GROUP_PUSHDRAGBUTTON_C = QString::fromUtf8(
  "QPushButton#activate_button_{"
  " background-color:rgba(0, 0, 0, 0);"
  " border-color: rgba(0, 0, 0, 0);"
  " border: none;"
  " height: 24px;"
  " text-align: left;"
  " color: white;"
  " margin: 0 0 0 0;"
  " padding: 0 0 0 0;"
  " font: bold;"
  "}"
  "QPushButton#activate_button_:pressed{"
  " background-color:rgba(0, 0, 0, 0);"
  " border: none;"
  " border-color: rgba(0, 0, 0, 0);"
  " color: gray;"
  " margin: 0 0 0 0;"
  " padding: 0 0 0 0;"
  "}");

const QString StyleSheet::DROPSPACEWIDGET_C = QString::fromUtf8(
  " border-radius: 3px;"
  " margin-top: 0;"
  " margin-bottom: 0;"
  " padding-top: 0;"
  " padding-bottom: 0;"
  " border: 1px solid rgba(50, 50, 50, 40);"
  " background-color: qlineargradient( spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227,"
  "   stop:0 rgba(0, 0, 0, 50)," 
  "   stop:0.3 rgba(0, 0, 0, 100),"
  "   stop:0.7 rgba(0, 0, 0, 100),"
  "   stop:1 rgba(0, 0, 0, 50) );");

// - ViewerWidget - //
const QString StyleSheet::VIEWERSELECTED_C = QString::fromUtf8(
  "QWidget#border_{"
  " border-top: 3px solid " ACCENT_COLOR ";"
  " border-bottom: 3px solid " ACCENT_COLOR ";"
  " border-left: 3px solid " ACCENT_COLOR ";"
  " border-right: 3px solid " ACCENT_COLOR ";"
  "}");

const QString StyleSheet::VIEWERNOTSELECTED_C = QString::fromUtf8(
   "QWidget#border_{"
   "  border-top: 3px solid rgb( 100, 100, 100 );"
   "  border-bottom: 3px solid rgb( 100, 100, 100 );"
   "  border-left: 3px solid rgb( 100, 100, 100 );"
   "  border-right: 3px solid rgb(100, 100, 100 );"
   "}");


const QString StyleSheet::LAYERIO_C = QString::fromUtf8(
  "QPushButton#mask_single_button:checked{"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(80, 80, 80, 50), stop:0.155779 rgba(90, 90, 90, 50), stop:1 rgba(78, 78, 78, 50));"
  " border-radius: 4px;"
  " border: 2px solid rgb( 90,90,90 );"
  "}"
  "QPushButton#mask_single_button:pressed{"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(80, 80, 80, 150), stop:0.155779 rgba(90, 90, 90, 150), stop:1 rgba(78, 78, 78, 150));"
  " border-radius: 4px;"
  " border: 2px solid rgb( 90,90,90 );"
  "}"
  "QPushButton#mask_single_button{}"
  "QPushButton#data_volume_button:checked{"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(80, 80, 80, 50), stop:0.155779 rgba(90, 90, 90, 50), stop:1 rgba(78, 78, 78, 50));"
  " border-radius: 4px;"
  " border: 2px solid rgb( 90,90,90 );"
  "}"
  "QPushButton#data_volume_button:pressed{"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(80, 80, 80, 150), stop:0.155779 rgba(90, 90, 90, 150), stop:1 rgba(78, 78, 78, 150));;"
  " border-radius: 4px;"
  " border: 2px solid rgb( 90,90,90 );"
  "}"
  "QPushButton#data_volume_button{}"
  "QPushButton#mask_1234_button:checked{"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(80, 80, 80, 50), stop:0.155779 rgba(90, 90, 90, 50), stop:1 rgba(78, 78, 78, 50));"
  " border-radius: 4px;"
  " border: 2px solid rgb( 90,90,90 );"
  "}"
  "QPushButton#mask_1234_button:pressed{"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(80, 80, 80, 150), stop:0.155779 rgba(90, 90, 90, 150), stop:1 rgba(78, 78, 78, 150));"
  " border-radius: 4px;"
  " border: 2px solid rgb( 90,90,90 );"
  "}"
  "QPushButton#mask_1234_button{}"
  "QPushButton#mask_1248_button:checked{"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(80, 80, 80, 50), stop:0.155779 rgba(90, 90, 90, 50), stop:1 rgba(78, 78, 78, 50));"
  " border-radius: 4px;"
  " border: 2px solid rgb( 90,90,90 );"
  "}"
  "QPushButton#mask_1248_button:pressed{"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(80, 80, 80, 150), stop:0.155779 rgba(90, 90, 90, 150), stop:1 rgba(78, 78, 78, 150));"
  " border-radius: 4px;"
  " border: 2px solid rgb( 90,90,90 );"
  "}"
  "QPushButton#mask_1248_button{}"
  "QWidget#data_{"
" background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(241, 241, 241, 255), stop:0.155779 rgba(248, 248, 248, 255), stop:1 rgba(224, 224, 224, 255));"
" border-radius: 4px;"
" border: 1px solid  rgb( 224, 224, 224 );"
"}"
"QWidget#single_mask_{"
" background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(241, 241, 241, 255), stop:0.155779 rgba(248, 248, 248, 255), stop:1 rgba(224, 224, 224, 255));"
" border-radius: 4px;"
" border: 1px solid  rgb( 224, 224, 224 );"
"}"
"QWidget#bitplane_mask_{"
" background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(241, 241, 241, 255), stop:0.155779 rgba(248, 248, 248, 255), stop:1 rgba(224, 224, 224, 255));"
" border-radius: 4px;"
" border: 1px solid  rgb( 224, 224, 224 );"
"}"
"QWidget#label_mask_{"
" background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(241, 241, 241, 255), stop:0.155779 rgba(248, 248, 248, 255), stop:1 rgba(224, 224, 224, 255));"
" border-radius: 4px;"
" border: 1px solid  rgb( 224, 224, 224 );"
"}"
"QWidget#scanning_file_{"
" background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(241, 241, 241, 255), stop:0.155779 rgba(248, 248, 248, 255), stop:1 rgba(224, 224, 224, 255));"
" border-radius: 3px;"
" border: 1px solid  rgb( 215, 215, 215 );"
"}"
"QWidget#data_volume_icon_widget_{"
" background-color: " ACCENT_COLOR ";"
" border-radius: 3px;"
" border: 1px solid rgb(130, 130, 130);"
"}"
"QWidget#single_mask_icon_widget_{"
" background-color: " ACCENT_COLOR ";"
" border-radius: 3px;"
" border: 1px solid rgb(130, 130, 130);"
"}"
"QWidget#series_of_masks_1234_icon_widget_{"
" background-color: " ACCENT_COLOR ";"
" border-radius: 3px;"
" border: 1px solid rgb(130, 130, 130);"
"}"
"QWidget#series_of_masks_1248_icon_widget_{"
" background-color: " ACCENT_COLOR ";"
" border-radius: 3px;"
" border: 1px solid rgb(130, 130, 130);"
"}"
"QToolButton#data_button_{"
" border: 1px solid rgba( 0, 0, 0, 0 );"
" background-color: rgba( 0, 0, 0, 0 );"
"}"
"QToolButton#data_button_:pressed{"
" border: 1px solid rgba( 0, 0, 0, 0 );"
" background-color: rgba( 0, 0, 0, 0 );"
"}"
"QToolButton#single_mask_button_{"
" border: 1px solid rgba( 0, 0, 0, 0 );"
" background-color: rgba( 0, 0, 0, 0 );"
"}"
"QToolButton#single_mask_button_:pressed{"
" border: 1px solid rgba( 0, 0, 0, 0 );"
" background-color: rgba( 0, 0, 0, 0 );"
"}"
"QToolButton#series_1248_button_{"
" border: 1px solid rgba( 0, 0, 0, 0 );"
" background-color: rgba( 0, 0, 0, 0 );"
"}"
"QToolButton#series_1248_button_:pressed{"
" border: 1px solid rgba( 0, 0, 0, 0 );"
" background-color: rgba( 0, 0, 0, 0 );"
"}"
"QToolButton#series_1234_button_{"
" border: 1px solid rgba( 0, 0, 0, 0 );"
" background-color: rgba( 0, 0, 0, 0 );"
"}"
"QToolButton#series_1234_button_:pressed{"
" border: 1px solid rgba( 0, 0, 0, 0 );"
" background-color: rgba( 0, 0, 0, 0 );"
"}"
);

const QString StyleSheet::PROVENANCE_LIST_HEADER_C = QString::fromUtf8(
  "QWidget#header_bkg_{"
  " background-color: " ACCENT_COLOR ";"
  " border-radius: 3px;"
  " border: 1px solid rgb(100, 100, 100);"
  "}"
);

}  // end namespace Seg3D
