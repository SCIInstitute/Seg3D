#ifndef BINARYDIALATEERODEFILTERINTERFACE_H
#define BINARYDIALATEERODEFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class BinaryDialateErodeFilterInterface;
}

class BinaryDialateErodeFilterInterface : public QWidget {
    Q_OBJECT
public:
    BinaryDialateErodeFilterInterface(QWidget *parent = 0);
    ~BinaryDialateErodeFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::BinaryDialateErodeFilterInterface *ui;
};

#endif // BINARYDIALATEERODEFILTERINTERFACE_H
