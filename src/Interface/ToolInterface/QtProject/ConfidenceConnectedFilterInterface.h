#ifndef CONFIDENCECONNECTEDFILTERINTERFACE_H
#define CONFIDENCECONNECTEDFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class ConfidenceConnectedFilterInterface;
}

class ConfidenceConnectedFilterInterface : public QWidget {
    Q_OBJECT
public:
    ConfidenceConnectedFilterInterface(QWidget *parent = 0);
    ~ConfidenceConnectedFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ConfidenceConnectedFilterInterface *ui;
};

#endif // CONFIDENCECONNECTEDFILTERINTERFACE_H
