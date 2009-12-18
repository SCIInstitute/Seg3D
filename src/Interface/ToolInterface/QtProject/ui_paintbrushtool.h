/********************************************************************************
** Form generated from reading UI file 'paintbrushtool.ui'
**
** Created: Wed Dec 16 12:39:58 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAINTBRUSHTOOL_H
#define UI_PAINTBRUSHTOOL_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PaintBrushTool
{
public:
    QWidget *layoutWidget;
    QHBoxLayout *eraseHLayout;
    QSpacerItem *eraseHSpacer_left;
    QCheckBox *eraseCheckBox;
    QSpacerItem *eraseHSpacer_right;
    QWidget *layoutWidget_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QWidget *layoutWidget_4;
    QHBoxLayout *maskHLayout;
    QLabel *maskLabel;
    QComboBox *maskComboBox;
    QWidget *layoutWidget_5;
    QHBoxLayout *brushHLayout;
    QLabel *brushLabel;
    QSpacerItem *brushHSpacer;
    QGroupBox *thresholdGroupBox;
    QWidget *layoutWidget_6;
    QHBoxLayout *lowerHLayout_bottom;
    QWidget *layoutWidget_7;
    QHBoxLayout *upperHLayout_bottom;
    QWidget *layoutWidget_8;
    QHBoxLayout *upperHLayout_top;
    QLabel *upperLabel;
    QSpacerItem *upperHSpacer;
    QWidget *layoutWidget_9;
    QHBoxLayout *lowerHLayout_top;
    QLabel *lowerLabel;
    QSpacerItem *lowerHSpacer;
    QWidget *layoutWidget1;
    QVBoxLayout *verticalLayout;

    void setupUi(QWidget *PaintBrushTool)
    {
        if (PaintBrushTool->objectName().isEmpty())
            PaintBrushTool->setObjectName(QString::fromUtf8("PaintBrushTool"));
        PaintBrushTool->resize(200, 326);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(PaintBrushTool->sizePolicy().hasHeightForWidth());
        PaintBrushTool->setSizePolicy(sizePolicy);
        QFont font;
        font.setPointSize(12);
        PaintBrushTool->setFont(font);
        layoutWidget = new QWidget(PaintBrushTool);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(4, 54, 193, 27));
        eraseHLayout = new QHBoxLayout(layoutWidget);
        eraseHLayout->setObjectName(QString::fromUtf8("eraseHLayout"));
        eraseHLayout->setContentsMargins(0, 0, 0, 0);
        eraseHSpacer_left = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        eraseHLayout->addItem(eraseHSpacer_left);

        eraseCheckBox = new QCheckBox(layoutWidget);
        eraseCheckBox->setObjectName(QString::fromUtf8("eraseCheckBox"));

        eraseHLayout->addWidget(eraseCheckBox);

        eraseHSpacer_right = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        eraseHLayout->addItem(eraseHSpacer_right);

        layoutWidget_2 = new QWidget(PaintBrushTool);
        layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(4, 4, 193, 26));
        activeHLayout = new QHBoxLayout(layoutWidget_2);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeHLayout->setContentsMargins(0, 0, 0, 0);
        activeLabel = new QLabel(layoutWidget_2);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(layoutWidget_2);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));

        activeHLayout->addWidget(activeComboBox);

        layoutWidget_4 = new QWidget(PaintBrushTool);
        layoutWidget_4->setObjectName(QString::fromUtf8("layoutWidget_4"));
        layoutWidget_4->setGeometry(QRect(4, 29, 193, 26));
        maskHLayout = new QHBoxLayout(layoutWidget_4);
        maskHLayout->setObjectName(QString::fromUtf8("maskHLayout"));
        maskHLayout->setContentsMargins(0, 0, 0, 0);
        maskLabel = new QLabel(layoutWidget_4);
        maskLabel->setObjectName(QString::fromUtf8("maskLabel"));

        maskHLayout->addWidget(maskLabel);

        maskComboBox = new QComboBox(layoutWidget_4);
        maskComboBox->setObjectName(QString::fromUtf8("maskComboBox"));

        maskHLayout->addWidget(maskComboBox);

        layoutWidget_5 = new QWidget(PaintBrushTool);
        layoutWidget_5->setObjectName(QString::fromUtf8("layoutWidget_5"));
        layoutWidget_5->setGeometry(QRect(4, 80, 193, 22));
        brushHLayout = new QHBoxLayout(layoutWidget_5);
        brushHLayout->setSpacing(0);
        brushHLayout->setObjectName(QString::fromUtf8("brushHLayout"));
        brushHLayout->setContentsMargins(0, 0, 0, 0);
        brushLabel = new QLabel(layoutWidget_5);
        brushLabel->setObjectName(QString::fromUtf8("brushLabel"));

        brushHLayout->addWidget(brushLabel);

        brushHSpacer = new QSpacerItem(118, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        brushHLayout->addItem(brushHSpacer);

        thresholdGroupBox = new QGroupBox(PaintBrushTool);
        thresholdGroupBox->setObjectName(QString::fromUtf8("thresholdGroupBox"));
        thresholdGroupBox->setGeometry(QRect(4, 152, 193, 171));
        thresholdGroupBox->setFlat(false);
        layoutWidget_6 = new QWidget(thresholdGroupBox);
        layoutWidget_6->setObjectName(QString::fromUtf8("layoutWidget_6"));
        layoutWidget_6->setGeometry(QRect(5, 112, 183, 51));
        lowerHLayout_bottom = new QHBoxLayout(layoutWidget_6);
        lowerHLayout_bottom->setObjectName(QString::fromUtf8("lowerHLayout_bottom"));
        lowerHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget_7 = new QWidget(thresholdGroupBox);
        layoutWidget_7->setObjectName(QString::fromUtf8("layoutWidget_7"));
        layoutWidget_7->setGeometry(QRect(5, 41, 183, 51));
        upperHLayout_bottom = new QHBoxLayout(layoutWidget_7);
        upperHLayout_bottom->setObjectName(QString::fromUtf8("upperHLayout_bottom"));
        upperHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget_8 = new QWidget(thresholdGroupBox);
        layoutWidget_8->setObjectName(QString::fromUtf8("layoutWidget_8"));
        layoutWidget_8->setGeometry(QRect(5, 20, 183, 22));
        upperHLayout_top = new QHBoxLayout(layoutWidget_8);
        upperHLayout_top->setObjectName(QString::fromUtf8("upperHLayout_top"));
        upperHLayout_top->setContentsMargins(0, 0, 0, 0);
        upperLabel = new QLabel(layoutWidget_8);
        upperLabel->setObjectName(QString::fromUtf8("upperLabel"));

        upperHLayout_top->addWidget(upperLabel);

        upperHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        upperHLayout_top->addItem(upperHSpacer);

        layoutWidget_9 = new QWidget(thresholdGroupBox);
        layoutWidget_9->setObjectName(QString::fromUtf8("layoutWidget_9"));
        layoutWidget_9->setGeometry(QRect(5, 91, 183, 22));
        lowerHLayout_top = new QHBoxLayout(layoutWidget_9);
        lowerHLayout_top->setObjectName(QString::fromUtf8("lowerHLayout_top"));
        lowerHLayout_top->setContentsMargins(0, 0, 0, 0);
        lowerLabel = new QLabel(layoutWidget_9);
        lowerLabel->setObjectName(QString::fromUtf8("lowerLabel"));

        lowerHLayout_top->addWidget(lowerLabel);

        lowerHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        lowerHLayout_top->addItem(lowerHSpacer);

        layoutWidget1 = new QWidget(PaintBrushTool);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(4, 101, 193, 51));
        verticalLayout = new QVBoxLayout(layoutWidget1);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        verticalLayout->setContentsMargins(0, 0, 0, 0);

        retranslateUi(PaintBrushTool);

        QMetaObject::connectSlotsByName(PaintBrushTool);
    } // setupUi

    void retranslateUi(QWidget *PaintBrushTool)
    {
        PaintBrushTool->setWindowTitle(QApplication::translate("PaintBrushTool", "Form", 0, QApplication::UnicodeUTF8));
        eraseCheckBox->setText(QApplication::translate("PaintBrushTool", "Erase Mode", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("PaintBrushTool", "Active:", 0, QApplication::UnicodeUTF8));
        maskLabel->setText(QApplication::translate("PaintBrushTool", "Mask:", 0, QApplication::UnicodeUTF8));
        brushLabel->setText(QApplication::translate("PaintBrushTool", "Brush Size:", 0, QApplication::UnicodeUTF8));
        thresholdGroupBox->setTitle(QApplication::translate("PaintBrushTool", "Threshold", 0, QApplication::UnicodeUTF8));
        upperLabel->setText(QApplication::translate("PaintBrushTool", "Upper:", 0, QApplication::UnicodeUTF8));
        lowerLabel->setText(QApplication::translate("PaintBrushTool", "Lower:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class PaintBrushTool: public Ui_PaintBrushTool {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAINTBRUSHTOOL_H
