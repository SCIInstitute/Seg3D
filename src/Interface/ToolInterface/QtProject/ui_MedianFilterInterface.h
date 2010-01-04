/********************************************************************************
** Form generated from reading UI file 'MedianFilterInterface.ui'
**
** Created: Wed Dec 30 11:35:13 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MEDIANFILTERINTERFACE_H
#define UI_MEDIANFILTERINTERFACE_H

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

class Ui_MedianFilterInterface
{
public:
    QWidget *layoutWidget_8;
    QHBoxLayout *radiusHLayout_top;
    QLabel *radiusLabel;
    QSpacerItem *radiusHSpacer;
    QWidget *layoutWidget_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QWidget *layoutWidget_7;
    QHBoxLayout *radiusHLayout_bottom;
    QWidget *layoutWidget;
    QHBoxLayout *replaceInvertLayout;
    QCheckBox *replaceCheckBox;
    QPushButton *invertButton;

    void setupUi(QWidget *MedianFilterInterface)
    {
        if (MedianFilterInterface->objectName().isEmpty())
            MedianFilterInterface->setObjectName(QString::fromUtf8("MedianFilterInterface"));
        MedianFilterInterface->resize(213, 136);
        layoutWidget_8 = new QWidget(MedianFilterInterface);
        layoutWidget_8->setObjectName(QString::fromUtf8("layoutWidget_8"));
        layoutWidget_8->setGeometry(QRect(4, 29, 205, 22));
        radiusHLayout_top = new QHBoxLayout(layoutWidget_8);
        radiusHLayout_top->setObjectName(QString::fromUtf8("radiusHLayout_top"));
        radiusHLayout_top->setContentsMargins(0, 0, 0, 0);
        radiusLabel = new QLabel(layoutWidget_8);
        radiusLabel->setObjectName(QString::fromUtf8("radiusLabel"));

        radiusHLayout_top->addWidget(radiusLabel);

        radiusHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        radiusHLayout_top->addItem(radiusHSpacer);

        layoutWidget_2 = new QWidget(MedianFilterInterface);
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
        layoutWidget_7 = new QWidget(MedianFilterInterface);
        layoutWidget_7->setObjectName(QString::fromUtf8("layoutWidget_7"));
        layoutWidget_7->setGeometry(QRect(4, 50, 205, 51));
        radiusHLayout_bottom = new QHBoxLayout(layoutWidget_7);
        radiusHLayout_bottom->setObjectName(QString::fromUtf8("radiusHLayout_bottom"));
        radiusHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget = new QWidget(MedianFilterInterface);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(4, 100, 205, 32));
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


        retranslateUi(MedianFilterInterface);

        QMetaObject::connectSlotsByName(MedianFilterInterface);
    } // setupUi

    void retranslateUi(QWidget *MedianFilterInterface)
    {
        MedianFilterInterface->setWindowTitle(QApplication::translate("MedianFilterInterface", "Form", 0, QApplication::UnicodeUTF8));
        radiusLabel->setText(QApplication::translate("MedianFilterInterface", "Radius", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("MedianFilterInterface", "Target:", 0, QApplication::UnicodeUTF8));
        replaceCheckBox->setText(QApplication::translate("MedianFilterInterface", "Replace", 0, QApplication::UnicodeUTF8));
        invertButton->setText(QApplication::translate("MedianFilterInterface", "Invert", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MedianFilterInterface: public Ui_MedianFilterInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MEDIANFILTERINTERFACE_H
