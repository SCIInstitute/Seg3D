/********************************************************************************
** Form generated from reading UI file 'PolyLineToolInterface.ui'
**
** Created: Wed Dec 30 08:36:24 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_POLYLINETOOLINTERFACE_H
#define UI_POLYLINETOOLINTERFACE_H

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

class Ui_PolyLineToolInterface
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

    void setupUi(QWidget *PolyLineToolInterface)
    {
        if (PolyLineToolInterface->objectName().isEmpty())
            PolyLineToolInterface->setObjectName(QString::fromUtf8("PolyLineToolInterface"));
        PolyLineToolInterface->resize(213, 158);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(PolyLineToolInterface->sizePolicy().hasHeightForWidth());
        PolyLineToolInterface->setSizePolicy(sizePolicy);
        PolyLineToolInterface->setMinimumSize(QSize(213, 158));
        PolyLineToolInterface->setMaximumSize(QSize(213, 158));
        PolyLineToolInterface->setBaseSize(QSize(213, 158));
        layoutWidget_2 = new QWidget(PolyLineToolInterface);
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
        resetPLButton = new QPushButton(PolyLineToolInterface);
        resetPLButton->setObjectName(QString::fromUtf8("resetPLButton"));
        resetPLButton->setGeometry(QRect(6, 36, 201, 32));
        insidePLGroupBox = new QGroupBox(PolyLineToolInterface);
        insidePLGroupBox->setObjectName(QString::fromUtf8("insidePLGroupBox"));
        insidePLGroupBox->setGeometry(QRect(5, 62, 203, 91));
        layoutWidget = new QWidget(insidePLGroupBox);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(0, 22, 203, 69));
        insideVLayout = new QVBoxLayout(layoutWidget);
        insideVLayout->setSpacing(3);
        insideVLayout->setObjectName(QString::fromUtf8("insideVLayout"));
        insideVLayout->setContentsMargins(0, 0, 0, 0);
        insideFillButton = new QPushButton(layoutWidget);
        insideFillButton->setObjectName(QString::fromUtf8("insideFillButton"));

        insideVLayout->addWidget(insideFillButton);

        insideEraseButton = new QPushButton(layoutWidget);
        insideEraseButton->setObjectName(QString::fromUtf8("insideEraseButton"));

        insideVLayout->addWidget(insideEraseButton);


        retranslateUi(PolyLineToolInterface);

        QMetaObject::connectSlotsByName(PolyLineToolInterface);
    } // setupUi

    void retranslateUi(QWidget *PolyLineToolInterface)
    {
        PolyLineToolInterface->setWindowTitle(QApplication::translate("PolyLineToolInterface", "Form", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("PolyLineToolInterface", "Target:", 0, QApplication::UnicodeUTF8));
        resetPLButton->setText(QApplication::translate("PolyLineToolInterface", "Reset Polyline Tool", 0, QApplication::UnicodeUTF8));
        insidePLGroupBox->setTitle(QApplication::translate("PolyLineToolInterface", "Inside Polyline", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        insideFillButton->setToolTip(QApplication::translate("PolyLineToolInterface", "Flip across the axial plain", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        insideFillButton->setText(QApplication::translate("PolyLineToolInterface", "Fill", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        insideEraseButton->setToolTip(QApplication::translate("PolyLineToolInterface", "Flip across the coronal plain", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        insideEraseButton->setText(QApplication::translate("PolyLineToolInterface", "Erase", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class PolyLineToolInterface: public Ui_PolyLineToolInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_POLYLINETOOLINTERFACE_H
