/********************************************************************************
** Form generated from reading UI file 'MedianFilter.ui'
**
** Created: Wed Dec 30 08:47:28 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MEDIANFILTER_H
#define UI_MEDIANFILTER_H

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

class Ui_MedianFilter
{
public:
    QWidget *layoutWidget_8;
    QHBoxLayout *iterationsHLayout_top;
    QLabel *iterationsLabel;
    QSpacerItem *iterationsHSpacer;
    QWidget *layoutWidget_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QWidget *layoutWidget_7;
    QHBoxLayout *iterationsHLayout_bottom;
    QWidget *layoutWidget;
    QHBoxLayout *replaceInvertLayout;
    QCheckBox *replaceCheckBox;
    QPushButton *invertButton;

    void setupUi(QWidget *MedianFilter)
    {
        if (MedianFilter->objectName().isEmpty())
            MedianFilter->setObjectName(QString::fromUtf8("MedianFilter"));
        MedianFilter->resize(212, 136);
        layoutWidget_8 = new QWidget(MedianFilter);
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

        layoutWidget_2 = new QWidget(MedianFilter);
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
        layoutWidget_7 = new QWidget(MedianFilter);
        layoutWidget_7->setObjectName(QString::fromUtf8("layoutWidget_7"));
        layoutWidget_7->setGeometry(QRect(4, 50, 205, 51));
        iterationsHLayout_bottom = new QHBoxLayout(layoutWidget_7);
        iterationsHLayout_bottom->setObjectName(QString::fromUtf8("iterationsHLayout_bottom"));
        iterationsHLayout_bottom->setContentsMargins(0, 0, 0, 0);
        layoutWidget = new QWidget(MedianFilter);
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


        retranslateUi(MedianFilter);

        QMetaObject::connectSlotsByName(MedianFilter);
    } // setupUi

    void retranslateUi(QWidget *MedianFilter)
    {
        MedianFilter->setWindowTitle(QApplication::translate("MedianFilter", "Form", 0, QApplication::UnicodeUTF8));
        iterationsLabel->setText(QApplication::translate("MedianFilter", "Radius", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("MedianFilter", "Target:", 0, QApplication::UnicodeUTF8));
        replaceCheckBox->setText(QApplication::translate("MedianFilter", "Replace", 0, QApplication::UnicodeUTF8));
        invertButton->setText(QApplication::translate("MedianFilter", "Invert", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MedianFilter: public Ui_MedianFilter {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MEDIANFILTER_H
