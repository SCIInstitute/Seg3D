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
#include <QSharedPointer>

// Application includes
#include <Application/Tool/ToolInterface.h>
#include <Application/Tool/Tool.h>
#include <Application/Tool/ToolManager.h>

namespace Seg3D {

class ToolBoxWidget : public QWidget
{    
    Q_OBJECT
    
    Q_SIGNALS:
      void currentChanged( int index );
      void tool_removed_signal( int index );
    
  public:
    
    ToolBoxWidget(QWidget* parent=0);
    virtual ~ToolBoxWidget();
    
    struct Page {
      
      QWidget *page_;

      QWidget *background_;
      QWidget *header_;
      
      QHBoxLayout *hLayout_;
      QHBoxLayout *hLayout_2;
      
      QVBoxLayout *vLayout_;
      QVBoxLayout *vLayout_2;
      
      QPushButton *activate_button_;
      QToolButton *help_button_;
      QToolButton *close_button_;
      
      QFrame *tool_frame_;
      QWidget *tool_;
              
      inline bool operator==(const Page& other) const
      {
        return tool_ == other.tool_;
      }

      
    };
    
    QVBoxLayout* main_layout_;
    QGridLayout* main_layout;
    
    typedef QList <QSharedPointer<Page> > PageList;
    PageList tool_list_;
    
    void add_tool ( QWidget * tool, const QString & text );
    void remove_tool ( QWidget *tool );
    void remove_tool ( int index );
    void tool_removed( int index );
    
    int active_index_;
    QWidget *active_tool_;
    QSharedPointer <Page> active_page_;
    
    inline int get_active_index(){ return active_index_; }
    inline QWidget *get_active_tool(){ return active_tool_;}
    inline QWidget *get_tool_at(int index){ return tool_list_.at(index)->tool_; }
    inline QSharedPointer <Page> get_active_page(){ return active_page_; }
    
    void set_active_index(int index);
    void set_active_tool(QWidget *tool);
    void set_active_page(Page *page);
    
    QSharedPointer <Page> page(QWidget *tool_);
    
    int index_of(QWidget *index);
    
    private Q_SLOTS:
      void buttonClicked();
      void itemDestroyed(QObject*);
    
};


}  //endnamespace Seg3d

#endif