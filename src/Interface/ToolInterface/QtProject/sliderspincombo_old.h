#ifndef SLIDERSPINCOMBO_H
#define SLIDERSPINCOMBO_H

#include <QWidget>
#include <QtGui/QSlider>
#include <QtGui/QSpinBox>

class SliderSpinCombo : public QWidget
{
    Q_OBJECT
    public:
        SliderSpinCombo(QWidget *parent = 0);
        ~SliderSpinCombo() {}

    signals:
        void valueAdjusted(int);

    public slots:
        void setRanges(int, int);
        void setCurrentValue(int);

    private:
        QSlider *slider;
        QSpinBox *spinner;

        void createConnections();
};

#endif // SLIDERSPINCOMBO_H
