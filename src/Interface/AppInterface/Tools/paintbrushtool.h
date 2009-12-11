#ifndef PAINTBRUSHTOOL_H
#define PAINTBRUSHTOOL_H

#include <QtGui/QWidget>

#include "sliderspincombo.h"

namespace Ui {
    class PaintBrushTool;
}

class PaintBrushTool : public QWidget {

    Q_OBJECT

    signals:
        void activeChanged(int);
        void maskChanged(int);

        void eraseModeChanged(bool);

        void paintBrushSizeChanged(int);
        void upperThresholdChanged(int);
        void lowerThresholdChanged(int);

    public:
        PaintBrushTool(QWidget *parent = 0);
        ~PaintBrushTool();

    public slots:
        void setActive(int);
        void addToActive(QStringList&);
        void setMask(int);
        void addToMask(QStringList&);
        void setPaintBrushSize(int);
        void setLowerThreshold(double, double);
        void setLowerThresholdStep(double);

        void setUpperThreshold(double, double);
        void setUpperThresholdStep(double);


    private:
        Ui::PaintBrushTool *ui;

        SliderSpinCombo *paintBrushAdjuster;
        SliderSpinCombo *upperThresholdAdjuster;
        SliderSpinCombo *lowerThresholdAdjuster;

        void makeConnections();

    private slots:
        void senseActiveChanged(int);
        void senseMaskChanged(int);

        void senseEraseModeChanged(bool);

        void sensePaintBrushSizeChanged(double);
        void senseUpperThresholdChanged(double);
        void senselowerThresholdChanged(double);

};

#endif // PAINTBRUSHTOOL_H
