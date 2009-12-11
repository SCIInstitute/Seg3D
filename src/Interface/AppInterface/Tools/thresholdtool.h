#ifndef THRESHOLDTOOL_H
#define THRESHOLDTOOL_H

#include <QtGui/QWidget>

#include "sliderspincombo.h"

namespace Ui {
    class ThresholdTool;
}

class ThresholdTool : public QWidget {

    Q_OBJECT

    signals:
        void activeChanged(int);

        void createThresholdLayer();
        void clearSeeds();

        void upperThresholdChanged(int);
        void lowerThresholdChanged(int);

    public:
        ThresholdTool(QWidget *parent = 0);
        ~ThresholdTool();

    public slots:
        void setActive(int);
        void addToActive(QStringList&);

        void setLowerThreshold(double, double);
        void setLowerThresholdStep(double);

        void setUpperThreshold(double, double);
        void setUpperThresholdStep(double);

        void setHistogram();

    private:
        Ui::ThresholdTool *ui;

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

#endif // THRESHOLDTOOL_H
