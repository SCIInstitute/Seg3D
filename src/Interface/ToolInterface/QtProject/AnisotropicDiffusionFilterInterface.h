#ifndef ANISOTROPICDIFFUSIONFILTERINTERFACE_H
#define ANISOTROPICDIFFUSIONFILTERINTERFACE_H

#include <QtGui/QWidget>

#include "sliderspincombo.h"

namespace Ui {
    class AnisotropicDiffusionFilterInterface;
}

class AnisotropicDiffusionFilterInterface : public QWidget {

    Q_OBJECT


    public:
        AnisotropicDiffusionFilterInterface(QWidget *parent = 0);
        ~AnisotropicDiffusionFilterInterface();
        QWidget *widgeter;


    private:
        Ui::AnisotropicDiffusionFilterInterface *ui;

        SliderSpinCombo *iterationsAdjuster;
        SliderSpinCombo *stepAdjuster;
        SliderSpinCombo *conductanceAdjuster;




};

#endif // ANISOTROPICDIFFUSIONFILTERINTERFACE_H
