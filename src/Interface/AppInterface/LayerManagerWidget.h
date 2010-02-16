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

// Qt includes
#include <QtGui>
#include <QSharedPointer>

// boost includes
#include <boost/smart_ptr.hpp>


namespace Seg3D {

class LayerManagerWidgetPrivate;
typedef boost::shared_ptr< LayerManagerWidgetPrivate > LayerManagerWidgetPrivateHandle;

class LayerManagerWidget : public QScrollArea
{   
  // Needed to make it a Qt object
  Q_OBJECT

  //constructor - destructor
  public:
    LayerManagerWidget( QWidget *parent = 0 );
    virtual ~LayerManagerWidget();
    

  private Q_SLOTS:
    void hide_show_resample( bool );
    void hide_show_roi( bool );
    void hide_show_layers( bool );
    
    void hide_show_brightness_contrast_bar( bool );
    void hide_show_color_choose_bar( bool );
    void color_button_clicked();

  public:
    void new_group( const QString &dimensions );
    void new_layer( int type, const QString &label, const QString &dimensions );
    
    // enum for layer types
    enum 
    {
        DATA_LAYER_E,
        MASK_LAYER_E,
        LABEL_LAYER_E
    };

  private:
    // private Qt GUI Components for the LayerManagerWidget
    QWidget*     main_;
    QVBoxLayout* main_layout_;
    QVBoxLayout* group_layout_;
    int number_of_groups_;

    LayerManagerWidgetPrivateHandle private_;

    // Icons for the GUI
    QIcon active_close_icon_;    
    QIcon inactive_close_icon_;
    QIcon expand_close_group_icon_;

  private:
    bool validate_new_layer( const QString &dimensions );
     
};

}  //endnamespace Seg3d

#endif