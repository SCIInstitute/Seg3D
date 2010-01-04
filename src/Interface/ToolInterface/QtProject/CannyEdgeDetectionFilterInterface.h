#ifndef CANNYEDGEDETECTIONFILTERINTERFACE_H
#define CANNYEDGEDETECTIONFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class CannyEdgeDetectionFilterInterface;
}

class CannyEdgeDetectionFilterInterface : public QWidget {
    Q_OBJECT
public:
    CannyEdgeDetectionFilterInterface(QWidget *parent = 0);
    ~CannyEdgeDetectionFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::CannyEdgeDetectionFilterInterface *ui;
};

#endif // CANNYEDGEDETECTIONFILTERINTERFACE_H
