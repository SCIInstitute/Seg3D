#ifndef THRESHOLDTOOLINTERFACE_H
#define THRESHOLDTOOLINTERFACE_H

#include <QtGui/QWidget>

#include "sliderspincombo.h"

namespace Ui {
    class ThresholdToolInterface;
}

class ThresholdToolInterface : public QWidget {

    Q_OBJECT

    signals:
        void activeChanged(int);

        void createThresholdLayer();
        void clearSeeds();

        void upperThresholdChanged(int);
        void lowerThresholdChanged(int);

    public:
        ThresholdToolInterface(QWidget *parent = 0);
        ~ThresholdToolInterface();

    public slots:
        void setActive(int);
        void addToActive(QStringList&);

        void setLowerThreshold(double, double);
        void setLowerThresholdStep(double);

        void setUpperThreshold(double, double);
        void setUpperThresholdStep(double);

        void setHistogram();

    private:
        Ui::ThresholdToolInterface *ui;

        SliderSpinCombo *upperThresholdAdjuster;
        SliderSpinCombo *lowerThresholdAdjuster;

        void makeConnections();

    private slots:
        void senseActiveChanged(int);

        void senseCreateThresholdLayer();
        void senseClearSeeds();

        void senseUpperThresholdChanged(double);
        void senselowerThresholdChanged(double);

};

#endif // THRESHOLDTOOLINTERFACE_H
