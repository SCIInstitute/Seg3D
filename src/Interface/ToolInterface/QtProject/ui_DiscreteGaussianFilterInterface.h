/********************************************************************************
** Form generated from reading UI file 'DiscreteGaussianFilterInterface.ui'
**
** Created: Mon Jan 18 16:33:21 2010
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
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DiscreteGaussianFilterInterface
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QHBoxLayout *varianceHLayout_top;
    QLabel *varianceLabel;
    QSpacerItem *varianceHSpacer;
    QHBoxLayout *varianceHLayout_bottom;
    QHBoxLayout *kernelHLayout_top;
    QLabel *kernelLabel;
    QSpacerItem *kernelHSpacer;
    QHBoxLayout *kernelHLayout_bottom;
    QHBoxLayout *replaceInvertLayout;
    QCheckBox *replaceCheckBox;
    QPushButton *invertButton;

    void setupUi(QWidget *DiscreteGaussianFilterInterface)
    {
        if (DiscreteGaussianFilterInterface->objectName().isEmpty())
            DiscreteGaussianFilterInterface->setObjectName(QString::fromUtf8("DiscreteGaussianFilterInterface"));
        DiscreteGaussianFilterInterface->resize(650, 207);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DiscreteGaussianFilterInterface->sizePolicy().hasHeightForWidth());
        DiscreteGaussianFilterInterface->setSizePolicy(sizePolicy);
        DiscreteGaussianFilterInterface->setMinimumSize(QSize(213, 207));
        DiscreteGaussianFilterInterface->setMaximumSize(QSize(16777215, 207));
        DiscreteGaussianFilterInterface->setBaseSize(QSize(650, 0));
        verticalLayout = new QVBoxLayout(DiscreteGaussianFilterInterface);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(4, 0, 4, 0);
        activeHLayout = new QHBoxLayout();
        activeHLayout->setSpacing(0);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeLabel = new QLabel(DiscreteGaussianFilterInterface);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(DiscreteGaussianFilterInterface);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));

        activeHLayout->addWidget(activeComboBox);

        activeHLayout->setStretch(0, 1);
        activeHLayout->setStretch(1, 2);

        verticalLayout->addLayout(activeHLayout);

        varianceHLayout_top = new QHBoxLayout();
        varianceHLayout_top->setObjectName(QString::fromUtf8("varianceHLayout_top"));
        varianceLabel = new QLabel(DiscreteGaussianFilterInterface);
        varianceLabel->setObjectName(QString::fromUtf8("varianceLabel"));

        varianceHLayout_top->addWidget(varianceLabel);

        varianceHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        varianceHLayout_top->addItem(varianceHSpacer);


        verticalLayout->addLayout(varianceHLayout_top);

        varianceHLayout_bottom = new QHBoxLayout();
        varianceHLayout_bottom->setObjectName(QString::fromUtf8("varianceHLayout_bottom"));

        verticalLayout->addLayout(varianceHLayout_bottom);

        kernelHLayout_top = new QHBoxLayout();
        kernelHLayout_top->setObjectName(QString::fromUtf8("kernelHLayout_top"));
        kernelLabel = new QLabel(DiscreteGaussianFilterInterface);
        kernelLabel->setObjectName(QString::fromUtf8("kernelLabel"));

        kernelHLayout_top->addWidget(kernelLabel);

        kernelHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        kernelHLayout_top->addItem(kernelHSpacer);


        verticalLayout->addLayout(kernelHLayout_top);

        kernelHLayout_bottom = new QHBoxLayout();
        kernelHLayout_bottom->setObjectName(QString::fromUtf8("kernelHLayout_bottom"));

        verticalLayout->addLayout(kernelHLayout_bottom);

        replaceInvertLayout = new QHBoxLayout();
        replaceInvertLayout->setSpacing(0);
        replaceInvertLayout->setObjectName(QString::fromUtf8("replaceInvertLayout"));
        replaceCheckBox = new QCheckBox(DiscreteGaussianFilterInterface);
        replaceCheckBox->setObjectName(QString::fromUtf8("replaceCheckBox"));
        replaceCheckBox->setChecked(true);

        replaceInvertLayout->addWidget(replaceCheckBox);

        invertButton = new QPushButton(DiscreteGaussianFilterInterface);
        invertButton->setObjectName(QString::fromUtf8("invertButton"));

        replaceInvertLayout->addWidget(invertButton);


        verticalLayout->addLayout(replaceInvertLayout);


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
