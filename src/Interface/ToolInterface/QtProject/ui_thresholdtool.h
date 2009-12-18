/********************************************************************************
** Form generated from reading UI file 'thresholdtool.ui'
**
** Created: Thu Dec 17 14:31:49 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_THRESHOLDTOOL_H
#define UI_THRESHOLDTOOL_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGraphicsView>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ThresholdTool
{
public:
    QWidget *layoutWidget_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QWidget *layoutWidget_6;
    QHBoxLayout *lowerHLayout_bottom;
    QWidget *layoutWidget_9;
    QHBoxLayout *lowerHLayout_top;
    QLabel *lowerLabel;
    QSpacerItem *lowerHSpacer;
    QWidget *layoutWidget_8;
    QHBoxLayout *upperHLayout_top;
    QLabel *upperLabel;
    QSpacerItem *upperHSpacer;
    QWidget *layoutWidget_7;
    QHBoxLayout *upperHLayout_bottom;
    QPushButton *createThresholdLayerButton;
    QPushButton *clearSeedsButton;
    QGraphicsView *histogramView;
    QLabel *tempHistogramLabel;
    QWidget *widget;
    QHBoxLayout *minMaxHLayout;
    QLabel *minLabel;
    QLabel *maxLabel;

    void setupUi(QWidget *ThresholdTool)
    {
        if (ThresholdTool->objectName().isEmpty())
            ThresholdTool->setObjectName(QString::fromUtf8("ThresholdTool"));
        ThresholdTool->resize(231, 411);
        layoutWidget_2 = new QWidget(ThresholdTool);
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

        layoutWidget_6 = new QWidget(ThresholdTool);
        layoutWidget_6->setObjectName(QString::fromUtf8("layoutWidget_6"));
        layoutWidget_6->setGeometry(QRect(10, 127, 211, 51));
        lowerHLayout_bottom = new QHBoxLayout(layoutWidget_6);
        lowerHLayout_bottom->setObjectName(QString::fromUtf8("lowerHLayout_bottom"));
        lowerHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget_9 = new QWidget(ThresholdTool);
        layoutWidget_9->setObjectName(QString::fromUtf8("layoutWidget_9"));
        layoutWidget_9->setGeometry(QRect(10, 106, 211, 22));
        lowerHLayout_top = new QHBoxLayout(layoutWidget_9);
        lowerHLayout_top->setObjectName(QString::fromUtf8("lowerHLayout_top"));
        lowerHLayout_top->setContentsMargins(0, 0, 0, 0);
        lowerLabel = new QLabel(layoutWidget_9);
        lowerLabel->setObjectName(QString::fromUtf8("lowerLabel"));

        lowerHLayout_top->addWidget(lowerLabel);

        lowerHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        lowerHLayout_top->addItem(lowerHSpacer);

        layoutWidget_8 = new QWidget(ThresholdTool);
        layoutWidget_8->setObjectName(QString::fromUtf8("layoutWidget_8"));
        layoutWidget_8->setGeometry(QRect(10, 35, 211, 22));
        upperHLayout_top = new QHBoxLayout(layoutWidget_8);
        upperHLayout_top->setObjectName(QString::fromUtf8("upperHLayout_top"));
        upperHLayout_top->setContentsMargins(0, 0, 0, 0);
        upperLabel = new QLabel(layoutWidget_8);
        upperLabel->setObjectName(QString::fromUtf8("upperLabel"));

        upperHLayout_top->addWidget(upperLabel);

        upperHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        upperHLayout_top->addItem(upperHSpacer);

        layoutWidget_7 = new QWidget(ThresholdTool);
        layoutWidget_7->setObjectName(QString::fromUtf8("layoutWidget_7"));
        layoutWidget_7->setGeometry(QRect(10, 56, 211, 51));
        upperHLayout_bottom = new QHBoxLayout(layoutWidget_7);
        upperHLayout_bottom->setObjectName(QString::fromUtf8("upperHLayout_bottom"));
        upperHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        createThresholdLayerButton = new QPushButton(ThresholdTool);
        createThresholdLayerButton->setObjectName(QString::fromUtf8("createThresholdLayerButton"));
        createThresholdLayerButton->setGeometry(QRect(12, 180, 211, 32));
        clearSeedsButton = new QPushButton(ThresholdTool);
        clearSeedsButton->setObjectName(QString::fromUtf8("clearSeedsButton"));
        clearSeedsButton->setGeometry(QRect(60, 210, 113, 32));
        histogramView = new QGraphicsView(ThresholdTool);
        histogramView->setObjectName(QString::fromUtf8("histogramView"));
        histogramView->setGeometry(QRect(10, 250, 211, 131));
        tempHistogramLabel = new QLabel(ThresholdTool);
        tempHistogramLabel->setObjectName(QString::fromUtf8("tempHistogramLabel"));
        tempHistogramLabel->setGeometry(QRect(60, 290, 111, 51));
        tempHistogramLabel->setAlignment(Qt::AlignCenter);
        tempHistogramLabel->setWordWrap(true);
        widget = new QWidget(ThresholdTool);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(10, 380, 211, 20));
        minMaxHLayout = new QHBoxLayout(widget);
        minMaxHLayout->setObjectName(QString::fromUtf8("minMaxHLayout"));
        minMaxHLayout->setContentsMargins(0, 0, 0, 0);
        minLabel = new QLabel(widget);
        minLabel->setObjectName(QString::fromUtf8("minLabel"));
        QFont font;
        font.setPointSize(10);
        font.setBold(false);
        font.setWeight(50);
        minLabel->setFont(font);

        minMaxHLayout->addWidget(minLabel);

        maxLabel = new QLabel(widget);
        maxLabel->setObjectName(QString::fromUtf8("maxLabel"));
        QFont font1;
        font1.setPointSize(10);
        maxLabel->setFont(font1);
        maxLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        minMaxHLayout->addWidget(maxLabel);


        retranslateUi(ThresholdTool);

        QMetaObject::connectSlotsByName(ThresholdTool);
    } // setupUi

    void retranslateUi(QWidget *ThresholdTool)
    {
        ThresholdTool->setWindowTitle(QApplication::translate("ThresholdTool", "Form", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("ThresholdTool", "Active:", 0, QApplication::UnicodeUTF8));
        lowerLabel->setText(QApplication::translate("ThresholdTool", "Lower Threshold:", 0, QApplication::UnicodeUTF8));
        upperLabel->setText(QApplication::translate("ThresholdTool", "Upper Threshold:", 0, QApplication::UnicodeUTF8));
        createThresholdLayerButton->setText(QApplication::translate("ThresholdTool", "Create Threshold Layer", 0, QApplication::UnicodeUTF8));
        clearSeedsButton->setText(QApplication::translate("ThresholdTool", "Clear Seeds", 0, QApplication::UnicodeUTF8));
        tempHistogramLabel->setText(QApplication::translate("ThresholdTool", "Histogram goes here.", 0, QApplication::UnicodeUTF8));
        minLabel->setText(QApplication::translate("ThresholdTool", "min", 0, QApplication::UnicodeUTF8));
        maxLabel->setText(QApplication::translate("ThresholdTool", "max", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ThresholdTool: public Ui_ThresholdTool {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_THRESHOLDTOOL_H
