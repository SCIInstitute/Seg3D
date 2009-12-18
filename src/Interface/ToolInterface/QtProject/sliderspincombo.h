#ifndef SLIDERSPINCOMBO_H
#define SLIDERSPINCOMBO_H

#include <QWidget>
#include <QtGui/QSlider>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>

class SliderSpinCombo : public QWidget
{
    Q_OBJECT

    signals:
        void valueAdjusted(double);

    public:
        SliderSpinCombo(QWidget *parent = 0);
        SliderSpinCombo(QWidget *parent, double minRange, double maxRange, double startValue, double stepSize);
        SliderSpinCombo(QWidget *parent, double minRange, double maxRange, double stepSize);

        ~SliderSpinCombo() {}

    public slots:
        void setStep(double);
        void setRanges(double, double);
        void setCurrentValue(double);

    private:
        QVBoxLayout *vLayout;
        QHBoxLayout *hTopLayout;
        QHBoxLayout *hBottomLayout;

        QSpacerItem *spacer;

        QLabel *minValueLabel;
        QLabel *maxValueLabel;
        QString valueString;

        QSlider *slider;
        QDoubleSpinBox *spinner;

        void buildWidget();
        void makeConnections();

    private slots:
        void setSliderValue(int);
        void setSpinnerValue(double);
};

#endif // SLIDERSPINCOMBO_H
