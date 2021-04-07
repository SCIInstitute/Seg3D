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

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QColorDialog>

// Interface includes
#include <Interface/Application/SeedPointsPreferencesWidget.h>

// The interface from the designer
#include "ui_SeedPointsPreferencesWidget.h"

namespace Seg3D
{

class SeedPointsPreferencesWidgetPrivate
{
public:
  Ui::SeedPointsPreferencesWidget ui_;

};

SeedPointsPreferencesWidget::SeedPointsPreferencesWidget( QWidget *parent ) :
    QWidget( parent ),
    private_( new SeedPointsPreferencesWidgetPrivate )
{
  private_->ui_.setupUi( this );

  connect(this, SIGNAL(value_changed()), this, SLOT(set_size(int)));
  connect(this, SIGNAL(value_changed()), this, SLOT(set_thickness(double)));

  connect(this->private_->ui_.size_spinbox_, SIGNAL(value_changed()),
    this, SLOT(set_size()));
  connect(this->private_->ui_.thickness_spinbox_, SIGNAL(value_changed()),
    this, SLOT(set_thickness()));
}

SeedPointsPreferencesWidget::~SeedPointsPreferencesWidget()
{
}

void SeedPointsPreferencesWidget::set_size(int size)
{
  private_->ui_.size_spinbox_->setValue(size);
}

void SeedPointsPreferencesWidget::set_thickness(double thickness)
{
  private_->ui_.thickness_spinbox_->setValue(thickness);
}

void SeedPointsPreferencesWidget::set_size()
{
  private_->ui_.size_spinbox_->setValue(this->size_);
}

void SeedPointsPreferencesWidget::set_thickness()
{
  private_->ui_.thickness_spinbox_->setValue(this->thickness_);
}

} // end namespace Seg3D
