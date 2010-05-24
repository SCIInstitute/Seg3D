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

#ifndef INTERFACE_APPPROJECTWIZARD_APPPROJECTWIZARD_H
#define INTERFACE_APPPROJECTWIZARD_APPPROJECTWIZARD_H

//Qt includes
#include <QtGui>

namespace Seg3D
{

class AppProjectWizard : public QWizard
{
Q_OBJECT

public:
    AppProjectWizard( QWidget *parent = 0 );
    virtual ~AppProjectWizard();

private:
    void accept();


};

class IntroPage : public QWizardPage
{
Q_OBJECT

public:
    IntroPage( QWidget *parent = 0 );

private:
    QLabel *description_;
};

class ProjectInfoPage : public QWizardPage
{
Q_OBJECT

public:
    ProjectInfoPage( QWidget *parent = 0 );
  
protected:
    void initializePage();

private:
    QLabel *project_name_label_;
    QLabel *project_path_label_;
    QLineEdit *project_name_lineedit_;
    QLineEdit *project_path_lineedit_;
    QPushButton *project_path_change_button_;
    QCheckBox *automatically_consolidate_checkbox_;

private Q_SLOTS:
    void set_path();
};

class SummaryPage : public QWizardPage
{
    Q_OBJECT

public:
    SummaryPage(QWidget *parent = 0);

protected:
    void initializePage();

private:
  QLabel *description_;
    QLabel *project_name_;
    QLabel *project_path_;
  QLabel *consolidate_;

};


} // end namespace Seg3D
#endif // APPPROJECTWIZARD_H
