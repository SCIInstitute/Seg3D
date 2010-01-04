#ifndef THRESHOLDSEGMENTATIONLSFILTERINTERFACE_H
#define THRESHOLDSEGMENTATIONLSFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class ThresholdSegmentationLSFilterInterface;
}

class ThresholdSegmentationLSFilterInterface : public QWidget {
    Q_OBJECT
public:
    ThresholdSegmentationLSFilterInterface(QWidget *parent = 0);
    ~ThresholdSegmentationLSFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ThresholdSegmentationLSFilterInterface *ui;
};

#endif // THRESHOLDSEGMENTATIONLSFILTERINTERFACE_H
