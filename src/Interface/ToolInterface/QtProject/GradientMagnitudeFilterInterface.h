#ifndef GRADIENTMAGNITUDEFILTERINTERFACE_H
#define GRADIENTMAGNITUDEFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class GradientMagnitudeFilterInterface;
}

class GradientMagnitudeFilterInterface : public QWidget {
    Q_OBJECT
public:
    GradientMagnitudeFilterInterface(QWidget *parent = 0);
    ~GradientMagnitudeFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::GradientMagnitudeFilterInterface *ui;
};

#endif // GRADIENTMAGNITUDEFILTERINTERFACE_H
