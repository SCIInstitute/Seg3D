/********************************************************************************
** Form generated from reading UI file 'polylinetool.ui'
**
** Created: Thu Dec 17 11:25:49 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_POLYLINETOOL_H
#define UI_POLYLINETOOL_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PolyLineTool
{
public:
    QWidget *layoutWidget_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QPushButton *resetPLButton;
    QGroupBox *insidePLGroupBox;
    QWidget *layoutWidget;
    QVBoxLayout *insideVLayout;
    QPushButton *insideFillButton;
    QPushButton *insideEraseButton;

    void setupUi(QWidget *PolyLineTool)
    {
        if (PolyLineTool->objectName().isEmpty())
            PolyLineTool->setObjectName(QString::fromUtf8("PolyLineTool"));
        PolyLineTool->resize(200, 163);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(200);
        sizePolicy.setVerticalStretch(163);
        sizePolicy.setHeightForWidth(PolyLineTool->sizePolicy().hasHeightForWidth());
        PolyLineTool->setSizePolicy(sizePolicy);
        PolyLineTool->setMinimumSize(QSize(200, 163));
        PolyLineTool->setMaximumSize(QSize(200, 163));
        PolyLineTool->setBaseSize(QSize(200, 163));
        layoutWidget_2 = new QWidget(PolyLineTool);
        layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(6, 6, 189, 26));
        activeHLayout = new QHBoxLayout(layoutWidget_2);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeHLayout->setContentsMargins(0, 0, 0, 0);
        activeLabel = new QLabel(layoutWidget_2);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(layoutWidget_2);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));

        activeHLayout->addWidget(activeComboBox);

        resetPLButton = new QPushButton(PolyLineTool);
        resetPLButton->setObjectName(QString::fromUtf8("resetPLButton"));
        resetPLButton->setGeometry(QRect(6, 36, 189, 32));
        insidePLGroupBox = new QGroupBox(PolyLineTool);
        insidePLGroupBox->setObjectName(QString::fromUtf8("insidePLGroupBox"));
        insidePLGroupBox->setGeometry(QRect(6, 66, 189, 91));
        layoutWidget = new QWidget(insidePLGroupBox);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(0, 22, 189, 71));
        insideVLayout = new QVBoxLayout(layoutWidget);
        insideVLayout->setObjectName(QString::fromUtf8("insideVLayout"));
        insideVLayout->setContentsMargins(0, 0, 0, 0);
        insideFillButton = new QPushButton(layoutWidget);
        insideFillButton->setObjectName(QString::fromUtf8("insideFillButton"));

        insideVLayout->addWidget(insideFillButton);

        insideEraseButton = new QPushButton(layoutWidget);
        insideEraseButton->setObjectName(QString::fromUtf8("insideEraseButton"));

        insideVLayout->addWidget(insideEraseButton);


        retranslateUi(PolyLineTool);

        QMetaObject::connectSlotsByName(PolyLineTool);
    } // setupUi

    void retranslateUi(QWidget *PolyLineTool)
    {
        PolyLineTool->setWindowTitle(QApplication::translate("PolyLineTool", "Form", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("PolyLineTool", "Active:", 0, QApplication::UnicodeUTF8));
        resetPLButton->setText(QApplication::translate("PolyLineTool", "Reset Polyline Tool", 0, QApplication::UnicodeUTF8));
        insidePLGroupBox->setTitle(QApplication::translate("PolyLineTool", "Inside Polyline", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        insideFillButton->setToolTip(QApplication::translate("PolyLineTool", "Flip across the axial plain", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        insideFillButton->setText(QApplication::translate("PolyLineTool", "Fill", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        insideEraseButton->setToolTip(QApplication::translate("PolyLineTool", "Flip across the coronal plain", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        insideEraseButton->setText(QApplication::translate("PolyLineTool", "Erase", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class PolyLineTool: public Ui_PolyLineTool {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_POLYLINETOOL_H
