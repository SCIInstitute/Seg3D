#ifndef FILLHOLESFILTERINTERFACE_H
#define FILLHOLESFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class FillHolesFilterInterface;
}

class FillHolesFilterInterface : public QWidget {
    Q_OBJECT
public:
    FillHolesFilterInterface(QWidget *parent = 0);
    ~FillHolesFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::FillHolesFilterInterface *ui;
};

#endif // FILLHOLESFILTERINTERFACE_H
