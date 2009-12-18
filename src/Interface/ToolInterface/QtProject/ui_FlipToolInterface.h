/********************************************************************************
** Form generated from reading UI file 'FlipToolInterface.ui'
**
** Created: Fri Dec 18 15:00:15 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FLIPTOOLINTERFACE_H
#define UI_FLIPTOOLINTERFACE_H

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

class Ui_FlipToolInterface
{
public:
    QWidget *layoutWidget_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QGroupBox *flipGroupBox;
    QWidget *widget;
    QVBoxLayout *flipVLayout;
    QPushButton *flipAxialButton;
    QPushButton *flipCoronalButton;
    QPushButton *flipSagittalButton;
    QGroupBox *rotateGroupBox;
    QWidget *layoutWidget;
    QVBoxLayout *rotateVLayout;
    QPushButton *rotateACButton;
    QPushButton *rotateASButton;
    QPushButton *rotateSAButton;

    void setupUi(QWidget *FlipToolInterface)
    {
        if (FlipToolInterface->objectName().isEmpty())
            FlipToolInterface->setObjectName(QString::fromUtf8("FlipToolInterface"));
        FlipToolInterface->resize(231, 301);
        layoutWidget_2 = new QWidget(FlipToolInterface);
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

        flipGroupBox = new QGroupBox(FlipToolInterface);
        flipGroupBox->setObjectName(QString::fromUtf8("flipGroupBox"));
        flipGroupBox->setGeometry(QRect(10, 31, 211, 131));
        widget = new QWidget(flipGroupBox);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(0, 22, 211, 111));
        flipVLayout = new QVBoxLayout(widget);
        flipVLayout->setObjectName(QString::fromUtf8("flipVLayout"));
        flipVLayout->setContentsMargins(0, 0, 0, 0);
        flipAxialButton = new QPushButton(widget);
        flipAxialButton->setObjectName(QString::fromUtf8("flipAxialButton"));

        flipVLayout->addWidget(flipAxialButton);

        flipCoronalButton = new QPushButton(widget);
        flipCoronalButton->setObjectName(QString::fromUtf8("flipCoronalButton"));

        flipVLayout->addWidget(flipCoronalButton);

        flipSagittalButton = new QPushButton(widget);
        flipSagittalButton->setObjectName(QString::fromUtf8("flipSagittalButton"));

        flipVLayout->addWidget(flipSagittalButton);

        rotateGroupBox = new QGroupBox(FlipToolInterface);
        rotateGroupBox->setObjectName(QString::fromUtf8("rotateGroupBox"));
        rotateGroupBox->setGeometry(QRect(10, 160, 211, 131));
        layoutWidget = new QWidget(rotateGroupBox);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(0, 22, 211, 111));
        rotateVLayout = new QVBoxLayout(layoutWidget);
        rotateVLayout->setObjectName(QString::fromUtf8("rotateVLayout"));
        rotateVLayout->setContentsMargins(0, 0, 0, 0);
        rotateACButton = new QPushButton(layoutWidget);
        rotateACButton->setObjectName(QString::fromUtf8("rotateACButton"));

        rotateVLayout->addWidget(rotateACButton);

        rotateASButton = new QPushButton(layoutWidget);
        rotateASButton->setObjectName(QString::fromUtf8("rotateASButton"));

        rotateVLayout->addWidget(rotateASButton);

        rotateSAButton = new QPushButton(layoutWidget);
        rotateSAButton->setObjectName(QString::fromUtf8("rotateSAButton"));

        rotateVLayout->addWidget(rotateSAButton);


        retranslateUi(FlipToolInterface);

        QMetaObject::connectSlotsByName(FlipToolInterface);
    } // setupUi

    void retranslateUi(QWidget *FlipToolInterface)
    {
        FlipToolInterface->setWindowTitle(QApplication::translate("FlipToolInterface", "Form", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("FlipToolInterface", "Active:", 0, QApplication::UnicodeUTF8));
        flipGroupBox->setTitle(QApplication::translate("FlipToolInterface", "Flip", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        flipAxialButton->setToolTip(QApplication::translate("FlipToolInterface", "Flip across the axial plain", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        flipAxialButton->setText(QApplication::translate("FlipToolInterface", "Axial", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        flipCoronalButton->setToolTip(QApplication::translate("FlipToolInterface", "Flip across the coronal plain", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        flipCoronalButton->setText(QApplication::translate("FlipToolInterface", "Coronal", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        flipSagittalButton->setToolTip(QApplication::translate("FlipToolInterface", "Flip across the sagittal plain", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        flipSagittalButton->setText(QApplication::translate("FlipToolInterface", "Sagittal", 0, QApplication::UnicodeUTF8));
        rotateGroupBox->setTitle(QApplication::translate("FlipToolInterface", "Rotate", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        rotateACButton->setToolTip(QApplication::translate("FlipToolInterface", "Rotate axial to coronal", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        rotateACButton->setText(QApplication::translate("FlipToolInterface", "Axial - Coronal", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        rotateASButton->setToolTip(QApplication::translate("FlipToolInterface", "Rotate axial to sagittal", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        rotateASButton->setText(QApplication::translate("FlipToolInterface", "Axial - Sagittal", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        rotateSAButton->setToolTip(QApplication::translate("FlipToolInterface", "Rotate sagittal to axial", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        rotateSAButton->setText(QApplication::translate("FlipToolInterface", "Sagittal - Axial", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class FlipToolInterface: public Ui_FlipToolInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FLIPTOOLINTERFACE_H
