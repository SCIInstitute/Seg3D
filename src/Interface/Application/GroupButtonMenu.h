/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#ifndef INTERFACE_APPLICATION_GROUPBUTTONMENU_H
#define INTERFACE_APPLICATION_GROUPBUTTONMENU_H

#ifndef Q_MOC_RUN

// QT Includes
#include <QWidget>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QtCore/QPointer>

#include <Application/Layer/LayerGroup.h>

#endif

namespace Seg3D
{

class GroupButtonMenuPrivate;

class GroupButtonMenu : public QWidget
{
  Q_OBJECT
  
Q_SIGNALS:
  void delete_select_all_pressed( bool );
  void duplicate_select_all_pressed( bool );
  void duplicate_pressed();
  void delete_pressed();

  // -- constructor/destructor --
public:
  GroupButtonMenu( QWidget* parent, LayerGroupHandle group );
  virtual ~GroupButtonMenu();
  
public:
  void uncheck_delete_button();
  void uncheck_duplicate_button();
  void set_delete_enabled( bool );
  void set_duplicate_enabled( bool );
  void uncheck_delete_menu_button();
  
  LayerGroupHandle get_group() const;
  
protected:
  /// RESIZEEVENT:
  /// this is an overloaded function to keep the size of the overlay widget in sync with the 
  /// size of the LayerGroupWidget
  virtual void resizeEvent( QResizeEvent *event );
  
  /// DROPEVENT:
  /// Overloaded function that is triggered when a drop occurs on the group
  virtual void dropEvent( QDropEvent* event );

  /// DRAGENTEREVENT:
  /// Overloaded function that is triggered when a drag even enters the group
  virtual void dragEnterEvent( QDragEnterEvent* event );
  
  /// DRAGLEAVEEVENT:
  /// Overloaded function that is triggered when a drag even leaves the group
  virtual void dragLeaveEvent( QDragLeaveEvent* event );
  
public:
  void enable_drop_space( bool drop );
  
private:
  void prep_for_animation( bool move_time );

private Q_SLOTS:
  void handle_delete_menu_changed( bool visibility_status );
  void handle_duplicate_menu_changed( bool visibility_status );
  // -- widget internals --
private:
    boost::shared_ptr< GroupButtonMenuPrivate > private_;
    
};

} //end namespace Seg3D

#endif
