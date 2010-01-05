/********************************************************************************
** Form generated from reading UI file 'AnisotropicDiffusionFilterInterface.ui'
**
** Created: Tue Jan 5 13:59:09 2010
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ANISOTROPICDIFFUSIONFILTERINTERFACE_H
#define UI_ANISOTROPICDIFFUSIONFILTERINTERFACE_H

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

class Ui_AnisotropicDiffusionFilterInterface
{
public:
    QWidget *layoutWidget_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QWidget *layoutWidget_6;
    QHBoxLayout *integrationHLayout_bottom;
    QWidget *layoutWidget_9;
    QHBoxLayout *integrationHLayout_top;
    QLabel *integrationLabel;
    QSpacerItem *integrationHSpacer;
    QWidget *layoutWidget_8;
    QHBoxLayout *iterationsHLayout_top;
    QLabel *iterationsLabel;
    QSpacerItem *iterationsHSpacer;
    QWidget *layoutWidget_7;
    QHBoxLayout *iterationsHLayout_bottom;
    QWidget *layoutWidget_10;
    QHBoxLayout *conductanceHLayout;
    QLabel *conductanceLabel;
    QSpacerItem *conductanceHSpacer;
    QWidget *layoutWidget_11;
    QHBoxLayout *conductanceHLayout_bottom;
    QWidget *layoutWidget;
    QHBoxLayout *replaceInvertLayout;
    QCheckBox *replaceCheckBox;
    QPushButton *invertButton;
    QPushButton *pushButton;

    void setupUi(QWidget *AnisotropicDiffusionFilterInterface)
    {
        if (AnisotropicDiffusionFilterInterface->objectName().isEmpty())
            AnisotropicDiffusionFilterInterface->setObjectName(QString::fromUtf8("AnisotropicDiffusionFilterInterface"));
        AnisotropicDiffusionFilterInterface->resize(213, 306);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(AnisotropicDiffusionFilterInterface->sizePolicy().hasHeightForWidth());
        AnisotropicDiffusionFilterInterface->setSizePolicy(sizePolicy);
        AnisotropicDiffusionFilterInterface->setMinimumSize(QSize(213, 306));
        AnisotropicDiffusionFilterInterface->setMaximumSize(QSize(213, 306));
        layoutWidget_2 = new QWidget(AnisotropicDiffusionFilterInterface);
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
        layoutWidget_6 = new QWidget(AnisotropicDiffusionFilterInterface);
        layoutWidget_6->setObjectName(QString::fromUtf8("layoutWidget_6"));
        layoutWidget_6->setGeometry(QRect(4, 121, 205, 51));
        integrationHLayout_bottom = new QHBoxLayout(layoutWidget_6);
        integrationHLayout_bottom->setObjectName(QString::fromUtf8("integrationHLayout_bottom"));
        integrationHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget_9 = new QWidget(AnisotropicDiffusionFilterInterface);
        layoutWidget_9->setObjectName(QString::fromUtf8("layoutWidget_9"));
        layoutWidget_9->setGeometry(QRect(4, 100, 205, 22));
        integrationHLayout_top = new QHBoxLayout(layoutWidget_9);
        integrationHLayout_top->setObjectName(QString::fromUtf8("integrationHLayout_top"));
        integrationHLayout_top->setContentsMargins(0, 0, 0, 0);
        integrationLabel = new QLabel(layoutWidget_9);
        integrationLabel->setObjectName(QString::fromUtf8("integrationLabel"));

        integrationHLayout_top->addWidget(integrationLabel);

        integrationHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        integrationHLayout_top->addItem(integrationHSpacer);

        layoutWidget_8 = new QWidget(AnisotropicDiffusionFilterInterface);
        layoutWidget_8->setObjectName(QString::fromUtf8("layoutWidget_8"));
        layoutWidget_8->setGeometry(QRect(4, 29, 205, 22));
        iterationsHLayout_top = new QHBoxLayout(layoutWidget_8);
        iterationsHLayout_top->setObjectName(QString::fromUtf8("iterationsHLayout_top"));
        iterationsHLayout_top->setContentsMargins(0, 0, 0, 0);
        iterationsLabel = new QLabel(layoutWidget_8);
        iterationsLabel->setObjectName(QString::fromUtf8("iterationsLabel"));

        iterationsHLayout_top->addWidget(iterationsLabel);

        iterationsHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        iterationsHLayout_top->addItem(iterationsHSpacer);

        layoutWidget_7 = new QWidget(AnisotropicDiffusionFilterInterface);
        layoutWidget_7->setObjectName(QString::fromUtf8("layoutWidget_7"));
        layoutWidget_7->setGeometry(QRect(4, 50, 205, 51));
        iterationsHLayout_bottom = new QHBoxLayout(layoutWidget_7);
        iterationsHLayout_bottom->setObjectName(QString::fromUtf8("iterationsHLayout_bottom"));
        iterationsHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget_10 = new QWidget(AnisotropicDiffusionFilterInterface);
        layoutWidget_10->setObjectName(QString::fromUtf8("layoutWidget_10"));
        layoutWidget_10->setGeometry(QRect(4, 171, 205, 22));
        conductanceHLayout = new QHBoxLayout(layoutWidget_10);
        conductanceHLayout->setObjectName(QString::fromUtf8("conductanceHLayout"));
        conductanceHLayout->setContentsMargins(0, 0, 0, 0);
        conductanceLabel = new QLabel(layoutWidget_10);
        conductanceLabel->setObjectName(QString::fromUtf8("conductanceLabel"));

        conductanceHLayout->addWidget(conductanceLabel);

        conductanceHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        conductanceHLayout->addItem(conductanceHSpacer);

        layoutWidget_11 = new QWidget(AnisotropicDiffusionFilterInterface);
        layoutWidget_11->setObjectName(QString::fromUtf8("layoutWidget_11"));
        layoutWidget_11->setGeometry(QRect(4, 192, 205, 51));
        conductanceHLayout_bottom = new QHBoxLayout(layoutWidget_11);
        conductanceHLayout_bottom->setObjectName(QString::fromUtf8("conductanceHLayout_bottom"));
        conductanceHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget = new QWidget(AnisotropicDiffusionFilterInterface);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(4, 242, 205, 32));
        replaceInvertLayout = new QHBoxLayout(layoutWidget);
        replaceInvertLayout->setObjectName(QString::fromUtf8("replaceInvertLayout"));
        replaceInvertLayout->setContentsMargins(0, 0, 0, 0);
        replaceCheckBox = new QCheckBox(layoutWidget);
        replaceCheckBox->setObjectName(QString::fromUtf8("replaceCheckBox"));
        replaceCheckBox->setChecked(true);

        replaceInvertLayout->addWidget(replaceCheckBox);

        invertButton = new QPushButton(layoutWidget);
        invertButton->setObjectName(QString::fromUtf8("invertButton"));

        replaceInvertLayout->addWidget(invertButton);

        pushButton = new QPushButton(AnisotropicDiffusionFilterInterface);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(30, 274, 151, 32));

        retranslateUi(AnisotropicDiffusionFilterInterface);

        QMetaObject::connectSlotsByName(AnisotropicDiffusionFilterInterface);
    } // setupUi

    void retranslateUi(QWidget *AnisotropicDiffusionFilterInterface)
    {
        AnisotropicDiffusionFilterInterface->setWindowTitle(QApplication::translate("AnisotropicDiffusionFilterInterface", "Anisotropic Diffusion Filter", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("AnisotropicDiffusionFilterInterface", "Target:", 0, QApplication::UnicodeUTF8));
        integrationLabel->setText(QApplication::translate("AnisotropicDiffusionFilterInterface", "Integration Step:", 0, QApplication::UnicodeUTF8));
        iterationsLabel->setText(QApplication::translate("AnisotropicDiffusionFilterInterface", "Number of Iterations:", 0, QApplication::UnicodeUTF8));
        conductanceLabel->setText(QApplication::translate("AnisotropicDiffusionFilterInterface", "Conductance:", 0, QApplication::UnicodeUTF8));
        replaceCheckBox->setText(QApplication::translate("AnisotropicDiffusionFilterInterface", "Replace", 0, QApplication::UnicodeUTF8));
        invertButton->setText(QApplication::translate("AnisotropicDiffusionFilterInterface", "Run Filter", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("AnisotropicDiffusionFilterInterface", "Set to defaults", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AnisotropicDiffusionFilterInterface: public Ui_AnisotropicDiffusionFilterInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ANISOTROPICDIFFUSIONFILTERINTERFACE_H
