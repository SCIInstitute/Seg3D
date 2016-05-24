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

#ifndef INTERFACE_APPLICATION_SAVEPROJECTASWIZARD_H
#define INTERFACE_APPLICATION_SAVEPROJECTASWIZARD_H

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

class SaveProjectAsWizard : public QWizard
{
Q_OBJECT

public:
  SaveProjectAsWizard( QWidget *parent = 0 );
    virtual ~SaveProjectAsWizard();

private:
    void accept();
    
private Q_SLOTS:
  void finish_early();
  void set_delete_path( QString );
  
private:
  std::string path_to_delete_;
  
};

class SaveAsInfoPage : public QWizardPage
{
Q_OBJECT

Q_SIGNALS:
  // This signal is triggered when the user trys to do a save as without changing anything
  void just_a_save();
  
  void need_to_set_delete_path( QString );
  
public:
    SaveAsInfoPage( QWidget *parent = 0 );

public:
  
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
    QLabel *warning_message_;

private Q_SLOTS:
  /// SETPATH:
  /// helper function for setting the path for project export
    void set_path();
};

class SaveAsSummaryPage : public QWizardPage
{
    Q_OBJECT

public:
    SaveAsSummaryPage(QWidget *parent = 0);

protected:
  /// INITIALIZEPAGE:
  /// function for preloading the page information for the summary page
    void initializePage();

private:
  QLabel *description_;
    QLabel *project_name_;
    QLabel *project_path_;
  QCheckBox *anonymize_checkbox_;
};

} // end namespace Seg3D

#endif
