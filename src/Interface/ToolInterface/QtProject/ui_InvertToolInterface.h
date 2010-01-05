/********************************************************************************
** Form generated from reading UI file 'InvertToolInterface.ui'
**
** Created: Tue Jan 5 13:59:09 2010
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INVERTTOOLINTERFACE_H
#define UI_INVERTTOOLINTERFACE_H

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
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InvertToolInterface
{
public:
    QWidget *layoutWidget_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QWidget *layoutWidget;
    QHBoxLayout *replaceInvertLayout;
    QCheckBox *replaceCheckBox;
    QPushButton *invertButton;

    void setupUi(QWidget *InvertToolInterface)
    {
        if (InvertToolInterface->objectName().isEmpty())
            InvertToolInterface->setObjectName(QString::fromUtf8("InvertToolInterface"));
        InvertToolInterface->resize(213, 65);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(InvertToolInterface->sizePolicy().hasHeightForWidth());
        InvertToolInterface->setSizePolicy(sizePolicy);
        InvertToolInterface->setMinimumSize(QSize(213, 65));
        InvertToolInterface->setMaximumSize(QSize(213, 65));
        InvertToolInterface->setBaseSize(QSize(213, 65));
        layoutWidget_2 = new QWidget(InvertToolInterface);
        layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(4, 4, 207, 26));
        activeHLayout = new QHBoxLayout(layoutWidget_2);
        activeHLayout->setSpacing(0);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeHLayout->setContentsMargins(0, 0, 0, 0);
        activeLabel = new QLabel(layoutWidget_2);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(activeLabel->sizePolicy().hasHeightForWidth());
        activeLabel->setSizePolicy(sizePolicy1);

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(layoutWidget_2);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));

        activeHLayout->addWidget(activeComboBox);

        activeHLayout->setStretch(0, 1);
        activeHLayout->setStretch(1, 2);
        layoutWidget = new QWidget(InvertToolInterface);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(4, 29, 207, 32));
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


        retranslateUi(InvertToolInterface);

        QMetaObject::connectSlotsByName(InvertToolInterface);
    } // setupUi

    void retranslateUi(QWidget *InvertToolInterface)
    {
        InvertToolInterface->setWindowTitle(QApplication::translate("InvertToolInterface", "Invert Tool", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("InvertToolInterface", "Target:", 0, QApplication::UnicodeUTF8));
        replaceCheckBox->setText(QApplication::translate("InvertToolInterface", "Replace", 0, QApplication::UnicodeUTF8));
        invertButton->setText(QApplication::translate("InvertToolInterface", "Invert", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class InvertToolInterface: public Ui_InvertToolInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INVERTTOOLINTERFACE_H
