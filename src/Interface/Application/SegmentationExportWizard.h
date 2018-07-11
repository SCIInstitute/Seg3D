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

#ifndef INTERFACE_APPLICATION_SEGMENTATIONEXPORTWIZARD_H
#define INTERFACE_APPLICATION_SEGMENTATIONEXPORTWIZARD_H

#ifndef Q_MOC_RUN

#include <boost/shared_ptr.hpp>

//Qt includes
#include <QWizard>

// Interface Includes
#include <Interface/Application/QtLayerListWidget.h>

#endif

namespace Seg3D
{

class SegmentationPrivate;
typedef boost::shared_ptr< SegmentationPrivate > SegmentationPrivateHandle;
  
class SegmentationExportWizard : public QWizard
{
Q_OBJECT

public:
  SegmentationExportWizard( QWidget *parent = 0 );
    virtual ~SegmentationExportWizard();
    
private:
    void accept();
  SegmentationPrivateHandle private_;
};

class SegmentationSelectionPage : public QWizardPage
{
Q_OBJECT

public:
  SegmentationSelectionPage( SegmentationPrivateHandle private_handle, QWidget *parent = 0 );
  
protected:
  /// INITIALIZEPAGE:
  /// function that is called right before the page is loaded and used to populate
  /// the page with data that we dont have when the constructor is called
    virtual void initializePage();
  
  /// VALIDATEPAGE:
  /// function that is called right after the next button is clicked and used to process
  /// the entered data so it can be passed to the next page
  virtual bool validatePage();
  
private Q_SLOTS:
  void change_type_text( int index );
  void set_export_path();
  void set_filename( const QString& name );
  void radio_button_change_path();
  
private:
  SegmentationPrivateHandle private_;

};

class SegmentationSummaryPage : public QWizardPage
{
    Q_OBJECT

public:
    SegmentationSummaryPage( SegmentationPrivateHandle private_handle, QWidget *parent = 0 );

protected:
  /// INITIALIZEPAGE:
  /// function that is called right before the page is loaded and used to populate
  /// the page with data that we dont have when the constructor is called
    virtual void initializePage();
  
  /// ISCOMPLETE:
  /// function that is connected to all of the spinboxes so that any change in them is instantly 
  /// checked for validity
  virtual bool isComplete() const;
  
  /// VALIDATEPAGE:
  /// This function is actually called after "isComplete" is called and in this case we use it to
  /// validate the information on the summary page and dispatch the actions
  virtual bool validatePage();

private:
  SegmentationPrivateHandle private_;

};

} // end namespace Seg3D

#endif
