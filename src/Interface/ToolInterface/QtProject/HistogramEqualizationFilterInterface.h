#ifndef HISTOGRAMEQUALIZATIONFILTERINTERFACE_H
#define HISTOGRAMEQUALIZATIONFILTERINTERFACE_H

#include <QWidget>

namespace Ui {
    class HistogramEqualizationFilterInterface;
}

class HistogramEqualizationFilterInterface : public QWidget {
    Q_OBJECT
public:
    HistogramEqualizationFilterInterface(QWidget *parent = 0);
    ~HistogramEqualizationFilterInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::HistogramEqualizationFilterInterface *ui;
};

#endif // HISTOGRAMEQUALIZATIONFILTERINTERFACE_H
