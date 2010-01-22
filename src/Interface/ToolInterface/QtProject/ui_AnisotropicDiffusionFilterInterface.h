/********************************************************************************
** Form generated from reading UI file 'AnisotropicDiffusionFilterInterface.ui'
**
** Created: Mon Jan 18 16:33:21 2010
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
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AnisotropicDiffusionFilterInterface
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QHBoxLayout *iterationsHLayout_top;
    QLabel *iterationsLabel;
    QSpacerItem *iterationsHSpacer;
    QHBoxLayout *iterationsHLayout_bottom;
    QHBoxLayout *integrationHLayout_top;
    QLabel *integrationLabel;
    QSpacerItem *integrationHSpacer;
    QHBoxLayout *integrationHLayout_bottom;
    QHBoxLayout *conductanceHLayout;
    QLabel *conductanceLabel;
    QSpacerItem *conductanceHSpacer;
    QHBoxLayout *conductanceHLayout_bottom;
    QHBoxLayout *replaceInvertLayout;
    QCheckBox *replaceCheckBox;
    QPushButton *invertButton;
    QPushButton *pushButton;

    void setupUi(QWidget *AnisotropicDiffusionFilterInterface)
    {
        if (AnisotropicDiffusionFilterInterface->objectName().isEmpty())
            AnisotropicDiffusionFilterInterface->setObjectName(QString::fromUtf8("AnisotropicDiffusionFilterInterface"));
        AnisotropicDiffusionFilterInterface->resize(650, 306);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(AnisotropicDiffusionFilterInterface->sizePolicy().hasHeightForWidth());
        AnisotropicDiffusionFilterInterface->setSizePolicy(sizePolicy);
        AnisotropicDiffusionFilterInterface->setMinimumSize(QSize(213, 306));
        AnisotropicDiffusionFilterInterface->setMaximumSize(QSize(16777215, 306));
        AnisotropicDiffusionFilterInterface->setBaseSize(QSize(650, 0));
        verticalLayout = new QVBoxLayout(AnisotropicDiffusionFilterInterface);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(4, 0, 4, 0);
        activeHLayout = new QHBoxLayout();
        activeHLayout->setSpacing(0);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeLabel = new QLabel(AnisotropicDiffusionFilterInterface);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(AnisotropicDiffusionFilterInterface);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));

        activeHLayout->addWidget(activeComboBox);

        activeHLayout->setStretch(0, 1);
        activeHLayout->setStretch(1, 2);

        verticalLayout->addLayout(activeHLayout);

        iterationsHLayout_top = new QHBoxLayout();
        iterationsHLayout_top->setObjectName(QString::fromUtf8("iterationsHLayout_top"));
        iterationsLabel = new QLabel(AnisotropicDiffusionFilterInterface);
        iterationsLabel->setObjectName(QString::fromUtf8("iterationsLabel"));

        iterationsHLayout_top->addWidget(iterationsLabel);

        iterationsHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        iterationsHLayout_top->addItem(iterationsHSpacer);


        verticalLayout->addLayout(iterationsHLayout_top);

        iterationsHLayout_bottom = new QHBoxLayout();
        iterationsHLayout_bottom->setObjectName(QString::fromUtf8("iterationsHLayout_bottom"));

        verticalLayout->addLayout(iterationsHLayout_bottom);

        integrationHLayout_top = new QHBoxLayout();
        integrationHLayout_top->setObjectName(QString::fromUtf8("integrationHLayout_top"));
        integrationLabel = new QLabel(AnisotropicDiffusionFilterInterface);
        integrationLabel->setObjectName(QString::fromUtf8("integrationLabel"));

        integrationHLayout_top->addWidget(integrationLabel);

        integrationHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        integrationHLayout_top->addItem(integrationHSpacer);


        verticalLayout->addLayout(integrationHLayout_top);

        integrationHLayout_bottom = new QHBoxLayout();
        integrationHLayout_bottom->setObjectName(QString::fromUtf8("integrationHLayout_bottom"));

        verticalLayout->addLayout(integrationHLayout_bottom);

        conductanceHLayout = new QHBoxLayout();
        conductanceHLayout->setObjectName(QString::fromUtf8("conductanceHLayout"));
        conductanceLabel = new QLabel(AnisotropicDiffusionFilterInterface);
        conductanceLabel->setObjectName(QString::fromUtf8("conductanceLabel"));

        conductanceHLayout->addWidget(conductanceLabel);

        conductanceHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        conductanceHLayout->addItem(conductanceHSpacer);


        verticalLayout->addLayout(conductanceHLayout);

        conductanceHLayout_bottom = new QHBoxLayout();
        conductanceHLayout_bottom->setObjectName(QString::fromUtf8("conductanceHLayout_bottom"));

        verticalLayout->addLayout(conductanceHLayout_bottom);

        replaceInvertLayout = new QHBoxLayout();
        replaceInvertLayout->setObjectName(QString::fromUtf8("replaceInvertLayout"));
        replaceCheckBox = new QCheckBox(AnisotropicDiffusionFilterInterface);
        replaceCheckBox->setObjectName(QString::fromUtf8("replaceCheckBox"));
        replaceCheckBox->setChecked(true);

        replaceInvertLayout->addWidget(replaceCheckBox);

        invertButton = new QPushButton(AnisotropicDiffusionFilterInterface);
        invertButton->setObjectName(QString::fromUtf8("invertButton"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(invertButton->sizePolicy().hasHeightForWidth());
        invertButton->setSizePolicy(sizePolicy1);

        replaceInvertLayout->addWidget(invertButton);


        verticalLayout->addLayout(replaceInvertLayout);

        pushButton = new QPushButton(AnisotropicDiffusionFilterInterface);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        verticalLayout->addWidget(pushButton);


        retranslateUi(AnisotropicDiffusionFilterInterface);

        QMetaObject::connectSlotsByName(AnisotropicDiffusionFilterInterface);
    } // setupUi

    void retranslateUi(QWidget *AnisotropicDiffusionFilterInterface)
    {
        AnisotropicDiffusionFilterInterface->setWindowTitle(QApplication::translate("AnisotropicDiffusionFilterInterface", "Anisotropic Diffusion Filter", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("AnisotropicDiffusionFilterInterface", "Target:", 0, QApplication::UnicodeUTF8));
        iterationsLabel->setText(QApplication::translate("AnisotropicDiffusionFilterInterface", "Number of Iterations:", 0, QApplication::UnicodeUTF8));
        integrationLabel->setText(QApplication::translate("AnisotropicDiffusionFilterInterface", "Integration Step:", 0, QApplication::UnicodeUTF8));
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
