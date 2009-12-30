#ifndef ANISOTROPICDIFFUSIONFILTERINTERFACE_H
#define ANISOTROPICDIFFUSIONFILTERINTERFACE_H

#include <QtGui/QWidget>

#include "sliderspincombo.h"

namespace Ui {
    class AnisotropicDiffusionFilterInterface;
}

class AnisotropicDiffusionFilterInterface : public QWidget {

    Q_OBJECT

    signals:
        void activeChanged(int);

        void iterationsChanged(int);
        void stepChanged(double);
        void conductanceChanged(int);

        void filterRun(bool);

    public:
        AnisotropicDiffusionFilterInterface(QWidget *parent = 0);
        ~AnisotropicDiffusionFilterInterface();

    public slots:
        void setActive(int);
        void addToActive(QStringList&);

        void setIterations(int);
        void setIterationRange(int, int);

        void setStep(double);
        void setStepRange(double, double);

        void setConductance(int);
        void setConductanceRange(int, int);

    private:
        Ui::AnisotropicDiffusionFilterInterface *ui;

        SliderSpinCombo *iterationsAdjuster;
        SliderSpinCombo *stepAdjuster;
        SliderSpinCombo *conductanceAdjuster;

        void makeConnections();

    private slots:
        void senseActiveChanged(int);

        void senseIterationsChanged(int);
        void senseStepChanged(double);
        void senseConductanceChanged(int);

        void senseFilterRun(bool);



};

#endif // ANISOTROPICDIFFUSIONFILTERINTERFACE_H
