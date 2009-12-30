#ifndef ARITHMETICFILTERINTERFACE_H
#define ARITHMETICFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class ArithmeticFilterInterface;
}

class ArithmeticFilterInterface : public QWidget {
    Q_OBJECT
public:
    ArithmeticFilterInterface(QWidget *parent = 0);
    ~ArithmeticFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ArithmeticFilterInterface *ui;
};

#endif // ARITHMETICFILTERINTERFACE_H
