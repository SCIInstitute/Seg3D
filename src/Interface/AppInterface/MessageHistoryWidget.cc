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
#include <sstream>
#include <iostream>

#include <Core/Utils/Log.h>

#include <Interface/AppInterface/MessageHistoryWidget.h>
#include "ui_MessageHistoryWidget.h"

namespace Seg3D
{

class MessageHistoryWidgetPrivate
{
public:

  Ui::MessageHistoryWidget ui_;

};

MessageHistoryWidget::MessageHistoryWidget( QWidget *parent ) :
  private_( new MessageHistoryWidgetPrivate )
{
  setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  // Set up the private internals of the LayerManagerInterface class
  this->private_->ui_.setupUi( this );
  
  
}

MessageHistoryWidget::~MessageHistoryWidget()
{

}
  
void MessageHistoryWidget::add_history_item(const QString &message, const QColor &color)
{
  QListWidgetItem* item = new QListWidgetItem(message, this->private_->ui_.history_list_);
  item->setTextColor(color);
  item->setFont(QFont("Lucida Grande", 13, QFont::Bold));
  this->private_->ui_.history_list_->addItem(item);
  this->private_->ui_.history_list_->setCurrentItem(item);
}

} // end namespace Seg3D