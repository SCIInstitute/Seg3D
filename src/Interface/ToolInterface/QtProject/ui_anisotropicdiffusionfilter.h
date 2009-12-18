/********************************************************************************
** Form generated from reading UI file 'anisotropicdiffusionfilter.ui'
**
** Created: Fri Dec 18 15:00:15 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ANISOTROPICDIFFUSIONFILTER_H
#define UI_ANISOTROPICDIFFUSIONFILTER_H

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

class Ui_AnisotropicDiffusionFilter
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

    void setupUi(QWidget *AnisotropicDiffusionFilter)
    {
        if (AnisotropicDiffusionFilter->objectName().isEmpty())
            AnisotropicDiffusionFilter->setObjectName(QString::fromUtf8("AnisotropicDiffusionFilter"));
        AnisotropicDiffusionFilter->resize(231, 331);
        layoutWidget_2 = new QWidget(AnisotropicDiffusionFilter);
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

        layoutWidget_6 = new QWidget(AnisotropicDiffusionFilter);
        layoutWidget_6->setObjectName(QString::fromUtf8("layoutWidget_6"));
        layoutWidget_6->setGeometry(QRect(10, 127, 211, 51));
        integrationHLayout_bottom = new QHBoxLayout(layoutWidget_6);
        integrationHLayout_bottom->setObjectName(QString::fromUtf8("integrationHLayout_bottom"));
        integrationHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget_9 = new QWidget(AnisotropicDiffusionFilter);
        layoutWidget_9->setObjectName(QString::fromUtf8("layoutWidget_9"));
        layoutWidget_9->setGeometry(QRect(10, 106, 211, 22));
        integrationHLayout_top = new QHBoxLayout(layoutWidget_9);
        integrationHLayout_top->setObjectName(QString::fromUtf8("integrationHLayout_top"));
        integrationHLayout_top->setContentsMargins(0, 0, 0, 0);
        integrationLabel = new QLabel(layoutWidget_9);
        integrationLabel->setObjectName(QString::fromUtf8("integrationLabel"));

        integrationHLayout_top->addWidget(integrationLabel);

        integrationHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        integrationHLayout_top->addItem(integrationHSpacer);

        layoutWidget_8 = new QWidget(AnisotropicDiffusionFilter);
        layoutWidget_8->setObjectName(QString::fromUtf8("layoutWidget_8"));
        layoutWidget_8->setGeometry(QRect(10, 35, 211, 22));
        iterationsHLayout_top = new QHBoxLayout(layoutWidget_8);
        iterationsHLayout_top->setObjectName(QString::fromUtf8("iterationsHLayout_top"));
        iterationsHLayout_top->setContentsMargins(0, 0, 0, 0);
        iterationsLabel = new QLabel(layoutWidget_8);
        iterationsLabel->setObjectName(QString::fromUtf8("iterationsLabel"));

        iterationsHLayout_top->addWidget(iterationsLabel);

        iterationsHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        iterationsHLayout_top->addItem(iterationsHSpacer);

        layoutWidget_7 = new QWidget(AnisotropicDiffusionFilter);
        layoutWidget_7->setObjectName(QString::fromUtf8("layoutWidget_7"));
        layoutWidget_7->setGeometry(QRect(10, 56, 211, 51));
        iterationsHLayout_bottom = new QHBoxLayout(layoutWidget_7);
        iterationsHLayout_bottom->setObjectName(QString::fromUtf8("iterationsHLayout_bottom"));
        iterationsHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget_10 = new QWidget(AnisotropicDiffusionFilter);
        layoutWidget_10->setObjectName(QString::fromUtf8("layoutWidget_10"));
        layoutWidget_10->setGeometry(QRect(10, 177, 211, 22));
        conductanceHLayout = new QHBoxLayout(layoutWidget_10);
        conductanceHLayout->setObjectName(QString::fromUtf8("conductanceHLayout"));
        conductanceHLayout->setContentsMargins(0, 0, 0, 0);
        conductanceLabel = new QLabel(layoutWidget_10);
        conductanceLabel->setObjectName(QString::fromUtf8("conductanceLabel"));

        conductanceHLayout->addWidget(conductanceLabel);

        conductanceHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        conductanceHLayout->addItem(conductanceHSpacer);

        layoutWidget_11 = new QWidget(AnisotropicDiffusionFilter);
        layoutWidget_11->setObjectName(QString::fromUtf8("layoutWidget_11"));
        layoutWidget_11->setGeometry(QRect(10, 198, 211, 51));
        conductanceHLayout_bottom = new QHBoxLayout(layoutWidget_11);
        conductanceHLayout_bottom->setObjectName(QString::fromUtf8("conductanceHLayout_bottom"));
        conductanceHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget = new QWidget(AnisotropicDiffusionFilter);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 248, 211, 32));
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

        pushButton = new QPushButton(AnisotropicDiffusionFilter);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(40, 290, 151, 32));

        retranslateUi(AnisotropicDiffusionFilter);

        QMetaObject::connectSlotsByName(AnisotropicDiffusionFilter);
    } // setupUi

    void retranslateUi(QWidget *AnisotropicDiffusionFilter)
    {
        AnisotropicDiffusionFilter->setWindowTitle(QApplication::translate("AnisotropicDiffusionFilter", "Form", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("AnisotropicDiffusionFilter", "Active:", 0, QApplication::UnicodeUTF8));
        integrationLabel->setText(QApplication::translate("AnisotropicDiffusionFilter", "Integration Step:", 0, QApplication::UnicodeUTF8));
        iterationsLabel->setText(QApplication::translate("AnisotropicDiffusionFilter", "Number of Iterations:", 0, QApplication::UnicodeUTF8));
        conductanceLabel->setText(QApplication::translate("AnisotropicDiffusionFilter", "Conductance:", 0, QApplication::UnicodeUTF8));
        replaceCheckBox->setText(QApplication::translate("AnisotropicDiffusionFilter", "Replace", 0, QApplication::UnicodeUTF8));
        invertButton->setText(QApplication::translate("AnisotropicDiffusionFilter", "Invert", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("AnisotropicDiffusionFilter", "Set to defaults", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AnisotropicDiffusionFilter: public Ui_AnisotropicDiffusionFilter {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ANISOTROPICDIFFUSIONFILTER_H
