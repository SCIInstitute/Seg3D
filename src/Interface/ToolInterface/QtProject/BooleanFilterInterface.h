#ifndef BOOLEANFILTERINTERFACE_H
#define BOOLEANFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class BooleanFilterInterface;
}

class BooleanFilterInterface : public QWidget {
    Q_OBJECT
public:
    BooleanFilterInterface(QWidget *parent = 0);
    ~BooleanFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::BooleanFilterInterface *ui;
};

#endif // BOOLEANFILTERINTERFACE_H
