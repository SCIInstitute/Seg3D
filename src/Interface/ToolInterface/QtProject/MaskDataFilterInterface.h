#ifndef MASKDATAFILTERINTERFACE_H
#define MASKDATAFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class MaskDataFilterInterface;
}

class MaskDataFilterInterface : public QWidget {
    Q_OBJECT
public:
    MaskDataFilterInterface(QWidget *parent = 0);
    ~MaskDataFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MaskDataFilterInterface *ui;
};

#endif // MASKDATAFILTERINTERFACE_H
