/********************************************************************************
** Form generated from reading UI file 'PaintToolInterface.ui'
**
** Created: Mon Jan 18 16:33:21 2010
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAINTTOOLINTERFACE_H
#define UI_PAINTTOOLINTERFACE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PaintToolInterface
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QHBoxLayout *maskHLayout;
    QLabel *maskLabel;
    QComboBox *maskComboBox;
    QHBoxLayout *eraseHLayout;
    QSpacerItem *eraseHSpacer_left;
    QCheckBox *eraseCheckBox;
    QSpacerItem *eraseHSpacer_right;
    QHBoxLayout *brushHLayout;
    QLabel *brushLabel;
    QSpacerItem *brushHSpacer;
    QVBoxLayout *verticalLayout;
    QGroupBox *thresholdGroupBox;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *upperHLayout_top;
    QLabel *upperLabel;
    QSpacerItem *upperHSpacer;
    QHBoxLayout *upperHLayout_bottom;
    QHBoxLayout *lowerHLayout_top;
    QLabel *lowerLabel;
    QSpacerItem *lowerHSpacer;
    QHBoxLayout *lowerHLayout_bottom;

    void setupUi(QWidget *PaintToolInterface)
    {
        if (PaintToolInterface->objectName().isEmpty())
            PaintToolInterface->setObjectName(QString::fromUtf8("PaintToolInterface"));
        PaintToolInterface->resize(650, 326);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(PaintToolInterface->sizePolicy().hasHeightForWidth());
        PaintToolInterface->setSizePolicy(sizePolicy);
        PaintToolInterface->setMinimumSize(QSize(0, 326));
        PaintToolInterface->setMaximumSize(QSize(16777215, 326));
        QFont font;
        font.setPointSize(12);
        PaintToolInterface->setFont(font);
        verticalLayout_2 = new QVBoxLayout(PaintToolInterface);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(4, 0, 4, 4);
        activeHLayout = new QHBoxLayout();
        activeHLayout->setSpacing(3);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeLabel = new QLabel(PaintToolInterface);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(activeLabel->sizePolicy().hasHeightForWidth());
        activeLabel->setSizePolicy(sizePolicy1);

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(PaintToolInterface);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(activeComboBox->sizePolicy().hasHeightForWidth());
        activeComboBox->setSizePolicy(sizePolicy2);
        activeComboBox->setLayoutDirection(Qt::LeftToRight);

        activeHLayout->addWidget(activeComboBox);

        activeHLayout->setStretch(0, 1);
        activeHLayout->setStretch(1, 2);

        verticalLayout_2->addLayout(activeHLayout);

        maskHLayout = new QHBoxLayout();
        maskHLayout->setSpacing(3);
        maskHLayout->setObjectName(QString::fromUtf8("maskHLayout"));
        maskLabel = new QLabel(PaintToolInterface);
        maskLabel->setObjectName(QString::fromUtf8("maskLabel"));
        sizePolicy1.setHeightForWidth(maskLabel->sizePolicy().hasHeightForWidth());
        maskLabel->setSizePolicy(sizePolicy1);

        maskHLayout->addWidget(maskLabel);

        maskComboBox = new QComboBox(PaintToolInterface);
        maskComboBox->setObjectName(QString::fromUtf8("maskComboBox"));
        sizePolicy2.setHeightForWidth(maskComboBox->sizePolicy().hasHeightForWidth());
        maskComboBox->setSizePolicy(sizePolicy2);

        maskHLayout->addWidget(maskComboBox);

        maskHLayout->setStretch(0, 1);
        maskHLayout->setStretch(1, 2);

        verticalLayout_2->addLayout(maskHLayout);

        eraseHLayout = new QHBoxLayout();
        eraseHLayout->setObjectName(QString::fromUtf8("eraseHLayout"));
        eraseHSpacer_left = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        eraseHLayout->addItem(eraseHSpacer_left);

        eraseCheckBox = new QCheckBox(PaintToolInterface);
        eraseCheckBox->setObjectName(QString::fromUtf8("eraseCheckBox"));

        eraseHLayout->addWidget(eraseCheckBox);

        eraseHSpacer_right = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        eraseHLayout->addItem(eraseHSpacer_right);


        verticalLayout_2->addLayout(eraseHLayout);

        brushHLayout = new QHBoxLayout();
        brushHLayout->setSpacing(0);
        brushHLayout->setObjectName(QString::fromUtf8("brushHLayout"));
        brushLabel = new QLabel(PaintToolInterface);
        brushLabel->setObjectName(QString::fromUtf8("brushLabel"));

        brushHLayout->addWidget(brushLabel);

        brushHSpacer = new QSpacerItem(118, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        brushHLayout->addItem(brushHSpacer);


        verticalLayout_2->addLayout(brushHLayout);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);

        verticalLayout_2->addLayout(verticalLayout);

        thresholdGroupBox = new QGroupBox(PaintToolInterface);
        thresholdGroupBox->setObjectName(QString::fromUtf8("thresholdGroupBox"));
        sizePolicy2.setHeightForWidth(thresholdGroupBox->sizePolicy().hasHeightForWidth());
        thresholdGroupBox->setSizePolicy(sizePolicy2);
        thresholdGroupBox->setMinimumSize(QSize(0, 171));
        thresholdGroupBox->setMaximumSize(QSize(16777215, 171));
        thresholdGroupBox->setFlat(false);
        verticalLayout_3 = new QVBoxLayout(thresholdGroupBox);
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        upperHLayout_top = new QHBoxLayout();
        upperHLayout_top->setObjectName(QString::fromUtf8("upperHLayout_top"));
        upperLabel = new QLabel(thresholdGroupBox);
        upperLabel->setObjectName(QString::fromUtf8("upperLabel"));

        upperHLayout_top->addWidget(upperLabel);

        upperHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        upperHLayout_top->addItem(upperHSpacer);


        verticalLayout_3->addLayout(upperHLayout_top);

        upperHLayout_bottom = new QHBoxLayout();
        upperHLayout_bottom->setObjectName(QString::fromUtf8("upperHLayout_bottom"));

        verticalLayout_3->addLayout(upperHLayout_bottom);

        lowerHLayout_top = new QHBoxLayout();
        lowerHLayout_top->setObjectName(QString::fromUtf8("lowerHLayout_top"));
        lowerLabel = new QLabel(thresholdGroupBox);
        lowerLabel->setObjectName(QString::fromUtf8("lowerLabel"));

        lowerHLayout_top->addWidget(lowerLabel);

        lowerHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        lowerHLayout_top->addItem(lowerHSpacer);


        verticalLayout_3->addLayout(lowerHLayout_top);

        lowerHLayout_bottom = new QHBoxLayout();
        lowerHLayout_bottom->setObjectName(QString::fromUtf8("lowerHLayout_bottom"));

        verticalLayout_3->addLayout(lowerHLayout_bottom);


        verticalLayout_2->addWidget(thresholdGroupBox);


        retranslateUi(PaintToolInterface);

        QMetaObject::connectSlotsByName(PaintToolInterface);
    } // setupUi

    void retranslateUi(QWidget *PaintToolInterface)
    {
        PaintToolInterface->setWindowTitle(QApplication::translate("PaintToolInterface", "Paint Tool", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("PaintToolInterface", "Target:", 0, QApplication::UnicodeUTF8));
        maskLabel->setText(QApplication::translate("PaintToolInterface", "Mask:", 0, QApplication::UnicodeUTF8));
        eraseCheckBox->setText(QApplication::translate("PaintToolInterface", "Erase Mode", 0, QApplication::UnicodeUTF8));
        brushLabel->setText(QApplication::translate("PaintToolInterface", "Brush Size:", 0, QApplication::UnicodeUTF8));
        thresholdGroupBox->setTitle(QApplication::translate("PaintToolInterface", "Threshold", 0, QApplication::UnicodeUTF8));
        upperLabel->setText(QApplication::translate("PaintToolInterface", "Upper:", 0, QApplication::UnicodeUTF8));
        lowerLabel->setText(QApplication::translate("PaintToolInterface", "Lower:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class PaintToolInterface: public Ui_PaintToolInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAINTTOOLINTERFACE_H
