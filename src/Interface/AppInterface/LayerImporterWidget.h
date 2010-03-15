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

#ifndef INTERFACE_QTWIDGETS_LAYERIMPORTERWIDGET_H
#define INTERFACE_QTWIDGETS_LAYERIMPORTERWIDGET_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// Qt includes
#include <QWidget>

// boost includes
#include <boost/smart_ptr.hpp>

// Application Includes
#include <Application/LayerIO/LayerImporter.h>


namespace Seg3D
{

class LayerImporterWidgetPrivate;
typedef boost::shared_ptr< LayerImporterWidgetPrivate > LayerImporterWidgetPrivateHandle;

class LayerImporterWidget : public QDialog
{
  // Needed to make it a Qt object
Q_OBJECT

//constructor - destructor
public:
  LayerImporterWidget( LayerImporterHandle importer, QWidget *parent = 0 );
  virtual ~LayerImporterWidget();


private:
  // The importer that was chosen in the filedialog
  LayerImporterHandle importer_;
  
  // The private information for rendering the widget
  LayerImporterWidgetPrivateHandle private_;

  // The current active mode
  LayerImporterMode mode_;
  
  // Update the icons to reflect active mode
  void update_icons();
  
private Q_SLOTS:

  // SLOTS: These functions connect to the buttons for importing the data as a specific
  // layer type.
  void set_data() { set_mode( LayerImporterMode::DATA_E ); }
  void set_single_mask() { set_mode( LayerImporterMode::SINGLE_MASK_E ); }
  void set_bitplane_mask() { set_mode( LayerImporterMode::BITPLANE_MASK_E ); }
  void set_label_mask() { set_mode( LayerImporterMode::LABEL_MASK_E ); }

  void import();
  
private:
  // SET_MODE:
  // Set the mode of the importer
  void set_mode( LayerImporterMode mode );
};

} //endnamespace Seg3d

#endif
