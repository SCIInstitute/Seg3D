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

#ifndef INTERFACE_APPINTERFACE_TOOLBOXWIDGET_H
#define INTERFACE_APPINTERFACE_TOOLBOXWIDGET_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <QtGui>


namespace Seg3D {

class ToolBoxWidget : public QWidget
{    
  Q_OBJECT
  
public:
  ToolBoxWidget(QWidget* parent);
  int insertItem ( int index, QWidget * widget, const QString & text );
  virtual ~ToolBoxWidget();
  
  struct Page {
    
    QWidget *page_;
    QVBoxLayout *vLayout_;
    QWidget *background_;
    QHBoxLayout *hLayout_2;
    QWidget *header_;
    QHBoxLayout *hLayout_;
    QPushButton *activate_button_;
    QToolButton *help_button_;
    QToolButton *close_button_;
    QFrame *tool_frame_;
    QWidget *tool_;
    
    inline void setTitle ( const QString &text ) { activate_button_->setText(text); }
    
    inline bool operator==(const Page& other) const
    {
      return tool_ == other.tool_;
    }
    
  };
  
  typedef QList<Page> PageList;
  
  Page *page( QWidget *widget );
  Page *page( int index );
  
  Page *currentPage;
  
//  int currentIndex;
  
  PageList* tool_list_;
  
  void updatePages();


};


}  //endnamespace Seg3d

#endif