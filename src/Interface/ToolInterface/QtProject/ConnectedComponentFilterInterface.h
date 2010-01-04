#ifndef CONNECTEDCOMPONENTFILTERINTERFACE_H
#define CONNECTEDCOMPONENTFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class ConnectedComponentFilterInterface;
}

class ConnectedComponentFilterInterface : public QWidget {
    Q_OBJECT
public:
    ConnectedComponentFilterInterface(QWidget *parent = 0);
    ~ConnectedComponentFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ConnectedComponentFilterInterface *ui;
};

#endif // CONNECTEDCOMPONENTFILTERINTERFACE_H
