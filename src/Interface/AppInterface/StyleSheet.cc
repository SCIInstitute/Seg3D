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

//Volume Type Colors
const QString StyleSheet::DATA_VOLUME_COLOR_C = QString::fromUtf8(
  "QWidget#typeBackground_{ background-color: rgb(166, 12, 73); }");
  
const QString StyleSheet::MASK_VOLUME_COLOR_C = QString::fromUtf8(
  "QWidget#typeBackground_{ background-color: rgb(255, 128, 0); ]");
  
const QString StyleSheet::LABEL_VOLUME_COLOR_C = QString::fromUtf8(
  "QWidget#typeBackground_{ background-color: rgb(90, 90, 90); ]");

// === LayerWidget Styles === //
const QString StyleSheet::LAYER_WIDGET_BASE_ACTIVE_C = QString::fromUtf8(
  "QWidget#base_{"
  " border-radius: 6px;"
  " border: 1px solid rgb(94, 141, 176);"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1," 
  "   stop:0 rgba(216, 238, 245, 255),"
  "   stop:0.372881 rgba(226, 249, 255, 255),"
  "   stop:1 rgba(204, 224, 230, 255));"
  "}");

const QString StyleSheet::LAYER_WIDGET_LABEL_ACTIVE_C = QString::fromUtf8(
  "QLineEdit#label_{"
  " text-align: left;"
  " color: black;"
  " margin-right: 3px;"
  " background-color: rgba(216, 238, 245, 1);"
  "}");

const QString StyleSheet::LAYER_WIDGET_BASE_INACTIVE_C = QString::fromUtf8(
  "QWidget#base_{"
  " background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227,"
  "   stop:0 rgba(181, 181, 181, 255),"
  "   stop:0.155779 rgba(195, 195, 195, 255),"
  "   stop:1 rgba(164, 164, 164, 255));"
  " border-radius: 6px;"
  " border: 1px solid rgb(90, 90, 90);"
  "}");

const QString StyleSheet::LAYER_WIDGET_LABEL_INACTIVE_C = QString::fromUtf8(
  "QLineEdit#label_{"
  " text-align: left;"
  " color: black;"
  " margin-right: 3px;"
  " background-color: rgba( 190, 190, 190, 1 );"
  "}");


StyleSheet::StyleSheet()
{
}    
  
  


}  // end namespace Seg3D
