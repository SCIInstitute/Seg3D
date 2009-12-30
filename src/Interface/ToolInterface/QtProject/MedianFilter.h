#ifndef MEDIANFILTER_H
#define MEDIANFILTER_H

#include <QWidget>

namespace Ui {
    class MedianFilter;
}

class MedianFilter : public QWidget {
    Q_OBJECT
public:
    MedianFilter(QWidget *parent = 0);
    ~MedianFilter();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MedianFilter *ui;
};

#endif // MEDIANFILTER_H
