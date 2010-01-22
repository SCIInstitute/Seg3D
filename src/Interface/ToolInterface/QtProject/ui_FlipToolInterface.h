/********************************************************************************
** Form generated from reading UI file 'FlipToolInterface.ui'
**
** Created: Mon Jan 18 16:33:21 2010
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
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *activeHLayout;
    QLabel *activeLabel;
    QComboBox *activeComboBox;
    QGroupBox *flipGroupBox;
    QVBoxLayout *verticalLayout;
    QVBoxLayout *flipVLayout;
    QPushButton *flipAxialButton;
    QPushButton *flipCoronalButton;
    QPushButton *flipSagittalButton;
    QGroupBox *rotateGroupBox;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *rotateVLayout;
    QPushButton *rotateACButton;
    QPushButton *rotateASButton;
    QPushButton *rotateSAButton;

    void setupUi(QWidget *FlipToolInterface)
    {
        if (FlipToolInterface->objectName().isEmpty())
            FlipToolInterface->setObjectName(QString::fromUtf8("FlipToolInterface"));
        FlipToolInterface->resize(650, 297);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(FlipToolInterface->sizePolicy().hasHeightForWidth());
        FlipToolInterface->setSizePolicy(sizePolicy);
        FlipToolInterface->setMinimumSize(QSize(0, 297));
        FlipToolInterface->setMaximumSize(QSize(16777215, 297));
        FlipToolInterface->setSizeIncrement(QSize(0, 0));
        verticalLayout_3 = new QVBoxLayout(FlipToolInterface);
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(4, 0, 4, 4);
        activeHLayout = new QHBoxLayout();
        activeHLayout->setSpacing(0);
        activeHLayout->setObjectName(QString::fromUtf8("activeHLayout"));
        activeLabel = new QLabel(FlipToolInterface);
        activeLabel->setObjectName(QString::fromUtf8("activeLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(activeLabel->sizePolicy().hasHeightForWidth());
        activeLabel->setSizePolicy(sizePolicy1);

        activeHLayout->addWidget(activeLabel);

        activeComboBox = new QComboBox(FlipToolInterface);
        activeComboBox->setObjectName(QString::fromUtf8("activeComboBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(activeComboBox->sizePolicy().hasHeightForWidth());
        activeComboBox->setSizePolicy(sizePolicy2);

        activeHLayout->addWidget(activeComboBox);

        activeHLayout->setStretch(0, 1);
        activeHLayout->setStretch(1, 2);

        verticalLayout_3->addLayout(activeHLayout);

        flipGroupBox = new QGroupBox(FlipToolInterface);
        flipGroupBox->setObjectName(QString::fromUtf8("flipGroupBox"));
        sizePolicy2.setHeightForWidth(flipGroupBox->sizePolicy().hasHeightForWidth());
        flipGroupBox->setSizePolicy(sizePolicy2);
        flipGroupBox->setMinimumSize(QSize(0, 134));
        flipGroupBox->setMaximumSize(QSize(16777215, 134));
        verticalLayout = new QVBoxLayout(flipGroupBox);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        flipVLayout = new QVBoxLayout();
        flipVLayout->setSpacing(6);
        flipVLayout->setObjectName(QString::fromUtf8("flipVLayout"));
        flipAxialButton = new QPushButton(flipGroupBox);
        flipAxialButton->setObjectName(QString::fromUtf8("flipAxialButton"));

        flipVLayout->addWidget(flipAxialButton);

        flipCoronalButton = new QPushButton(flipGroupBox);
        flipCoronalButton->setObjectName(QString::fromUtf8("flipCoronalButton"));

        flipVLayout->addWidget(flipCoronalButton);

        flipSagittalButton = new QPushButton(flipGroupBox);
        flipSagittalButton->setObjectName(QString::fromUtf8("flipSagittalButton"));

        flipVLayout->addWidget(flipSagittalButton);


        verticalLayout->addLayout(flipVLayout);


        verticalLayout_3->addWidget(flipGroupBox);

        rotateGroupBox = new QGroupBox(FlipToolInterface);
        rotateGroupBox->setObjectName(QString::fromUtf8("rotateGroupBox"));
        sizePolicy2.setHeightForWidth(rotateGroupBox->sizePolicy().hasHeightForWidth());
        rotateGroupBox->setSizePolicy(sizePolicy2);
        rotateGroupBox->setMinimumSize(QSize(0, 134));
        rotateGroupBox->setMaximumSize(QSize(16777215, 134));
        verticalLayout_2 = new QVBoxLayout(rotateGroupBox);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        rotateVLayout = new QVBoxLayout();
        rotateVLayout->setSpacing(6);
        rotateVLayout->setObjectName(QString::fromUtf8("rotateVLayout"));
        rotateACButton = new QPushButton(rotateGroupBox);
        rotateACButton->setObjectName(QString::fromUtf8("rotateACButton"));

        rotateVLayout->addWidget(rotateACButton);

        rotateASButton = new QPushButton(rotateGroupBox);
        rotateASButton->setObjectName(QString::fromUtf8("rotateASButton"));

        rotateVLayout->addWidget(rotateASButton);

        rotateSAButton = new QPushButton(rotateGroupBox);
        rotateSAButton->setObjectName(QString::fromUtf8("rotateSAButton"));

        rotateVLayout->addWidget(rotateSAButton);


        verticalLayout_2->addLayout(rotateVLayout);


        verticalLayout_3->addWidget(rotateGroupBox);


        retranslateUi(FlipToolInterface);

        QMetaObject::connectSlotsByName(FlipToolInterface);
    } // setupUi

    void retranslateUi(QWidget *FlipToolInterface)
    {
        FlipToolInterface->setWindowTitle(QApplication::translate("FlipToolInterface", "Flip Tool", 0, QApplication::UnicodeUTF8));
        activeLabel->setText(QApplication::translate("FlipToolInterface", "Target:", 0, QApplication::UnicodeUTF8));
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
