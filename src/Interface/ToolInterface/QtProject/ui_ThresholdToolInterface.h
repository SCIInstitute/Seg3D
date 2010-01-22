/********************************************************************************
** Form generated from reading UI file 'ThresholdToolInterface.ui'
**
** Created: Mon Jan 18 16:33:21 2010
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
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ThresholdToolInterface
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QHBoxLayout *upperHLayout_top;
    QLabel *upperLabel;
    QSpacerItem *upperHSpacer;
    QHBoxLayout *upperHLayout_bottom;
    QHBoxLayout *lowerHLayout_top;
    QLabel *lowerLabel;
    QSpacerItem *lowerHSpacer;
    QHBoxLayout *lowerHLayout_bottom;
    QVBoxLayout *verticalLayout;
    QPushButton *createThresholdLayerButton;
    QPushButton *clearSeedsButton;
    QGraphicsView *histogramView;
    QHBoxLayout *minMaxHLayout;
    QLabel *minLabel;
    QLabel *maxLabel;

    void setupUi(QWidget *ThresholdToolInterface)
    {
        if (ThresholdToolInterface->objectName().isEmpty())
            ThresholdToolInterface->setObjectName(QString::fromUtf8("ThresholdToolInterface"));
        ThresholdToolInterface->resize(650, 396);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ThresholdToolInterface->sizePolicy().hasHeightForWidth());
        ThresholdToolInterface->setSizePolicy(sizePolicy);
        ThresholdToolInterface->setMinimumSize(QSize(213, 396));
        ThresholdToolInterface->setMaximumSize(QSize(16777215, 396));
        ThresholdToolInterface->setBaseSize(QSize(650, 396));
        verticalLayout_2 = new QVBoxLayout(ThresholdToolInterface);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(4, 0, 4, 0);
        activeHLayout = new QHBoxLayout();
        activeHLayout->setSpacing(0);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeLabel = new QLabel(ThresholdToolInterface);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(ThresholdToolInterface);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));

        activeHLayout->addWidget(activeComboBox);

        activeHLayout->setStretch(0, 1);
        activeHLayout->setStretch(1, 2);

        verticalLayout_2->addLayout(activeHLayout);

        upperHLayout_top = new QHBoxLayout();
        upperHLayout_top->setObjectName(QString::fromUtf8("upperHLayout_top"));
        upperLabel = new QLabel(ThresholdToolInterface);
        upperLabel->setObjectName(QString::fromUtf8("upperLabel"));

        upperHLayout_top->addWidget(upperLabel);

        upperHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        upperHLayout_top->addItem(upperHSpacer);


        verticalLayout_2->addLayout(upperHLayout_top);

        upperHLayout_bottom = new QHBoxLayout();
        upperHLayout_bottom->setObjectName(QString::fromUtf8("upperHLayout_bottom"));

        verticalLayout_2->addLayout(upperHLayout_bottom);

        lowerHLayout_top = new QHBoxLayout();
        lowerHLayout_top->setObjectName(QString::fromUtf8("lowerHLayout_top"));
        lowerLabel = new QLabel(ThresholdToolInterface);
        lowerLabel->setObjectName(QString::fromUtf8("lowerLabel"));

        lowerHLayout_top->addWidget(lowerLabel);

        lowerHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        lowerHLayout_top->addItem(lowerHSpacer);


        verticalLayout_2->addLayout(lowerHLayout_top);

        lowerHLayout_bottom = new QHBoxLayout();
        lowerHLayout_bottom->setObjectName(QString::fromUtf8("lowerHLayout_bottom"));

        verticalLayout_2->addLayout(lowerHLayout_bottom);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        createThresholdLayerButton = new QPushButton(ThresholdToolInterface);
        createThresholdLayerButton->setObjectName(QString::fromUtf8("createThresholdLayerButton"));

        verticalLayout->addWidget(createThresholdLayerButton);

        clearSeedsButton = new QPushButton(ThresholdToolInterface);
        clearSeedsButton->setObjectName(QString::fromUtf8("clearSeedsButton"));

        verticalLayout->addWidget(clearSeedsButton);


        verticalLayout_2->addLayout(verticalLayout);

        histogramView = new QGraphicsView(ThresholdToolInterface);
        histogramView->setObjectName(QString::fromUtf8("histogramView"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(histogramView->sizePolicy().hasHeightForWidth());
        histogramView->setSizePolicy(sizePolicy1);
        histogramView->setMinimumSize(QSize(0, 131));
        histogramView->setMaximumSize(QSize(16777215, 131));

        verticalLayout_2->addWidget(histogramView);

        minMaxHLayout = new QHBoxLayout();
        minMaxHLayout->setObjectName(QString::fromUtf8("minMaxHLayout"));
        minLabel = new QLabel(ThresholdToolInterface);
        minLabel->setObjectName(QString::fromUtf8("minLabel"));
        QFont font;
        font.setPointSize(10);
        font.setBold(false);
        font.setWeight(50);
        minLabel->setFont(font);

        minMaxHLayout->addWidget(minLabel);

        maxLabel = new QLabel(ThresholdToolInterface);
        maxLabel->setObjectName(QString::fromUtf8("maxLabel"));
        QFont font1;
        font1.setPointSize(10);
        maxLabel->setFont(font1);
        maxLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        minMaxHLayout->addWidget(maxLabel);


        verticalLayout_2->addLayout(minMaxHLayout);


        retranslateUi(ThresholdToolInterface);

        QMetaObject::connectSlotsByName(ThresholdToolInterface);
    } // setupUi

    void retranslateUi(QWidget *ThresholdToolInterface)
    {
        ThresholdToolInterface->setWindowTitle(QApplication::translate("ThresholdToolInterface", "Threshold Tool", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("ThresholdToolInterface", "Target:", 0, QApplication::UnicodeUTF8));
        upperLabel->setText(QApplication::translate("ThresholdToolInterface", "Upper Threshold:", 0, QApplication::UnicodeUTF8));
        lowerLabel->setText(QApplication::translate("ThresholdToolInterface", "Lower Threshold:", 0, QApplication::UnicodeUTF8));
        createThresholdLayerButton->setText(QApplication::translate("ThresholdToolInterface", "Create Threshold Layer", 0, QApplication::UnicodeUTF8));
        clearSeedsButton->setText(QApplication::translate("ThresholdToolInterface", "Clear Seeds", 0, QApplication::UnicodeUTF8));
        minLabel->setText(QApplication::translate("ThresholdToolInterface", "min", 0, QApplication::UnicodeUTF8));
        maxLabel->setText(QApplication::translate("ThresholdToolInterface", "max", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ThresholdToolInterface: public Ui_ThresholdToolInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_THRESHOLDTOOLINTERFACE_H
