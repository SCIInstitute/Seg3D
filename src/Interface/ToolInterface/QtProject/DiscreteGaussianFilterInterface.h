#ifndef DISCRETEGAUSSIANFILTERINTERFACE_H
#define DISCRETEGAUSSIANFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class DiscreteGaussianFilterInterface;
}

class DiscreteGaussianFilterInterface : public QWidget {
    Q_OBJECT
public:
    DiscreteGaussianFilterInterface(QWidget *parent = 0);
    ~DiscreteGaussianFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DiscreteGaussianFilterInterface *ui;
};

#endif // DISCRETEGAUSSIANFILTERINTERFACE_H
