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

#ifndef INTERFACE_APPLICATION_APPPROJECTEXPORTWIZARD_H
#define INTERFACE_APPLICATION_APPPROJECTEXPORTWIZARD_H

#ifndef Q_MOC_RUN

//Qt includes
#include <QWizard>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

#endif

namespace Seg3D
{

class ProjectExportWizard : public QWizard
{
Q_OBJECT

public:
  ProjectExportWizard( long long session_id, QWidget *parent = 0 );
    virtual ~ProjectExportWizard();

private:
    void accept();
  long long session_id_;
};

class ExportInfoPage : public QWizardPage
{
Q_OBJECT

public:
    ExportInfoPage( QWidget *parent = 0 );
  
protected:
  /// INITIALIZEPAGE:
  /// function for preloading the page information for the info page
    void initializePage();
    
  /// VALIDATEPAGE:
  /// function that is called right after the next button is clicked and used to process
  /// the entered data so it can be passed to the next page
  virtual bool validatePage();

private:
  QLabel *project_name_label_;
    QLabel *project_path_label_;
    QLineEdit *project_name_lineedit_;
    QLineEdit *project_path_lineedit_;
    QPushButton *project_path_change_button_;
    QCheckBox *automatically_consolidate_checkbox_;
    QLabel *warning_message_;

private Q_SLOTS:
  /// SETPATH:
  /// helper function for setting the path for project export
    void set_path();
};

class ExportSummaryPage : public QWizardPage
{
    Q_OBJECT

public:
    ExportSummaryPage( QWidget *parent = 0 );

protected:
  /// INITIALIZEPAGE:
  /// function for preloading the page information for the summary page
    void initializePage();

private:
  QLabel *description_;
    QLabel *project_name_;
    QLabel *project_path_;
  QLabel *consolidate_;
};

} // end namespace Seg3D

#endif
