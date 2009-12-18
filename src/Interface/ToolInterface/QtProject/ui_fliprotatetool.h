/********************************************************************************
** Form generated from reading UI file 'fliprotatetool.ui'
**
** Created: Thu Dec 17 14:31:49 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FLIPROTATETOOL_H
#define UI_FLIPROTATETOOL_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FlipRotateTool
{
public:
    QWidget *layoutWidget_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QGroupBox *flipGroupBox;
    QWidget *widget;
    QVBoxLayout *flipVLayout;
    QPushButton *flipAxialButton;
    QPushButton *flipCoronalButton;
    QPushButton *flipSagittalButton;
    QGroupBox *rotateGroupBox;
    QWidget *layoutWidget;
    QVBoxLayout *rotateVLayout;
    QPushButton *rotateACButton;
    QPushButton *rotateASButton;
    QPushButton *rotateSAButton;

    void setupUi(QWidget *FlipRotateTool)
    {
        if (FlipRotateTool->objectName().isEmpty())
            FlipRotateTool->setObjectName(QString::fromUtf8("FlipRotateTool"));
        FlipRotateTool->resize(231, 301);
        layoutWidget_2 = new QWidget(FlipRotateTool);
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

        flipGroupBox = new QGroupBox(FlipRotateTool);
        flipGroupBox->setObjectName(QString::fromUtf8("flipGroupBox"));
        flipGroupBox->setGeometry(QRect(10, 31, 211, 131));
        widget = new QWidget(flipGroupBox);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(0, 22, 211, 111));
        flipVLayout = new QVBoxLayout(widget);
        flipVLayout->setObjectName(QString::fromUtf8("flipVLayout"));
        flipVLayout->setContentsMargins(0, 0, 0, 0);
        flipAxialButton = new QPushButton(widget);
        flipAxialButton->setObjectName(QString::fromUtf8("flipAxialButton"));

        flipVLayout->addWidget(flipAxialButton);

        flipCoronalButton = new QPushButton(widget);
        flipCoronalButton->setObjectName(QString::fromUtf8("flipCoronalButton"));

        flipVLayout->addWidget(flipCoronalButton);

        flipSagittalButton = new QPushButton(widget);
        flipSagittalButton->setObjectName(QString::fromUtf8("flipSagittalButton"));

        flipVLayout->addWidget(flipSagittalButton);

        rotateGroupBox = new QGroupBox(FlipRotateTool);
        rotateGroupBox->setObjectName(QString::fromUtf8("rotateGroupBox"));
        rotateGroupBox->setGeometry(QRect(10, 160, 211, 131));
        layoutWidget = new QWidget(rotateGroupBox);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(0, 22, 211, 111));
        rotateVLayout = new QVBoxLayout(layoutWidget);
        rotateVLayout->setObjectName(QString::fromUtf8("rotateVLayout"));
        rotateVLayout->setContentsMargins(0, 0, 0, 0);
        rotateACButton = new QPushButton(layoutWidget);
        rotateACButton->setObjectName(QString::fromUtf8("rotateACButton"));

        rotateVLayout->addWidget(rotateACButton);

        rotateASButton = new QPushButton(layoutWidget);
        rotateASButton->setObjectName(QString::fromUtf8("rotateASButton"));

        rotateVLayout->addWidget(rotateASButton);

        rotateSAButton = new QPushButton(layoutWidget);
        rotateSAButton->setObjectName(QString::fromUtf8("rotateSAButton"));

        rotateVLayout->addWidget(rotateSAButton);


        retranslateUi(FlipRotateTool);

        QMetaObject::connectSlotsByName(FlipRotateTool);
    } // setupUi

    void retranslateUi(QWidget *FlipRotateTool)
    {
        FlipRotateTool->setWindowTitle(QApplication::translate("FlipRotateTool", "Form", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("FlipRotateTool", "Active:", 0, QApplication::UnicodeUTF8));
        flipGroupBox->setTitle(QApplication::translate("FlipRotateTool", "Flip", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        flipAxialButton->setToolTip(QApplication::translate("FlipRotateTool", "Flip across the axial plain", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        flipAxialButton->setText(QApplication::translate("FlipRotateTool", "Axial", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        flipCoronalButton->setToolTip(QApplication::translate("FlipRotateTool", "Flip across the coronal plain", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        flipCoronalButton->setText(QApplication::translate("FlipRotateTool", "Coronal", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        flipSagittalButton->setToolTip(QApplication::translate("FlipRotateTool", "Flip across the sagittal plain", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        flipSagittalButton->setText(QApplication::translate("FlipRotateTool", "Sagittal", 0, QApplication::UnicodeUTF8));
        rotateGroupBox->setTitle(QApplication::translate("FlipRotateTool", "Rotate", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        rotateACButton->setToolTip(QApplication::translate("FlipRotateTool", "Rotate axial to coronal", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        rotateACButton->setText(QApplication::translate("FlipRotateTool", "Axial - Coronal", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        rotateASButton->setToolTip(QApplication::translate("FlipRotateTool", "Rotate axial to sagittal", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        rotateASButton->setText(QApplication::translate("FlipRotateTool", "Axial - Sagittal", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        rotateSAButton->setToolTip(QApplication::translate("FlipRotateTool", "Rotate sagittal to axial", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        rotateSAButton->setText(QApplication::translate("FlipRotateTool", "Sagittal - Axial", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class FlipRotateTool: public Ui_FlipRotateTool {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FLIPROTATETOOL_H
