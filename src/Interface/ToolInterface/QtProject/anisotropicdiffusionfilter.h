#ifndef ANISOTROPICDIFFUSIONFILTER_H
#define ANISOTROPICDIFFUSIONFILTER_H

#include <QtGui/QWidget>

#include "sliderspincombo.h"

namespace Ui {
    class AnisotropicDiffusionFilter;
}

class AnisotropicDiffusionFilter : public QWidget {

    Q_OBJECT

    signals:
        void activeChanged(int);

        void iterationsChanged(int);
        void stepChanged(double);
        void conductanceChanged(int);

        void filterRun(bool);

    public:
        AnisotropicDiffusionFilter(QWidget *parent = 0);
        ~AnisotropicDiffusionFilter();

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
        Ui::AnisotropicDiffusionFilter *ui;

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

#endif // ANISOTROPICDIFFUSIONFILTER_H
