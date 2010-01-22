/********************************************************************************
** Form generated from reading UI file 'InvertToolInterface.ui'
**
** Created: Mon Jan 18 16:33:21 2010
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
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InvertToolInterface
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QHBoxLayout *replaceInvertLayout;
    QCheckBox *replaceCheckBox;
    QPushButton *invertButton;

    void setupUi(QWidget *InvertToolInterface)
    {
        if (InvertToolInterface->objectName().isEmpty())
            InvertToolInterface->setObjectName(QString::fromUtf8("InvertToolInterface"));
        InvertToolInterface->resize(650, 65);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(InvertToolInterface->sizePolicy().hasHeightForWidth());
        InvertToolInterface->setSizePolicy(sizePolicy);
        InvertToolInterface->setMinimumSize(QSize(0, 65));
        InvertToolInterface->setMaximumSize(QSize(16777215, 65));
        verticalLayout = new QVBoxLayout(InvertToolInterface);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(4, 0, 4, 0);
        activeHLayout = new QHBoxLayout();
        activeHLayout->setSpacing(0);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeLabel = new QLabel(InvertToolInterface);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(activeLabel->sizePolicy().hasHeightForWidth());
        activeLabel->setSizePolicy(sizePolicy1);

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(InvertToolInterface);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));

        activeHLayout->addWidget(activeComboBox);

        activeHLayout->setStretch(0, 1);
        activeHLayout->setStretch(1, 2);

        verticalLayout->addLayout(activeHLayout);

        replaceInvertLayout = new QHBoxLayout();
        replaceInvertLayout->setObjectName(QString::fromUtf8("replaceInvertLayout"));
        replaceCheckBox = new QCheckBox(InvertToolInterface);
        replaceCheckBox->setObjectName(QString::fromUtf8("replaceCheckBox"));
        replaceCheckBox->setChecked(true);

        replaceInvertLayout->addWidget(replaceCheckBox);

        invertButton = new QPushButton(InvertToolInterface);
        invertButton->setObjectName(QString::fromUtf8("invertButton"));

        replaceInvertLayout->addWidget(invertButton);


        verticalLayout->addLayout(replaceInvertLayout);


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
