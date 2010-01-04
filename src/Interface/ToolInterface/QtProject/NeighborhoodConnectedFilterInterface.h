#ifndef NEIGHBORHOODCONNECTEDFILTERINTERFACE_H
#define NEIGHBORHOODCONNECTEDFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class NeighborhoodConnectedFilterInterface;
}

class NeighborhoodConnectedFilterInterface : public QWidget {
    Q_OBJECT
public:
    NeighborhoodConnectedFilterInterface(QWidget *parent = 0);
    ~NeighborhoodConnectedFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::NeighborhoodConnectedFilterInterface *ui;
};

#endif // NEIGHBORHOODCONNECTEDFILTERINTERFACE_H
