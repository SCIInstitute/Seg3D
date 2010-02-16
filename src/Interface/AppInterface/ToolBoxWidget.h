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

#ifndef INTERFACE_QTWIDGETS_TOOLBOXWIDGET_H
#define INTERFACE_QTWIDGETS_TOOLBOXWIDGET_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <QtGui>
#include <QSharedPointer>

// Application includes
#include <Application/Tool/ToolInterface.h>
#include <Application/Tool/Tool.h>
#include <Application/ToolManager/ToolManager.h>

namespace Seg3D {

class ToolBoxWidgetPrivate;

class ToolBoxWidget : public QScrollArea
{   
    // Needed to make it a Qt object
    Q_OBJECT

  public:
    
    ToolBoxWidget(QWidget* parent=0);
    virtual ~ToolBoxWidget();
    
    void add_tool ( QWidget * tool, const QString & text, 
                    boost::function<void ()> close_function, 
                    boost::function<void ()> activate_function, 
                    const std::string& help_url );
    
    void remove_tool ( int index );
    
    inline int      get_active_index(){ return active_index_; }
    inline QWidget* get_active_tool(){ return active_tool_;}

    QWidget* get_tool_at(int index);

    void set_active_index(int index);
    void set_active_tool(QWidget *tool);

    int index_of(QWidget *index);
    
  Q_SIGNALS:
      void tool_removed_signal( int index );

private:
    
    QWidget*     main_;
    QVBoxLayout* main_layout_;
    QVBoxLayout* tool_layout_;

    typedef boost::shared_ptr<ToolBoxWidgetPrivate> ToolBoxWidgetPrivateHandle;
    ToolBoxWidgetPrivateHandle private_;
    
    int active_index_ ;
    QWidget *active_tool_;
        
    QIcon active_close_icon_;    
    QIcon inactive_close_icon_;    

    QIcon active_help_icon_;    
    QIcon inactive_help_icon_;    
        
    //void tool_removed( int index );

    
  private Q_SLOTS:
    void help_button_clicked();
  
};

}  //endnamespace Seg3d

#endif
