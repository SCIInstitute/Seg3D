#ifndef OTSUTHRESHOLDFILTERINTERFACE_H
#define OTSUTHRESHOLDFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class OtsuThresholdFilterInterface;
}

class OtsuThresholdFilterInterface : public QWidget {
    Q_OBJECT
public:
    OtsuThresholdFilterInterface(QWidget *parent = 0);
    ~OtsuThresholdFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::OtsuThresholdFilterInterface *ui;
};

#endif // OTSUTHRESHOLDFILTERINTERFACE_H
