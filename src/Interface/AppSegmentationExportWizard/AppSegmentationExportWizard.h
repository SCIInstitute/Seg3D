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

#ifndef INTERFACE_APPPROJECTWIZARD_APPSEGMENTATIONEXPORTWIZARD_H
#define INTERFACE_APPPROJECTWIZARD_APPSEGMENTATIONEXPORTWIZARD_H

//Qt includes
#include <QtGui>

namespace Seg3D
{

class AppSegmentationExportWizard : public QWizard
{
Q_OBJECT

public:
    AppSegmentationExportWizard( QWidget *parent = 0 );
    virtual ~AppSegmentationExportWizard();

private:
    void accept();


};

class SegmentationSelectionPage : public QWizardPage
{
Q_OBJECT

public:
    SegmentationSelectionPage( QWidget *parent = 0 );
  
protected:
  // INITIALIZEPAGE:
  // function that is called right before the page is loaded and used to populate
  // the page with data that we dont have when the constructor is called
    virtual void initializePage();
  
  // VALIDATEPAGE:
  // function that is called right after the next button is clicked and used to process
  // the entered data so it can be passed to the next page
  virtual bool validatePage();

private:
  QVBoxLayout *main_layout_;
  QWidget *segmentation_title_widget_;
  QHBoxLayout *segmentation_name_layout_;
  QLabel *segmentation_name_label_;
  QLineEdit *segmentation_name_lineedit_;
  QTreeWidget *group_with_masks_tree_;
  QLineEdit *mask_list_;

};

class SegmentationSummaryPage : public QWizardPage
{
    Q_OBJECT

public:
    SegmentationSummaryPage(QWidget *parent = 0);

protected:
    void initializePage();

private:
  QLabel *description_;
  QVBoxLayout *main_layout_;
  QScrollArea *mask_scroll_area_;
  QWidget *scroll_contents_;
  QVBoxLayout *masks_layout_;
  QTreeWidget *group_with_masks_tree_;


};


} // end namespace Seg3D
#endif // APPSEGMENTATIONEXPORTWIZARD_H

