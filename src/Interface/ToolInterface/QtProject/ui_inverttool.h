/********************************************************************************
** Form generated from reading UI file 'inverttool.ui'
**
** Created: Thu Dec 17 14:31:49 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INVERTTOOL_H
#define UI_INVERTTOOL_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InvertTool
{
public:
    QWidget *layoutWidget_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QWidget *widget;
    QHBoxLayout *replaceInvertLayout;
    QCheckBox *replaceCheckBox;
    QPushButton *invertButton;

    void setupUi(QWidget *InvertTool)
    {
        if (InvertTool->objectName().isEmpty())
            InvertTool->setObjectName(QString::fromUtf8("InvertTool"));
        InvertTool->resize(231, 81);
        layoutWidget_2 = new QWidget(InvertTool);
        layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(10, 10, 211, 26));
        activeHLayout = new QHBoxLayout(layoutWidget_2);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeHLayout->setContentsMargins(0, 0, 0, 0);
        activeLabel = new QLabel(layoutWidget_2);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(layoutWidget_2);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));

        activeHLayout->addWidget(activeComboBox);

        widget = new QWidget(InvertTool);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(10, 40, 211, 32));
        replaceInvertLayout = new QHBoxLayout(widget);
        replaceInvertLayout->setObjectName(QString::fromUtf8("replaceInvertLayout"));
        replaceInvertLayout->setContentsMargins(0, 0, 0, 0);
        replaceCheckBox = new QCheckBox(widget);
        replaceCheckBox->setObjectName(QString::fromUtf8("replaceCheckBox"));
        replaceCheckBox->setChecked(true);

        replaceInvertLayout->addWidget(replaceCheckBox);

        invertButton = new QPushButton(widget);
        invertButton->setObjectName(QString::fromUtf8("invertButton"));

        replaceInvertLayout->addWidget(invertButton);


        retranslateUi(InvertTool);

        QMetaObject::connectSlotsByName(InvertTool);
    } // setupUi

    void retranslateUi(QWidget *InvertTool)
    {
        InvertTool->setWindowTitle(QApplication::translate("InvertTool", "Form", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("InvertTool", "Active:", 0, QApplication::UnicodeUTF8));
        replaceCheckBox->setText(QApplication::translate("InvertTool", "Replace", 0, QApplication::UnicodeUTF8));
        invertButton->setText(QApplication::translate("InvertTool", "Invert", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class InvertTool: public Ui_InvertTool {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INVERTTOOL_H
