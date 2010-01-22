/********************************************************************************
** Form generated from reading UI file 'PolyLineToolInterface.ui'
**
** Created: Mon Jan 18 16:33:21 2010
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
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QPushButton *resetPLButton;
    QGroupBox *insidePLGroupBox;
    QVBoxLayout *verticalLayout;
    QVBoxLayout *insideVLayout;
    QPushButton *insideFillButton;
    QPushButton *insideEraseButton;

    void setupUi(QWidget *PolyLineToolInterface)
    {
        if (PolyLineToolInterface->objectName().isEmpty())
            PolyLineToolInterface->setObjectName(QString::fromUtf8("PolyLineToolInterface"));
        PolyLineToolInterface->resize(650, 158);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(PolyLineToolInterface->sizePolicy().hasHeightForWidth());
        PolyLineToolInterface->setSizePolicy(sizePolicy);
        PolyLineToolInterface->setMinimumSize(QSize(0, 158));
        PolyLineToolInterface->setMaximumSize(QSize(16777215, 158));
        verticalLayout_2 = new QVBoxLayout(PolyLineToolInterface);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        activeHLayout = new QHBoxLayout();
        activeHLayout->setSpacing(0);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeLabel = new QLabel(PolyLineToolInterface);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(PolyLineToolInterface);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));

        activeHLayout->addWidget(activeComboBox);

        activeHLayout->setStretch(0, 1);
        activeHLayout->setStretch(1, 2);

        verticalLayout_2->addLayout(activeHLayout);

        resetPLButton = new QPushButton(PolyLineToolInterface);
        resetPLButton->setObjectName(QString::fromUtf8("resetPLButton"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(resetPLButton->sizePolicy().hasHeightForWidth());
        resetPLButton->setSizePolicy(sizePolicy1);

        verticalLayout_2->addWidget(resetPLButton);

        insidePLGroupBox = new QGroupBox(PolyLineToolInterface);
        insidePLGroupBox->setObjectName(QString::fromUtf8("insidePLGroupBox"));
        sizePolicy1.setHeightForWidth(insidePLGroupBox->sizePolicy().hasHeightForWidth());
        insidePLGroupBox->setSizePolicy(sizePolicy1);
        insidePLGroupBox->setMinimumSize(QSize(0, 91));
        insidePLGroupBox->setMaximumSize(QSize(16777215, 91));
        verticalLayout = new QVBoxLayout(insidePLGroupBox);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        insideVLayout = new QVBoxLayout();
        insideVLayout->setSpacing(3);
        insideVLayout->setObjectName(QString::fromUtf8("insideVLayout"));
        insideFillButton = new QPushButton(insidePLGroupBox);
        insideFillButton->setObjectName(QString::fromUtf8("insideFillButton"));

        insideVLayout->addWidget(insideFillButton);

        insideEraseButton = new QPushButton(insidePLGroupBox);
        insideEraseButton->setObjectName(QString::fromUtf8("insideEraseButton"));

        insideVLayout->addWidget(insideEraseButton);


        verticalLayout->addLayout(insideVLayout);


        verticalLayout_2->addWidget(insidePLGroupBox);


        retranslateUi(PolyLineToolInterface);

        QMetaObject::connectSlotsByName(PolyLineToolInterface);
    } // setupUi

    void retranslateUi(QWidget *PolyLineToolInterface)
    {
        PolyLineToolInterface->setWindowTitle(QApplication::translate("PolyLineToolInterface", "Polyline Tool", 0, QApplication::UnicodeUTF8));
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
