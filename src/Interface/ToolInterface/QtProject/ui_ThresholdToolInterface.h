/********************************************************************************
** Form generated from reading UI file 'ThresholdToolInterface.ui'
**
** Created: Fri Dec 18 15:00:15 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_THRESHOLDTOOLINTERFACE_H
#define UI_THRESHOLDTOOLINTERFACE_H

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

class Ui_ThresholdToolInterface
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
    QWidget *layoutWidget;
    QHBoxLayout *minMaxHLayout;
    QLabel *minLabel;
    QLabel *maxLabel;

    void setupUi(QWidget *ThresholdToolInterface)
    {
        if (ThresholdToolInterface->objectName().isEmpty())
            ThresholdToolInterface->setObjectName(QString::fromUtf8("ThresholdToolInterface"));
        ThresholdToolInterface->resize(213, 396);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ThresholdToolInterface->sizePolicy().hasHeightForWidth());
        ThresholdToolInterface->setSizePolicy(sizePolicy);
        ThresholdToolInterface->setMinimumSize(QSize(213, 396));
        ThresholdToolInterface->setMaximumSize(QSize(213, 396));
        ThresholdToolInterface->setBaseSize(QSize(213, 396));
        layoutWidget_2 = new QWidget(ThresholdToolInterface);
        layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(4, 4, 207, 26));
        activeHLayout = new QHBoxLayout(layoutWidget_2);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeHLayout->setContentsMargins(0, 0, 0, 0);
        activeLabel = new QLabel(layoutWidget_2);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(layoutWidget_2);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));

        activeHLayout->addWidget(activeComboBox);

        layoutWidget_6 = new QWidget(ThresholdToolInterface);
        layoutWidget_6->setObjectName(QString::fromUtf8("layoutWidget_6"));
        layoutWidget_6->setGeometry(QRect(4, 121, 205, 51));
        lowerHLayout_bottom = new QHBoxLayout(layoutWidget_6);
        lowerHLayout_bottom->setObjectName(QString::fromUtf8("lowerHLayout_bottom"));
        lowerHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget_9 = new QWidget(ThresholdToolInterface);
        layoutWidget_9->setObjectName(QString::fromUtf8("layoutWidget_9"));
        layoutWidget_9->setGeometry(QRect(4, 100, 205, 22));
        lowerHLayout_top = new QHBoxLayout(layoutWidget_9);
        lowerHLayout_top->setObjectName(QString::fromUtf8("lowerHLayout_top"));
        lowerHLayout_top->setContentsMargins(0, 0, 0, 0);
        lowerLabel = new QLabel(layoutWidget_9);
        lowerLabel->setObjectName(QString::fromUtf8("lowerLabel"));

        lowerHLayout_top->addWidget(lowerLabel);

        lowerHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        lowerHLayout_top->addItem(lowerHSpacer);

        layoutWidget_8 = new QWidget(ThresholdToolInterface);
        layoutWidget_8->setObjectName(QString::fromUtf8("layoutWidget_8"));
        layoutWidget_8->setGeometry(QRect(4, 29, 205, 22));
        upperHLayout_top = new QHBoxLayout(layoutWidget_8);
        upperHLayout_top->setObjectName(QString::fromUtf8("upperHLayout_top"));
        upperHLayout_top->setContentsMargins(0, 0, 0, 0);
        upperLabel = new QLabel(layoutWidget_8);
        upperLabel->setObjectName(QString::fromUtf8("upperLabel"));

        upperHLayout_top->addWidget(upperLabel);

        upperHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        upperHLayout_top->addItem(upperHSpacer);

        layoutWidget_7 = new QWidget(ThresholdToolInterface);
        layoutWidget_7->setObjectName(QString::fromUtf8("layoutWidget_7"));
        layoutWidget_7->setGeometry(QRect(4, 50, 205, 51));
        upperHLayout_bottom = new QHBoxLayout(layoutWidget_7);
        upperHLayout_bottom->setObjectName(QString::fromUtf8("upperHLayout_bottom"));
        upperHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        createThresholdLayerButton = new QPushButton(ThresholdToolInterface);
        createThresholdLayerButton->setObjectName(QString::fromUtf8("createThresholdLayerButton"));
        createThresholdLayerButton->setGeometry(QRect(6, 176, 201, 32));
        clearSeedsButton = new QPushButton(ThresholdToolInterface);
        clearSeedsButton->setObjectName(QString::fromUtf8("clearSeedsButton"));
        clearSeedsButton->setGeometry(QRect(50, 208, 115, 32));
        histogramView = new QGraphicsView(ThresholdToolInterface);
        histogramView->setObjectName(QString::fromUtf8("histogramView"));
        histogramView->setGeometry(QRect(6, 240, 201, 131));
        tempHistogramLabel = new QLabel(ThresholdToolInterface);
        tempHistogramLabel->setObjectName(QString::fromUtf8("tempHistogramLabel"));
        tempHistogramLabel->setGeometry(QRect(60, 290, 111, 51));
        tempHistogramLabel->setAlignment(Qt::AlignCenter);
        tempHistogramLabel->setWordWrap(true);
        layoutWidget = new QWidget(ThresholdToolInterface);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(6, 370, 201, 20));
        minMaxHLayout = new QHBoxLayout(layoutWidget);
        minMaxHLayout->setObjectName(QString::fromUtf8("minMaxHLayout"));
        minMaxHLayout->setContentsMargins(0, 0, 0, 0);
        minLabel = new QLabel(layoutWidget);
        minLabel->setObjectName(QString::fromUtf8("minLabel"));
        QFont font;
        font.setPointSize(10);
        font.setBold(false);
        font.setWeight(50);
        minLabel->setFont(font);

        minMaxHLayout->addWidget(minLabel);

        maxLabel = new QLabel(layoutWidget);
        maxLabel->setObjectName(QString::fromUtf8("maxLabel"));
        QFont font1;
        font1.setPointSize(10);
        maxLabel->setFont(font1);
        maxLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        minMaxHLayout->addWidget(maxLabel);


        retranslateUi(ThresholdToolInterface);

        QMetaObject::connectSlotsByName(ThresholdToolInterface);
    } // setupUi

    void retranslateUi(QWidget *ThresholdToolInterface)
    {
        ThresholdToolInterface->setWindowTitle(QApplication::translate("ThresholdToolInterface", "Form", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("ThresholdToolInterface", "Active:", 0, QApplication::UnicodeUTF8));
        lowerLabel->setText(QApplication::translate("ThresholdToolInterface", "Lower Threshold:", 0, QApplication::UnicodeUTF8));
        upperLabel->setText(QApplication::translate("ThresholdToolInterface", "Upper Threshold:", 0, QApplication::UnicodeUTF8));
        createThresholdLayerButton->setText(QApplication::translate("ThresholdToolInterface", "Create Threshold Layer", 0, QApplication::UnicodeUTF8));
        clearSeedsButton->setText(QApplication::translate("ThresholdToolInterface", "Clear Seeds", 0, QApplication::UnicodeUTF8));
        tempHistogramLabel->setText(QApplication::translate("ThresholdToolInterface", "Histogram goes here.", 0, QApplication::UnicodeUTF8));
        minLabel->setText(QApplication::translate("ThresholdToolInterface", "min", 0, QApplication::UnicodeUTF8));
        maxLabel->setText(QApplication::translate("ThresholdToolInterface", "max", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ThresholdToolInterface: public Ui_ThresholdToolInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_THRESHOLDTOOLINTERFACE_H
