/********************************************************************************
** Form generated from reading UI file 'MedianFilterInterface.ui'
**
** Created: Mon Jan 18 16:33:21 2010
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
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MedianFilterInterface
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QHBoxLayout *radiusHLayout_top;
    QLabel *radiusLabel;
    QSpacerItem *radiusHSpacer;
    QHBoxLayout *radiusHLayout_bottom;
    QHBoxLayout *replaceRunLayout;
    QCheckBox *replaceCheckBox;
    QPushButton *runButton;

    void setupUi(QWidget *MedianFilterInterface)
    {
        if (MedianFilterInterface->objectName().isEmpty())
            MedianFilterInterface->setObjectName(QString::fromUtf8("MedianFilterInterface"));
        MedianFilterInterface->resize(650, 136);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MedianFilterInterface->sizePolicy().hasHeightForWidth());
        MedianFilterInterface->setSizePolicy(sizePolicy);
        MedianFilterInterface->setMinimumSize(QSize(213, 136));
        MedianFilterInterface->setMaximumSize(QSize(16777215, 136));
        MedianFilterInterface->setBaseSize(QSize(650, 136));
        verticalLayout = new QVBoxLayout(MedianFilterInterface);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(4, 0, 4, 0);
        activeHLayout = new QHBoxLayout();
        activeHLayout->setSpacing(0);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeLabel = new QLabel(MedianFilterInterface);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(MedianFilterInterface);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));

        activeHLayout->addWidget(activeComboBox);

        activeHLayout->setStretch(0, 1);
        activeHLayout->setStretch(1, 2);

        verticalLayout->addLayout(activeHLayout);

        radiusHLayout_top = new QHBoxLayout();
        radiusHLayout_top->setObjectName(QString::fromUtf8("radiusHLayout_top"));
        radiusLabel = new QLabel(MedianFilterInterface);
        radiusLabel->setObjectName(QString::fromUtf8("radiusLabel"));

        radiusHLayout_top->addWidget(radiusLabel);

        radiusHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        radiusHLayout_top->addItem(radiusHSpacer);


        verticalLayout->addLayout(radiusHLayout_top);

        radiusHLayout_bottom = new QHBoxLayout();
        radiusHLayout_bottom->setObjectName(QString::fromUtf8("radiusHLayout_bottom"));

        verticalLayout->addLayout(radiusHLayout_bottom);

        replaceRunLayout = new QHBoxLayout();
        replaceRunLayout->setObjectName(QString::fromUtf8("replaceRunLayout"));
        replaceCheckBox = new QCheckBox(MedianFilterInterface);
        replaceCheckBox->setObjectName(QString::fromUtf8("replaceCheckBox"));
        replaceCheckBox->setChecked(true);

        replaceRunLayout->addWidget(replaceCheckBox);

        runButton = new QPushButton(MedianFilterInterface);
        runButton->setObjectName(QString::fromUtf8("runButton"));

        replaceRunLayout->addWidget(runButton);


        verticalLayout->addLayout(replaceRunLayout);


        retranslateUi(MedianFilterInterface);

        QMetaObject::connectSlotsByName(MedianFilterInterface);
    } // setupUi

    void retranslateUi(QWidget *MedianFilterInterface)
    {
        MedianFilterInterface->setWindowTitle(QApplication::translate("MedianFilterInterface", "Median Filter", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("MedianFilterInterface", "Target:", 0, QApplication::UnicodeUTF8));
        radiusLabel->setText(QApplication::translate("MedianFilterInterface", "Radius", 0, QApplication::UnicodeUTF8));
        replaceCheckBox->setText(QApplication::translate("MedianFilterInterface", "Replace", 0, QApplication::UnicodeUTF8));
        runButton->setText(QApplication::translate("MedianFilterInterface", "Run Filter", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MedianFilterInterface: public Ui_MedianFilterInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MEDIANFILTERINTERFACE_H
