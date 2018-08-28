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

#ifndef INTERFACE_APPLICATION_PROJECTWIZARD_H
#define INTERFACE_APPLICATION_PROJECTWIZARD_H

#ifndef Q_MOC_RUN

//Qt includes
#include <QWizard>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QFileDialog>
#include <QDebug>

#endif

namespace Seg3D
{

class ProjectWizard : public QWizard
{
Q_OBJECT

Q_SIGNALS:
  void finished();
  void canceled();

public:
    ProjectWizard( QWidget *parent = 0 );
    virtual ~ProjectWizard();

private Q_SLOTS:
  void set_delete_path( QString );

    void accept();
    void reject();
    
private:
  std::string path_to_delete_;
};

class ProjectInfoPage : public QWizardPage
{
Q_OBJECT

Q_SIGNALS:
  void need_to_set_delete_path( QString );;

public:
    ProjectInfoPage( QWidget *parent = 0 );
  
protected:
    void initializePage();
    
  virtual bool validatePage();

private:
    QLabel *project_name_label_;
    QLabel *project_path_label_;
    QLineEdit *project_name_lineedit_;
    QLineEdit *project_path_lineedit_;
    QPushButton *project_path_change_button_;
    QLabel *warning_message_;
    QDir *most_recent_path_;

private Q_SLOTS:
    void set_path();
    void set_name(const QString& name);
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
    QLabel *project_location_;

};

} // end namespace Seg3D

#endif
