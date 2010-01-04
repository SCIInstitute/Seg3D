#ifndef INTENSITYCORRECTIONFILTERINTERFACE_H
#define INTENSITYCORRECTIONFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class IntensityCorrectionFilterInterface;
}

class IntensityCorrectionFilterInterface : public QWidget {
    Q_OBJECT
public:
    IntensityCorrectionFilterInterface(QWidget *parent = 0);
    ~IntensityCorrectionFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::IntensityCorrectionFilterInterface *ui;
};

#endif // INTENSITYCORRECTIONFILTERINTERFACE_H
