/********************************************************************************
** Form generated from reading UI file 'DiscreteGaussianFilterInterface.ui'
**
** Created: Tue Jan 5 13:59:09 2010
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DISCRETEGAUSSIANFILTERINTERFACE_H
#define UI_DISCRETEGAUSSIANFILTERINTERFACE_H

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
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DiscreteGaussianFilterInterface
{
public:
    QWidget *layoutWidget_7;
    QHBoxLayout *varianceHLayout_bottom;
    QWidget *layoutWidget_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QWidget *layoutWidget_8;
    QHBoxLayout *varianceHLayout_top;
    QLabel *varianceLabel;
    QSpacerItem *varianceHSpacer;
    QWidget *layoutWidget_9;
    QHBoxLayout *kernelHLayout_top;
    QLabel *kernelLabel;
    QSpacerItem *kernelHSpacer;
    QWidget *layoutWidget_10;
    QHBoxLayout *kernelHLayout_bottom;
    QWidget *layoutWidget;
    QHBoxLayout *replaceInvertLayout;
    QCheckBox *replaceCheckBox;
    QPushButton *invertButton;

    void setupUi(QWidget *DiscreteGaussianFilterInterface)
    {
        if (DiscreteGaussianFilterInterface->objectName().isEmpty())
            DiscreteGaussianFilterInterface->setObjectName(QString::fromUtf8("DiscreteGaussianFilterInterface"));
        DiscreteGaussianFilterInterface->resize(213, 207);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DiscreteGaussianFilterInterface->sizePolicy().hasHeightForWidth());
        DiscreteGaussianFilterInterface->setSizePolicy(sizePolicy);
        DiscreteGaussianFilterInterface->setMinimumSize(QSize(213, 207));
        DiscreteGaussianFilterInterface->setMaximumSize(QSize(213, 207));
        layoutWidget_7 = new QWidget(DiscreteGaussianFilterInterface);
        layoutWidget_7->setObjectName(QString::fromUtf8("layoutWidget_7"));
        layoutWidget_7->setGeometry(QRect(4, 50, 205, 51));
        varianceHLayout_bottom = new QHBoxLayout(layoutWidget_7);
        varianceHLayout_bottom->setObjectName(QString::fromUtf8("varianceHLayout_bottom"));
        varianceHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget_2 = new QWidget(DiscreteGaussianFilterInterface);
        layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(4, 4, 207, 26));
        activeHLayout = new QHBoxLayout(layoutWidget_2);
        activeHLayout->setSpacing(0);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeHLayout->setContentsMargins(0, 0, 0, 0);
        activeLabel = new QLabel(layoutWidget_2);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(layoutWidget_2);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));

        activeHLayout->addWidget(activeComboBox);

        activeHLayout->setStretch(0, 1);
        activeHLayout->setStretch(1, 2);
        layoutWidget_8 = new QWidget(DiscreteGaussianFilterInterface);
        layoutWidget_8->setObjectName(QString::fromUtf8("layoutWidget_8"));
        layoutWidget_8->setGeometry(QRect(4, 29, 205, 22));
        varianceHLayout_top = new QHBoxLayout(layoutWidget_8);
        varianceHLayout_top->setObjectName(QString::fromUtf8("varianceHLayout_top"));
        varianceHLayout_top->setContentsMargins(0, 0, 0, 0);
        varianceLabel = new QLabel(layoutWidget_8);
        varianceLabel->setObjectName(QString::fromUtf8("varianceLabel"));

        varianceHLayout_top->addWidget(varianceLabel);

        varianceHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        varianceHLayout_top->addItem(varianceHSpacer);

        layoutWidget_9 = new QWidget(DiscreteGaussianFilterInterface);
        layoutWidget_9->setObjectName(QString::fromUtf8("layoutWidget_9"));
        layoutWidget_9->setGeometry(QRect(4, 100, 205, 22));
        kernelHLayout_top = new QHBoxLayout(layoutWidget_9);
        kernelHLayout_top->setObjectName(QString::fromUtf8("kernelHLayout_top"));
        kernelHLayout_top->setContentsMargins(0, 0, 0, 0);
        kernelLabel = new QLabel(layoutWidget_9);
        kernelLabel->setObjectName(QString::fromUtf8("kernelLabel"));

        kernelHLayout_top->addWidget(kernelLabel);

        kernelHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        kernelHLayout_top->addItem(kernelHSpacer);

        layoutWidget_10 = new QWidget(DiscreteGaussianFilterInterface);
        layoutWidget_10->setObjectName(QString::fromUtf8("layoutWidget_10"));
        layoutWidget_10->setGeometry(QRect(4, 121, 205, 51));
        kernelHLayout_bottom = new QHBoxLayout(layoutWidget_10);
        kernelHLayout_bottom->setObjectName(QString::fromUtf8("kernelHLayout_bottom"));
        kernelHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget = new QWidget(DiscreteGaussianFilterInterface);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(4, 171, 205, 32));
        replaceInvertLayout = new QHBoxLayout(layoutWidget);
        replaceInvertLayout->setSpacing(0);
        replaceInvertLayout->setObjectName(QString::fromUtf8("replaceInvertLayout"));
        replaceInvertLayout->setContentsMargins(0, 0, 0, 0);
        replaceCheckBox = new QCheckBox(layoutWidget);
        replaceCheckBox->setObjectName(QString::fromUtf8("replaceCheckBox"));
        replaceCheckBox->setChecked(true);

        replaceInvertLayout->addWidget(replaceCheckBox);

        invertButton = new QPushButton(layoutWidget);
        invertButton->setObjectName(QString::fromUtf8("invertButton"));

        replaceInvertLayout->addWidget(invertButton);


        retranslateUi(DiscreteGaussianFilterInterface);

        QMetaObject::connectSlotsByName(DiscreteGaussianFilterInterface);
    } // setupUi

    void retranslateUi(QWidget *DiscreteGaussianFilterInterface)
    {
        DiscreteGaussianFilterInterface->setWindowTitle(QApplication::translate("DiscreteGaussianFilterInterface", "Discrete Gaussian Filter", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("DiscreteGaussianFilterInterface", "Target:", 0, QApplication::UnicodeUTF8));
        varianceLabel->setText(QApplication::translate("DiscreteGaussianFilterInterface", "Variance", 0, QApplication::UnicodeUTF8));
        kernelLabel->setText(QApplication::translate("DiscreteGaussianFilterInterface", "Maximum Kernel Width", 0, QApplication::UnicodeUTF8));
        replaceCheckBox->setText(QApplication::translate("DiscreteGaussianFilterInterface", "Replace", 0, QApplication::UnicodeUTF8));
        invertButton->setText(QApplication::translate("DiscreteGaussianFilterInterface", "Run Filter", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DiscreteGaussianFilterInterface: public Ui_DiscreteGaussianFilterInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DISCRETEGAUSSIANFILTERINTERFACE_H
